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

#include "etj_color_parser.h"

#include "../game/etj_string_utilities.h"

namespace ETJump {
void ColorParser::parseColorString(const std::string &colorString,
                                   vec4_t &color) {
  Vector4Set(color, 0.0f, 0.0f, 0.0f, 1.0); // set defaults

  std::string token{StringUtil::toLowerCase(trim(colorString))};

  if (std::regex_match(token, alphaRegex)) {
    parseNamedColorString(token, color);
    return;
  }

  if (std::regex_match(token, digitRegex)) {
    parseRGBAValuedColorString(token, color);
  } else if (std::regex_match(token, hexedRegex)) {
    parseHexValuedColorString(token.substr(2, 8), color);
  } else if (std::regex_match(token, hashdRegex)) {
    parseHexValuedColorString(token.substr(1, 8), color);
  }

  normalizeColorIfRequired(color);
}

void ColorParser::parseNamedColorString(const std::string &token,
                                        vec4_t &color) {
  const auto *validColor = validColorNames[token];

  if (validColor) {
    color[0] = (*validColor)[0];
    color[1] = (*validColor)[1];
    color[2] = (*validColor)[2];
  }
}

void ColorParser::parseRGBAValuedColorString(const std::string &colorString,
                                             vec4_t &color) {
  std::istringstream tokenStream{colorString};
  std::string token;
  int32_t colorChannel = 0;

  while (tokenStream >> token) {
    if (colorChannel > 3) {
      break;
    }

    const auto value = std::min(std::max(std::stof(token), 0.f), 255.0f);
    color[colorChannel] = value;
    colorChannel++;
  }
}

void ColorParser::parseHexValuedColorString(const std::string &token,
                                            vec4_t &color) {
  const int64_t colorValue = std::stoll(token, nullptr, 16);
  const size_t channelCount = ((token.size() - 1) >> 1) + 1;
  const size_t maxShift = 8 * channelCount;

  for (size_t i = 0; i < channelCount; i++) {
    color[i] = (colorValue >> (maxShift - 8 * (i + 1))) & 0xff;
  }
}

void ColorParser::normalizeColorIfRequired(vec4_t &v) {
  float max = 0.f;
  for (auto i = 0; i < 3; i++) {
    max = std::max(v[i], max);
  }

  // non-normalized color
  if (max > 1.0f) {
    for (auto i = 0; i < 3; i++) {
      v[i] /= 255.f;
    }
  }

  // handle alpha separately
  if (v[3] > 1.0) {
    v[3] /= 255.f;
  }
}

} // namespace ETJump
