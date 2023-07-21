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
#include <string>

#include "etj_synchronization_context.h"
#include "etj_timerun_models.h"
#include "etj_time_utilities.h"

namespace ETJump {
class DatabaseV2;

class TimerunRepository {
public:
  explicit TimerunRepository(std::unique_ptr<DatabaseV2> database)
    : _database(std::move(database)) {
  }

  void initialize();
  void shutdown();

  std::vector<Timerun::Season> getActiveSeasons(const Time &currentTime) const;
  std::vector<Timerun::Record> getRecordsForPlayer(
      const std::vector<int> activeSeasons, const std::string &map, int userId);
  Timerun::Season addSeason(Timerun::AddSeasonParams params);
  std::vector<Timerun::Record> getRecordsForPlayer(
      const std::vector<int> &activeSeasons,
      const std::string &map, const std::string& run, int userId);
  std::vector<Timerun::Record> getRecordsForRun(const std::string &map,
                                                const std::string &run) const;
  void insertRecord(const Timerun::Record & record);
  void updateRecord(const Timerun::Record & record);

private:
  void migrate();
  std::string serializeMetadata(std::map<std::string, std::string> metadata);
  std::unique_ptr<DatabaseV2> _database;
};
}
