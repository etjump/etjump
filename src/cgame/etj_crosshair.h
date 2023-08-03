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

#include "cg_local.h"
#include "etj_irenderable.h"

namespace ETJump {
class Crosshair : public IRenderable {
  void startListeners();
  void parseColors();
  void adjustSize();
  void adjustPosition();
  static bool canSkipDraw();

  enum class ETJumpCrosshairs {
    VerticalLine = 10,
    Cross = 11,
    DiagonalCross = 12,
    V = 13,
    Triangle = 14,
    T = 15,
    TwoVerticalLines = 16
  };

protected:
  typedef struct {
    vec4_t color;
    vec4_t colorAlt;

    float x;
    float y;
    float w;
    float h;
    float t; // crosshair thickness

    float f; // cg_crosshairPulse size modifier
  } crosshair_t;

  crosshair_t crosshair{};

public:
  Crosshair();
  void render() const override;
  void beforeRender() override;
};
} // namespace ETJump