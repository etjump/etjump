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

#include "etj_accel_color.h"
#include "etj_pmove_utils.h"
#include "etj_cgaz.h"
#include "../game/etj_numeric_utilities.h"

namespace ETJump {
void AccelColor::setAccelColor(int &style, float &speed, float &alpha,
                               pmove_t *pm,
                               std::list<StoredSpeed> &storedSpeeds,
                               vec3_t &accel, vec4_t &color) {
  if (style == Style::Simple ||
      (style == Style::Advanced &&
       lowSpeedOnGround(speed, pm->ps->groundEntityNum))) {
    float avgAccel = calcAvgAccel(storedSpeeds);
    vec4_t currentAccelColor;
    Vector4Copy(colorGreen, currentAccelColor);

    if (avgAccel < 0) {
      Vector4Copy(colorRed, currentAccelColor);
      avgAccel = -avgAccel;
    }

    float frac = avgAccel / ACCEL_FOR_SOLID_COLOR;
    frac = std::min(frac, 1.f);

    LerpColor(colorWhite, currentAccelColor, color, frac);
  } else if (style == Style::Advanced) {
    calcAccelColor(pm, accel, color);
  }

  color[3] *= Numeric::clamp(0.0f, 1.0f, alpha);
}

void AccelColor::calcAccelColor(pmove_t *pm, vec3_t &accel, vec4_t &outColor) {
  vec4_t color;
  const playerState_t &ps = cg.predictedPlayerState;
  const int8_t ucmdScale = CMDSCALE_DEFAULT;
  const usercmd_t cmd = PmoveUtils::getUserCmd(ps, ucmdScale);

  float speedX = ps.velocity[0];
  float speedY = ps.velocity[1];

  const float scale = PmoveUtils::PM_SprintScale(&ps);

  const float accelAngle = RAD2DEG(std::atan2(-cmd.rightmove, cmd.forwardmove));
  const float accelAngleAlt =
      RAD2DEG(std::atan2(cmd.rightmove, cmd.forwardmove));

  // max acceleration possible per frame
  const float frameAccel = CGaz::getFrameAccel(ps, pm);
  const float gravityAccel =
      -std::round(static_cast<float>(ps.gravity) * pm->pmext->frametime);

  if (accel[0] != 0.0f || accel[1] != 0.0f) {
    // get opt angles on both sides of velocity vector
    const float optAngle = CGaz::getOptAngle(ps, pm, false);
    const float altOptAngle = CGaz::getOptAngle(ps, pm, true);

    // get accels for opt angle
    float optAccelX = std::roundf(
        frameAccel *
        static_cast<float>(std::cos(DEG2RAD(accelAngle + optAngle)) * scale));
    float optAccelY = std::roundf(
        frameAccel *
        static_cast<float>(std::sin(DEG2RAD(accelAngle + optAngle)) * scale));

    // get accels for alt opt angle
    float altOptAccelX =
        std::round(frameAccel *
                   static_cast<float>(
                       std::cos(DEG2RAD(accelAngleAlt + altOptAngle)) * scale));
    float altOptAccelY =
        std::round(frameAccel *
                   static_cast<float>(
                       std::sin(DEG2RAD(accelAngleAlt + altOptAngle)) * scale));

    vec3_t optAccel = {optAccelX, optAccelY, gravityAccel};
    vec3_t altOptAccel = {altOptAccelX, altOptAccelY, gravityAccel};
    vec3_t normal = {0, 0, 0};

    VectorCopy(pm->groundTrace.plane.normal, normal);

    if (pm->groundPlane) {
      PM_ClipVelocity(optAccel, normal, optAccel, OVERCLIP);
      PM_ClipVelocity(altOptAccel, normal, altOptAccel, OVERCLIP);
    }

    if ((normal)[0] != 0.0f) {
      optAccelX = std::roundf(optAccel[0]);
      altOptAccelX = std::roundf(altOptAccel[0]);
    }

    if ((normal)[1] != 0.0f) {
      optAccelX = std::roundf(optAccel[1]);
      altOptAccelX = std::roundf(altOptAccel[1]);
    }

    // find max accel possible between opt and altOpt angles
    float maxAccelX;
    float maxAccelY;
    bool isMovingX;

    if (std::abs(speedX) > std::abs(speedY)) {
      // we're advancing on x-axis
      isMovingX = true;

      if (std::abs(optAccelX) >= std::abs(altOptAccelX)) {
        maxAccelX = optAccelX;
        maxAccelY = optAccelY;
      } else {
        maxAccelX = altOptAccelX;
        maxAccelY = altOptAccelY;
      }
    } else {
      // we're advancing on y-axis
      isMovingX = false;

      if (std::abs(optAccelY) >= std::abs(altOptAccelY)) {
        maxAccelX = optAccelX;
        maxAccelY = optAccelY;
      } else {
        maxAccelX = altOptAccelX;
        maxAccelY = altOptAccelY;
      }
    }

    // generate the color based on the average normalized acceleration
    // interpolate between red and green based on
    // the average normalized acceleration
    float frac = 0.0f;
    float absDiffX, absDiffY;

    if (isMovingX) {
      if ((maxAccelX > 0 && accel[0] >= 0) ||
          (maxAccelX < 0 && accel[0] <= 0)) {
        absDiffX = std::abs(maxAccelX) - std::abs(accel[0] + maxAccelX) / 2.0f;
        absDiffY =
            std::abs(accel[1]) -
            std::abs(std::max(std::abs(optAccelY), std::abs(altOptAccelY)));

        frac = std::min(std::max(1.0f - absDiffX - absDiffY, 0.0f), 1.0f);
      }
    } else {
      if ((maxAccelY > 0 && accel[1] >= 0) ||
          (maxAccelY < 0 && accel[1] <= 0)) {
        absDiffY = std::abs(maxAccelY) - std::abs(accel[1] + maxAccelY) / 2.0f;
        absDiffX =
            std::abs(accel[0]) -
            std::abs(std::max(std::abs(optAccelX), std::abs(altOptAccelX)));

        frac = std::min(std::max(1.0f - absDiffY - absDiffX, 0.0f), 1.0f);
      }
    }

    LerpColor(colorRed, colorGreen, color, frac);
  } else {
    Vector4Copy(colorWhite, color);
  }

  // we want a solid color all the time, no dark tints
  if (color[0] != 0.0f && color[1] != 0.0f) { // if we have a mix of R & G
    size_t maxColorIndex = color[0] > color[1] ? 0 : 1;
    float maxShade = 1.0f; // min value to show per color
    float coef = maxShade / color[maxColorIndex];

    VectorScale(color, coef, color);
  }

  Vector4Copy(color, outColor);
}

float AccelColor::calcAvgAccel(std::list<StoredSpeed> &storedSpeeds) {
  if (storedSpeeds.size() < 2) { // need 2 speed points to compute acceleration
    return 0;
  }

  float totalSpeedDelta = 0;
  auto iter = storedSpeeds.begin();
  for (auto prevIter = iter++; iter != storedSpeeds.end(); prevIter = iter++) {
    totalSpeedDelta += iter->speed - prevIter->speed;
  }

  const auto timeDeltaMs =
      static_cast<float>(storedSpeeds.back().time - storedSpeeds.front().time);

  return totalSpeedDelta / (timeDeltaMs / 1000.f);
}

void AccelColor::popOldStoredSpeeds(std::list<StoredSpeed> &storedSpeeds) {
  while (!storedSpeeds.empty()) {
    auto &front = storedSpeeds.front();

    if (cg.time - front.time > ACCEL_COLOR_SMOOTHING_TIME ||
        cg.time < front.time) {
      storedSpeeds.pop_front();
    } else {
      break;
    }
  }
}

bool AccelColor::lowSpeedOnGround(float speed, int groundEntityNum) {
  return (speed <= MAX_GROUNDSTRAFE && groundEntityNum != ENTITYNUM_NONE);
}
} // namespace ETJump
