/*
 * MIT License
 *
 * Copyright (c) 2024 ETJump team <zero@etjump.com>
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

#include <regex>
#include <unordered_map>
#include <string>
#include <sstream>
#include <memory>

#include "etj_utilities.h"
#include "etj_event_loop.h"
#include "../game/etj_string_utilities.h"
#include "cg_local.h"

namespace ETJump {
extern std::shared_ptr<EventLoop> eventLoop;
}

std::string ETJump::composeShader(const char *name, ShaderStage general,
                                  ShaderStages stages) {
  std::string shader;
  // shader name
  shader = shader + name + " {\n";
  // General Directives
  for (auto &dir : general) {
    shader = shader + dir + "\n";
  }
  // Stage Directives
  for (auto &stage : stages) {
    shader += "{\n";
    for (auto &dir : stage) {
      shader = shader + dir + "\n";
    }
    shader += "}\n";
  }
  shader += "}\n";

  return shader;
}

std::string ETJump::composeShader(const char *name, ShaderStages stages) {
  return composeShader(name, {""}, stages);
}

static std::unordered_map<std::string, vec4_t *> validColorNames = {
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

static void parseNamedColorString(const std::string &token, vec4_t &color) {
  auto validColor = validColorNames[token];
  if (validColor) {
    color[0] = (*validColor)[0];
    color[1] = (*validColor)[1];
    color[2] = (*validColor)[2];
  }
}

static void parseRGBAValuedColorString(const std::string &colorString,
                                       vec4_t &color) {
  std::istringstream tokenStream{colorString};
  std::string token;
  auto colorChannel = 0;
  while (tokenStream >> token) {
    if (colorChannel > 3) {
      break;
    }
    auto value = std::min(std::max(std::stof(token), 0.f), 255.f);
    color[colorChannel] = value;
    colorChannel++;
  }
}

static void parseHexValuedColorString(const std::string &token, vec4_t &color) {
  auto colorValue = std::stoll(token, nullptr, 16);
  auto channelCount = ((token.size() - 1) >> 1) + 1;
  auto maxShift = 8 * channelCount;
  for (unsigned long i = 0; i < channelCount; i++) {
    color[i] = (colorValue >> (maxShift - 8 * (i + 1))) & 0xff;
  }
}

static void normalizeColorIfRequired(vec4_t &v) {
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

const std::string alphaRegexStr{"^[a-z]+"}; // white, black, etc
const std::string digitRegexStr{
    "^([-+]?[0-9]*\\.?[0-9]+\\s*)+"};              // 255 0 0, 1.0 0 0
const std::string hexedRegexStr{"^0[x][a-f0-9]+"}; // 0xff0000
const std::string hashdRegexStr{"^#[a-f0-9]+"};    // #ff0000

const std::basic_regex<char> alphaRegex = std::regex(alphaRegexStr);
const std::basic_regex<char> digitRegex = std::regex(digitRegexStr);
const std::basic_regex<char> hexedRegex = std::regex(hexedRegexStr);
const std::basic_regex<char> hashdRegex = std::regex(hashdRegexStr);

void ETJump::parseColorString(const std::string &colorString, vec4_t &color) {
  Vector4Set(color, 0.0f, 0.0f, 0.0f, 1.0); // set defaults

  std::string token{ETJump::StringUtil::toLowerCase(ETJump::trim(colorString))};

  if (std::regex_match(token, alphaRegex)) {
    parseNamedColorString(token, color);
    return;
  } else if (std::regex_match(token, digitRegex)) {
    parseRGBAValuedColorString(token, color);
  } else if (std::regex_match(token, hexedRegex)) {
    parseHexValuedColorString(token.substr(2, 8), color);
  } else if (std::regex_match(token, hashdRegex)) {
    parseHexValuedColorString(token.substr(1, 8), color);
  }

  normalizeColorIfRequired(color);
}

#ifdef CGAMEDLL

int ETJump::setTimeout(std::function<void()> fun, int delay) {
  return eventLoop->schedule(fun, delay);
}

bool ETJump::clearTimeout(int handle) { return eventLoop->unschedule(handle); }

int ETJump::setInterval(std::function<void()> fun, int delay) {
  return eventLoop->schedulePersistent(fun, delay);
}

bool ETJump::clearInterval(int handle) { return eventLoop->unschedule(handle); }

int ETJump::setImmediate(std::function<void()> fun) {
  return eventLoop->schedule(fun, 0, TaskPriorities::Immediate);
}

bool ETJump::clearImmediate(int handle) {
  return eventLoop->unschedule(handle);
}

void ETJump::executeTimeout(int handle) { eventLoop->execute(handle); }

bool ETJump::configFileExists(const std::string &filename) {
  bool fileExists = true;
  int len;
  fileHandle_t f;
  std::string str = filename + ".cfg";

  len = trap_FS_FOpenFile(str.c_str(), &f, FS_READ);
  if (!f || len < 0) {
    // file not found
    fileExists = false;
  }

  trap_FS_FCloseFile(f);
  return fileExists;
}

void ETJump::execFile(const std::string &filename) {
  trap_SendConsoleCommand(va("exec \"%s.cfg\"\n", filename.c_str()));
}

bool ETJump::isPlaying(const int clientNum) {
  return (cgs.clientinfo[clientNum].team == TEAM_ALLIES ||
          cgs.clientinfo[clientNum].team == TEAM_AXIS);
}

#endif
