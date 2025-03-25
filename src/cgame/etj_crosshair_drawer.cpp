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

#include "etj_crosshair_drawer.h"

namespace ETJump {
void CrosshairDrawer::drawShader(const crosshair_t &crosshair,
                                 qhandle_t shader) {
  drawPic(crosshair.x - (crosshair.w * 0.5f),
          crosshair.y - (crosshair.h * 0.5f), crosshair.w, crosshair.h, shader,
          crosshair.color);
}

void CrosshairDrawer::drawLineOutline(crosshair_t &crosshair, qhandle_t shader,
                                      bool flipY) {
  crosshair.w += crosshair.w > 0 ? 0.5f : -0.5f;
  crosshair.h += crosshair.h > 0 ? 0.5f : -0.5f;
  drawLine(crosshair, shader, flipY);
}

void CrosshairDrawer::drawLine(const crosshair_t &crosshair, qhandle_t shader,
                               bool flipY) {
  if (flipY) {
    drawPic(crosshair.x - (crosshair.t * 0.5f), crosshair.y, crosshair.t,
            crosshair.h * 0.5f, shader, crosshair.color);
    drawPic(crosshair.x - (crosshair.t * 0.5f),
            crosshair.y + (crosshair.h * 0.5f), crosshair.t, crosshair.h * 0.5f,
            shader, crosshair.colorAlt);
  } else {
    drawPic(crosshair.x - (crosshair.t * 0.5f),
            crosshair.y - (crosshair.h * 0.5f), crosshair.t, crosshair.h * 0.5f,
            shader, crosshair.color);
    drawPic(crosshair.x - (crosshair.t * 0.5f), crosshair.y, crosshair.t,
            crosshair.h * 0.5f, shader, crosshair.colorAlt);
  }
}

void CrosshairDrawer::drawCrossOutline(crosshair_t &crosshair,
                                       qhandle_t shader) {
  crosshair.w += crosshair.w > 0 ? 0.5f : -0.5f;
  crosshair.h += crosshair.h > 0 ? 0.5f : -0.5f;
  drawCross(crosshair, shader);
}

void CrosshairDrawer::drawCross(const crosshair_t &crosshair,
                                qhandle_t shader) {
  // horizontal line
  drawPic(crosshair.x - std::abs(crosshair.w * 0.5f),
          crosshair.y - (crosshair.t * 0.5f), crosshair.w, crosshair.t, shader,
          crosshair.color);
  // vertical line
  drawPic(crosshair.x - (crosshair.t * 0.5f),
          crosshair.y - std::abs(crosshair.h * 0.5f), crosshair.t, crosshair.h,
          shader, crosshair.colorAlt);
}

void CrosshairDrawer::drawDiagCross(const crosshair_t &crosshair) {
  // top-left -> bottom-right
  DrawLine(crosshair.x - (crosshair.w * 0.5f) - (crosshair.t * 0.5f),
           crosshair.y - (crosshair.h * 0.5f) - (crosshair.t * 0.5f),
           crosshair.x + (crosshair.w * 0.5f) - (crosshair.t * 0.5f),
           crosshair.y + (crosshair.h * 0.5f) - (crosshair.t * 0.5f),
           crosshair.t, crosshair.t, crosshair.color);
  // top-right -> bottom-left
  DrawLine(crosshair.x + (crosshair.w * 0.5f) - (crosshair.t * 0.5f),
           crosshair.y - (crosshair.h * 0.5f) - (crosshair.t * 0.5f),
           crosshair.x - (crosshair.w * 0.5f) - (crosshair.t * 0.5f),
           crosshair.y + (crosshair.h * 0.5f) - (crosshair.t * 0.5f),
           crosshair.t, crosshair.t, crosshair.colorAlt);
}

void CrosshairDrawer::drawV(const crosshair_t &crosshair) {
  // left line
  DrawLine(crosshair.x - (crosshair.t * 0.5f),
           crosshair.y - (crosshair.t * 0.5f),
           crosshair.x - (crosshair.w * 0.5f) - (crosshair.t * 0.5f),
           crosshair.y + crosshair.h - (crosshair.t * 0.5f), crosshair.t,
           crosshair.t, crosshair.color);
  // right line
  DrawLine(crosshair.x - (crosshair.t * 0.5f),
           crosshair.y - (crosshair.t * 0.5f),
           crosshair.x + (crosshair.w * 0.5f) - (crosshair.t * 0.5f),
           crosshair.y + crosshair.h - (crosshair.t * 0.5f), crosshair.t,
           crosshair.t, crosshair.colorAlt);
}

void CrosshairDrawer::drawTriangle(const crosshair_t &crosshair,
                                   const bool fill) {
  DrawTriangle(crosshair.x - (crosshair.w * 0.5f) - (crosshair.t * 0.5f),
               crosshair.y - (crosshair.t * 0.5f), crosshair.w, crosshair.h,
               crosshair.t, 0, fill, crosshair.color, crosshair.colorAlt);
}

void CrosshairDrawer::drawTOutline(crosshair_t &crosshair, qhandle_t shader,
                                   bool flipX, bool flipY) {
  crosshair.w += crosshair.w > 0 ? 0.5f : -0.5f;
  crosshair.h += crosshair.h > 0 ? 0.5f : -0.5f;
  crosshair.y += flipY ? -0.5f : 0.5f;
  drawT(crosshair, shader, flipX, flipY);
}

void CrosshairDrawer::drawT(const crosshair_t &crosshair, qhandle_t shader,
                            bool flipX, bool flipY) {
  // middle line
  if (flipY) {
    drawPic(crosshair.x - (crosshair.t * 0.5f), crosshair.y, crosshair.t,
            crosshair.h, shader, crosshair.color);
  } else {
    drawPic(crosshair.x - (crosshair.t * 0.5f), crosshair.y - crosshair.h,
            crosshair.t, crosshair.h, shader, crosshair.color);
  }
  // top line
  if (flipX) {
    drawPic(crosshair.x + (crosshair.w * 0.5f),
            crosshair.y - crosshair.h - (crosshair.t * 0.5f), crosshair.w,
            crosshair.t, shader, crosshair.colorAlt);
  } else {
    drawPic(crosshair.x - (crosshair.w * 0.5f),
            crosshair.y - crosshair.h - (crosshair.t * 0.5f), crosshair.w,
            crosshair.t, shader, crosshair.colorAlt);
  }
}

void CrosshairDrawer::drawTwoLinesOutline(crosshair_t &crosshair,
                                          qhandle_t shader) {
  crosshair.h += crosshair.h > 0 ? 0.5f : -0.5f;
  drawTwoLines(crosshair, shader);
}

void CrosshairDrawer::drawTwoLines(const crosshair_t &crosshair,
                                   qhandle_t shader) {
  // left line
  drawPic(crosshair.x - (crosshair.w * 0.5f) - (crosshair.t * 0.5f),
          crosshair.y - std::abs(crosshair.h * 0.5f), crosshair.t, crosshair.h,
          shader, crosshair.color);
  // right line
  drawPic(crosshair.x + (crosshair.w * 0.5f) - (crosshair.t * 0.5f),
          crosshair.y - std::abs(crosshair.h * 0.5f), crosshair.t, crosshair.h,
          shader, crosshair.colorAlt);
}
} // namespace ETJump
