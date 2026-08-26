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

  const auto radians = DEG2RAD(degrees);
  const auto sinR = std::sin(radians);
  const auto cosR = std::cos(radians);

  const auto dx = p.x - pivot.x;
  const auto dy = p.y - pivot.y;

  return {pivot.x + (dx * cosR) - (dy * sinR),
          pivot.y + (dx * sinR) + (dy * cosR)};
}

// segment count for faking a circle, scaled with radius so a tiny dot doesn't
// pay for 64 segments and a big ring still looks round
constexpr int maxCircleSegments = 64;

int circleSegments(const float radius) {
  const auto segments = static_cast<int>(std::lround(radius * 2.5f));
  return std::clamp(segments, 8, maxCircleSegments);
}

// shapes that put down a solid area instead of just strokes. their outline gets
// stroked around a slightly grown silhouette, since fattening the shape's own
// stroke does nothing once the fill covers it
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

// scanline fill for any convex polygon, so neither module needs polygon support.
// one span per row rather than one per triangle: splitting a quad into two
// triangles would draw the shared edge twice, which a translucent fill shows as
// a seam.
void fillConvex(const CrosshairPainter &painter, const Point *pts,
                const size_t count, const vec4_t color) {
  if (count < 3) {
    return;
  }

  auto minY = pts[0].y;
  auto maxY = pts[0].y;

  for (size_t i = 1; i < count; i++) {
    minY = std::min(minY, pts[i].y);
    maxY = std::max(maxY, pts[i].y);
  }

  const auto height = maxY - minY;

  if (height <= 0.0f) {
    return;
  }

  const auto rows =
      std::clamp(static_cast<int>(std::lround(height)), 1, maxFillRows);
  const auto step = height / static_cast<float>(rows);

  for (int i = 0; i < rows; i++) {
    const auto y = minY + (step * (static_cast<float>(i) + 0.5f));
    auto x0 = 0.0f;
    auto x1 = 0.0f;
    auto hit = false;

    // convex, so a row crosses the outline exactly twice and everything between
    // those two crossings is inside the shape
    for (size_t j = 0; j < count; j++) {
      const auto &p0 = pts[j];
      const auto &p1 = pts[(j + 1) % count];

      if ((p0.y <= y) == (p1.y <= y)) {
        continue;
      }

      const auto x = p0.x + ((p1.x - p0.x) * ((y - p0.y) / (p1.y - p0.y)));

      if (!hit) {
        x0 = x;
        x1 = x;
        hit = true;
      } else {
        x0 = std::min(x0, x);
        x1 = std::max(x1, x);
      }
    }

    if (!hit || x1 <= x0) {
      continue;
    }

    painter.fillRect(x0, y - (step * 0.5f), x1 - x0, step, color);
  }
}

// closed outline through the given points
void strokePoly(const CrosshairPainter &painter, const Point *pts,
                const size_t count, const float thickness, const vec4_t color) {
  for (size_t i = 0; i < count; i++) {
    const auto &p0 = pts[i];
    const auto &p1 = pts[(i + 1) % count];
    painter.drawLine(p0.x, p0.y, p1.x, p1.y, thickness, color);
  }
}

// radii are independent, so this handles circles and stretched ellipses both
void drawEllipse(const CrosshairPainter &painter, const Point center,
                 const float radiusX, const float radiusY,
                 const float thickness, const float rotation, const bool fill,
                 const vec4_t color) {
  if (radiusX <= 0.0f || radiusY <= 0.0f) {
    return;
  }

  const auto segments = circleSegments(std::max(radiusX, radiusY));
  const auto angleStep = (2.0f * M_PI) / static_cast<float>(segments);

  // build the ring once so the fill and the stroke agree on the shape, and so
  // a stretched ellipse can actually be rotated. the old fill walked rows of
  // the unrotated ellipse, which meant rotation had to be ignored for it.
  std::array<Point, maxCircleSegments> ring{};

  for (int i = 0; i < segments; i++) {
    const auto a = angleStep * static_cast<float>(i);
    ring[i] = rotatePoint({center.x + (std::cos(a) * radiusX),
                           center.y + (std::sin(a) * radiusY)},
                          center, rotation);
  }

  const auto count = static_cast<size_t>(segments);

  if (fill) {
    fillConvex(painter, ring.data(), count, color);
    return;
  }

  strokePoly(painter, ring.data(), count, thickness, color);
}

// outlinePass draws the element's outline in the primary color: a fattened
// stroke for stroke based shapes, and a stroke around a slightly grown
// silhouette for filled ones
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
      // a filled quad looks better than a tiny ellipse at these sizes
      const auto halfX = lenX * 0.5f;
      const auto halfY = lenY * 0.5f;
      const std::array<Point, 4> corners{
          rotatePoint({center.x - halfX, center.y - halfY}, center, rot),
          rotatePoint({center.x + halfX, center.y - halfY}, center, rot),
          rotatePoint({center.x + halfX, center.y + halfY}, center, rot),
          rotatePoint({center.x - halfX, center.y + halfY}, center, rot)};

      if (outlinePass) {
        strokePoly(painter, corners.data(), corners.size(), thickness,
                   primary);
        break;
      }

      fillConvex(painter, corners.data(), corners.size(), secondary);
      break;
    }

    case CrosshairElementType::Circle:
      if (outlinePass && filled) {
        drawEllipse(painter, center, lenX, lenY, thickness, rot, false,
                    primary);
        break;
      }

      if (filled) {
        drawEllipse(painter, center, lenX, lenY, thickness, rot, true,
                    secondary);
      }

      drawEllipse(painter, center, lenX, lenY, thickness, rot, false, primary);
      break;

    case CrosshairElementType::Square: {
      const std::array<Point, 4> corners{
          rotatePoint({center.x - lenX, center.y - lenY}, center, rot),
          rotatePoint({center.x + lenX, center.y - lenY}, center, rot),
          rotatePoint({center.x + lenX, center.y + lenY}, center, rot),
          rotatePoint({center.x - lenX, center.y + lenY}, center, rot)};

      const auto drawBox = [&](const vec4_t topBottom, const vec4_t leftRight) {
        arm(-lenX, -lenY, lenX, -lenY, topBottom);
        arm(-lenX, lenY, lenX, lenY, topBottom);
        arm(-lenX, -lenY, -lenX, lenY, leftRight);
        arm(lenX, -lenY, lenX, lenY, leftRight);
      };

      if (outlinePass && filled) {
        strokePoly(painter, corners.data(), corners.size(), thickness,
                   primary);
        break;
      }

      if (filled) {
        fillConvex(painter, corners.data(), corners.size(), secondary);
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

      const std::array<Point, 3> corners{apex, left, right};

      if (outlinePass && filled) {
        strokePoly(painter, corners.data(), corners.size(), thickness,
                   primary);
        break;
      }

      if (filled) {
        fillConvex(painter, corners.data(), corners.size(), secondary);
      }

      strokePoly(painter, corners.data(), corners.size(), thickness, primary);
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
      // stroke the silhouette in the band just outside the fill instead of
      // putting a solid copy behind it. a solid backing sits between the fill
      // and the world, so lowering the fill's alpha would blend it towards the
      // outline color rather than letting the world through.
      outlineGeometry.lengthX += outlineWidth * 0.5f;
      outlineGeometry.lengthY += outlineWidth * 0.5f;

      // dot's length is a full side, not a half-extent, so it needs double the
      // growth to move its edge out by half the outline width
      if (element.type == CrosshairElementType::Dot) {
        outlineGeometry.lengthX += outlineWidth * 0.5f;
        outlineGeometry.lengthY += outlineWidth * 0.5f;
      }

      drawShape(element, painter, outlineGeometry, outlineWidth,
                element.outlineColor, element.outlineColor, true);
    } else {
      drawShape(element, painter, outlineGeometry,
                geometry.thickness + (outlineWidth * 2.0f),
                element.outlineColor, element.outlineColor, true);
    }
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
