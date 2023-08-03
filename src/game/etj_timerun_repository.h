/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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
#include <sqlite_modern_cpp.h>
#include <string>

#include "etj_synchronization_context.h"
#include "etj_timerun_models.h"
#include "etj_time_utilities.h"

namespace ETJump {
class DatabaseV2;

class TimerunRepository {
public:
  explicit TimerunRepository(std::unique_ptr<DatabaseV2> database,
                             std::unique_ptr<DatabaseV2> oldDatabase)
    : _database(std::move(database)), _oldDatabase(std::move(oldDatabase)) {
  }

  void initialize();
  void shutdown();

  std::vector<Timerun::Season> getActiveSeasons(const Time &currentTime) const;
  std::vector<Timerun::Record> getRecordsForPlayer(
      const std::vector<int> activeSeasons, const std::string &map, int userId);
  Timerun::Season addSeason(Timerun::AddSeasonParams params);
  std::vector<Timerun::Record> getRecordsForPlayer(
      const std::vector<int> &activeSeasons,
      const std::string &map, const std::string &run, int userId);
  std::vector<Timerun::Record> getRecordsForRun(const std::string &map,
                                                const std::string &run) const;
  void insertRecord(const Timerun::Record &record);
  void updateRecord(const Timerun::Record &record);
  opt<Timerun::Record>
  getTopRecord(int seasonId, const std::string &map,
               const std::string &run);
  std::vector<Timerun::Record> getTopRecords(const std::vector<int>& seasonIds,
                                             const std::string &map,
                                             const std::string &run) const;
  void editSeason(const Timerun::EditSeasonParams &params);
  std::vector<std::string> getMapsForName(const std::string &map, bool exact);
  std::vector<Timerun::Record> getRecords(
      const Timerun::PrintRecordsParams &params);
  std::vector<Timerun::Season> getSeasonsForName(
      const std::string &name, bool exact);
  opt<Timerun::Record> getRecord(const std::string & map, const std::string & run, int rank);

private:
  void tryToMigrateRecords();
  void migrate();

  const std::vector<std::string> _defaultSeasonFields{"id", "name",
    "start_time", "end_time"};
  const std::string _defaultSeasonFieldsStr =
      StringUtil::join(_defaultSeasonFields, ",");
  const std::string _defaultSeasonQueryBase = stringFormat(R"(
    select
      %s
    from season    
  )", _defaultSeasonFieldsStr);
  const std::vector<std::string> _defaultRecordFields{
      "season_id", "map", "run", "user_id", "time",
      "checkpoints", "record_date", "player_name", "metadata"};
  const std::string _defaultRecordFieldsStr =
      StringUtil::join(_defaultRecordFields, ",");
  const std::string _defaultRecordQueryBase =
      stringFormat(R"(
        select
          %s
        from record
      )", _defaultRecordFieldsStr);

  static std::vector<Timerun::Record> getRecordsFromQuery(
      sqlite::database_binder &binder);

  std::string serializeMetadata(std::map<std::string, std::string> metadata);
  std::unique_ptr<DatabaseV2> _database;
  std::unique_ptr<DatabaseV2> _oldDatabase;
};
}
