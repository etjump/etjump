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

#include "etj_string_utilities.h"
#include "etj_savepos_shared.h"

namespace ETJump {
std::string SavePosData::serialize(const SavePosData &data) {
  const std::string &origin = StringUtil::join(data.pos.origin, " ");
  const std::string &angles = StringUtil::join(data.pos.angles, " ");
  const std::string &velocity = StringUtil::join(data.pos.velocity, " ");

  // omit timerun data if this position wasn't saved during a timerun
  if (data.timerunInfo.runName.empty()) {
    return stringFormat("loadpos %s %s %s %i", origin, angles, velocity,
                        static_cast<int>(data.pos.stance));
  }

  const std::string &checkpoints =
      StringUtil::join(data.timerunInfo.checkpoints, ",");
  const std::string &previousRecordCheckpoints =
      StringUtil::join(data.timerunInfo.previousRecordCheckpoints, ",");

  return stringFormat(
      "loadpos %s %s %s %i \"%s\" %i %i %s %s", origin, angles, velocity,
      static_cast<int>(data.pos.stance), data.timerunInfo.runName,
      data.timerunInfo.currentRunTimer, data.timerunInfo.previousRecord,
      checkpoints, previousRecordCheckpoints);
}

SavePosData SavePosData::deserialize(const std::vector<std::string> &args) {
  // minimum valid argument count, position with timerun data contains more
  static constexpr int expectedMinArgs = 10;
  SavePosData data{};

  try {
    for (int i = 0; i < 3; i++) {
      data.pos.origin[i] = std::stof(args[0 + i]);
      data.pos.angles[i] = std::stof(args[3 + i]);
      data.pos.velocity[i] = std::stof(args[6 + i]);
    }

    data.pos.stance = static_cast<PlayerStance>(std::stoi(args[9]));

    if (args.size() == expectedMinArgs || args[10].empty()) {
      return data;
    }

    data.timerunInfo.runName = args[10];
    data.timerunInfo.currentRunTimer = std::stoi(args[11]);
    data.timerunInfo.previousRecord = std::stoi(args[12]);

    const auto checkpoints = StringUtil::split(args[13], ",");
    const auto previousRecordCheckpoints = StringUtil::split(args[14], ",");

    // sanity check
    if (checkpoints.size() != MAX_TIMERUN_CHECKPOINTS ||
        previousRecordCheckpoints.size() != MAX_TIMERUN_CHECKPOINTS) {
      return data;
    }

    for (int i = 0; i < MAX_TIMERUN_CHECKPOINTS; i++) {
      data.timerunInfo.checkpoints[i] = std::stoi(checkpoints[i]);
      data.timerunInfo.previousRecordCheckpoints[i] =
          std::stoi(previousRecordCheckpoints[i]);
    }
  } catch (const std::invalid_argument &e) {
    data.error = stringFormat("invalid argument for %s\n", e.what());
    return data;
  } catch (const std::out_of_range &e) {
    data.error = stringFormat("argument out of range for %s\n", e.what());
    return data;
  }

  return data;
}
} // namespace ETJump
