/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
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

#include "etj_irenderable.h"
#include "etj_cvar_parser.h"

namespace ETJump {
class SpectatorInfo : public IRenderable {
  CvarValue::Scale scale{};
  int32_t textStyle{};
  float sizeX{};
  float sizeY{};
  float rowHeight{};
  std::vector<std::pair<int32_t, bool>> spectators;

  enum class DrawDirection {
    DOWN = 0,
    UP = 1,
  };

  static constexpr vec4_t inactiveColor = {1.0f, 1.0f, 1.0f, 0.33f};

  static bool canSkipDraw();
  void startListeners();

  void setTextSize(const vmCvar_t &cvar);
  void setRowHeight();
  void setTextStyle(const vmCvar_t &cvar);
  static float getTextOffset(const char *name, float fontWidth);

public:
  SpectatorInfo();

  bool beforeRender() override;
  void render() const override;
};
} // namespace ETJump
