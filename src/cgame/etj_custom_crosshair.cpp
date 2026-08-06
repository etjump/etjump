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

#include "etj_custom_crosshair.h"
#include "cg_local.h"

namespace ETJump {
const CrosshairPainter &CustomCrosshairDrawer::painter() {
  // built once, the primitives are stateless and rebuilding two std::function's
  // every frame would just be wasted work
  static const CrosshairPainter painter{
      [](const float x, const float y, const float w, const float h,
         const vec4_t color) { CG_FillRect(x, y, w, h, color); },
      [](const float x0, const float y0, const float x1, const float y1,
         const float thickness, const vec4_t color) {
        // drawLineDDA turns axis aligned lines into a single DrawPic and only
        // walks pixel by pixel when the line is actually diagonal
        drawLineDDA(x0, y0, x1, y1, thickness, color);
      }};

  return painter;
}

void CustomCrosshairDrawer::draw(const CrosshairDefinition &definition,
                                 const float centerX, const float centerY,
                                 const float scale) {
  drawCrosshairDefinition(definition, painter(), centerX, centerY, scale);
}
} // namespace ETJump
