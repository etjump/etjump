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

#include "etj_cgaz_v2.h"
#include "cg_local.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_utilities.h"

namespace ETJump {
inline constexpr float CGAZ_FOV_MIN = 1.0f;
inline constexpr float CGAZ_FOV_MAX = 179.0f;

CGazV2::CGazV2(const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : cvarUpdate(cvarUpdate) {

  // CGaz 1
  cgame.utils.colorParser->parseColorString(etj_CGaz1Color1.string,
                                            cgaz1.colors[0]);
  cgame.utils.colorParser->parseColorString(etj_CGaz1Color2.string,
                                            cgaz1.colors[1]);
  cgame.utils.colorParser->parseColorString(etj_CGaz1Color3.string,
                                            cgaz1.colors[2]);
  cgame.utils.colorParser->parseColorString(etj_CGaz1Color4.string,
                                            cgaz1.colors[3]);

  cgame.utils.colorParser->parseColorString(etj_CGaz1MidlineColor.string,
                                            cgaz1.midlineColor);

  // CGaz 2
  cgame.utils.colorParser->parseColorString(etj_CGaz2Color1.string,
                                            cgaz2.colors[0]);
  cgame.utils.colorParser->parseColorString(etj_CGaz2Color2.string,
                                            cgaz2.colors[1]);

  setThickness(&etj_CGaz2Thickness1);
  setThickness(&etj_CGaz2Thickness2);
  setDefaultInput();
  startListeners();
}

CGazV2::~CGazV2() {
  cvarUpdate->unsubscribe(&etj_CGaz1Color1);
  cvarUpdate->unsubscribe(&etj_CGaz1Color2);
  cvarUpdate->unsubscribe(&etj_CGaz1Color3);
  cvarUpdate->unsubscribe(&etj_CGaz1Color4);
  cvarUpdate->unsubscribe(&etj_CGaz1MidlineColor);

  cvarUpdate->unsubscribe(&etj_CGaz2Color1);
  cvarUpdate->unsubscribe(&etj_CGaz2Color2);
  cvarUpdate->unsubscribe(&etj_CGaz2Thickness1);
  cvarUpdate->unsubscribe(&etj_CGaz2Thickness2);
}

void CGazV2::startListeners() {
  // CGaz 1
  cvarUpdate->subscribe(&etj_CGaz1Color1, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, cgaz1.colors[0]);
  });

  cvarUpdate->subscribe(&etj_CGaz1Color2, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, cgaz1.colors[1]);
  });

  cvarUpdate->subscribe(&etj_CGaz1Color3, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, cgaz1.colors[2]);
  });

  cvarUpdate->subscribe(&etj_CGaz1Color4, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, cgaz1.colors[3]);
  });

  cvarUpdate->subscribe(&etj_CGaz1MidlineColor, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, cgaz1.midlineColor);
  });

  // CGaz 2
  cvarUpdate->subscribe(&etj_CGaz2Color1, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, cgaz2.colors[0]);
  });

  cvarUpdate->subscribe(&etj_CGaz2Color2, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, cgaz2.colors[1]);
  });

  cvarUpdate->subscribe(&etj_CGaz2Thickness1,
                        [this](const vmCvar_t *cvar) { setThickness(cvar); });

  cvarUpdate->subscribe(&etj_CGaz2Thickness2,
                        [this](const vmCvar_t *cvar) { setThickness(cvar); });
}

void CGazV2::setThickness(const vmCvar_t *cvar) {
  if (cvar == &etj_CGaz2Thickness1) {
    cgaz2.thickness[0] = std::clamp(cvar->value, 0.5f, 100.0f);
  } else if (cvar == &etj_CGaz2Thickness2) {
    cgaz2.thickness[1] = std::clamp(cvar->value, 0.5f, 100.0f);
  }
}

void CGazV2::setDefaultInput() {
  defaultInput.set(PmoveUtilsV2::PmoveDefaultInput::FORWARD);
  defaultInput.set(PmoveUtilsV2::PmoveDefaultInput::SPRINT);
}

void CGazV2::updateCGazState(const float wishspeed, const float accel,
                             const float slickGravity) {
  assert(slickGravity >= 0);
  assert(slickGravity == 0 || pml.groundTrace.surfaceFlags & SURF_SLICK ||
         pm.ps->pm_flags & PMF_TIME_KNOCKBACK);

  s.gSquared = std::pow(slickGravity, 2.0f);
  s.vSquared = VectorLengthSquared2(pml.previous_velocity);
  s.vfSquared = VectorLengthSquared2(pm.ps->velocity);
  s.wishspeed = wishspeed;

  s.a = accel * s.wishspeed * PmoveUtilsV2::PM_FRAMETIME;
  s.aSquared = std::pow(s.a, 2.0f);

  if (!(etj_CGazTrueness.integer &
        static_cast<int32_t>(CGazTrueness::GROUND)) ||
      s.vSquared - s.vfSquared >= (2 * s.a * s.wishspeed) - s.aSquared) {
    s.vSquared = s.vfSquared;
  }

  s.v = std::sqrt(s.vSquared);
  s.vf = std::sqrt(s.vfSquared);

  assert(s.a * PmoveUtilsV2::PM_FRAMETIME <= 1);

  s.velAngle = std::atan2(pm.ps->velocity[1], pm.ps->velocity[0]);
}

void CGazV2::updateCGaz1() {
  cgaz1.minAngle = updateMinAngle();
  cgaz1.optAngle = updateOptAngle();
  cgaz1.maxCosAngle = updateMaxCosAngle(cgaz1.optAngle);
  cgaz1.maxAngle = updateMaxAngle(cgaz1.maxCosAngle);

  assert(cgaz1.minAngle <= cgaz1.optAngle);
  assert(cgaz1.optAngle <= cgaz1.maxCosAngle);
  assert(cgaz1.maxCosAngle <= cgaz1.maxAngle);

  // no need to update this unless we're drawing midline
  if (etj_CGaz1DrawMidLine.integer) {
    cgaz1.midlineStart =
        cgaz1.optAngle + ((cgaz1.maxCosAngle - cgaz1.optAngle) / 2);
    cgaz1.midLineEnd =
        cgaz1.midlineStart + (((cgaz1.optAngle - cgaz1.minAngle) +
                               (cgaz1.maxAngle - cgaz1.maxCosAngle)) /
                              2);
  }

  cgaz1.yaw = std::atan2(s.wishvel[1], s.wishvel[0]) - s.velAngle;

  cgaz1.y =
      std::clamp(etj_CGazY.value, 0.0f, static_cast<float>(SCREEN_HEIGHT));
  cgaz1.h =
      std::clamp(etj_CGazHeight.value, 0.0f, static_cast<float>(SCREEN_HEIGHT));

  cgaz1.fov = etj_CGazFov.value > 0
                  ? std::clamp(etj_CGazFov.value, CGAZ_FOV_MIN, CGAZ_FOV_MAX)
                  : cg.refdef.fov_x;

  // TODO: A/B testing, remove
  cgaz1.y += cgaz1.h + 1;
}

void CGazV2::updateCGaz2() {
  cgaz2.velAngle = AngleNormalize180(pm.ps->viewangles[YAW] -
                                     AngleNormalize180(RAD2DEG(s.velAngle)));
  cgaz2.velAngle = DEG2RAD(cgaz2.velAngle);
  cgaz2.velSize = etj_CGaz2FixedSpeed.value > 0
                      ? etj_CGaz2FixedSpeed.value / 5.0f
                      : std::min(s.vf / 5.0f, SCREEN_HEIGHT / 2.0f);

  cgaz2.y =
      std::clamp(etj_CGaz2Y.value, 0.0f, static_cast<float>(SCREEN_HEIGHT));

  cgaz2.highRes = etj_CGaz2HighRes.integer;
  cgaz2.drawSides = s.vf > s.wishspeed;

  // TODO: A/B testing, remove
  cgaz2.y -= 10;
}

float CGazV2::updateMinAngle() const {
#ifndef NDEBUG
  if (s.a == 0) {
    assert(s.vSquared - s.vfSquared == (2 * s.a * s.wishspeed) - s.aSquared);
  } else {
    assert(s.vSquared - s.vfSquared < (2 * s.a * s.wishspeed) - s.aSquared);
  }
#endif

  const float numSquared =
      std::pow(s.wishspeed, 2.0f) - s.vSquared + s.vfSquared + s.gSquared;
  assert(numSquared > 0);

  const float num = std::sqrt(numSquared);
  return num >= s.vf ? 0 : std::acos(num / s.vf);
}

float CGazV2::updateOptAngle() const {
  const float num = s.wishspeed - s.a;
  return num >= s.vf ? 0 : std::acos(num / s.vf);
}

// This needs to take into account some edge cases with low speeds on slick:
//
// - player takes fall damage and enters slick state, while holding movement
//   inputs (immediate acceleration after velocity reset)
// - player starts accelerating on slick with really low velocity
//   (e.g. with '+strafe')
//
// In these cases, the contribution from gravity to the total velocity vector
// may be higher than the contribution from player's horizontal velocity,
// which breaks down the math here, as the size of the optimal acceleration
// angle would exceed 180 degrees.
// When this happens, we simply return Pi as the angle, as there's no
// meaningful way to represent the angle on screen.
float CGazV2::updateMaxCosAngle(const float angleOpt) const {
  const float vXYSquared = s.vSquared - s.gSquared;

  // gravity has higher contribution - no meaningful angle
  if (vXYSquared < 0) {
    return M_PIf;
  }

  const float num = std::sqrt(vXYSquared) - s.vf;
  float angleMaxCos = 0.0f;

  if (num < s.a) {
    // gravity has higher contribution - no meaningful angle
    if (num <= -s.a) {
      angleMaxCos = M_PIf;
    } else {
      angleMaxCos = std::acos(num / s.a);
    }
  }

  if (angleMaxCos < angleOpt) {
    assert((s.v * s.vf) - s.vfSquared >= (s.a * s.wishspeed) - s.aSquared);
    angleMaxCos = angleOpt;
  }

  return angleMaxCos;
}

float CGazV2::updateMaxAngle(const float angleMaxCos) const {
#ifndef NDEBUG
  if (s.a == 0) {
    assert(s.vSquared - s.vfSquared == (2 * s.a * s.wishspeed) - s.aSquared);
  } else {
    assert(s.vSquared - s.vfSquared < (2 * s.a * s.wishspeed) - s.aSquared);
  }
#endif

  const float num = s.vSquared - s.vfSquared - s.aSquared - s.gSquared;
  const float den = 2 * s.a * s.vf;

  if (num >= den) {
    return 0;
  }

  if (-num >= den) {
    return M_PIf;
  }

  float angleMax = std::acos(num / den);

  if (angleMax < angleMaxCos) {
    assert(s.a == 0);
    angleMax = angleMaxCos;
  }

  return angleMax;
}

bool CGazV2::beforeRender() {
  ps = cg.predictedPlayerState;

  if (canSkipDraw()) {
    return false;
  }

  pm.ps = &ps;
  pm.pmext = &pmext;
  PmoveUtilsV2::setupPmove(pm);

  PmoveUtilsV2::PmoveSingleResult result =
      PmoveUtilsV2::pmoveSingle(pm, pml, defaultInput);

  switch (result) {
    case PmoveUtilsV2::PmoveSingleResult::WALK:
      walkMove();
      break;
    case PmoveUtilsV2::PmoveSingleResult::AIR:
      airMove();
      break;
    default:
      return false;
  }

  // TODO: drawsnap

  if (etj_drawCGaz.integer & 1) {
    updateCGaz1();
  }

  if (etj_drawCGaz.integer & 2) {
    updateCGaz2();
  }

  return true;
}

void CGazV2::render() const {
  if (etj_drawCGaz.integer & 1) {
    // TODO: drawsnap

    // no accel zone
    CG_FillAngleYaw(-cgaz1.minAngle, +cgaz1.minAngle, cgaz1.yaw, cgaz1.y,
                    cgaz1.h, cgaz1.fov, cgaz1.colors[0]);

    // partial accel zone
    CG_FillAngleYaw(+cgaz1.minAngle, +cgaz1.optAngle, cgaz1.yaw, cgaz1.y,
                    cgaz1.h, cgaz1.fov, cgaz1.colors[1]);
    CG_FillAngleYaw(-cgaz1.optAngle, -cgaz1.minAngle, cgaz1.yaw, cgaz1.y,
                    cgaz1.h, cgaz1.fov, cgaz1.colors[1]);

    // full accel zone
    CG_FillAngleYaw(+cgaz1.optAngle, +cgaz1.maxCosAngle, cgaz1.yaw, cgaz1.y,
                    cgaz1.h, cgaz1.fov, cgaz1.colors[2]);
    CG_FillAngleYaw(-cgaz1.maxCosAngle, -cgaz1.optAngle, cgaz1.yaw, cgaz1.y,
                    cgaz1.h, cgaz1.fov, cgaz1.colors[2]);

    // max angle
    CG_FillAngleYaw(+cgaz1.maxCosAngle, +cgaz1.maxAngle, cgaz1.yaw, cgaz1.y,
                    cgaz1.h, cgaz1.fov, cgaz1.colors[3]);
    CG_FillAngleYaw(-cgaz1.maxAngle, -cgaz1.maxCosAngle, cgaz1.yaw, cgaz1.y,
                    cgaz1.h, cgaz1.fov, cgaz1.colors[3]);

    if (etj_CGaz1DrawMidLine.integer) {
      CG_FillAngleYaw(+cgaz1.midlineStart, +cgaz1.midLineEnd, cgaz1.yaw,
                      cgaz1.y, cgaz1.h, cgaz1.fov, cgaz1.midlineColor);
      CG_FillAngleYaw(-cgaz1.midlineStart, -cgaz1.midLineEnd, cgaz1.yaw,
                      cgaz1.y, cgaz1.h, cgaz1.fov, cgaz1.midlineColor);
    }
  }

  if (etj_drawCGaz.integer & 2) {
    float x = SCREEN_CENTER_X;

    if (etj_stretchCgaz.integer) {
      ETJump_EnableWidthScale(false);
      x -= SCREEN_OFFSET_X;
    }

    if (pm.cmd.forwardmove || pm.cmd.rightmove) {
      float mult = 1.0f;

      if (etj_CGaz2WishDirFixedSpeed.value > 0) {
        constexpr float wishDirScale = 2.0f * 5.0f * CMDSCALE_DEFAULT;
        mult = etj_CGaz2WishDirFixedSpeed.value / wishDirScale;
      }

      if (etj_CGaz2WishDirUniformLength.integer && pm.cmd.rightmove &&
          pm.cmd.forwardmove) {
        mult /= M_SQRT2;
      }

      const float fmove = mult * static_cast<float>(pm.cmd.forwardmove);
      const float smove = mult * static_cast<float>(pm.cmd.rightmove);

      if (cgaz2.highRes) {
        drawLineWu(x, cgaz2.y, x + smove, cgaz2.y - fmove, cgaz2.thickness[1],
                   cgaz2.colors[1]);
      } else {
        drawLineDDA(x, cgaz2.y, x + smove, cgaz2.y - fmove, cgaz2.colors[1]);
      }
    }

    // draw velocity direction if requested
    if (!etj_CGaz2NoVelocityDir.integer ||
        (!cgaz2.drawSides && etj_CGaz2NoVelocityDir.integer == 2)) {
      float dirSize = cgaz2.velSize;

      // prevent comically long velocity direction lines on fixed speeds
      if (!cgaz2.drawSides && etj_CGaz2FixedSpeed.value > 0) {
        dirSize = std::min(static_cast<float>(CMDSCALE_DEFAULT), dirSize);
      }

      const float velDirSin = dirSize * std::sin(cgaz2.velAngle);
      const float velDirCos = dirSize * std::cos(cgaz2.velAngle);

      if (cgaz2.highRes) {
        drawLineWu(x, cgaz2.y, x + velDirSin, cgaz2.y - velDirCos,
                   cgaz2.thickness[0], cgaz2.colors[0]);
      } else {
        drawLineDDA(x, cgaz2.y, x + velDirSin, cgaz2.y - velDirCos,
                    cgaz2.colors[0]);
      }
    }

    if (cgaz2.drawSides) {
      const float velAngleSinL =
          (cgaz2.velSize / 2) * std::sin(cgaz2.velAngle - cgaz1.optAngle);
      const float velAngleCosL =
          (cgaz2.velSize / 2) * std::cos(cgaz2.velAngle - cgaz1.optAngle);

      const float velAngleSinR =
          (cgaz2.velSize / 2) * std::sin(cgaz2.velAngle + cgaz1.optAngle);
      const float velAngleCosR =
          (cgaz2.velSize / 2) * std::cos(cgaz2.velAngle + cgaz1.optAngle);

      if (cgaz2.highRes) {
        drawLineWu(x, cgaz2.y, x + velAngleSinL, cgaz2.y - velAngleCosL,
                   cgaz2.thickness[0], cgaz2.colors[0]);
        drawLineWu(x, cgaz2.y, x + velAngleSinR, cgaz2.y - velAngleCosR,
                   cgaz2.thickness[0], cgaz2.colors[0]);
      } else {
        drawLineDDA(x, cgaz2.y, x + velAngleSinL, cgaz2.y - velAngleCosL,
                    cgaz2.colors[0]);
        drawLineDDA(x, cgaz2.y, x + velAngleSinR, cgaz2.y - velAngleCosR,
                    cgaz2.colors[0]);
      }
    }

    if (etj_stretchCgaz.integer) {
      ETJump_EnableWidthScale(true);
    }
  }
}

void CGazV2::walkMove() {
  if (pm.waterlevel > 2 &&
      DotProduct(pml.forward, pml.groundTrace.plane.normal) > 0) {
    return;
  }

  // don't let interpolated frames modify jump times & sprint consumption
  const bool isLerpFrame = pm.pmove_msec > cg.time - pm.cmd.serverTime;

  if (PmoveUtilsV2::checkJump(pm, pml, isLerpFrame)) {
    // jumped away
    if (pm.waterlevel <= 1) {
      airMove();
    }

    if (!isLerpFrame &&
        !(pm.cmd.serverTime - pm.pmext->jumpTime < JUMP_DELAY_TIME)) {
      pm.pmext->sprintTime -= 2500;

      if (pm.pmext->sprintTime < 0) {
        pm.pmext->sprintTime = 0;
      }

      if (pm.pmext->jumpDelayBug) {
        pm.pmext->jumpTime = pm.cmd.serverTime;
      }
    }

    if (!isLerpFrame && !pm.pmext->jumpDelayBug) {
      pm.pmext->jumpTime = pm.cmd.serverTime;
    }

    return;
  }

  friction();

  const float scale = PmoveUtilsV2::cmdScale(
      pm, pm.cmd,
      etj_CGazTrueness.integer & static_cast<int32_t>(CGazTrueness::UPMOVE));

  // project moves down to flat plane
  pml.forward[2] = 0;
  pml.right[2] = 0;

  // FIXME: no slopes :(
  // project the forward and right directions onto the ground plane
  // PM_ClipVelocity(pml.forward, pml.groundTrace.plane.normal, pml.forward,
  //                 OVERCLIP);
  // PM_ClipVelocity(pml.right, pml.groundTrace.plane.normal, pml.right,
  // OVERCLIP);

  VectorNormalize(pml.forward);
  VectorNormalize(pml.right);

  PmoveUtilsV2::updateWishvel(s.wishvel, pm, pml);
  float wishspeed = scale * VectorLength2(s.wishvel);

  // clamp the speed lower if prone
  if (pm.ps->eFlags & EF_PRONE) {
    if (wishspeed > static_cast<float>(pm.ps->speed) * pm_proneSpeedScale) {
      wishspeed = static_cast<float>(pm.ps->speed) * pm_proneSpeedScale;
    }
  } else if (pm.ps->pm_flags & PMF_DUCKED) {
    // clamp the speed lower if ducking
    if (wishspeed >
        static_cast<float>(pm.ps->speed) * pm.ps->crouchSpeedScale) {
      wishspeed = static_cast<float>(pm.ps->speed) * pm.ps->crouchSpeedScale;
    }
  }

  // clamp the speed lower if wading or walking on the bottom
  if (pm.waterlevel) {
    float waterScale = static_cast<float>(pm.waterlevel) / 3.0f;

    if (pm.watertype == CONTENTS_SLIME) {
      waterScale = 1.0f - ((1.0f - pm_slagSwimScale) * waterScale);
    } else {
      waterScale = 1.0f - ((1.0f - pm_waterSwimScale) * waterScale);
    }

    if (wishspeed > static_cast<float>(pm.ps->speed) * waterScale) {
      wishspeed = static_cast<float>(pm.ps->speed) * waterScale;
    }
  }

  // when a player gets hit, they temporarily lose
  // full control, which allows them to be moved a bit
  if ((pml.groundTrace.surfaceFlags & SURF_SLICK) ||
      pm.ps->pm_flags & PMF_TIME_KNOCKBACK) {
    accelerate(wishspeed, pm_airaccelerate, true);
  } else {
    accelerate(wishspeed, pm_accelerate, false);
  }
}

void CGazV2::airMove() {
  friction();

  const float scale = PmoveUtilsV2::cmdScale(
      pm, pm.cmd,
      etj_CGazTrueness.integer & static_cast<int32_t>(CGazTrueness::UPMOVE));

  // project moves down to flat plane
  pml.forward[2] = 0;
  pml.right[2] = 0;
  VectorNormalize(pml.forward);
  VectorNormalize(pml.right);

  PmoveUtilsV2::updateWishvel(s.wishvel, pm, pml);

  // not on ground, so little effect on velocity
  accelerate(scale * VectorLength2(s.wishvel), pm_airaccelerate, false);

  // FIXME: no slopes :(
  // we may have a ground plane that is very steep,
  // even though we don't have a groundentity
  // slide along the steep plane
  // if (pml.groundPlane) {
  //   PM_ClipVelocity(pm.ps->velocity, pml.groundTrace.plane.normal,
  //                   pm.ps->velocity, OVERCLIP);
  // }
}

void CGazV2::friction() const {
  vec3_t vec;

  const float speed = pml.walking ? VectorLength2(pm.ps->velocity)
                                  : VectorLength(pm.ps->velocity);

  if (speed == 0) {
    return;
  }

  // rain - #179 don't do this for PM_SPECTATOR/PM_NOCLIP, we always
  // want them to stop
  if (speed < 1 && pm.ps->pm_type != PM_SPECTATOR &&
      pm.ps->pm_type != PM_NOCLIP) {
    pm.ps->velocity[0] = 0;
    pm.ps->velocity[1] = 0; // allow sinking underwater
    // FIXME: still have z friction underwater?
    return;
  }

  float drop = 0;

  // apply ground friction
  if (pm.waterlevel <= 1) {
    if (pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK)) {
      // if getting knocked back, no friction
      if (!(pm.ps->pm_flags & PMF_TIME_KNOCKBACK)) {
        const float control = speed < pm_stopspeed ? pm_stopspeed : speed;
        drop += control * pm_friction * PmoveUtilsV2::PM_FRAMETIME;
      }
    }
  }

  // apply water friction even if just wading
  if (pm.waterlevel) {
    if (pm.watertype == CONTENTS_SLIME) {
      drop += speed * pm_slagfriction * static_cast<float>(pm.waterlevel) *
              PmoveUtilsV2::PM_FRAMETIME;
    } else {
      drop += speed * pm_waterfriction * static_cast<float>(pm.waterlevel) *
              PmoveUtilsV2::PM_FRAMETIME;
    }
  }

  // uncomment if we ever start drawing cgaz in spec
  // if (pm.ps->pm_type == PM_SPECTATOR) {
  //   drop += speed * pm_spectatorfriction * PmoveUtilsV2::PM_FRAMETIME;
  // }

  // scale the velocity
  float newspeed = speed - drop;

  if (newspeed < 0) {
    newspeed = 0;
  }

  newspeed /= speed;

  // uncomment if we ever start drawing cgaz in spec/noclip
  // rain - if we're barely moving and barely slowing down, we want to
  // help things along--we don't want to end up getting snapped back to
  // our previous speed
  // if (pm.ps->pm_type == PM_SPECTATOR || pm.ps->pm_type == PM_NOCLIP) {
  //   if (drop < 1.0f && speed < 3.0f) {
  //     newspeed = 0.0;
  //   }
  // }

  // rain - used VectorScale instead of multiplying by hand
  VectorScale(pm.ps->velocity, newspeed, pm.ps->velocity);
}

void CGazV2::accelerate(const float wishspeed, const float accel,
                        const bool slick) {
  const float slickGravity =
      slick ? static_cast<float>(pm.ps->gravity) * PmoveUtilsV2::PM_FRAMETIME
            : 0;

  updateCGazState(wishspeed, accel, slickGravity);
}

bool CGazV2::canSkipDraw() const {
  if (!etj_drawCGaz.integer) {
    return true;
  }

  if (VectorLengthSquared2(ps.velocity) == 0) {
    return true;
  }

  if (ps.persistant[PERS_TEAM] == TEAM_SPECTATOR || ps.pm_type == PM_NOCLIP ||
      ps.pm_type == PM_DEAD) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  if (cg.zoomedBinoc || BG_IsScopedWeapon(weapnumForClient())) {
    return true;
  }

  if (BG_PlayerMounted(ps.eFlags) || ps.weapon == WP_MOBILE_MG42_SET ||
      ps.weapon == WP_MORTAR_SET) {
    return true;
  }

  return false;
}
} // namespace ETJump
