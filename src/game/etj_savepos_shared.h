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

#pragma once

#include "../game/etj_timerun_shared.h"

namespace ETJump {
class SavePosData {
  struct Position {
    vec3_t origin{};
    vec3_t angles{};
    vec3_t velocity{};
    PlayerStance stance{};
  };

  struct TimerunInfo {
    TimerunInfo() {
      checkpoints.fill(TIMERUN_CHECKPOINT_NOT_SET);
      previousRecordCheckpoints.fill(TIMERUN_CHECKPOINT_NOT_SET);
      previousRecord = -1;
    };

    std::string runName{};
    int currentRunTimer{};
    int previousRecord{};
    std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints{};
    std::array<int, MAX_TIMERUN_CHECKPOINTS> previousRecordCheckpoints{};
  };

public:
  std::string name{};
  std::string mapname{};

  Position pos{};
  TimerunInfo timerunInfo{};
};
} // namespace ETJump
