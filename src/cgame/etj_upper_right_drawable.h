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

#include <array>

#include "etj_irenderable.h"
#include "cg_local.h"

namespace ETJump {
class UpperRight : public IRenderable {
  static constexpr float y0 = 152.0f;
  static constexpr float textScale = 0.19f;

  vec4_t textColor = {0.625f, 0.625f, 0.6f, 1.0f};
  vec4_t backgroundColor = {0.16f, 0.2f, 0.17f, 0.8f};
  vec4_t borderColor = {0.5f, 0.5f, 0.5f, 0.5f};
  int upperRight_x = SCREEN_WIDTH - 6;

  static constexpr std::size_t FPSFrames = 20;
  std::array<int64_t, FPSFrames> FPSFrameTimes{};
  std::size_t FPSIndex;
  std::size_t FPSInit;
  int64_t FPSLastUpdate;
  int64_t FPSLastTime;
  int64_t fps;

  void drawRoundTimer(float &y) const;
  void drawClock(float &y) const;
  void drawFPS(float &y) const;
  void drawSpeed(float &y) const;
  void drawSnapshot(float &y) const;

public:
  UpperRight();
  void render() const override;
  bool beforeRender() override;
};
} // namespace ETJump
