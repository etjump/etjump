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
#include <optional>
#include <string>

#include "etj_time_utilities.h"

namespace ETJump::Timerun {
inline constexpr int32_t DEFAULT_PAGE_SIZE_ALL_RUNS = 3;
inline constexpr int32_t DEFAULT_PAGE_SIZE_SINGLE_RUN = 20;
inline constexpr int32_t MAX_PAGE_SIZE_SINGLE_RUN = 100;
inline constexpr int32_t MAX_PAGE_SIZE_ALL_RUNS = 10;
inline constexpr int32_t RECORD_HISTORY_DEFAULT_SIZE = 20;

struct Season {
  int id;
  std::string name;
  TimeUtils::Time startTime;
  std::optional<TimeUtils::Time> endTime;
};

struct Record {
  int seasonId;
  std::string map;
  std::string run;
  int userId;
  int time;
  std::vector<int> checkpoints;
  TimeUtils::Time recordDate;
  std::string playerName;
  std::map<std::string, std::string> metadata;

  bool isSameRunAs(const Record *otherRecord) const {
    return this->seasonId == otherRecord->seasonId &&
           this->map == otherRecord->map && this->run == otherRecord->run;
  }
};

struct Checkpoints {
  int32_t seasonID;
  int32_t runTime;
  std::string playerName;
  std::string map;
  std::string run;
  std::vector<int32_t> checkpoints;
};

// a segment whose best time a completion improved; personal and global say
// which tables the improvement landed in, and the previous times say what it
// beat. A previous time is absent when that table had no entry for the
// segment, meaning this completion set it rather than beat it.
struct BestSegmentUpdate {
  int32_t segment{};                       // 1-based segment number
  int32_t time{};                          // the new best duration
  bool personal{};                         // improved the player's own best
  bool global{};                           // improved the server-wide best
  std::optional<int32_t> previousPersonal; // the personal time it beat
  std::optional<int32_t> previousGlobal;   // the global time it beat
};

struct AddSeasonParams {
  int clientNum;
  std::string name;
  TimeUtils::Time startTime;
  std::optional<TimeUtils::Time> endTime;
};

struct EditSeasonParams {
  int clientNum;
  std::string name;
  std::optional<TimeUtils::Time> startTime;
  std::optional<TimeUtils::Time> endTime;
};

struct PrintRecordsParams {
  int clientNum{};
  std::optional<std::string> season;
  std::string map;
  bool exactMap{};
  std::optional<std::string> run;
  int page{};
  int pageSize{};
  int userId{};
};

struct PrintRankingsParams {
  int clientNum{};
  int userId{};
  std::optional<std::string> season;
  int page{};
  int pageSize{};
};

struct ListCheckpointsParams {
  int32_t clientNum{};
  std::optional<std::string> season;
  std::string map;
  std::string run;
  int32_t rank{};
  bool exactMap;
};

struct CompareCheckpointsParams {
  int32_t clientNum{};
  std::optional<std::string> season;
  std::string map;
  std::string run;
  int32_t rankBase{};
  int32_t rankCmp{};
  bool exactMap;
};

struct RecordDetailsParams {
  int32_t clientNum{};
  std::string season;
  std::string map;
  std::string run;
  int32_t rank{};
  bool exactMap;
};

struct RemoveRecordParams {
  int32_t clientNum{};
  std::string season;                // exact match if given
  std::string map;                   // exact match only
  std::string run;                   // exact match only
  int32_t userId{};                  // id of the record owner
  int32_t removedBy{};               // id of whoever issued the removal
  std::optional<std::string> reason; // required when removing others' records
};

struct RemovedRecord {
  int32_t id{}; // id of the archived record in 'removed_records'
  Record record;
  int32_t removedBy{};
  TimeUtils::Time removedAt;
  std::optional<std::string> reason;
};

struct RemovedRecordEntry {
  // the season copies of a single completion
  // contains a single record when a season filter is applied
  std::vector<RemovedRecord> records;
};

struct ListRemovedRecordsParams {
  int32_t clientNum{};
  int32_t callerId{};               // user ID of the client using the command
  std::string season;               // empty = no filter
  std::string map;                  // empty = no filter
  std::string run;                  // empty = no filter
  std::optional<int32_t> userId;    // only list records belonging to this user
  std::optional<int32_t> removedBy; // only list records removed by this user
  int32_t page{};
  int32_t pageSize{};
};

struct RemovedRecordsPage {
  std::vector<RemovedRecordEntry> entries;
  int32_t page{}; // effective page, capped to the last page if out of range
  int32_t numPages{};
};

struct RestoreRecordParams {
  int32_t clientNum{};
  int32_t callerId{};
  int32_t recordId{};
  bool force{};   // force restore in case of conflicts (swaps records)
  bool isAdmin{}; // whether we have the 'TIMERUN_MANAGEMENT' admin flag
};

struct RestoreConflict {
  Record existing;  // the record currently occupying the slot
  Record toRestore; // the removed record that would be restored
};

struct RestoreRecordResult {
  int32_t numTargeted{};                  // total targeted records
  std::vector<Record> restored;           // records restored into 'record'
  std::vector<Record> swapped;            // occupants archived by the swap
  std::vector<RestoreConflict> conflicts; // unresolved conflicts (no --force)
  std::vector<RemovedRecord> skipped;     // copies skipped due to permissions
};

struct RecordHistoryParams {
  int32_t clientNum{};
  int32_t userId{};
  std::string season;
  std::string map;
  std::string run;
  int32_t maxRecords{};
  bool exactMap{};
};

struct BestSegmentsParams {
  int32_t clientNum{};
  std::string season;
  std::string map;
  std::string run;
  int32_t userId{};
  bool global{}; // server-wide bests when true, personal bests otherwise
  bool exactMap{};
};

struct BestSegmentHistoryParams {
  int32_t clientNum{};
  std::string season;
  std::string map;
  std::string run;
  int32_t userId{};
  int32_t segment{};
  int32_t maxRecords{};
  bool exactMap{};
};

// one row of a best-segment listing; 'userId' is the holder for the global
// listing, and the owning player for a personal listing. A partial season
// match can return rows from more than one season, so each row carries its
// 'seasonId'
struct BestSegmentDetails {
  int32_t seasonId{};
  std::string map;
  std::string run;
  int32_t segment{};
  int32_t time{};
  int32_t userId{};
  std::string playerName;
  TimeUtils::Time recordDate;
  int32_t rank{};
};

// one improvement row from 'player_best_segments_history'. 'removed' is
// computed at query time: either the source run or the split itself has been
// removed, and history rows are never deleted
struct BestSegmentHistoryEntry {
  int32_t seasonId{};
  std::string map;
  std::string run;
  int32_t segment{};
  int32_t time{};
  int32_t rank{};
  std::string playerName;
  TimeUtils::Time recordDate;
  bool removed{};
};

struct HistoricalRecord {
  Record r{};
  int32_t rank{};
  bool removed{};
};

struct HistoricalRunGroup {
  std::string map;
  std::string run;
  std::vector<HistoricalRecord> records;
  int32_t numHidden{};
};

struct HistoricalSeasonGroup {
  int32_t seasonId{};
  std::vector<HistoricalRunGroup> runs;
};
} // namespace ETJump::Timerun
