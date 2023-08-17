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
#include <array>
#include <string>

#include "etj_shared.h"
#include "etj_string_utilities.h"
#include "etj_utilities.h"
#include "q_shared.h"

namespace ETJump {
namespace TimerunCommands {
constexpr int INVALID_CLIENT_NUM = -1;
const int NO_PREVIOUS_RECORD = -1;

int parseClientNum(const std::string &arg);
opt<int> parseTime(const std::string &arg);
opt<int> parseInteger(const std::string &arg);

class Start {
public:
  Start();
  Start(int clientNum, int startTime, const std::string &runName,
        const opt<int> &previousRecord,
        std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints,
        std::array<int, MAX_TIMERUN_CHECKPOINTS> currentRunCheckpoints);

  int clientNum{};
  int startTime{};
  std::string runName{};
  opt<int> previousRecord{};
  std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints{};
  std::array<int, MAX_TIMERUN_CHECKPOINTS> currentRunCheckpoints{};

  std::string serialize();

  static opt<Start> deserialize(const std::vector<std::string> &args);
};

class Checkpoint {
public:
  Checkpoint() = default;

  Checkpoint(int clientNum, int checkpointIndex, int checkpointTime,
             const std::string &runName)
      : clientNum(clientNum), checkpointIndex(checkpointIndex),
        checkpointTime(checkpointTime), runName(runName) {}

  int clientNum{};
  int checkpointIndex{};
  int checkpointTime{};
  std::string runName{};

  std::string serialize();

  static opt<Checkpoint> deserialize(const std::vector<std::string> &args);
};

class Interrupt {
public:
  Interrupt() = default;

  explicit Interrupt(int clientNum) : clientNum(clientNum) {}

  int clientNum{};

  std::string serialize();

  static opt<Interrupt> deserialize(const std::vector<std::string> &args);
};

class Completion {
public:
  Completion() = default;

  Completion(int clientNum, int completionTime, opt<int> previousRecordTime,
             const std::string &runName)
      : clientNum(clientNum), completionTime(completionTime),
        previousRecordTime(previousRecordTime), runName(runName) {}

  int clientNum{};
  int completionTime{};
  opt<int> previousRecordTime{};
  std::string runName{};

  std::string serialize();

  static opt<Completion> deserialize(const std::vector<std::string> &args);
};

class Record {
public:
  Record() = default;

  Record(int clientNum, int completionTime, opt<int> previousRecordTime,
         const std::string &runName)
      : clientNum(clientNum), completionTime(completionTime),
        previousRecordTime(previousRecordTime), runName(runName) {}

  int clientNum{};
  int completionTime{};
  opt<int> previousRecordTime{};
  std::string runName{};

  std::string serialize();

  static opt<Record> deserialize(const std::vector<std::string> &args);
};

class Stop {
public:
  Stop() = default;

  Stop(int clientNum, int time, const std::string &runName)
      : clientNum(clientNum), completionTime(time), runName(runName) {}

  int clientNum{};
  int completionTime{};
  std::string runName{};

  std::string serialize();

  static opt<Stop> deserialize(const std::vector<std::string> &args);
};
} // namespace TimerunCommands
} // namespace ETJump
