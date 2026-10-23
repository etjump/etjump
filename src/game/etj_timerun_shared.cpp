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

#include <stdexcept>

#include "etj_timerun_shared.h"
#include "etj_container_utilities.h"
#include "etj_string_utilities.h"

namespace ETJump {
std::vector<int32_t>
TimerunSegments::computeSegmentTimes(const std::vector<int32_t> &checkpoints,
                                     int32_t completionTime) {
  std::vector<int32_t> segments;
  int32_t previous = 0;

  for (size_t i = 0; i < checkpoints.size(); ++i) {
    const int32_t checkpoint = checkpoints[i];

    // writers only ever store checkpoints densely, with unset entries at the
    // end, so a gap can only come from a corrupt or hand-edited row. Not
    // reachable through the game; kept as defense-in-depth, because the
    // positions after a gap cannot be trusted, so the prefix is kept and the
    // tail dropped rather than recording a segment under the wrong number
    if (checkpoint == TIMERUN_CHECKPOINT_NOT_SET) {
      if (std::any_of((checkpoints.cbegin() + static_cast<int32_t>(i) + 1),
                      checkpoints.cend(), [](const auto cp) {
                        return cp != TIMERUN_CHECKPOINT_NOT_SET;
                      })) {
        return segments;
      }

      break;
    }

    // a checkpoint hit on the same command time as the previous one is a
    // zero-length leg. Keep its slot, so the following segments keep their
    // numbers, but leave it as 0, which no writer records as a best
    const int32_t duration = checkpoint - previous;
    segments.push_back(duration > 0 ? duration : 0);

    if (duration > 0) {
      previous = checkpoint;
    }
  }

  // the finish closes the run, but only for a run that has checkpoints:
  // a completion time at or before the last checkpoint means the finish was
  // itself a checkpoint, so there is nothing to add
  if (!segments.empty() && completionTime > previous) {
    segments.push_back(completionTime - previous);
  }

  return segments;
}

std::pair<bool, bool> TimerunSegments::bestSegmentBeaten(
    const std::map<std::string, std::vector<int32_t>> &personalBestSegments,
    const std::map<std::string, std::vector<int32_t>> &globalBestSegments,
    const std::string &runName, int32_t segmentIdx, int32_t duration) {
  if (duration <= 0) {
    return {false, false};
  }

  const auto beaten =
      [&runName, segmentIdx,
       duration](const std::map<std::string, std::vector<int32_t>> &cache) {
        const auto it = cache.find(runName);

        return it != cache.end() &&
               segmentIdx < static_cast<int32_t>(it->second.size()) &&
               duration < it->second[segmentIdx];
      };

  return {beaten(personalBestSegments), beaten(globalBestSegments)};
}

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

std::optional<int> TimerunCommands::parseTime(const std::string &arg) {
  try {
    const auto time = std::stoi(arg);

    if (time < 0) {
      return std::nullopt;
    }

    return time;
  } catch (const std::logic_error &) {
    return std::nullopt;
  }
}

std::optional<int> TimerunCommands::parseInteger(const std::string &arg) {
  try {
    return std::stoi(arg);
  } catch (const std::logic_error &) {
    return std::nullopt;
  }
}

TimerunCommands::Start::Start(
    int clientNum, int startTime, const std::string &runName,
    const std::optional<int> &previousRecord, bool runHasCheckpoints,
    std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints,
    std::array<int, MAX_TIMERUN_CHECKPOINTS> currentRunCheckpoints,
    std::array<bool, MAX_TIMERUN_CHECKPOINTS> currentRunBestPersonalSegments,
    std::array<bool, MAX_TIMERUN_CHECKPOINTS> currentRunBestGlobalSegments)
    : clientNum(clientNum), startTime(startTime), runName(runName),
      previousRecord(previousRecord), runHasCheckpoints(runHasCheckpoints),
      checkpoints(checkpoints), currentRunCheckpoints(currentRunCheckpoints),
      currentRunBestPersonalSegments(currentRunBestPersonalSegments),
      currentRunBestGlobalSegments(currentRunBestGlobalSegments) {}

std::string TimerunCommands::Start::serialize() const {
  return StringUtils::format(
      "timerun start %d %d \"%s\" %d %d \"%s\" \"%s\" \"%s\" \"%s\"", clientNum,
      startTime, runName, previousRecord.value_or(-1), runHasCheckpoints,
      StringUtils::join(checkpoints, ","),
      StringUtils::join(currentRunCheckpoints, ","),
      StringUtils::join(currentRunBestPersonalSegments, ","),
      StringUtils::join(currentRunBestGlobalSegments, ","));
}

std::optional<TimerunCommands::Start>
TimerunCommands::Start::deserialize(const std::vector<std::string> &args) {
  const int numExpectedFields = 9;

  if (args.size() < numExpectedFields) {
    return std::nullopt;
  }

  if (args[0] != "timerun") {
    return std::nullopt;
  }

  if (args[1] != "start" && args[1] != "saveposstart") {
    return std::nullopt;
  }

  Start start;

  start.clientNum = parseClientNum(args[2]);
  if (start.clientNum == INVALID_CLIENT_NUM) {
    return std::nullopt;
  }

  std::optional<int> startTime;

  // savepos parses startTime as an int instead of time,
  // because it needs to support negative startTime
  if (args[1] == "saveposstart") {
    startTime = parseInteger(args[3]);
  } else {
    startTime = parseTime((args[3]));
  }

  if (!startTime.has_value()) {
    return std::nullopt;
  }

  start.startTime = startTime.value();

  start.runName = args[4];

  start.previousRecord = parseTime(args[5]);

  start.runHasCheckpoints = std::stoi(args[6]);

  unsigned idx = 0;
  for (const auto &v :
       Container::map(StringUtils::split(args[7], ","),
                      [](const auto &c) { return std::stoi(c); })) {
    if (idx >= start.checkpoints.size()) {
      break;
    }

    start.checkpoints[idx] = v;

    idx++;
  }

  idx = 0;
  for (const auto &v :
       Container::map(StringUtils::split(args[8], ","),
                      [](const auto &c) { return std::stoi(c); })) {
    if (idx >= start.currentRunCheckpoints.size()) {
      break;
    }

    start.currentRunCheckpoints[idx] = v;

    idx++;
  }

  // ETJump 3.7.0 onwards - the run's gold verdicts. Absent from old commands
  // in demo playback. Default filled to false, so no gold splits ever show
  // in old demos, since they were never tracked to begin with
  if (args.size() >= numExpectedFields + 2) {
    idx = 0;
    for (const auto &v :
         Container::map(StringUtils::split(args[9], ","),
                        [](const auto &c) { return std::stoi(c); })) {
      if (idx >= start.currentRunBestPersonalSegments.size()) {
        break;
      }

      start.currentRunBestPersonalSegments[idx] = v != 0;

      idx++;
    }

    idx = 0;
    for (const auto &v :
         Container::map(StringUtils::split(args[10], ","),
                        [](const auto &c) { return std::stoi(c); })) {
      if (idx >= start.currentRunBestGlobalSegments.size()) {
        break;
      }

      start.currentRunBestGlobalSegments[idx] = v != 0;

      idx++;
    }
  }

  return start;
}

TimerunCommands::SavePosStart::SavePosStart(
    int clientNum, int startTime, std::string runName,
    const std::optional<int> &previousRecord, bool runHasCheckpoints,
    std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints,
    std::array<int, MAX_TIMERUN_CHECKPOINTS> currentRunCheckpoints)
    : clientNum(clientNum), startTime(startTime), runName(std::move(runName)),
      previousRecord(previousRecord), runHasCheckpoints(runHasCheckpoints),
      checkpoints(checkpoints), currentRunCheckpoints(currentRunCheckpoints) {}

std::string TimerunCommands::SavePosStart::serialize() const {
  return StringUtils::format(
      "timerun saveposstart %d %d \"%s\" %d %d \"%s\" \"%s\"", clientNum,
      startTime, runName, previousRecord.value_or(-1), runHasCheckpoints,
      StringUtils::join(checkpoints, ","),
      StringUtils::join(currentRunCheckpoints, ","));
}

std::string TimerunCommands::Checkpoint::serialize() const {
  return StringUtils::format("timerun checkpoint %d %d %d \"%s\" %d %d %d",
                             clientNum, checkpointNum, checkpointTime, runName,
                             checkpointIndex, bestPersonalSegment,
                             bestGlobalSegment);
}

std::optional<TimerunCommands::Checkpoint>
TimerunCommands::Checkpoint::deserialize(const std::vector<std::string> &args) {
  constexpr int expectedFields = 6;

  if (args.size() < expectedFields) {
    return std::nullopt;
  }

  if (args[0] != "timerun") {
    return std::nullopt;
  }

  if (args[1] != "checkpoint") {
    return std::nullopt;
  }

  Checkpoint cp;

  cp.clientNum = parseClientNum(args[2]);
  if (cp.clientNum == INVALID_CLIENT_NUM) {
    return std::nullopt;
  }

  auto cpn = parseInteger(args[3]);
  if (!cpn.has_value()) {
    return std::nullopt;
  }
  cp.checkpointNum = cpn.value();

  auto time = parseTime(args[4]);
  if (!time.has_value()) {
    return std::nullopt;
  }

  cp.checkpointTime = time.value();
  cp.runName = args[5];

  if (args.size() == 6) {
    return cp;
  }

  // ETJump 3.3.0 onwards, the checkpoint index we've hit is part of the command
  auto cpi = parseInteger(args[6]);

  if (!cpi.has_value()) {
    return std::nullopt;
  }

  cp.checkpointIndex = cpi.value();

  // ETJump 3.7.0 onwards, best segment verdicts - absent from older demos,
  // which simply never render gold
  if (args.size() >= 9) {
    cp.bestPersonalSegment = parseInteger(args[7]).value_or(0) != 0;
    cp.bestGlobalSegment = parseInteger(args[8]).value_or(0) != 0;
  }

  return cp;
}

std::string TimerunCommands::Interrupt::serialize() const {
  return StringUtils::format("timerun interrupt %d", clientNum);
}

std::optional<TimerunCommands::Interrupt>
TimerunCommands::Interrupt::deserialize(const std::vector<std::string> &args) {
  if (args[0] != "timerun" || args[1] != "interrupt") {
    return std::nullopt;
  }

  Interrupt interrupt{};
  interrupt.clientNum = parseClientNum(args[2]);
  if (interrupt.clientNum == INVALID_CLIENT_NUM) {
    return std::nullopt;
  }

  return interrupt;
}

std::string TimerunCommands::Completion::serialize() const {
  return StringUtils::format(
      "timerun completion %d %d %d \"%s\"", clientNum, completionTime,
      previousRecordTime.value_or(NO_PREVIOUS_RECORD), runName);
}

std::optional<TimerunCommands::Completion>
TimerunCommands::Completion::deserialize(const std::vector<std::string> &args) {
  if (args[0] != "timerun" || args[1] != "completion") {
    return std::nullopt;
  }

  Completion completion{};
  completion.clientNum = parseClientNum(args[2]);

  if (completion.clientNum == INVALID_CLIENT_NUM) {
    return std::nullopt;
  }

  auto completionTime = parseTime(args[3]);

  if (!completionTime.has_value()) {
    return std::nullopt;
  }

  completion.completionTime = completionTime.value();

  completion.previousRecordTime = parseTime(args[4]);

  completion.runName = args[5];

  return completion;
}

std::string TimerunCommands::Record::serialize() const {
  return StringUtils::format(
      "timerun record %d %d %d \"%s\"", clientNum, completionTime,
      previousRecordTime.value_or(NO_PREVIOUS_RECORD), runName);
}

std::optional<TimerunCommands::Record>
TimerunCommands::Record::deserialize(const std::vector<std::string> &args) {
  if (args[0] != "timerun" || args[1] != "record") {
    return std::nullopt;
  }

  Record record{};
  record.clientNum = parseClientNum(args[2]);
  if (record.clientNum == INVALID_CLIENT_NUM) {
    return std::nullopt;
  }

  auto completionTime = parseTime(args[3]);
  if (!completionTime.has_value()) {
    return std::nullopt;
  }

  record.completionTime = completionTime.value();

  record.previousRecordTime = parseTime(args[4]);

  record.runName = args[5];

  return record;
}

std::string TimerunCommands::Stop::serialize() const {
  return StringUtils::format("timerun stop %d %d \"%s\" %d %d", clientNum,
                             completionTime, runName, beatPersonal, beatGlobal);
}

std::optional<TimerunCommands::Stop>
TimerunCommands::Stop::deserialize(const std::vector<std::string> &args) {
  if (args[0] != "timerun" || args[1] != "stop") {
    return std::nullopt;
  }

  Stop stop{};
  stop.clientNum = parseClientNum(args[2]);
  if (stop.clientNum == INVALID_CLIENT_NUM) {
    return std::nullopt;
  }

  auto completionTime = parseTime(args[3]);
  if (!completionTime.has_value()) {
    return std::nullopt;
  }

  stop.completionTime = completionTime.value();

  stop.runName = args[4];

  // ETJump 3.7.0 onwards, best segment verdict - absent from older demos,
  // which simply never render gold
  if (args.size() >= 7) {
    stop.beatPersonal = parseInteger(args[5]).value_or(0) != 0;
    stop.beatGlobal = parseInteger(args[6]).value_or(0) != 0;
  }

  return stop;
}

} // namespace ETJump
