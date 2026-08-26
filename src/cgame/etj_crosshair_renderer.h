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

#include <functional>

#include "etj_crosshair_definition.h"

namespace ETJump {
// the shape maths lives here instead of in cgame because ui has to draw the
// exact same crosshair for the editor preview, and the two modules have their
// own 2d drawing apis. each one passes in these two primitives and gets the
// same geometry back.
struct CrosshairPainter {
  // axis aligned filled rect, 640x480 virtual units
  std::function<void(float x, float y, float w, float h, const vec4_t color)>
      fillRect;
  // line of the given thickness between two arbitrary points
  std::function<void(float x0, float y0, float x1, float y1, float thickness,
                     const vec4_t color)>
      drawLine;

  bool isValid() const { return fillRect && drawLine; }
};

// scale multiplies every dimension
void drawCrosshairDefinition(const CrosshairDefinition &definition,
                             const CrosshairPainter &painter, float centerX,
                             float centerY, float scale);

void drawCrosshairElement(const CrosshairElement &element,
                          const CrosshairPainter &painter, float centerX,
                          float centerY, float scale);

// furthest this element can paint from the crosshair center, used by the editor
// to fit a crosshair inside the preview box
float crosshairElementExtent(const CrosshairElement &element);
float crosshairDefinitionExtent(const CrosshairDefinition &definition);
} // namespace ETJump
