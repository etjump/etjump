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

#include "etj_cgaz_data.h"
#include "cg_local.h"
#include "etj_pmove_utils_v2.h"

namespace ETJump {
CGazData::CGazData() {
  defaultInput.set(PmoveUtilsV2::PmoveDefaultInput::FORWARD);
  defaultInput.set(PmoveUtilsV2::PmoveDefaultInput::SPRINT);
}

void CGazData::runFrame() {
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

const CGazData::State &CGazData::getState() const { return s; }

void CGazData::updateState(const float wishspeed, const float accel,
                           const float slickGravity) {
  assert(slickGravity >= 0);
  assert(slickGravity == 0 || s.pml.groundTrace.surfaceFlags & SURF_SLICK ||
         s.pm.ps->pm_flags & PMF_TIME_KNOCKBACK);

  s.gSquared = std::pow(slickGravity, 2.0f);
  s.vSquared = VectorLengthSquared2(s.pml.previous_velocity);
  s.vfSquared = VectorLengthSquared2(s.pm.ps->velocity);
  s.wishspeed = wishspeed;

  s.a = accel * wishspeed * PmoveUtilsV2::PM_FRAMETIME;
  s.aSquared = std::pow(s.a, 2.0f);

  if (!(etj_CGazTrueness.integer &
        static_cast<int32_t>(CGazTrueness::GROUND)) ||
      s.vSquared - s.vfSquared >= (2 * s.a * s.wishspeed) - s.aSquared) {
    s.vSquared = s.vfSquared;
  }

  s.v = std::sqrt(s.vSquared);
  s.vf = std::sqrt(s.vfSquared);

  // this is meant for default speed - if it breaks then,
  // it will break with other speeds as well
  assert(s.pm.ps->speed != G_SPEED || s.a * PmoveUtilsV2::PM_FRAMETIME <= 1);

  s.velAngle = std::atan2(s.pm.ps->velocity[1], s.pm.ps->velocity[0]);
}

void CGazData::walkMove() {
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

  const float scale = PmoveUtilsV2::cmdScale(
      s.pm, s.pm.cmd,
      etj_CGazTrueness.integer & static_cast<int32_t>(CGazTrueness::UPMOVE));

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
    accelerate(wishspeed, pm_airaccelerate, true);
  } else {
    accelerate(wishspeed, pm_accelerate, false);
  }
}

void CGazData::airMove() {
  friction();

  const float scale = PmoveUtilsV2::cmdScale(
      s.pm, s.pm.cmd,
      etj_CGazTrueness.integer & static_cast<int32_t>(CGazTrueness::UPMOVE));

  // project moves down to flat plane
  s.pml.forward[2] = 0;
  s.pml.right[2] = 0;
  VectorNormalize(s.pml.forward);
  VectorNormalize(s.pml.right);

  PmoveUtilsV2::updateWishvel(s.wishvel, s.pm, s.pml);

  // not on ground, so little effect on velocity
  accelerate(scale * VectorLength2(s.wishvel), pm_airaccelerate, false);

  // FIXME: no slopes :(
  // we may have a ground plane that is very steep,
  // even though we don't have a groundentity
  // slide along the steep plane
  // if (s.pml.groundPlane) {
  //   PM_ClipVelocity(s.pm.ps->velocity, s.pml.groundTrace.plane.normal,
  //                   s.pm.ps->velocity, OVERCLIP);
  // }
}

void CGazData::friction() const {
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

void CGazData::accelerate(const float wishspeed, const float accel,
                          const bool slick) {
  const float slickGravity =
      slick ? static_cast<float>(s.pm.ps->gravity) * PmoveUtilsV2::PM_FRAMETIME
            : 0;

  updateState(wishspeed, accel, slickGravity);
}
} // namespace ETJump
