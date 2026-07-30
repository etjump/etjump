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

#include "etj_crosshair_renderer.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace ETJump {
namespace {
// everything here is in 640x480 virtual units, like the rest of the 2d code.
// length/lengthY are the x/y size of a shape, gap/gapY push the arms away
// from the center.
//
// two-part shapes (cross, V, two lines...) draw one part in the primary color
// and the other in the secondary, and filled shapes use the secondary as the
// fill, same as the built-in crosshairs do with cg_crosshairColorAlt.

// diagonal arms get projected onto both axes so a diagonal cross ends up the
// same visual size as a straight one with the same numbers
constexpr float diagonalFactor = 0.70710678f; // sqrt(0.5)
constexpr float pi = 3.14159265358979323846f;

// the scanline fills lay down one rect per row, so a crosshair sized to cover
// the screen would otherwise cost hundreds of draw calls a frame. rows are
// spread over the full height either way, this just makes them thicker.
constexpr int maxFillRows = 480;

struct Point {
  float x;
  float y;
};

// resolved geometry for one element, after scale, pulse and offset
struct ElementGeometry {
  float centerX;
  float centerY;
  float lengthX;
  float lengthY;
  float gapX;
  float gapY;
  float thickness;
  float rotation;
};

Point rotatePoint(const Point p, const Point pivot, const float degrees) {
  if (degrees == 0.0f) {
    return p;
  }

  const auto radians = degrees * (pi / 180.0f);
  const auto sinR = std::sin(radians);
  const auto cosR = std::cos(radians);

  const auto dx = p.x - pivot.x;
  const auto dy = p.y - pivot.y;

  return {pivot.x + (dx * cosR) - (dy * sinR),
          pivot.y + (dx * sinR) + (dy * cosR)};
}

// segment count for faking a circle, scaled with radius so a tiny dot doesn't
// pay for 64 segments and a big ring still looks round
int circleSegments(const float radius) {
  const auto segments = static_cast<int>(std::lround(radius * 2.5f));
  return std::clamp(segments, 8, 64);
}

// shapes that put down a solid area instead of just strokes. their outline has
// to be an expanded solid behind them, since fattening a stroke does nothing
// once the fill covers it
bool shapeIsFilled(const CrosshairElement &element) {
  switch (element.type) {
    case CrosshairElementType::Dot:
      return true;
    case CrosshairElementType::Circle:
    case CrosshairElementType::Square:
    case CrosshairElementType::Triangle:
      return element.fill;
    default:
      return false;
  }
}

ElementGeometry resolveGeometry(const CrosshairElement &element,
                                const float centerX, const float centerY,
                                const float scale) {
  ElementGeometry geometry{};

  geometry.centerX = centerX + (element.offsetX * scale);
  geometry.centerY = centerY + (element.offsetY * scale);
  geometry.lengthX = element.length * scale;
  geometry.lengthY = element.effectiveLengthY() * scale;
  geometry.gapX = element.gap * scale;
  geometry.gapY = element.effectiveGapY() * scale;
  // keep thickness slightly above zero so thin elements don't disappear when
  // scaled down
  geometry.thickness = std::max(element.thickness * scale, 0.1f);
  geometry.rotation = element.rotation;

  return geometry;
}

// radii are independent, so this handles circles and stretched ellipses both
void drawEllipse(const CrosshairPainter &painter, const Point center,
                 const float radiusX, const float radiusY,
                 const float thickness, const float rotation, const bool fill,
                 const vec4_t color) {
  if (radiusX <= 0.0f || radiusY <= 0.0f) {
    return;
  }

  // scanline fill, one row per virtual unit is plenty at crosshair sizes and
  // saves needing a real polygon path. rotating it would mean rotating the rows
  // too, so a rotated ellipse just falls through to the segment ring below
  if (fill && rotation == 0.0f) {
    const auto rows = std::clamp(static_cast<int>(std::lround(radiusY * 2.0f)),
                                 1, maxFillRows);
    const auto step = (radiusY * 2.0f) / static_cast<float>(rows);

    for (int i = 0; i < rows; i++) {
      const auto dy = -radiusY + (step * (static_cast<float>(i) + 0.5f));
      // x = rx * sqrt(1 - (dy/ry)^2)
      const auto normalized = dy / radiusY;
      const auto halfWidth =
          radiusX *
          std::sqrt(std::max(1.0f - (normalized * normalized), 0.0f));

      if (halfWidth <= 0.0f) {
        continue;
      }

      painter.fillRect(center.x - halfWidth, center.y + dy - (step * 0.5f),
                       halfWidth * 2.0f, step, color);
    }

    return;
  }

  const auto segments = circleSegments(std::max(radiusX, radiusY));
  const auto angleStep = (2.0f * pi) / static_cast<float>(segments);

  for (int i = 0; i < segments; i++) {
    const auto a0 = angleStep * static_cast<float>(i);
    const auto a1 = angleStep * static_cast<float>(i + 1);

    const auto p0 = rotatePoint({center.x + (std::cos(a0) * radiusX),
                                 center.y + (std::sin(a0) * radiusY)},
                                center, rotation);
    const auto p1 = rotatePoint({center.x + (std::cos(a1) * radiusX),
                                 center.y + (std::sin(a1) * radiusY)},
                                center, rotation);

    painter.drawLine(p0.x, p0.y, p1.x, p1.y, thickness, color);
  }
}

// scanline filled triangle from three arbitrary points, so neither module
// needs polygon support
void fillTriangle(const CrosshairPainter &painter, const Point a, const Point b,
                  const Point c, const vec4_t color) {
  std::array<Point, 3> pts{a, b, c};
  std::sort(pts.begin(), pts.end(),
            [](const Point &l, const Point &r) { return l.y < r.y; });

  const auto height = pts[2].y - pts[0].y;

  if (height <= 0.0f) {
    return;
  }

  const auto rows =
      std::clamp(static_cast<int>(std::lround(height)), 1, maxFillRows);
  const auto step = height / static_cast<float>(rows);

  const auto edgeX = [](const Point &p0, const Point &p1, const float y) {
    if (p1.y == p0.y) {
      return p1.x;
    }
    return p0.x + ((p1.x - p0.x) * ((y - p0.y) / (p1.y - p0.y)));
  };

  for (int i = 0; i < rows; i++) {
    const auto y = pts[0].y + (step * (static_cast<float>(i) + 0.5f));

    // long edge spans the full height, the other two split at pts[1]
    const auto xLong = edgeX(pts[0], pts[2], y);
    const auto xShort =
        y < pts[1].y ? edgeX(pts[0], pts[1], y) : edgeX(pts[1], pts[2], y);

    const auto x0 = std::min(xLong, xShort);
    const auto x1 = std::max(xLong, xShort);

    if (x1 <= x0) {
      continue;
    }

    painter.fillRect(x0, y - (step * 0.5f), x1 - x0, step, color);
  }
}

// outlinePass draws the element as a flat silhouette in the primary color so it
// can sit behind the real shape as its outline
void drawShape(const CrosshairElement &element, const CrosshairPainter &painter,
               const ElementGeometry &geometry, const float thickness,
               const vec4_t primary, const vec4_t secondary,
               const bool outlinePass) {
  const Point center{geometry.centerX, geometry.centerY};
  const auto lenX = geometry.lengthX;
  const auto lenY = geometry.lengthY;
  const auto gapX = geometry.gapX;
  const auto gapY = geometry.gapY;
  const auto rot = geometry.rotation;
  const auto filled = shapeIsFilled(element);

  // line with endpoints relative to the element center, rotated around it
  const auto arm = [&](const float x0, const float y0, const float x1,
                       const float y1, const vec4_t color) {
    const auto start = rotatePoint({center.x + x0, center.y + y0}, center, rot);
    const auto end = rotatePoint({center.x + x1, center.y + y1}, center, rot);
    painter.drawLine(start.x, start.y, end.x, end.y, thickness, color);
  };

  switch (element.type) {
    case CrosshairElementType::Cross:
      arm(0, -gapY, 0, -gapY - lenY, secondary); // up
      arm(0, gapY, 0, gapY + lenY, secondary);   // down
      arm(-gapX, 0, -gapX - lenX, 0, primary);   // left
      arm(gapX, 0, gapX + lenX, 0, primary);     // right
      break;

    case CrosshairElementType::HorizontalLine:
      arm(-gapX, 0, -gapX - lenX, 0, primary);
      arm(gapX, 0, gapX + lenX, 0, secondary);
      break;

    case CrosshairElementType::VerticalLine:
      arm(0, -gapY, 0, -gapY - lenY, primary);
      arm(0, gapY, 0, gapY + lenY, secondary);
      break;

    case CrosshairElementType::DiagonalCross: {
      const auto gx = gapX * diagonalFactor;
      const auto gy = gapY * diagonalFactor;
      const auto lx = (gapX + lenX) * diagonalFactor;
      const auto ly = (gapY + lenY) * diagonalFactor;
      // one color per diagonal, like the built-in version
      arm(-gx, -gy, -lx, -ly, primary);
      arm(gx, gy, lx, ly, primary);
      arm(gx, -gy, lx, -ly, secondary);
      arm(-gx, gy, -lx, ly, secondary);
      break;
    }

    case CrosshairElementType::Dot: {
      const auto halfX = lenX * 0.5f;
      const auto halfY = lenY * 0.5f;
      // a filled rect looks better than a tiny ellipse at these sizes
      painter.fillRect(center.x - halfX, center.y - halfY, lenX, lenY,
                       outlinePass ? primary : secondary);
      break;
    }

    case CrosshairElementType::Circle:
      if (outlinePass && filled) {
        drawEllipse(painter, center, lenX, lenY, thickness, rot, true, primary);
        break;
      }

      if (filled) {
        drawEllipse(painter, center, lenX, lenY, thickness, rot, true,
                    secondary);
      }

      drawEllipse(painter, center, lenX, lenY, thickness, rot, false, primary);
      break;

    case CrosshairElementType::Square: {
      const auto drawBox = [&](const vec4_t topBottom, const vec4_t leftRight) {
        arm(-lenX, -lenY, lenX, -lenY, topBottom);
        arm(-lenX, lenY, lenX, lenY, topBottom);
        arm(-lenX, -lenY, -lenX, lenY, leftRight);
        arm(lenX, -lenY, lenX, lenY, leftRight);
      };

      if (outlinePass && filled) {
        painter.fillRect(center.x - lenX, center.y - lenY, lenX * 2.0f,
                         lenY * 2.0f, primary);
        break;
      }

      if (filled) {
        painter.fillRect(center.x - lenX, center.y - lenY, lenX * 2.0f,
                         lenY * 2.0f, secondary);
      }

      drawBox(primary, primary);
      break;
    }

    case CrosshairElementType::Triangle: {
      const auto apex = rotatePoint({center.x, center.y - lenY}, center, rot);
      const auto left =
          rotatePoint({center.x - lenX, center.y + lenY}, center, rot);
      const auto right =
          rotatePoint({center.x + lenX, center.y + lenY}, center, rot);

      if (outlinePass && filled) {
        fillTriangle(painter, apex, left, right, primary);
        break;
      }

      if (filled) {
        fillTriangle(painter, apex, left, right, secondary);
      }

      painter.drawLine(apex.x, apex.y, left.x, left.y, thickness, primary);
      painter.drawLine(left.x, left.y, right.x, right.y, thickness, primary);
      painter.drawLine(right.x, right.y, apex.x, apex.y, thickness, primary);
      break;
    }

    case CrosshairElementType::T:
      arm(-lenX, -gapY, lenX, -gapY, secondary);  // top bar
      arm(0, -gapY, 0, -gapY + lenY, primary);    // downward arm
      break;

    case CrosshairElementType::V: {
      const auto gx = gapX * diagonalFactor;
      const auto gy = gapY * diagonalFactor;
      const auto lx = (gapX + lenX) * diagonalFactor;
      const auto ly = (gapY + lenY) * diagonalFactor;
      arm(-gx, -gy, -lx, -ly, primary);
      arm(gx, -gy, lx, -ly, secondary);
      break;
    }

    case CrosshairElementType::TwoVerticalLines:
      // gap sets the separation, lengthY the height
      arm(-gapX, -lenY, -gapX, lenY, primary);
      arm(gapX, -lenY, gapX, lenY, secondary);
      break;

    case CrosshairElementType::None:
    case CrosshairElementType::NumTypes:
    default:
      break;
  }
}
} // namespace

void drawCrosshairElement(const CrosshairElement &element,
                          const CrosshairPainter &painter, const float centerX,
                          const float centerY, const float scale) {
  if (element.isNoOp() || scale <= 0.0f || !painter.isValid()) {
    return;
  }

  const auto geometry = resolveGeometry(element, centerX, centerY, scale);

  // outline first so the real shape lands on top of it
  if (element.outline && element.outlineThickness > 0.0f &&
      element.outlineAlpha > 0.0f) {
    const auto outlineWidth = element.outlineThickness * scale;
    auto outlineGeometry = geometry;

    if (shapeIsFilled(element)) {
      // fattening the stroke does nothing once the fill covers it, so grow the
      // silhouette and draw it solid behind instead
      outlineGeometry.lengthX += outlineWidth;
      outlineGeometry.lengthY += outlineWidth;

      // dot's length is a full side, not a half-extent, so it needs double the
      // growth to gain outlineWidth on each side
      if (element.type == CrosshairElementType::Dot) {
        outlineGeometry.lengthX += outlineWidth;
        outlineGeometry.lengthY += outlineWidth;
      }
    }

    drawShape(element, painter, outlineGeometry,
              geometry.thickness + (outlineWidth * 2.0f), element.outlineColor,
              element.outlineColor, true);
  }

  drawShape(element, painter, geometry, geometry.thickness, element.color,
            element.colorAlt, false);
}

void drawCrosshairDefinition(const CrosshairDefinition &definition,
                             const CrosshairPainter &painter,
                             const float centerX, const float centerY,
                             const float scale) {
  for (const auto *element : definition.activeElements()) {
    drawCrosshairElement(*element, painter, centerX, centerY, scale);
  }
}

float crosshairElementExtent(const CrosshairElement &element) {
  if (element.isNoOp()) {
    return 0.0f;
  }

  // this is a bounding radius, so take the larger axis of each
  const auto len = std::max(element.length, element.effectiveLengthY());
  const auto gap = std::max(element.gap, element.effectiveGapY());
  float reach;

  switch (element.type) {
    case CrosshairElementType::Dot:
      reach = len * 0.5f;
      break;

    case CrosshairElementType::Circle:
    case CrosshairElementType::Square:
    case CrosshairElementType::Triangle:
      reach = len;
      break;

    case CrosshairElementType::TwoVerticalLines:
      reach = std::max(gap, len);
      break;

    default:
      // arm shapes all reach gap + length
      reach = gap + len;
      break;
  }

  // thickness and outline both bleed outwards
  reach += element.thickness * 0.5f;

  if (element.outline) {
    reach += element.outlineThickness;
  }

  const auto offset =
      std::max(std::abs(element.offsetX), std::abs(element.offsetY));

  return reach + offset;
}

float crosshairDefinitionExtent(const CrosshairDefinition &definition) {
  float extent = 0.0f;

  for (const auto *element : definition.activeElements()) {
    extent = std::max(extent, crosshairElementExtent(*element));
  }

  return extent;
}
} // namespace ETJump
