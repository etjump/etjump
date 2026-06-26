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
#include "etj_irenderable.h"
#include "etj_cvar_parser.h"
#include "etj_pmove_utils_v2.h"

namespace ETJump {
class CvarUpdateHandler;
class ClientCommandsHandler;
class PlayerEventsHandler;

class UpmoveMeterV2 : public IRenderable {
public:
  UpmoveMeterV2(const std::shared_ptr<CvarUpdateHandler> &cvarUpdate,
                const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
                const std::shared_ptr<PlayerEventsHandler> &playerEvents);
  ~UpmoveMeterV2() override;

  bool beforeRender() override;
  void render() const override;

private:
  void startListeners();
  void setTextSize(const vmCvar_t &cvar);
  void resetUpmoveMeter();

  void updateJumpState();
  void updateUpmoveMeter();

  [[nodiscard]] bool canSkipDraw() const;
  [[nodiscard]] bool canSkipUpdate(const PmoveUtilsV2::State &s) const;

  enum class JumpState {
    AIR_NOJUMP = 0,         // in air, not holding jump
    AIR_JUMP = 1,           // in air, holding jump
    GROUND_JUMP = 2,        // on ground, holding jump
    GROUND_NOJUMP = 3,      // on ground, not holding jump
    AIR_JUMP_NORELEASE = 4, // in air (post jump), holding jump
  };

  JumpState jumpState{};
  JumpState lastJumpState{};

  struct Graph {
    int32_t preDelay;
    int32_t postDelay;
    int32_t fullDelay;

    rectDef_t rect;
    float upHeight;
    float downHeight;

    vec4_t colorBg;
    vec4_t colorPreJump;
    vec4_t colorPostJump;
    vec4_t colorOnGround;
    vec4_t colorOutline;
  };

  Graph graph{};
  int32_t jumpPreGroundTime{};
  int32_t groundTouchTime{};

  float textX{};
  float textH{};
  float textHeightOffset{};
  CvarValue::Size textSize{};
  int32_t textStyle{};
  vec4_t colorText{};

  team_t team{};
  int32_t lastUpdateTime{};

  bool inAir{};
  bool jumping{};

  const playerState_t *ps = &cg.predictedPlayerState;

  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
  std::shared_ptr<ClientCommandsHandler> consoleCommands;
  std::shared_ptr<PlayerEventsHandler> playerEvents;
};
} // namespace ETJump
