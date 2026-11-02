/*
 * MIT License
 *
 * Copyright (c) 2026 ETJump team <zero@etjump.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <map>
#include <set>
#include <sqlite_modern_cpp.h>
#include <string>
#include <tuple>

#include "etj_synchronization_context.h"
#include "etj_timerun_models.h"
#include "etj_time_utilities.h"
#include "etj_database_v2.h"

namespace ETJump {
class TimerunRepository {
public:
  explicit TimerunRepository(std::unique_ptr<DatabaseV2> database,
                             std::unique_ptr<DatabaseV2> oldDatabase)
      : _database(std::move(database)), _oldDatabase(std::move(oldDatabase)) {}

  void initialize();
  void shutdown();

  std::vector<Timerun::Record>
  getRecordsForPlayer(const std::vector<int> activeSeasons,
                      const std::string &map, int userId);
  Timerun::Season addSeason(Timerun::AddSeasonParams params);
  std::vector<Timerun::Record>
  getRecordsForPlayer(const std::vector<int> &activeSeasons,
                      const std::string &map, const std::string &run,
                      int userId);
  std::vector<Timerun::Record> getRecordsForRun(const std::string &map,
                                                const std::string &run) const;
  std::map<std::string, std::vector<int32_t>>
  getBestSegments(int32_t seasonId, const std::string &map) const;
  std::map<std::string, std::vector<int32_t>>
  getPlayerBestSegments(int32_t seasonId, const std::string &map,
                        int userId) const;
  // writes every season copy of a completion (record + history + best
  // segments) in a single transaction, returning the segment improvements
  // grouped by season
  std::map<int32_t, std::vector<Timerun::BestSegmentUpdate>>
  recordCompletion(const std::vector<Timerun::RecordCompletion> &completions);
  std::optional<Timerun::Record>
  getTopRecord(int seasonId, const std::string &map, const std::string &run);
  std::vector<Timerun::Record> getTopRecords(const std::vector<int> &seasonIds,
                                             const std::string &map,
                                             const std::string &run) const;
  void editSeason(const Timerun::EditSeasonParams &params);
  std::vector<std::string> getMapsForName(const std::string &map, bool exact,
                                          bool fromHistory = false);
  std::vector<std::string> getRunsForName(const std::string &map,
                                          const std::string &run, bool exact,
                                          bool sanitizeResults,
                                          bool fromHistory = false);
  std::vector<Timerun::Record> getRecords();
  std::vector<Timerun::Record>
  getRecords(const Timerun::PrintRecordsParams &params);
  std::vector<Timerun::Season> getSeasonsForName(const std::string &name,
                                                 bool exact);
  std::optional<Timerun::Record> getRecord(const std::string &map,
                                           const std::string &run, int rank);
  std::vector<Timerun::Record> getRecordFromSeason(int32_t seasonId,
                                                   const std::string &map,
                                                   const std::string &run,
                                                   int32_t rank, bool exactMap);
  std::vector<Timerun::HistoricalRecord>
  getHistoricalRecords(const Timerun::RecordHistoryParams &params);
  std::vector<Timerun::BestSegmentDetails>
  getBestSegmentDetails(const Timerun::BestSegmentsParams &params);
  std::vector<Timerun::BestSegmentHistoryEntry>
  getBestSegmentHistory(const Timerun::BestSegmentHistoryParams &params);
  std::vector<Timerun::Season> getSeasons();
  void deleteSeason(const std::string &name);
  std::vector<Timerun::Checkpoints>
  getCheckpoints(const Timerun::ListCheckpointsParams &params);
  std::vector<Timerun::Record>
  removeRecord(const Timerun::RemoveRecordParams &params,
               const TimeUtils::Time &removedAt);
  Timerun::RemovedRecordsPage
  getRemovedRecords(const Timerun::ListRemovedRecordsParams &params);
  std::vector<Timerun::RemovedSegment>
  removeSegment(const Timerun::RemoveSegmentParams &params,
                const TimeUtils::Time &removedAt);
  Timerun::RemovedSegmentsPage
  getRemovedSegments(const Timerun::ListRemovedSegmentsParams &params);
  Timerun::RestoreSegmentResult
  restoreSegment(const Timerun::RestoreSegmentParams &params,
                 const TimeUtils::Time &restoredAt);
  Timerun::RestoreRecordResult
  restoreRecord(const Timerun::RestoreRecordParams &params,
                const TimeUtils::Time &restoredAt);

  static std::map<std::string, std::string>
  deserializeMetadata(const std::string &metadata);

private:
  void tryToMigrateRecords();
  void seedRecordHistory();
  void seedBestSegments();
  void migrate();

  void insertRecordRow(const Timerun::Record &record);
  void updateRecordRow(const Timerun::Record &record);
  void insertRecordHistory(const Timerun::Record &record);

  // writes a run's segments into the live tables, improve-only. No transaction
  // of its own - the caller must provide one. 'appendHistory' controls whether
  // a personal improvement is appended to 'player_best_segments_history', and
  // 'skipRemovedSegments' excludes splits listed in 'removed_segments'
  std::vector<Timerun::BestSegmentUpdate>
  applyBestSegments(const Timerun::Record &record, bool appendHistory,
                    bool skipRemovedSegments);

  // re-derives the global row for each affected segment from the surviving
  // personal rows, deleting the global row when none remain. Must be called
  // inside the caller's transaction. 'map' and 'run' are the stored values
  // from the record, not resolved user input.
  void recomputeGlobalSegments(int32_t seasonId, const std::string &map,
                               const std::string &run,
                               const std::vector<int32_t> &segments);

  // global segments that need re-deriving, grouped by season, map and run
  using AffectedSegments =
      std::map<std::tuple<int32_t, std::string, std::string>,
               std::set<int32_t>>;

  // voids the personal splits held by one exact run, identified by its
  // record date, and records the affected global segments for a later
  // re-derive. Must be called inside the caller's transaction.
  void voidBestSegments(AffectedSegments &affected, int32_t seasonId,
                        const std::string &map, const std::string &run,
                        int32_t userId, const TimeUtils::Time &recordDate);

  // re-derives the global rows for every segment collected in 'affected'.
  // Must be called inside the caller's transaction.
  void recomputeAffectedSegments(const AffectedSegments &affected);

  // fetches the live personal split for an exact season/map/run/user/segment
  std::optional<Timerun::BestSegmentDetails>
  getBestSegmentRow(int32_t seasonId, const std::string &map,
                    const std::string &run, int32_t userId, int32_t segment);

  static std::vector<Timerun::RemovedSegment>
  getRemovedSegmentsFromQuery(sqlite::database_binder &binder);

  const std::vector<std::string> _defaultSeasonFields{"id", "name",
                                                      "start_time", "end_time"};
  const std::string _defaultSeasonFieldsStr =
      StringUtils::join(_defaultSeasonFields, ",");
  const std::string _defaultSeasonQueryBase =
      StringUtils::format(R"(
    select
      %s
    from season
  )",
                          _defaultSeasonFieldsStr);
  const std::vector<std::string> _defaultRecordFields{
      "season_id",   "map",         "run",         "user_id", "time",
      "checkpoints", "record_date", "player_name", "metadata"};
  const std::string _defaultRecordFieldsStr =
      StringUtils::join(_defaultRecordFields, ",");
  const std::string _defaultRecordQueryBase =
      StringUtils::format(R"(
        select
          %s
        from record
      )",
                          _defaultRecordFieldsStr);

  static std::vector<Timerun::Record>
  getRecordsFromQuery(sqlite::database_binder &binder);
  static std::vector<Timerun::RemovedRecord>
  getRemovedRecordsFromQuery(sqlite::database_binder &binder);
  static std::vector<Timerun::Season>
  getSeasonsFromQuery(sqlite::database_binder &binder);

  std::string resolveMapName(const std::string &map, bool exact,
                             const std::string &commandPrefix,
                             bool fromHistory = false);
  std::string resolveRunName(const std::string &map, const std::string &run,
                             bool exact, bool fromHistory = false);
  std::string resolveSegmentRunName(const std::string &map,
                                    const std::string &run,
                                    const std::string &commandPrefix);
  int32_t resolveSeasonForRemoval(const std::string &season,
                                  const std::string &commandPrefix);

  static std::string
  serializeMetadata(const std::map<std::string, std::string> &metadata);
  std::unique_ptr<DatabaseV2> _database;
  std::unique_ptr<DatabaseV2> _oldDatabase;
};
} // namespace ETJump
