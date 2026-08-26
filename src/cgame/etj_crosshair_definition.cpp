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

#include "etj_crosshair_definition.h"
#include "../game/etj_string_utilities.h"

#include <algorithm>
#include <cmath>

namespace ETJump {
namespace {
// indexed by CrosshairElementType, order has to match the enum
struct TypeInfo {
  const char *token;
  const char *name;
};

constexpr std::array<TypeInfo, static_cast<size_t>(
                                   CrosshairElementType::NumTypes)>
    typeInfo = {{
        {"none", "None"},
        {"cross", "Cross"},
        {"hline", "Horizontal line"},
        {"vline", "Vertical line"},
        {"diagcross", "Diagonal cross"},
        {"dot", "Dot"},
        {"circle", "Circle"},
        {"square", "Square"},
        {"triangle", "Triangle"},
        {"t", "T shape"},
        {"v", "V shape"},
        {"twolines", "Two vertical lines"},
    }};

bool isValidType(const CrosshairElementType type) {
  const auto idx = static_cast<int>(type);
  return idx >= 0 && idx < static_cast<int>(CrosshairElementType::NumTypes);
}

std::string floatToString(const float value) {
  auto str = StringUtils::normalizeNumberString(std::to_string(value));
  return str.empty() ? "0" : str;
}

// keeps the fallback on anything unparsable, so one bad field can't wreck the
// rest of the element
float parseFloat(const std::string &value, const float fallback) {
  try {
    return std::stof(value);
  } catch (...) {
    return fallback;
  }
}

bool parseBool(const std::string &value, const bool fallback) {
  if (value.empty()) {
    return fallback;
  }

  if (StringUtils::iEqual(value, "true") || StringUtils::iEqual(value, "yes")) {
    return true;
  }

  if (StringUtils::iEqual(value, "false") || StringUtils::iEqual(value, "no")) {
    return false;
  }

  try {
    return std::stoi(value) != 0;
  } catch (...) {
    return fallback;
  }
}
} // namespace

const char *crosshairElementTypeName(const CrosshairElementType type) {
  if (!isValidType(type)) {
    return typeInfo[0].name;
  }

  return typeInfo[static_cast<size_t>(type)].name;
}

const char *crosshairElementTypeToken(const CrosshairElementType type) {
  if (!isValidType(type)) {
    return typeInfo[0].token;
  }

  return typeInfo[static_cast<size_t>(type)].token;
}

CrosshairElementType crosshairElementTypeFromToken(const std::string &token) {
  const auto trimmed = StringUtils::trim(token);

  for (size_t i = 0; i < typeInfo.size(); i++) {
    if (StringUtils::iEqual(trimmed, typeInfo[i].token)) {
      return static_cast<CrosshairElementType>(i);
    }
  }

  return CrosshairElementType::None;
}

bool CrosshairElement::isNoOp() const {
  if (!enabled || type == CrosshairElementType::None || !isValidType(type)) {
    return true;
  }

  // transparent on every channel that could draw. checked together since a
  // two-part shape still shows its secondary half at primary alpha 0
  const bool outlineVisible = outline && outlineThickness > 0.0f &&
                              outlineAlpha > 0.0f;

  if (alpha <= 0.0f && alphaAlt <= 0.0f && !outlineVisible) {
    return true;
  }

  // everything is sized off length, so zero length draws nothing. zero
  // thickness still draws for filled shapes though.
  if (length <= 0.0f) {
    return true;
  }

  const bool filledShape = type == CrosshairElementType::Dot ||
                           (fill && (type == CrosshairElementType::Square ||
                                     type == CrosshairElementType::Triangle ||
                                     type == CrosshairElementType::Circle));

  if (!filledShape && thickness <= 0.0f) {
    return true;
  }

  return false;
}

float CrosshairElement::effectiveLengthY() const {
  return lengthY > 0.0f ? lengthY : length;
}

float CrosshairElement::effectiveGapY() const {
  return gapY > 0.0f ? gapY : gap;
}

void CrosshairElement::clampToLimits() {
  using namespace CrosshairLimits;

  length = std::clamp(length, minLength, maxLength);
  lengthY = std::clamp(lengthY, minLength, maxLength);
  gap = std::clamp(gap, minGap, maxGap);
  gapY = std::clamp(gapY, minGap, maxGap);
  thickness = std::clamp(thickness, minThickness, maxThickness);
  offsetX = std::clamp(offsetX, minOffset, maxOffset);
  offsetY = std::clamp(offsetY, minOffset, maxOffset);
  outlineThickness =
      std::clamp(outlineThickness, minOutlineThickness, maxOutlineThickness);
  alpha = std::clamp(alpha, minAlpha, maxAlpha);
  alphaAlt = std::clamp(alphaAlt, minAlpha, maxAlpha);
  outlineAlpha = std::clamp(outlineAlpha, minAlpha, maxAlpha);

  // rotation wraps instead of clamping so dragging past 360 keeps going
  rotation = std::fmod(rotation, maxRotation);

  if (rotation < 0.0f) {
    rotation += maxRotation;
  }

  if (!isValidType(type)) {
    type = CrosshairElementType::None;
  }
}

void CrosshairElement::resolveColors(
    const std::function<void(const std::string &, vec4_t &)>
        &parseColorString) {
  if (parseColorString) {
    parseColorString(colorString, color);
    parseColorString(colorAltString, colorAlt);
    parseColorString(outlineColorString, outlineColor);
  }

  using namespace CrosshairLimits;

  color[3] = std::clamp(alpha, minAlpha, maxAlpha);
  colorAlt[3] = std::clamp(alphaAlt, minAlpha, maxAlpha);
  outlineColor[3] = std::clamp(outlineAlpha, minAlpha, maxAlpha);
}

std::string serializeCrosshairElement(const CrosshairElement &element) {
  // nothing to store for an empty slot, and leaving it empty keeps the cvar out
  // of the written config too
  if (element.type == CrosshairElementType::None) {
    return "";
  }

  const CrosshairElement defaults{};
  std::vector<std::string> fields;

  fields.emplace_back(std::string("t=") +
                      crosshairElementTypeToken(element.type));

  const auto addFloat = [&](const char *key, const float value,
                            const float defaultValue) {
    if (value != defaultValue) {
      fields.emplace_back(std::string(key) + "=" + floatToString(value));
    }
  };

  const auto addBool = [&](const char *key, const bool value,
                           const bool defaultValue) {
    if (value != defaultValue) {
      fields.emplace_back(std::string(key) + "=" + (value ? "1" : "0"));
    }
  };

  addBool("e", element.enabled, defaults.enabled);
  addFloat("l", element.length, defaults.length);
  addFloat("l2", element.lengthY, defaults.lengthY);
  addFloat("g", element.gap, defaults.gap);
  addFloat("g2", element.gapY, defaults.gapY);
  addFloat("th", element.thickness, defaults.thickness);
  addFloat("x", element.offsetX, defaults.offsetX);
  addFloat("y", element.offsetY, defaults.offsetY);
  addFloat("r", element.rotation, defaults.rotation);
  addBool("o", element.outline, defaults.outline);
  addFloat("ot", element.outlineThickness, defaults.outlineThickness);
  addBool("f", element.fill, defaults.fill);

  if (element.colorString != defaults.colorString) {
    fields.emplace_back("c=" + element.colorString);
  }

  if (element.colorAltString != defaults.colorAltString) {
    fields.emplace_back("c2=" + element.colorAltString);
  }

  if (element.outlineColorString != defaults.outlineColorString) {
    fields.emplace_back("oc=" + element.outlineColorString);
  }

  addFloat("a", element.alpha, defaults.alpha);
  addFloat("a2", element.alphaAlt, defaults.alphaAlt);
  addFloat("oa", element.outlineAlpha, defaults.outlineAlpha);

  return StringUtils::join(fields, ";");
}

CrosshairElement parseCrosshairElement(const std::string &input) {
  CrosshairElement element{};

  const auto trimmedInput = StringUtils::trim(input);

  if (trimmedInput.empty()) {
    return element;
  }

  for (const auto &field : StringUtils::split(trimmedInput, ";")) {
    const auto separator = field.find('=');

    if (separator == std::string::npos) {
      continue;
    }

    const auto key = StringUtils::toLowerCase(StringUtils::trim(field.substr(0, separator)));
    // only trim the ends, names like "light blue" need their inner space
    const auto value = StringUtils::trim(field.substr(separator + 1));

    if (key.empty()) {
      continue;
    }

    if (key == "t") {
      element.type = crosshairElementTypeFromToken(value);
    } else if (key == "e") {
      element.enabled = parseBool(value, element.enabled);
    } else if (key == "l") {
      element.length = parseFloat(value, element.length);
    } else if (key == "l2") {
      element.lengthY = parseFloat(value, element.lengthY);
    } else if (key == "g") {
      element.gap = parseFloat(value, element.gap);
    } else if (key == "g2") {
      element.gapY = parseFloat(value, element.gapY);
    } else if (key == "th") {
      element.thickness = parseFloat(value, element.thickness);
    } else if (key == "x") {
      element.offsetX = parseFloat(value, element.offsetX);
    } else if (key == "y") {
      element.offsetY = parseFloat(value, element.offsetY);
    } else if (key == "r") {
      element.rotation = parseFloat(value, element.rotation);
    } else if (key == "o") {
      element.outline = parseBool(value, element.outline);
    } else if (key == "ot") {
      element.outlineThickness = parseFloat(value, element.outlineThickness);
    } else if (key == "f") {
      element.fill = parseBool(value, element.fill);
    } else if (key == "c") {
      if (!value.empty()) {
        element.colorString = value;
      }
    } else if (key == "c2") {
      if (!value.empty()) {
        element.colorAltString = value;
      }
    } else if (key == "oc") {
      if (!value.empty()) {
        element.outlineColorString = value;
      }
    } else if (key == "a") {
      element.alpha = parseFloat(value, element.alpha);
    } else if (key == "a2") {
      element.alphaAlt = parseFloat(value, element.alphaAlt);
    } else if (key == "oa") {
      element.outlineAlpha = parseFloat(value, element.outlineAlpha);
    }
  }

  element.clampToLimits();

  return element;
}

std::vector<const CrosshairElement *>
CrosshairDefinition::activeElements() const {
  std::vector<const CrosshairElement *> active;

  for (const auto &element : elements) {
    if (!element.isNoOp()) {
      active.push_back(&element);
    }
  }

  return active;
}
} // namespace ETJump
