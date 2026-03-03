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

#include <map>

#include "etj_irenderable.h"
#include "etj_cvar_parser.h"

#include "../game/q_shared.h"

namespace ETJump {
class ClientCommandsHandler;
class CvarUpdateHandler;

class OverbounceWatcher : public IRenderable {
public:
  OverbounceWatcher(
      const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
      const std::shared_ptr<CvarUpdateHandler> &cvarUpdate);
  ~OverbounceWatcher() override;

private:
  void render() const override;
  bool beforeRender() override;
  [[nodiscard]] bool canSkipDraw() const;

  void startListeners();

  // saves the position with name
  void save(const std::vector<std::string> &args);

  // stop displaying anything
  void reset();

  // loads the position to currently displayed position
  // if position is not found, returns false
  void load(const std::vector<std::string> &args);

  // lists all available positions
  void list() const;

  void setSize(const vmCvar_t *cvar);

  std::map<std::string, vec3_t> positions;

  // Currently displayed position
  vec3_t *current = nullptr;

  bool overbounce = false;

  const playerState_t *ps = nullptr;

  float x{};
  float pmoveSec{};
  float zVel{};
  float zVelSnapped{};

  float startHeight{};
  float endHeight{};
  vec3_t start{};
  vec3_t end{};
  vec3_t snap{};

  int gravity{};

  CvarValue::Size size{};
  qhandle_t shader;
  vec4_t color{};

  std::shared_ptr<ClientCommandsHandler> consoleCommands;
  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
};
} // namespace ETJump
