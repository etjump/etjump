/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
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

#include <list>

#include "etj_irenderable.h"
#include "etj_accel_color.h"
#include "cg_local.h"
#include "etj_cvar_parser.h"

namespace ETJump {
class DrawSpeed : public IRenderable {
  std::list<AccelColor::StoredSpeed> storedSpeeds;

  float maxSpeed{0};
  float currentSpeed{};
  std::string speedStr;
  float y{};
  float w{};
  CvarValue::Size size{};

  enum Alignment {
    Left = 1,
    Right = 2,
  };

  vec3_t lastSpeed{};
  vec3_t accel{};
  vec4_t speedColor{};

  const playerState_t *ps = &cg.predictedPlayerState;
  const pmove_t *pm{};
  int textStyle{};
  int lastUpdateTime{0};
  int accelColorStyle{};
  bool playing{};

  std::string getSpeedString() const;

  void resetMaxSpeed();
  void setTextStyle();
  void setAccelColorStyle();
  void setSize();

  void startListeners();
  static void parseColor(const std::string &color, vec4_t &out);
  static bool canSkipDraw();

public:
  DrawSpeed();
  ~DrawSpeed() override;

  void render() const override;
  bool beforeRender() override;
};
} // namespace ETJump
