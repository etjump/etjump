/*
 * MIT License
 *
 * Copyright (c) 2024 ETJump team <zero@etjump.com>
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

#include "etj_timerun_shared.h"

#include "etj_container_utilities.h"

namespace ETJump {
int TimerunCommands::parseClientNum(const std::string &arg) {
  try {
    const auto clientNum = std::stoi(arg);

    if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
      return INVALID_CLIENT_NUM;
    }

    return clientNum;
  } catch (const std::logic_error &) {
    return INVALID_CLIENT_NUM;
  }
}

opt<int> TimerunCommands::parseTime(const std::string &arg) {
  try {
    const auto time = std::stoi(arg);

    if (time < 0) {
      return {};
    }

    return time;
  } catch (const std::logic_error &) {
    return {};
  }
}

opt<int> TimerunCommands::parseInteger(const std::string &arg) {
  try {
    return std::stoi(arg);
  } catch (const std::logic_error &) {
    return {};
  }
}

TimerunCommands::Start::Start(
    int clientNum, int startTime, const std::string &runName,
    const opt<int> &previousRecord, bool runHasCheckpoints,
    std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints,
    std::array<int, MAX_TIMERUN_CHECKPOINTS> currentRunCheckpoints)
    : clientNum(clientNum), startTime(startTime), runName(runName),
      previousRecord(previousRecord), runHasCheckpoints(runHasCheckpoints),
      checkpoints(checkpoints), currentRunCheckpoints(currentRunCheckpoints) {}

std::string TimerunCommands::Start::serialize() {
  return stringFormat("timerun start %d %d \"%s\" %d %d \"%s\" \"%s\"",
                      clientNum, startTime, runName,
                      previousRecord.hasValue() ? previousRecord.value() : -1,
                      runHasCheckpoints, StringUtil::join(checkpoints, ","),
                      StringUtil::join(currentRunCheckpoints, ","));
}

opt<TimerunCommands::Start>
TimerunCommands::Start::deserialize(const std::vector<std::string> &args) {
  const int numExpectedFields = 9;

  if (args.size() < numExpectedFields) {
    return {};
  }

  if (args[0] != "timerun") {
    return {};
  }

  if (args[1] != "start" && args[1] != "saveposstart") {
    return {};
  }

  Start start;

  start.clientNum = parseClientNum(args[2]);
  if (start.clientNum == INVALID_CLIENT_NUM) {
    return {};
  }

  opt<int> startTime;

  // savepos parses startTime as an int instead of time,
  // because it needs to support negative startTime
  if (args[1] == "saveposstart") {
    startTime = parseInteger(args[3]);
  } else {
    startTime = parseTime((args[3]));
  }

  if (!startTime.hasValue()) {
    return {};
  }

  start.startTime = startTime.value();

  start.runName = args[4];

  start.previousRecord = parseTime(args[5]);

  start.runHasCheckpoints = std::stoi(args[6]);

  unsigned idx = 0;
  for (const auto &v : Container::map(StringUtil::split(args[7], ","),
                                      [](auto c) { return std::stoi(c); })) {
    if (idx >= start.checkpoints.size()) {
      break;
    }

    start.checkpoints[idx] = v;

    idx++;
  }

  idx = 0;
  for (const auto &v : Container::map(StringUtil::split(args[8], ","),
                                      [](auto c) { return std::stoi(c); })) {
    if (idx >= start.currentRunCheckpoints.size()) {
      break;
    }

    start.currentRunCheckpoints[idx] = v;

    idx++;
  }

  return start;
}

TimerunCommands::SavePosStart::SavePosStart(
    int clientNum, int startTime, std::string runName,
    const opt<int> &previousRecord, bool runHasCheckpoints,
    std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints,
    std::array<int, MAX_TIMERUN_CHECKPOINTS> currentRunCheckpoints)
    : clientNum(clientNum), startTime(startTime), runName(std::move(runName)),
      previousRecord(previousRecord), runHasCheckpoints(runHasCheckpoints),
      checkpoints(checkpoints), currentRunCheckpoints(currentRunCheckpoints) {}

std::string TimerunCommands::SavePosStart::serialize() {
  return stringFormat("timerun saveposstart %d %d \"%s\" %d %d \"%s\" \"%s\"",
                      clientNum, startTime, runName,
                      previousRecord.hasValue() ? previousRecord.value() : -1,
                      runHasCheckpoints, StringUtil::join(checkpoints, ","),
                      StringUtil::join(currentRunCheckpoints, ","));
}

std::string TimerunCommands::Checkpoint::serialize() const {
  return stringFormat("timerun checkpoint %d %d %d \"%s\" %d", clientNum,
                      checkpointNum, checkpointTime, runName, checkpointIndex);
}

opt<TimerunCommands::Checkpoint>
TimerunCommands::Checkpoint::deserialize(const std::vector<std::string> &args) {
  auto empty = opt<Checkpoint>();
  constexpr int expectedFields = 6;

  if (args.size() < expectedFields) {
    return empty;
  }

  if (args[0] != "timerun") {
    return empty;
  }

  if (args[1] != "checkpoint") {
    return empty;
  }

  Checkpoint cp;

  cp.clientNum = parseClientNum(args[2]);
  if (cp.clientNum == INVALID_CLIENT_NUM) {
    return empty;
  }

  auto cpn = parseInteger(args[3]);
  if (!cpn.hasValue()) {
    return empty;
  }
  cp.checkpointNum = cpn.value();

  auto time = parseTime(args[4]);
  if (!time.hasValue()) {
    return empty;
  }
  cp.checkpointTime = time.value();
  cp.runName = args[5];

  if (args.size() == 6) {
    return cp;
  }

  // ETJump 3.3.0 onwards, the checkpoint index we've hit is part of the command
  auto cpi = parseInteger(args[6]);

  if (!cpi.hasValue()) {
    return empty;
  }

  cp.checkpointIndex = cpi.value();

  return cp;
}

std::string TimerunCommands::Interrupt::serialize() {
  return stringFormat("timerun interrupt %d", clientNum);
}

opt<TimerunCommands::Interrupt>
TimerunCommands::Interrupt::deserialize(const std::vector<std::string> &args) {
  auto empty = opt<Interrupt>();

  if (args[0] != "timerun" || args[1] != "interrupt") {
    return empty;
  }

  Interrupt interrupt{};
  interrupt.clientNum = parseClientNum(args[2]);
  if (interrupt.clientNum == INVALID_CLIENT_NUM) {
    return empty;
  }

  return interrupt;
}

std::string TimerunCommands::Completion::serialize() {
  return stringFormat("timerun completion %d %d %d \"%s\"", clientNum,
                      completionTime,
                      previousRecordTime.valueOr(NO_PREVIOUS_RECORD), runName);
}

opt<TimerunCommands::Completion>
TimerunCommands::Completion::deserialize(const std::vector<std::string> &args) {
  auto empty = opt<Completion>();

  if (args[0] != "timerun" || args[1] != "completion") {
    return empty;
  }

  Completion completion{};
  completion.clientNum = parseClientNum(args[2]);
  if (completion.clientNum == INVALID_CLIENT_NUM) {
    return empty;
  }

  auto completionTime = parseTime(args[3]);
  if (!completionTime.hasValue()) {
    return empty;
  }

  completion.completionTime = completionTime.value();

  completion.previousRecordTime = parseTime(args[4]);

  completion.runName = args[5];

  return completion;
}

std::string TimerunCommands::Record::serialize() {
  return stringFormat("timerun record %d %d %d \"%s\"", clientNum,
                      completionTime,
                      previousRecordTime.valueOr(NO_PREVIOUS_RECORD), runName);
}

opt<TimerunCommands::Record>
TimerunCommands::Record::deserialize(const std::vector<std::string> &args) {
  auto empty = opt<Record>();

  if (args[0] != "timerun" || args[1] != "record") {
    return empty;
  }

  Record record{};
  record.clientNum = parseClientNum(args[2]);
  if (record.clientNum == INVALID_CLIENT_NUM) {
    return empty;
  }

  auto completionTime = parseTime(args[3]);
  if (!completionTime.hasValue()) {
    return empty;
  }

  record.completionTime = completionTime.value();

  record.previousRecordTime = parseTime(args[4]);

  record.runName = args[5];

  return record;
}

std::string TimerunCommands::Stop::serialize() {
  return stringFormat("timerun stop %d %d \"%s\"", clientNum, completionTime,
                      runName);
}

opt<TimerunCommands::Stop>
TimerunCommands::Stop::deserialize(const std::vector<std::string> &args) {
  auto empty = opt<Stop>();

  if (args[0] != "timerun" || args[1] != "stop") {
    return empty;
  }

  Stop stop{};
  stop.clientNum = parseClientNum(args[2]);
  if (stop.clientNum == INVALID_CLIENT_NUM) {
    return empty;
  }

  auto completionTime = parseTime(args[3]);
  if (!completionTime.hasValue()) {
    return empty;
  }

  stop.completionTime = completionTime.value();

  stop.runName = args[4];

  return stop;
}

} // namespace ETJump
