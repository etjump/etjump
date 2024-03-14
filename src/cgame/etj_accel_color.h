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

#include "cg_local.h"
#include <list>

namespace ETJump {
class AccelColor {
public:
  AccelColor() = default;
  ~AccelColor() = default;

  struct StoredSpeed {
    int time;
    float speed;
  };

  enum Style { Simple = 1, Advanced = 2 };

  static void popOldStoredSpeeds(std::list<StoredSpeed> &storedSpeeds);
  static void setAccelColor(int &style, float &speed, float &alpha, pmove_t *pm,
                            std::list<StoredSpeed> &storedSpeeds, vec3_t &accel,
                            vec4_t &color);
  static bool lowSpeedOnGround(float speed, int groundEntityNum);

private:
  static constexpr int ACCEL_COLOR_SMOOTHING_TIME = 250;
  static constexpr float ACCEL_FOR_SOLID_COLOR = 100;

  static float calcAvgAccel(std::list<StoredSpeed> &storedSpeeds);
  static void calcAccelColor(pmove_t *pm, vec3_t &accel, vec4_t &outColor);
};
} // namespace ETJump
