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
#include <functional>
#include <string>
#include <vector>

#include "../game/q_shared.h"

namespace ETJump {
// a custom crosshair is a stack of these, one per cvar (etj_crosshairElement1..8)
// so a single element always fits in MAX_CVAR_VALUE_STRING and can still be set
// from the console.
//
// shared between cgame (draws it) and ui (edits it), so it can't depend on either.

enum class CrosshairElementType {
  None = 0,
  Cross,
  HorizontalLine,
  VerticalLine,
  DiagonalCross,
  Dot,
  Circle,
  Square,
  Triangle,
  T,
  V,
  TwoVerticalLines,
  NumTypes
};

// display name for a type, same order as the enum
const char *crosshairElementTypeName(CrosshairElementType type);

// short token used in the serialized form, eg "cross". case insensitive on
// parse, returns "none" if the type is out of range
const char *crosshairElementTypeToken(CrosshairElementType type);

// returns None if the token isn't recognized
CrosshairElementType crosshairElementTypeFromToken(const std::string &token);

// shared by the renderer and the editor so both agree on what's valid
namespace CrosshairLimits {
inline constexpr int maxElements = 8;

// deliberately far past anything the sliders reach. the value boxes let you
// type a number directly, and a crosshair spanning the whole 640x480 virtual
// screen is a reasonable thing to want, so these only exist to stop something
// absurd getting into the renderer.
inline constexpr float minLength = 0.0f;
inline constexpr float maxLength = 1024.0f;
inline constexpr float minGap = 0.0f;
inline constexpr float maxGap = 1024.0f;
inline constexpr float minThickness = 0.0f;
inline constexpr float maxThickness = 128.0f;
inline constexpr float minOffset = -1024.0f;
inline constexpr float maxOffset = 1024.0f;
inline constexpr float minRotation = 0.0f;
inline constexpr float maxRotation = 360.0f;
inline constexpr float minAlpha = 0.0f;
inline constexpr float maxAlpha = 1.0f;
inline constexpr float minOutlineThickness = 0.0f;
inline constexpr float maxOutlineThickness = 64.0f;
} // namespace CrosshairLimits

struct CrosshairElement {
  CrosshairElementType type = CrosshairElementType::None;
  bool enabled = true;

  // means different things per type: arm length for lines and crosses, radius
  // for circle/dot, half-extent for square/triangle. this is the x axis for
  // shapes that have two.
  float length = 6.0f;
  // y axis, for stretching a shape into a rectangle, ellipse or uneven cross.
  // 0 means same as length, like cg_crosshairSize treats a single value as
  // uniform. also means older definitions still draw the same.
  float lengthY = 0.0f;
  // where the arms start, measured from the center. unused by dot, circle,
  // square and triangle.
  float gap = 3.0f;
  // vertical counterpart to gap, 0 means same as gap like lengthY does
  float gapY = 0.0f;
  float thickness = 1.0f;

  float offsetX = 0.0f;
  float offsetY = 0.0f;
  float rotation = 0.0f; // degrees, clockwise

  bool outline = true;
  float outlineThickness = 1.0f;
  bool fill = false;

  // colors stay as raw cvar strings ("green", "0x00ff00", "255 0 0") so the
  // color picker and named colors keep working. resolveColors() turns them into
  // the vec4_t's below.
  //
  // primary color, used for the first part of a multi-part shape
  std::string colorString = "white";
  float alpha = 1.0f;
  // secondary color, same idea as cg_crosshairColorAlt: the other half of a
  // two-part shape (vertical arms of a cross, right arm of a V) and the fill
  std::string colorAltString = "white";
  float alphaAlt = 1.0f;
  // outline color, black by default but sometimes you want it contrasting
  std::string outlineColorString = "black";
  float outlineAlpha = 1.0f;

  // filled in by resolveColors(), with the matching alpha folded in
  vec4_t color = {1.0f, 1.0f, 1.0f, 1.0f};
  vec4_t colorAlt = {1.0f, 1.0f, 1.0f, 1.0f};
  vec4_t outlineColor = {0.0f, 0.0f, 0.0f, 1.0f};

  // true if this draws nothing at all, so the renderer can skip it
  bool isNoOp() const;

  // lengthY / gapY with the "0 means uniform" rule applied
  float effectiveLengthY() const;
  float effectiveGapY() const;

  // clamps every numeric field into the limits above
  void clampToLimits();

  // ColorParser lives in the module using this, not here, so the caller passes
  // it in. alphas get applied after.
  void resolveColors(const std::function<void(const std::string &, vec4_t &)>
                         &parseColorString);
};

// serializes to a compact key=value;key=value form, skipping anything still at
// its default. ';' rather than whitespace because color strings can contain
// spaces ("255 0 0").
std::string serializeCrosshairElement(const CrosshairElement &element);

// deliberately tolerant: unknown keys are ignored and missing ones keep their
// default, so something written by a newer build still loads. empty or garbage
// input gives back a default (type None) element.
CrosshairElement parseCrosshairElement(const std::string &input);

// the whole crosshair, empty slots being type None
struct CrosshairDefinition {
  std::array<CrosshairElement, CrosshairLimits::maxElements> elements;

  // slots that'll actually draw something, in draw order
  std::vector<const CrosshairElement *> activeElements() const;
};
} // namespace ETJump
