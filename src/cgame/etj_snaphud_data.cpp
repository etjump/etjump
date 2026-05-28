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

#include "etj_snaphud_data.h"
#include "cg_local.h"

// wishspeed may be modified by 'target_scale_velocity' up to 3x
// (wishspeed * 3) / 125 * pm_accelerate = 84.48
inline constexpr float MAX_SNAP_ACCEL = 85.0f;

namespace ETJump {
SnaphudData::SnaphudData() {
  defaultInput.set(PmoveUtilsV2::PmoveDefaultInput::FORWARD);
  defaultInput.set(PmoveUtilsV2::PmoveDefaultInput::SIDE);
  defaultInput.set(PmoveUtilsV2::PmoveDefaultInput::SPRINT);
}

void SnaphudData::runFrame() {
  s.ps = cg.predictedPlayerState;
  s.pm.ps = &s.ps;
  s.pm.pmext = &s.pmext;
  PmoveUtilsV2::setupPmove(s.pm);

  s.result = PmoveUtilsV2::pmoveSingle(s.pm, s.pml, defaultInput);

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

const SnaphudData::State &SnaphudData::getState() const { return s; }

void SnaphudData::updateState(const float accel) {
  assert(accel > 0);
  s.a = accel;

  float step = std::round(s.a) - 0.5f;
  s.snapAngles.clear();

  while (step > 0.0f) {
    s.snapAngles.push_back(std::acos(step / s.a));
    s.snapAngles.push_back(std::asin(step / s.a));

    step -= 1.0f;
  }

  std::sort(s.snapAngles.begin(), s.snapAngles.end());
  // we might not have any valid angles on low speeds
  s.snapAngles.push_back(s.snapAngles.empty() ? M_PI_2f
                                              : s.snapAngles[0] + M_PI_2f);
}

void SnaphudData::walkMove() {
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

  const float scale = PmoveUtilsV2::cmdScale(
      s.pm, s.pm.cmd,
      etj_snapHUDTrueness.integer & static_cast<int32_t>(SnapTrueness::UPMOVE));

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
  if (etj_snapHUDTrueness.integer &
      static_cast<int32_t>(SnapTrueness::GROUND)) {
    if ((s.pml.groundTrace.surfaceFlags & SURF_SLICK) ||
        s.pm.ps->pm_flags & PMF_TIME_KNOCKBACK) {
      accelerate(wishspeed, pm_airaccelerate);
    } else {
      accelerate(wishspeed, pm_accelerate);
    }
  } else {
    accelerate(wishspeed, pm_airaccelerate);
  }
}

void SnaphudData::airMove() {
  const float scale = PmoveUtilsV2::cmdScale(
      s.pm, s.pm.cmd,
      etj_snapHUDTrueness.integer & static_cast<int32_t>(SnapTrueness::UPMOVE));

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

void SnaphudData::accelerate(const float wishspeed, const float accel) {
  const float a =
      std::min(accel * wishspeed * PmoveUtilsV2::PM_FRAMETIME, MAX_SNAP_ACCEL);

  if (s.a != a) {
    updateState(a);
  }
}
} // namespace ETJump
