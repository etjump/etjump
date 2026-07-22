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

#include <vector>

#include "etj_pmove_utils_v2.h"

namespace ETJump {
class SnaphudData {
public:
  SnaphudData();
  ~SnaphudData() = default;

  struct State {
    std::vector<float> snapAngles;
    float a;
    float wishspeed;
    vec2_t wishvel;

    PmoveUtilsV2::PmoveSingleResult result;

    playerState_t ps;
    pmove_t pm;
    pmoveExt_t pmext;
    pml_t pml;
  };

  void runFrame();
  [[nodiscard]] const State &getState() const;

  // angles should be radians
  bool inMainAccelZone(const vec2_t wishvel, float wishspeed, float velAngle,
                       float optAngle, const pmove_t &pm);

private:
  void updateState(float accel);

  void walkMove();
  void airMove();
  void accelerate(float wishspeed, float accel);

  enum class SnapTrueness { UPMOVE = 1, GROUND = 2 };

  State s{};
  EnumBitset<PmoveUtilsV2::PmoveDefaultInput> defaultInput;
};
} // namespace ETJump
