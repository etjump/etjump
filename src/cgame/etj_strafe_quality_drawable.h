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

namespace ETJump {
class CvarUpdateHandler;
class ClientCommandsHandler;
class PlayerEventsHandler;

class StrafeQuality : public IRenderable {
  double totalFrames{};
  double goodFrames{};
  double strafeQuality{};
  int lastUpdateTime{};

  float oldSpeed{};
  int team{};
  vec4_t color{};

  // default absolute hud position
  static constexpr float posX = 100;
  static constexpr float posY = 100;
  // amount of digits to show on hud
  static constexpr std::size_t digits = 4;

  CvarValue::Size size{};

  void startListeners();
  void parseColor(const vmCvar_t *cvar);
  void setSize(const vmCvar_t *cvar);
  void resetStrafeQuality();
  [[nodiscard]] bool canSkipDraw() const;
  bool canSkipUpdate(usercmd_t cmd);

  const pmove_t *pm{};
  const playerState_t *ps = &cg.predictedPlayerState;

  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
  std::shared_ptr<ClientCommandsHandler> consoleCommands;
  std::shared_ptr<PlayerEventsHandler> playerEvents;

public:
  StrafeQuality(const std::shared_ptr<CvarUpdateHandler> &cvarUpdate,
                const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
                const std::shared_ptr<PlayerEventsHandler> &playerEvents);
  ~StrafeQuality() override;

  bool beforeRender() override;
  void render() const override;
};
} // namespace ETJump
