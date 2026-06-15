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

#include <algorithm>

#include "etj_accel_color_v2.h"
#include "etj_local.h"
#include "etj_pmove_utils_v2.h"

#include "../ui/ui_shared.h"

namespace ETJump {
inline constexpr int32_t ACCEL_COLOR_SMOOTHING_TIME = 250;

void AccelColorV2::popOldStoredSpeeds(std::list<StoredSpeed> &storedSpeeds,
                                      int time) {
  while (!storedSpeeds.empty()) {
    auto &front = storedSpeeds.front();

    if (time - front.time > ACCEL_COLOR_SMOOTHING_TIME || time < front.time) {
      storedSpeeds.pop_front();
    } else {
      break;
    }
  }
}

float AccelColorV2::calcAvgAccel(const std::list<StoredSpeed> &storedSpeeds) {
  // need 2 speed points to compute acceleration
  if (storedSpeeds.size() < 2) {
    return 0;
  }

  float totalSpeedDelta = 0;
  auto it = storedSpeeds.begin();

  for (auto prevIter = it++; it != storedSpeeds.end(); prevIter = it++) {
    totalSpeedDelta += it->speed - prevIter->speed;
  }

  const auto timeDeltaMs =
      static_cast<float>(storedSpeeds.back().time - storedSpeeds.front().time);

  return totalSpeedDelta / (timeDeltaMs / 1000.f);
}

bool AccelColorV2::lowSpeedOnGround(const float speed,
                                    const int32_t groundEntityNum) {
  return speed <= MAX_GROUNDSTRAFE && groundEntityNum != ENTITYNUM_NONE;
}

void AccelColorV2::calcAdvancedAccelColor(
    const pmove_t &pm, const pml_t &pml, const vec3_t accelVec,
    const float wishspeed, const vec2_t wishvel, const float velAngle,
    const float optAngle, float accel, vec4_t outColor) {
  const bool forwards = PmoveUtilsV2::strafingForwards(pm, wishspeed, wishvel);
  const bool rightStrafe =
      (forwards && pm.cmd.rightmove > 0) ||
      (!forwards && (pm.cmd.rightmove < 0 ||
                     (pm.cmd.forwardmove != 0 && pm.cmd.rightmove == 0)));

  // get the optimal angles on both sides of the velocity vector
  float optAngleAbsolute =
      rightStrafe ? velAngle - optAngle : velAngle + optAngle;
  optAngleAbsolute = AngleNormalizePI(optAngleAbsolute);
  float altOptAngleAbsolute =
      rightStrafe ? velAngle + optAngle : velAngle - optAngle;
  altOptAngleAbsolute = AngleNormalizePI(altOptAngleAbsolute);

  float optAccelX = std::round(accel * std::cos(optAngleAbsolute));
  float optAccelY = std::round(accel * std::sin(optAngleAbsolute));

  float altOptAccelX = std::round(accel * std::cos(altOptAngleAbsolute));
  float altOptAccelY = std::round(accel * std::sin(altOptAngleAbsolute));

  // we're on a slope, recalculate optimal acceleration
  // TODO: verify if this actually works correctly? I have no idea
  if (pml.groundPlane && (pml.groundTrace.plane.normal[0] != 0 ||
                          pml.groundTrace.plane.normal[1] != 0)) {
    const float gAccel = -std::round(static_cast<float>(pm.ps->gravity) *
                                     PmoveUtilsV2::PM_FRAMETIME);
    vec3_t optAccel = {optAccelX, optAccelY, gAccel};
    vec3_t altOptAccel = {altOptAccelX, altOptAccelY, gAccel};

    PM_ClipVelocity(optAccel, pml.groundTrace.plane.normal, optAccel, OVERCLIP);
    PM_ClipVelocity(altOptAccel, pml.groundTrace.plane.normal, altOptAccel,
                    OVERCLIP);

    optAccelX = std::round(optAccel[0]);
    optAccelY = std::round(optAccel[1]);

    altOptAccelX = std::round(altOptAccel[0]);
    altOptAccelY = std::round(altOptAccel[1]);
  }

  // find the max acceleration possible between opt and alt opt angles
  const float velX = pm.ps->velocity[0];
  const float velY = pm.ps->velocity[1];
  const bool isMovingX = std::abs(velX) > std::abs(velY);
  float maxAccelX = 0.0f;
  float maxAccelY = 0.0f;

  if (isMovingX) {
    if (std::abs(optAccelX) >= std::abs(altOptAccelX)) {
      maxAccelX = optAccelX;
      maxAccelY = optAccelY;
    } else {
      maxAccelX = altOptAccelX;
      maxAccelY = altOptAccelY;
    }
  } else {
    if (std::abs(optAccelY) >= std::abs(altOptAccelY)) {
      maxAccelX = optAccelX;
      maxAccelY = optAccelY;
    } else {
      maxAccelX = altOptAccelX;
      maxAccelY = altOptAccelY;
    }
  }

  // generate color based on the average normalized acceleration,
  // interpolating between red and green
  float frac = 0.0f;
  float absDiffX = 0.0f;
  float absDiffY = 0.0f;

  if (isMovingX) {
    if ((maxAccelX > 0 && accelVec[0] >= 0) ||
        (maxAccelX < 0 && accelVec[0] <= 0)) {
      absDiffX =
          std::abs(maxAccelX) - (std::abs(accelVec[0] + maxAccelX) / 2.0f);
      absDiffY =
          std::abs(accelVec[1]) -
          std::abs(std::max(std::abs(optAccelY), std::abs(altOptAccelY)));

      frac = std::clamp(1.0f - absDiffX - absDiffY, 0.0f, 1.0f);
    }
  } else {
    if ((maxAccelY > 0 && accelVec[1] >= 0) ||
        (maxAccelY < 0 && accelVec[1] <= 0)) {
      absDiffY =
          std::abs(maxAccelY) - (std::abs(accelVec[1] + maxAccelY) / 2.0f);
      absDiffX =
          std::abs(accelVec[0]) -
          std::abs(std::max(std::abs(optAccelX), std::abs(altOptAccelX)));

      frac = std::clamp(1.0f - absDiffY - absDiffX, 0.0f, 1.0f);
    }
  }

  LerpColor(colorRed, colorGreen, outColor, frac);

  // we want a solid color all the time, no dark tints
  if (outColor[0] != 0.0f && outColor[1] != 0.0f) { // if we have a mix of R & G
    const size_t maxColorIndex = outColor[0] > outColor[1] ? 0 : 1;
    constexpr float maxShade = 1.0f; // min value to show per color
    const float coef = maxShade / outColor[maxColorIndex];

    VectorScale(outColor, coef, outColor);
  }
}
} // namespace ETJump
