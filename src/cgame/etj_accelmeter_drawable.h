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

#pragma once

#include "etj_irenderable.h"
#include "cg_local.h"
#include "etj_accel_color.h"
#include <list>

namespace ETJump {
class AccelMeter : public IRenderable {
  int textStyle{};
  float y{};
  float halfW{};
  float size{};
  int accelColorStyle{};
  std::vector<std::string> accelStr{};
  bool playing{};

  std::list<AccelColor::StoredSpeed> storedSpeeds;

  vec3_t accel{};
  vec4_t accelColor{};
  int lastUpdateTime{};
  vec3_t lastSpeed{};

  enum Alignment { Left = 1, Right = 2 };

  static void parseColor(const std::string &color, vec4_t &out);
  void setTextStyle();
  void setSize();
  void setAccelColorStyle();
  void startListeners();
  static bool canSkipDraw();

  pmove_t *pm{};
  playerState_t *ps = &cg.predictedPlayerState;

public:
  AccelMeter();
  ~AccelMeter() override = default;

  bool beforeRender() override;
  void render() const override;
};
} // namespace ETJump
