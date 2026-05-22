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

#include "etj_snaphud_v2.h"
#include "cg_local.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_utilities.h"

// wishspeed may be modified by 'target_scale_velocity' up to 3x
// (wishspeed * 3) / 125 * pm_accelerate = 84.48
inline constexpr float MAX_SNAP_ACCEL = 85.0f;

inline constexpr float SNAPHUD_MIN_FOV = 1.0f;
inline constexpr float SNAPHUD_MAX_FOV = 179.0f;

namespace ETJump {
SnaphudV2::SnaphudV2(const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : cvarUpdate(cvarUpdate) {

  cgame.utils.colorParser->parseColorString(etj_snapHUDColor1.string,
                                            snaphud.colors[0]);
  cgame.utils.colorParser->parseColorString(etj_snapHUDColor2.string,
                                            snaphud.colors[1]);

  cgame.utils.colorParser->parseColorString(etj_snapHUDHLColor1.string,
                                            snaphud.colors[2]);
  cgame.utils.colorParser->parseColorString(etj_snapHUDHLColor2.string,
                                            snaphud.colors[3]);
  setDefaultInput();
  startListeners();
}

SnaphudV2::~SnaphudV2() {
  cvarUpdate->unsubscribe(&etj_snapHUDColor1);
  cvarUpdate->unsubscribe(&etj_snapHUDColor2);
  cvarUpdate->unsubscribe(&etj_snapHUDHLColor1);
  cvarUpdate->unsubscribe(&etj_snapHUDHLColor2);
}

void SnaphudV2::startListeners() {
  cvarUpdate->subscribe(&etj_snapHUDColor1, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, snaphud.colors[0]);
  });

  cvarUpdate->subscribe(&etj_snapHUDColor2, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, snaphud.colors[1]);
  });

  cvarUpdate->subscribe(&etj_snapHUDHLColor1, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, snaphud.colors[2]);
  });

  cvarUpdate->subscribe(&etj_snapHUDHLColor2, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, snaphud.colors[3]);
  });
}

void SnaphudV2::setDefaultInput() {
  defaultInput.set(PmoveUtilsV2::PmoveDefaultInput::FORWARD);
  defaultInput.set(PmoveUtilsV2::PmoveDefaultInput::SIDE);
  defaultInput.set(PmoveUtilsV2::PmoveDefaultInput::SPRINT);
}

void SnaphudV2::updateSnapState() {
  assert(s.a > 0);

  float step = std::round(s.a) - 0.5f;
  s.snapAngles.clear();

  while (step > 0.0f) {
    s.snapAngles.push_back(std::acos(step / s.a));
    s.snapAngles.push_back(std::asin(step / s.a));

    step -= 1.0f;
  }

  // can happen at very low speeds
  if (s.snapAngles.empty()) {
    return;
  }

  std::sort(s.snapAngles.begin(), s.snapAngles.end());
  s.snapAngles.push_back(s.snapAngles[0] + M_PI_2f);
}

void SnaphudV2::updateSnaphud() {
  snaphud.yaw = std::atan2(s.wishvel[1], s.wishvel[0]);
  snaphud.y = SCREEN_CENTER_Y + std::clamp(etj_snapHUDOffsetY.value,
                                           -SCREEN_CENTER_Y, SCREEN_CENTER_Y);
  snaphud.h = std::clamp(etj_snapHUDHeight.value, 0.0f,
                         static_cast<float>(SCREEN_HEIGHT));
  snaphud.fov =
      etj_snapHUDFov.value > 0
          ? std::clamp(etj_snapHUDFov.value, SNAPHUD_MIN_FOV, SNAPHUD_MAX_FOV)
          : cg.refdef.fov_x;

  // TODO: A/B testing, remove
  snaphud.y += snaphud.h + 1;

  snaphud.style = static_cast<SnaphudStyle>(etj_drawSnapHUD.integer);

  switch (snaphud.style) {
    case SnaphudStyle::EDGE:
      // FIXME: the unit of this cvar no longer really makes sense,
      // but by doing SHORT2RAD here, it behaves the same as with old hud
      snaphud.edgeThickness =
          SHORT2RAD(std::clamp(etj_snapHUDEdgeThickness.integer, 0, 128));
      break;
    case SnaphudStyle::BORDER:
      snaphud.borderOnly = true;
      snaphud.borderThickness =
          std::clamp(etj_snapHUDBorderThickness.value, 0.1f,
                     std::min(etj_snapHUDHeight.value * 2, 10.0f));
      break;
    default:
      snaphud.borderOnly = false;
      break;
  }

  buildSnapZones();
}

void SnaphudV2::buildSnapZones() {
  snaphud.zones.clear();
  snaphud.isCurrentAlt = false;

  for (int32_t i = 0; i < s.snapAngles.size() - 1; i++) {
    for (int32_t q = 0; q < 4; q++) {
      const float offset = static_cast<float>(q) * M_PI_2f;
      const float start = s.snapAngles[i] + offset;
      const float end = s.snapAngles[i + 1] + offset;

      const bool active = AngleNormalizePI(snaphud.yaw - start) >= 0 &&
                          (AngleNormalizePI(snaphud.yaw - start) <=
                           AngleNormalizePI(end - start));
      const bool alt = i % 2;
      snaphud.isCurrentAlt |= active && alt;

      snaphud.zones.push_back({start, end, alt, active});
    }
  }
}

bool SnaphudV2::beforeRender() {
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

  // this will be true at the very beginning of the game, before the server
  // has ran a client think at least once, or with very low speeds
  // ('target_scale_velocity' with base scaling < 1)
  if (s.a == 0 || s.snapAngles.empty()) {
    return false;
  }

  updateSnaphud();

  return true;
}

void SnaphudV2::render() const {
  for (const auto &zone : snaphud.zones) {
    int8_t colorIndex = zone.alt ? 1 : 0;

    if (etj_snapHUDActiveIsPrimary.integer && snaphud.isCurrentAlt) {
      colorIndex ^= 1;
    }

    if (etj_snapHUDHLActive.integer && zone.active) {
      colorIndex += 2;
    }

    if (snaphud.style == SnaphudStyle::EDGE) {
      CG_FillAngleYaw(zone.start, zone.start + snaphud.edgeThickness,
                      snaphud.yaw, snaphud.y, snaphud.h, snaphud.fov,
                      snaphud.colors[colorIndex]);
      CG_FillAngleYaw(zone.end, zone.end - snaphud.edgeThickness, snaphud.yaw,
                      snaphud.y, snaphud.h, snaphud.fov,
                      snaphud.colors[colorIndex]);
    } else {
      CG_FillAngleYawExt(zone.start, zone.end, snaphud.yaw, snaphud.y,
                         snaphud.h, snaphud.fov, snaphud.colors[colorIndex],
                         snaphud.borderOnly, snaphud.borderThickness);
    }
  }
}

void SnaphudV2::walkMove() {
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

  const float scale = PmoveUtilsV2::cmdScale(
      pm, pm.cmd,
      etj_snapHUDTrueness.integer & static_cast<int32_t>(SnapTrueness::UPMOVE));

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
  if (etj_snapHUDTrueness.integer &
      static_cast<int32_t>(SnapTrueness::GROUND)) {
    if ((pml.groundTrace.surfaceFlags & SURF_SLICK) ||
        pm.ps->pm_flags & PMF_TIME_KNOCKBACK) {
      accelerate(wishspeed, pm_airaccelerate);
    } else {
      accelerate(wishspeed, pm_accelerate);
    }
  } else {
    accelerate(wishspeed, pm_airaccelerate);
  }
}

void SnaphudV2::airMove() {
  const float scale = PmoveUtilsV2::cmdScale(
      pm, pm.cmd,
      etj_snapHUDTrueness.integer & static_cast<int32_t>(SnapTrueness::UPMOVE));

  // project moves down to flat plane
  pml.forward[2] = 0;
  pml.right[2] = 0;
  VectorNormalize(pml.forward);
  VectorNormalize(pml.right);

  PmoveUtilsV2::updateWishvel(s.wishvel, pm, pml);

  // not on ground, so little effect on velocity
  accelerate(scale * VectorLength2(s.wishvel), pm_airaccelerate);

  // FIXME: no slopes :(
  // we may have a ground plane that is very steep,
  // even though we don't have a groundentity
  // slide along the steep plane
  // if (pml.groundPlane) {
  //   PM_ClipVelocity(pm.ps->velocity, pml.groundTrace.plane.normal,
  //                   pm.ps->velocity, OVERCLIP);
  // }
}

void SnaphudV2::accelerate(const float wishspeed, const float accel) {
  const float a =
      std::min(accel * wishspeed * PmoveUtilsV2::PM_FRAMETIME, MAX_SNAP_ACCEL);

  if (s.a != a) {
    s.a = a;
    updateSnapState();
  }
}

bool SnaphudV2::canSkipDraw() const {
  if (!etj_drawSnapHUD.integer) {
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
