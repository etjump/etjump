/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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
#include "../game/q_shared.h"

namespace ETJump {
class OverbounceWatcher : public IRenderable {
public:
  explicit OverbounceWatcher(ClientCommandsHandler *clientCommandsHandler);
  ~OverbounceWatcher() override;

private:
  void render() const override;
  void beforeRender() override;
  bool canSkipDraw() const;

  ClientCommandsHandler *_clientCommandsHandler;
  std::map<std::string, vec3_t> _positions;

  // Currently displayed position
  vec3_t *_current;

  // saves the position with name
  void save(const std::string &name, const vec3_t coordinate);

  // stop displaying anything
  void reset();

  // loads the position to currently displayed position
  // if position is not found, returns false
  bool load(const std::string &name);

  // lists all available positions
  void list() const;

  bool overbounce = false;

  playerState_t *ps;
  float x{};
  float pmoveSec{};
  float v0{}, v0Snapped{};

  float startHeight{}, endHeight{};
  vec3_t start{}, end{};
  vec3_t snap{};

  int gravity{};

  float sizeX{}, sizeY{};
  qhandle_t shader;
  vec4_t _color{};
};
} // namespace ETJump
