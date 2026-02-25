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
class EntityEventsHandler;
class ClientCommandsHandler;
class CvarUpdateHandler;
class EntityEventsHandler;
class PlayerEventsHandler;

class JumpSpeeds : public IRenderable {
public:
  explicit JumpSpeeds(
      const std::shared_ptr<EntityEventsHandler> &entityEvents,
      const std::shared_ptr<PlayerEventsHandler> &playerEvents,
      const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
      const std::shared_ptr<ClientCommandsHandler> &serverCommands,
      const std::shared_ptr<CvarUpdateHandler> &cvarUpdate);
  ~JumpSpeeds() override;

  void render() const override;
  bool beforeRender() override;

private:
  static constexpr int MAX_JUMPS = 10;

  // jumpspeeds + color string
  std::vector<std::pair<int, std::string>> jumpSpeeds;
  // jumpspeed colors parsed to vec4_t for drawing
  vec4_t jumpSpeedsColors[MAX_JUMPS]{};

  std::string baseColorStr = etj_jumpSpeedsColor.string;
  std::string fasterColorStr = etj_jumpSpeedsFasterColor.string;
  std::string slowerColorStr = etj_jumpSpeedsSlowerColor.string;
  bool resetQueued = false;
  int team = 0;
  std::string label = "Jump Speeds:";

  enum class jumpSpeedStyle { Horizontal = 1, NoLabel = 2, Reversed = 4 };

  CvarValue::Size size{};

  std::shared_ptr<EntityEventsHandler> entityEvents;
  std::shared_ptr<PlayerEventsHandler> playerEvents;
  std::shared_ptr<ClientCommandsHandler> consoleCommands;
  std::shared_ptr<ClientCommandsHandler> serverCommands;
  std::shared_ptr<CvarUpdateHandler> cvarUpdate;

  void startListeners();
  void updateJumpSpeeds();
  void queueJumpSpeedsReset();
  void adjustColors();
  void colorStrToVec();
  void adjustSize(const vmCvar_t &cvar);
  [[nodiscard]] bool canSkipDraw() const;
};
} // namespace ETJump
