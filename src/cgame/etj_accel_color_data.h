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

#include <list>

#include "etj_pmove_utils_v2.h"

namespace ETJump {
class AccelColorData {
public:
  struct StoredSpeed {
    int32_t time{};
    float speed{};
  };

  enum class Style {
    SIMPLE = 1,
    ADVANCED = 2,
  };

  struct State {
    float vfSquared; // velocity final squared (after friction)
    float vf;        // velocity final (after friction)
    float a;         // accel

    vec2_t wishvel;
    float wishspeed;
    float velAngle;
    float optAngle;

    PmoveUtilsV2::PmoveSingleResult result;

    playerState_t ps;
    pmove_t pm;
    pmoveExt_t pmext;
    pml_t pml;
  };

  void runFrame();
  [[nodiscard]] const AccelColorData::State &getState() const;

  void popOldStoredSpeeds(std::list<StoredSpeed> &storedSpeeds, int32_t time);
  float calcAvgAccel(const std::list<StoredSpeed> &storedSpeeds);
  void calcAdvancedAccelColor(const pmove_t &pm, const pml_t &pml,
                              const vec2_t accel, float wishspeed,
                              const vec2_t wishvel, float velAngle,
                              float optAngle, vec4_t outColor) const;
  bool lowSpeedOnGround(float speed, int32_t groundEntityNum);

private:
  void updateState(float wishspeed, float accel);
  float updateOptAngle(const AccelColorData::State &s);

  void walkMove();
  void airMove();
  void friction() const;
  void accelerate(float wishspeed, float accel);

  State s;
};
} // namespace ETJump
