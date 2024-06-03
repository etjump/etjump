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

#include <memory>

#include "etj_cvar_parser.h"
#include "etj_irenderable.h"

#include "../game/q_shared.h"

namespace ETJump {
class CvarUpdateHandler;

class DrawYaw : public IRenderable {
public:
  explicit DrawYaw(const std::shared_ptr<CvarUpdateHandler> &cvarUpdate);
  ~DrawYaw() override;

  bool beforeRender() override;
  void render() const override;

private:
  void parseColor(const std::string &colorStr);
  void setSize(const vmCvar_t &cvar);
  void startListeners();

  static bool canSkipDraw();

  bool moveLeft{};  // view yaw turned left this frame
  bool moveRight{}; // view yaw turned right this frame

  bool hasLastYaw{}; // false until lastYaw holds a valid reference angle
  int lastYaw{};
  int lastClientNum{-1};
  int32_t lastUpdateTime{};

  float x{};
  float y{};
  int32_t textStyle{};
  CvarValue::Size size{};
  const char *str{}; // arrow glyph to draw, set in beforeRender

  vec4_t color{};

  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
};
} // namespace ETJump
