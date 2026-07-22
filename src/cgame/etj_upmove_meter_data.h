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

#include "cg_local.h"
#include "etj_pmove_utils_v2.h"

namespace ETJump {
class UpmoveMeterData {
private:
  enum class JumpState {
    AIR_NOJUMP = 0,         // in air, not holding jump
    AIR_JUMP = 1,           // in air, holding jump
    GROUND_JUMP = 2,        // on ground, holding jump
    GROUND_NOJUMP = 3,      // on ground, not holding jump
    AIR_JUMP_NORELEASE = 4, // in air (post jump), holding jump
  };

  struct State {
    JumpState jumpState;
    JumpState lastJumpState;

    bool inAir;
    bool jumping;

    int32_t preDelay;
    int32_t postDelay;
    int32_t fullDelay;

    int32_t jumpPreGroundTime;
    int32_t groundTouchTime;

    int32_t lastUpdateTime;
  };

  void startListeners();
  void updateJumpState();
  void updateUpmoveValues();
  void reset();
  static bool canSkipUpdate(const PmoveUtilsV2::State &pmove);

  State s{};
  team_t team{};
  const playerState_t *ps = &cg.predictedPlayerState;

  std::shared_ptr<ClientCommandsHandler> consoleCommands;
  std::shared_ptr<PlayerEventsHandler> playerEvents;

public:
  UpmoveMeterData(const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
                  const std::shared_ptr<PlayerEventsHandler> &playerEvents);
  ~UpmoveMeterData();

  // the maximum abs value for pre/post jump delay for upmove meter data
  static constexpr int32_t MAX_UPMOVE_TIME = 1000;

  static bool check();
  void runFrame();
  [[nodiscard]] const State &getState() const;
};
} // namespace ETJump
