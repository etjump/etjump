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

#include "etj_accel_color_v2.h"
#include "etj_cvar_parser.h"
#include "etj_irenderable.h"
#include "etj_pmove_utils_v2.h"

namespace ETJump {
class CvarUpdateHandler;
class ClientCommandsHandler;

class DrawSpeed2 : public IRenderable {
public:
  DrawSpeed2(const std::shared_ptr<CvarUpdateHandler> &cvarUpdate,
             const std::shared_ptr<ClientCommandsHandler> &consoleCommands);
  ~DrawSpeed2() override;

  bool beforeRender() override;
  void render() const override;

private:
  void startListeners();
  void resetMaxSpeed();
  void setAccelColorStyle(const vmCvar_t &cvar);
  void setSize(const vmCvar_t &cvar);
  void parseColor(const std::string &colorStr);

  void setupAccelColor(const PmoveUtilsV2::State &s, float speed,
                       const vec2_t accelVec);
  [[nodiscard]] std::string getSpeedString() const;

  static bool canSkipDraw();

  enum class Style {
    NONE = 0,
    SPEED = 1,                       // speed
    SPEED_MAX = 2,                   // speed max
    SPEED_MAX_COLOR = 3,             // speed ^zmax
    SPEED_MAX_PARENTHESIS = 4,       // speed (max)
    SPEED_MAX_COLOR_PARENTHESIS = 5, // speed ^z(max)
    SPEED_MAX_COLOR_BRACKETS = 6,    // speed ^z[max]
    SPEED_MAX_PIPE = 7,              // speed | max
    SPEED_TEXT = 8,                  // Speed: speed
    SPEED_TENS = 9,                  // "tens" only
  };

  enum class Align {
    CENTER = 0,
    LEFT = 1,
    RIGHT = 2,
  };

  std::string speedStr;
  float currentSpeed{};
  float maxSpeed{};

  AccelColorV2::Style accelColorStyle{};
  int32_t textStyle{};

  vec2_t lastSpeed{};
  vec2_t accelVec{};

  float x{};
  float y{};
  CvarValue::Size size{};
  vec4_t color{};

  pmtype_t lastPmType{};
  int32_t lastUpdateTime{};

  std::list<AccelColorV2::StoredSpeed> storedSpeeds;

  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
  std::shared_ptr<ClientCommandsHandler> consoleCommands;
};
} // namespace ETJump
