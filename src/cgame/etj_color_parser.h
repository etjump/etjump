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

#include <unordered_map>
#include <regex>

#include "../game/q_shared.h"

namespace ETJump {
class ColorParser {
public:
  ColorParser() = default;
  ~ColorParser() = default;

  void parseColorString(const std::string &colorString, vec4_t &color);

private:
  void parseNamedColorString(const std::string &token, vec4_t &color);
  void parseRGBAValuedColorString(const std::string &colorString,
                                  vec4_t &color);
  void parseHexValuedColorString(const std::string &token, vec4_t &color);
  void normalizeColorIfRequired(vec4_t &v);

  std::unordered_map<std::string, const vec4_t *> validColorNames = {
      {"white", &colorWhite},       {"red", &colorRed},
      {"green", &colorGreen},       {"blue", &colorBlue},
      {"yellow", &colorYellow},     {"magenta", &colorMagenta},
      {"cyan", &colorCyan},         {"orange", &colorOrange},
      {"mdred", &colorMdRed},       {"mdgreen", &colorMdGreen},
      {"dkgreen", &colorDkGreen},   {"mdcyan", &colorMdCyan},
      {"mdyellow", &colorMdYellow}, {"mdorange", &colorMdOrange},
      {"mdblue", &colorMdBlue},     {"gray", &colorMdGrey},
      {"grey", &colorMdGrey},       {"ltgrey", &colorLtGrey},
      {"mdgrey", &colorMdGrey},     {"dkgrey", &colorDkGrey},
      {"black", &colorBlack},
  };

  // white, black, etc
  std::string alphaRegexStr = "^[a-z]+";
  // 255 0 0, 1.0 0 0
  std::string digitRegexStr = "^([-+]?[0-9]*\\.?[0-9]+\\s*)+";
  // 0xff0000
  std::string hexedRegexStr = "^0[x][a-f0-9]+";
  // #ff0000
  std::string hashdRegexStr = "^#[a-f0-9]+";

  std::basic_regex<char> alphaRegex = std::regex(alphaRegexStr);
  std::basic_regex<char> digitRegex = std::regex(digitRegexStr);
  std::basic_regex<char> hexedRegex = std::regex(hexedRegexStr);
  std::basic_regex<char> hashdRegex = std::regex(hashdRegexStr);
};
} // namespace ETJump
