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

#include "etj_crosshair.h"

namespace ETJump {
class CrosshairDrawer : public Crosshair {
public:
  static void drawShader(const crosshair_t &crosshair, qhandle_t shader);
  static void drawLine(const crosshair_t &crosshair, qhandle_t shader,
                       bool flipY);
  static void drawLineOutline(crosshair_t &crosshair, qhandle_t shader,
                              bool flipY);
  static void drawCross(const crosshair_t &crosshair, qhandle_t shader);
  static void drawCrossOutline(crosshair_t &crosshair, qhandle_t shader);
  static void drawDiagCross(const crosshair_t &crosshair);
  static void drawV(const crosshair_t &crosshair);
  static void drawTriangle(const crosshair_t &crosshair, bool fill);
  static void drawT(const crosshair_t &crosshair, qhandle_t shader, bool flipX,
                    bool flipY);
  static void drawTOutline(crosshair_t &crosshair, qhandle_t shader, bool flipX,
                           bool flipY);
  static void drawTwoLines(const crosshair_t &crosshair, qhandle_t shader);
  static void drawTwoLinesOutline(crosshair_t &crosshair, qhandle_t shader);
};
} // namespace ETJump
