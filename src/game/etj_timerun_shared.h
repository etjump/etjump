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
#include <array>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "q_shared.h"

namespace ETJump::TimerunSegments {
// Segment durations for a completed run: segment i is the time from the
// previous checkpoint (the run start, for the first) to checkpoint i, and the
// final segment runs from the last checkpoint to the finish. A run with no
// usable checkpoint data yields no segments.
std::vector<int32_t>
computeSegmentTimes(const std::vector<int32_t> &checkpoints,
                    int32_t completionTime);

// Whether a segment's duration beats the best on record for it, personal and
// global. A zero-length segment, or one with no recorded best, never counts,
// so callers need no guard of their own.
std::pair<bool, bool> bestSegmentBeaten(
    const std::map<std::string, std::vector<int32_t>> &personalBestSegments,
    const std::map<std::string, std::vector<int32_t>> &globalBestSegments,
    const std::string &runName, int32_t segmentIdx, int32_t duration);
} // namespace ETJump::TimerunSegments

namespace ETJump::TimerunCommands {
constexpr int INVALID_CLIENT_NUM = -1;
const int NO_PREVIOUS_RECORD = -1;

int parseClientNum(const std::string &arg);
std::optional<int> parseTime(const std::string &arg);
std::optional<int> parseInteger(const std::string &arg);

class Start {
public:
  Start() = default;
  Start(
      int clientNum, int startTime, const std::string &runName,
      const std::optional<int> &previousRecord, bool runHasCheckpoints,
      std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints,
      std::array<int, MAX_TIMERUN_CHECKPOINTS> currentRunCheckpoints,
      std::array<bool, MAX_TIMERUN_CHECKPOINTS> currentRunBestPersonalSegments,
      std::array<bool, MAX_TIMERUN_CHECKPOINTS> currentRunBestGlobalSegments);

  int clientNum{};
  int startTime{};
  std::string runName;
  std::optional<int> previousRecord;
  bool runHasCheckpoints = false;
  std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints{};
  std::array<int, MAX_TIMERUN_CHECKPOINTS> currentRunCheckpoints{};
  // which of currentRunCheckpoints' segments were gold when they were hit, so
  // a client that resyncs mid-run (e.g. after 'vid_restart') gets its
  // verdicts back. All false on a fresh start - only the hits so far are set.
  std::array<bool, MAX_TIMERUN_CHECKPOINTS> currentRunBestPersonalSegments{};
  std::array<bool, MAX_TIMERUN_CHECKPOINTS> currentRunBestGlobalSegments{};

  [[nodiscard]] std::string serialize() const;

  static std::optional<Start> deserialize(const std::vector<std::string> &args);
};

// this uses deserialize() from Start class for simplicity
class SavePosStart {
public:
  SavePosStart() = default;
  SavePosStart(int clientNum, int startTime, std::string runName,
               const std::optional<int> &previousRecord, bool runHasCheckpoints,
               std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints,
               std::array<int, MAX_TIMERUN_CHECKPOINTS> currentRunCheckpoints);

  int clientNum{};
  int startTime{};
  std::string runName;
  std::optional<int> previousRecord;
  bool runHasCheckpoints = false;
  std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints{};
  std::array<int, MAX_TIMERUN_CHECKPOINTS> currentRunCheckpoints{};

  [[nodiscard]] std::string serialize() const;
};

class Checkpoint {
public:
  Checkpoint() = default;

  Checkpoint(const int clientNum, const int checkpointNum,
             const int checkpointTime, std::string runName,
             const int checkpointIndex, const bool beatPersonal,
             const bool beatGlobal)
      : clientNum(clientNum), checkpointNum(checkpointNum),
        checkpointTime(checkpointTime), runName(std::move(runName)),
        checkpointIndex(checkpointIndex), bestPersonalSegment(beatPersonal),
        bestGlobalSegment(beatGlobal) {}

  int clientNum{};
  int checkpointNum{}; // sequential counter, how many checkpoints we've hit
  int checkpointTime{};
  std::string runName;
  int checkpointIndex{}; // index for Player.checkpointIndexesHit
  // whether this segment beat the best on record when it was hit, so the
  // client can render gold without knowing the best times.
  // Only present in server commands since ETJump 3.7.0
  bool bestPersonalSegment{};
  bool bestGlobalSegment{};

  [[nodiscard]] std::string serialize() const;

  static std::optional<Checkpoint>
  deserialize(const std::vector<std::string> &args);
};

class Interrupt {
public:
  Interrupt() = default;

  explicit Interrupt(int clientNum) : clientNum(clientNum) {}

  int clientNum{};

  [[nodiscard]] std::string serialize() const;

  static std::optional<Interrupt>
  deserialize(const std::vector<std::string> &args);
};

class Completion {
public:
  Completion() = default;

  Completion(int clientNum, int completionTime,
             std::optional<int> previousRecordTime, std::string runName)
      : clientNum(clientNum), completionTime(completionTime),
        previousRecordTime(previousRecordTime), runName(std::move(runName)) {}

  int clientNum{};
  int completionTime{};
  std::optional<int> previousRecordTime;
  std::string runName;

  [[nodiscard]] std::string serialize() const;

  static std::optional<Completion>
  deserialize(const std::vector<std::string> &args);
};

class Record {
public:
  Record() = default;

  Record(int clientNum, int completionTime,
         std::optional<int> previousRecordTime, std::string runName)
      : clientNum(clientNum), completionTime(completionTime),
        previousRecordTime(previousRecordTime), runName(std::move(runName)) {}

  int clientNum{};
  int completionTime{};
  std::optional<int> previousRecordTime;
  std::string runName;

  [[nodiscard]] std::string serialize() const;

  static std::optional<Record>
  deserialize(const std::vector<std::string> &args);
};

class Stop {
public:
  Stop() = default;

  Stop(int clientNum, int time, std::string runName, const bool beatPersonal,
       const bool beatGlobal)
      : clientNum(clientNum), completionTime(time), runName(std::move(runName)),
        beatPersonal(beatPersonal), beatGlobal(beatGlobal) {}

  int clientNum{};
  int completionTime{};
  std::string runName;
  // the finish segment's verdict - the finish closes the run,
  // so we transmit the verdict on the end command, rather than checkpoint
  bool beatPersonal{};
  bool beatGlobal{};

  [[nodiscard]] std::string serialize() const;

  static std::optional<Stop> deserialize(const std::vector<std::string> &args);
};
} // namespace ETJump::TimerunCommands
