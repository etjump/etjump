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

#include "etj_accel_color_data.h"
#include "cg_local.h"

namespace ETJump {
inline constexpr int32_t ACCEL_COLOR_SMOOTHING_TIME = 250;

void AccelColorData::runFrame() {
  s.ps = cg.predictedPlayerState;
  s.pm.ps = &s.ps;
  s.pm.pmext = &s.pmext;
  PmoveUtilsV2::setupPmove(s.pm);

  s.result = PmoveUtilsV2::pmoveSingle(s.pm, s.pml, {});

  switch (s.result) {
    case PmoveUtilsV2::PmoveSingleResult::WALK:
      walkMove();
      break;
    case PmoveUtilsV2::PmoveSingleResult::AIR:
      airMove();
      break;
    default:
      break;
  }
}

const AccelColorData::State &AccelColorData::getState() const { return s; }

void AccelColorData::popOldStoredSpeeds(std::list<StoredSpeed> &storedSpeeds,
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

float AccelColorData::calcAvgAccel(const std::list<StoredSpeed> &storedSpeeds) {
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

bool AccelColorData::lowSpeedOnGround(const float speed,
                                      const int32_t groundEntityNum) {
  return speed <= MAX_GROUNDSTRAFE && groundEntityNum != ENTITYNUM_NONE;
}

void AccelColorData::calcAdvancedAccelColor(
    const pmove_t &pm, const pml_t &pml, const vec3_t accel,
    const float wishspeed, const vec2_t wishvel, const float velAngle,
    const float optAngle, vec4_t outColor) const {
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

  float optAccelX = std::round(s.a * std::cos(optAngleAbsolute));
  float optAccelY = std::round(s.a * std::sin(optAngleAbsolute));

  float altOptAccelX = std::round(s.a * std::cos(altOptAngleAbsolute));
  float altOptAccelY = std::round(s.a * std::sin(altOptAngleAbsolute));

  // we're on a slope, recalculate optimal acceleration
  // TODO: verify if this actually works correctly? I have no idea
  if (s.pml.groundPlane && (pml.groundTrace.plane.normal[0] != 0 ||
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
    if ((maxAccelX > 0 && accel[0] >= 0) || (maxAccelX < 0 && accel[0] <= 0)) {
      absDiffX = std::abs(maxAccelX) - (std::abs(accel[0] + maxAccelX) / 2.0f);
      absDiffY =
          std::abs(accel[1]) -
          std::abs(std::max(std::abs(optAccelY), std::abs(altOptAccelY)));

      frac = std::clamp(1.0f - absDiffX - absDiffY, 0.0f, 1.0f);
    }
  } else {
    if ((maxAccelY > 0 && accel[1] >= 0) || (maxAccelY < 0 && accel[1] <= 0)) {
      absDiffY = std::abs(maxAccelY) - (std::abs(accel[1] + maxAccelY) / 2.0f);
      absDiffX =
          std::abs(accel[0]) -
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

void AccelColorData::updateState(const float wishspeed, const float accel) {
  s.vfSquared = VectorLengthSquared2(s.pm.ps->velocity);
  s.wishspeed = wishspeed;

  s.a = accel * wishspeed * PmoveUtilsV2::PM_FRAMETIME;
  s.vf = std::sqrt(s.vfSquared);

  assert(s.a * PmoveUtilsV2::PM_FRAMETIME <= 1);

  s.velAngle = std::atan2(s.pm.ps->velocity[1], s.pm.ps->velocity[0]);

  s.optAngle = updateOptAngle(s);
}

float AccelColorData::updateOptAngle(const AccelColorData::State &s) {
  const float num = s.wishspeed - s.a;
  return num >= s.vf ? 0 : std::acos(num / s.vf);
}

void AccelColorData::walkMove() {
  if (s.pm.waterlevel > 2 &&
      DotProduct(s.pml.forward, s.pml.groundTrace.plane.normal) > 0) {
    return;
  }

  // don't let interpolated frames modify jump times & sprint consumption
  const bool isLerpFrame = s.pm.pmove_msec > cg.time - s.pm.cmd.serverTime;

  if (PmoveUtilsV2::checkJump(s.pm, s.pml, isLerpFrame)) {
    // jumped away
    if (s.pm.waterlevel <= 1) {
      airMove();
    }

    if (!isLerpFrame &&
        !(s.pm.cmd.serverTime - s.pm.pmext->jumpTime < JUMP_DELAY_TIME)) {
      s.pm.pmext->sprintTime -= 2500;

      if (s.pm.pmext->sprintTime < 0) {
        s.pm.pmext->sprintTime = 0;
      }

      if (s.pm.pmext->jumpDelayBug) {
        s.pm.pmext->jumpTime = s.pm.cmd.serverTime;
      }
    }

    if (!isLerpFrame && !s.pm.pmext->jumpDelayBug) {
      s.pm.pmext->jumpTime = s.pm.cmd.serverTime;
    }

    return;
  }

  friction();

  const float scale = PmoveUtilsV2::cmdScale(s.pm, s.pm.cmd, true);

  // project moves down to flat plane
  s.pml.forward[2] = 0;
  s.pml.right[2] = 0;

  // FIXME: no slopes :(
  // project the forward and right directions onto the ground plane
  // PM_ClipVelocity(s.pml.forward, s.pml.groundTrace.plane.normal,
  // s.pml.forward,
  //                 OVERCLIP);
  // PM_ClipVelocity(s.pml.right, s.pml.groundTrace.plane.normal, s.pml.right,
  //                 OVERCLIP);

  VectorNormalize(s.pml.forward);
  VectorNormalize(s.pml.right);

  PmoveUtilsV2::updateWishvel(s.wishvel, s.pm, s.pml);
  float wishspeed = scale * VectorLength2(s.wishvel);

  // clamp the speed lower if prone
  if (s.pm.ps->eFlags & EF_PRONE) {
    if (wishspeed > static_cast<float>(s.pm.ps->speed) * pm_proneSpeedScale) {
      wishspeed = static_cast<float>(s.pm.ps->speed) * pm_proneSpeedScale;
    }
  } else if (s.pm.ps->pm_flags & PMF_DUCKED) {
    // clamp the speed lower if ducking
    if (wishspeed >
        static_cast<float>(s.pm.ps->speed) * s.pm.ps->crouchSpeedScale) {
      wishspeed =
          static_cast<float>(s.pm.ps->speed) * s.pm.ps->crouchSpeedScale;
    }
  }

  // clamp the speed lower if wading or walking on the bottom
  if (s.pm.waterlevel) {
    float waterScale = static_cast<float>(s.pm.waterlevel) / 3.0f;

    if (s.pm.watertype == CONTENTS_SLIME) {
      waterScale = 1.0f - ((1.0f - pm_slagSwimScale) * waterScale);
    } else {
      waterScale = 1.0f - ((1.0f - pm_waterSwimScale) * waterScale);
    }

    if (wishspeed > static_cast<float>(s.pm.ps->speed) * waterScale) {
      wishspeed = static_cast<float>(s.pm.ps->speed) * waterScale;
    }
  }

  // when a player gets hit, they temporarily lose
  // full control, which allows them to be moved a bit
  if ((s.pml.groundTrace.surfaceFlags & SURF_SLICK) ||
      s.pm.ps->pm_flags & PMF_TIME_KNOCKBACK) {
    accelerate(wishspeed, pm_airaccelerate);
  } else {
    accelerate(wishspeed, pm_accelerate);
  }
}

void AccelColorData::airMove() {
  friction();

  const float scale = PmoveUtilsV2::cmdScale(s.pm, s.pm.cmd, true);

  // project moves down to flat plane
  s.pml.forward[2] = 0;
  s.pml.right[2] = 0;
  VectorNormalize(s.pml.forward);
  VectorNormalize(s.pml.right);

  PmoveUtilsV2::updateWishvel(s.wishvel, s.pm, s.pml);

  // not on ground, so little effect on velocity
  accelerate(scale * VectorLength2(s.wishvel), pm_airaccelerate);

  // FIXME: no slopes :(
  // we may have a ground plane that is very steep,
  // even though we don't have a groundentity
  // slide along the steep plane
  // if (s.pml.groundPlane) {
  //   PM_ClipVelocity(s.pm.ps->velocity, s.pml.groundTrace.plane.normal,
  //                   s.pm.ps->velocity, OVERCLIP);
  // }
}

void AccelColorData::friction() const {
  const float speed = s.pml.walking ? VectorLength2(s.pm.ps->velocity)
                                    : VectorLength(s.pm.ps->velocity);

  if (speed == 0) {
    return;
  }

  // rain - #179 don't do this for PM_SPECTATOR/PM_NOCLIP, we always
  // want them to stop
  if (speed < 1 && s.pm.ps->pm_type != PM_SPECTATOR &&
      s.pm.ps->pm_type != PM_NOCLIP) {
    s.pm.ps->velocity[0] = 0;
    s.pm.ps->velocity[1] = 0; // allow sinking underwater
    // FIXME: still have z friction underwater?
    return;
  }

  float drop = 0;

  // apply ground friction
  if (s.pm.waterlevel <= 1) {
    if (s.pml.walking && !(s.pml.groundTrace.surfaceFlags & SURF_SLICK)) {
      // if getting knocked back, no friction
      if (!(s.pm.ps->pm_flags & PMF_TIME_KNOCKBACK)) {
        const float control = speed < pm_stopspeed ? pm_stopspeed : speed;
        drop += control * pm_friction * PmoveUtilsV2::PM_FRAMETIME;
      }
    }
  }

  // apply water friction even if just wading
  if (s.pm.waterlevel) {
    if (s.pm.watertype == CONTENTS_SLIME) {
      drop += speed * pm_slagfriction * static_cast<float>(s.pm.waterlevel) *
              PmoveUtilsV2::PM_FRAMETIME;
    } else {
      drop += speed * pm_waterfriction * static_cast<float>(s.pm.waterlevel) *
              PmoveUtilsV2::PM_FRAMETIME;
    }
  }

  if (s.pm.ps->pm_type == PM_SPECTATOR) {
    drop += speed * pm_spectatorfriction * PmoveUtilsV2::PM_FRAMETIME;
  }

  // scale the velocity
  float newspeed = speed - drop;

  if (newspeed < 0) {
    newspeed = 0;
  }

  newspeed /= speed;

  // rain - if we're barely moving and barely slowing down, we want to
  // help things along--we don't want to end up getting snapped back to
  // our previous speed
  if (s.pm.ps->pm_type == PM_SPECTATOR || s.pm.ps->pm_type == PM_NOCLIP) {
    if (drop < 1.0f && speed < 3.0f) {
      newspeed = 0.0;
    }
  }

  // rain - used VectorScale instead of multiplying by hand
  VectorScale(s.pm.ps->velocity, newspeed, s.pm.ps->velocity);
}

void AccelColorData::accelerate(const float wishspeed, const float accel) {
  updateState(wishspeed, accel);
}
} // namespace ETJump
