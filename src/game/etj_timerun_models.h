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
#include "etj_time_utilities.h"

namespace ETJump {
namespace Timerun {
struct Season {
  int id;
  std::string name;
  Time startTime;
  opt<Time> endTime;
};

struct Record {
  int seasonId;
  std::string map;
  std::string run;
  int userId;
  int time;
  std::vector<int> checkpoints;
  Time recordDate;
  std::string playerName;
  std::map<std::string, std::string> metadata;

  bool isSameRunAs(const Record *otherRecord) const {
    return this->seasonId == otherRecord->seasonId &&
           this->map == otherRecord->map && this->run == otherRecord->run;
  }
};

struct AddSeasonParams {
  int clientNum;
  std::string name;
  Time startTime;
  opt<Time> endTime;
};

struct EditSeasonParams {
  int clientNum;
  std::string name;
  opt<Time> startTime;
  opt<Time> endTime;
};

struct PrintRecordsParams {
  int clientNum{};
  opt<std::string> season;
  std::string map;
  bool exactMap{};
  opt<std::string> run;
  int page{};
  int pageSize{};
  int userId{};
};
}
}
