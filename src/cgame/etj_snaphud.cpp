/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
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

#include "etj_snaphud.h"
#include "etj_utilities.h"
#include "etj_pmove_utils.h"
#include "etj_cgaz.h"
#include "etj_cvar_update_handler.h"

// Snaphud implementation based on cgame_proxymod
// HUGE thanks to Jelvan1 for his snaphud code
// https://github.com/Jelvan1/cgame_proxymod/

namespace ETJump {
Snaphud::Snaphud() {
  parseColorString(etj_snapHUDColor1.string, snaphudColors[0]);
  parseColorString(etj_snapHUDColor2.string, snaphudColors[1]);

  // always parse the highlighting colors on init to make sure they are
  // initialized even in cases where client connects to server with
  // highlighting disabled
  parseColorString(etj_snapHUDHLColor1.string, snaphudColors[2]);
  parseColorString(etj_snapHUDHLColor2.string, snaphudColors[3]);

  startListeners();
}

void Snaphud::startListeners() {
  cvarUpdateHandler->subscribe(&etj_snapHUDColor1, [&](const vmCvar_t *cvar) {
    parseColorString(etj_snapHUDColor1.string, snaphudColors[0]);
  });
  cvarUpdateHandler->subscribe(&etj_snapHUDColor2, [&](const vmCvar_t *cvar) {
    parseColorString(etj_snapHUDColor2.string, snaphudColors[1]);
  });
  cvarUpdateHandler->subscribe(&etj_snapHUDHLColor1, [&](const vmCvar_t *cvar) {
    parseColorString(etj_snapHUDHLColor1.string, snaphudColors[2]);
  });
  cvarUpdateHandler->subscribe(&etj_snapHUDHLColor2, [&](const vmCvar_t *cvar) {
    parseColorString(etj_snapHUDHLColor2.string, snaphudColors[3]);
  });
}

void Snaphud::InitSnaphud(vec3_t wishvel, const int8_t uCmdScale) {
  // set default key combination if no user input
  if (!pm->cmd.forwardmove && !pm->cmd.rightmove) {
    usercmd_t cmd = pm->cmd;
    cmd.forwardmove = uCmdScale;
    cmd.rightmove = uCmdScale;

    // recalculate wishvel with defaulted forward/rightmove
    pmoveUtils->updateWishvel(wishvel, pm->pmext->forward, pm->pmext->right,
                              pm->pmext->up, cmd);
  }

  // set correct yaw based on strafe style/keys pressed
  const float tempYaw = RAD2SHORT(atan2f(wishvel[1], wishvel[0]));
  yaw = AngleNormalize65536(lroundf(tempYaw));
}

void Snaphud::UpdateSnapState(void) {
  snap.maxAccel = (unsigned char)(snap.a + 0.5f);
  unsigned char xnyAccel =
      (unsigned char)(snap.a / sqrtf(2.0f) + 0.5f); // xAccel and yAccel
                                                    // at 45deg ^ ^  ^

  // find the last shortangle in each snapzone which is smaller than
  // 45deg (= 8192) using
  //  /asin -> increasing angles
  //  \acos -> decreasing angles
  // and concatenate those 2 sorted arrays in the upperhalf of 'zones'
  // so we can merge them in the lower half afterwards ^^^^^^^^^ =>
  // s.maxAccel +
  for (unsigned char i = 0; i <= xnyAccel - 1; ++i) {
    snap.zones[snap.maxAccel + i] =
        16383 - (unsigned short)(RAD2SHORT(acosf((i + 0.5f) / snap.a)));
  }
  for (unsigned char i = xnyAccel; i <= snap.maxAccel - 1; ++i) {
    snap.zones[snap.maxAccel + (snap.maxAccel - 1) - (i - xnyAccel)] =
        (unsigned short)(RAD2SHORT(acosf((i + 0.5f) / snap.a)));
  }

  // merge 2 sorted arrays in the lowerhalf
  unsigned char bi = snap.maxAccel;                 // begin i
  unsigned char ei = snap.maxAccel + xnyAccel;      // end i
  unsigned char bj = snap.maxAccel + xnyAccel;      // begin j
  unsigned char ej = snap.maxAccel + snap.maxAccel; // end j
  unsigned char i = bi;
  unsigned char j = bj;
  unsigned char k = 0;
  unsigned char xAccel_ = snap.maxAccel - (j - bj);
  unsigned char yAccel_ = i - bi;
  float absAccel_;
  snap.minAbsAccel =
      (float)(2 * snap.maxAccel); // upperbound > sqrt(2).s.maxAccel
  snap.maxAbsAccel = 0;           // lowerbound
  while (i < ei && j < ej) {
    absAccel_ = sqrtf((float)(xAccel_ * xAccel_ + yAccel_ * yAccel_));

    if (absAccel_ < snap.minAbsAccel) {
      snap.minAbsAccel = absAccel_;
    }

    if (absAccel_ > snap.maxAbsAccel) {
      snap.maxAbsAccel = absAccel_;
    }

    snap.xAccel[k] = xAccel_;
    snap.yAccel[k] = yAccel_;
    snap.absAccel[k] = absAccel_;
    snap.xAccel[2 * snap.maxAccel - k] = yAccel_;
    snap.yAccel[2 * snap.maxAccel - k] = xAccel_;
    snap.absAccel[2 * snap.maxAccel - k] = absAccel_;

    if (snap.zones[i] < snap.zones[j]) {
      snap.zones[k++] = snap.zones[i++];
      yAccel_ = i - bi;
    } else {
      snap.zones[k++] = snap.zones[j++];
      xAccel_ = snap.maxAccel - (j - bj);
    }
  }
  // store remaining elements
  while (i < ei) {
    absAccel_ = sqrtf((float)(xAccel_ * xAccel_ + yAccel_ * yAccel_));

    if (absAccel_ < snap.minAbsAccel) {
      snap.minAbsAccel = absAccel_;
    }

    if (absAccel_ > snap.maxAbsAccel) {
      snap.maxAbsAccel = absAccel_;
    }

    snap.xAccel[k] = xAccel_;
    snap.yAccel[k] = yAccel_;
    snap.absAccel[k] = absAccel_;
    snap.xAccel[2 * snap.maxAccel - k] = yAccel_;
    snap.yAccel[2 * snap.maxAccel - k] = xAccel_;
    snap.absAccel[2 * snap.maxAccel - k] = absAccel_;
    snap.zones[k++] = snap.zones[i++];
    yAccel_ = i - bi;
  }
  // store remaining elements
  while (j < ej) {
    absAccel_ = sqrtf((float)(xAccel_ * xAccel_ + yAccel_ * yAccel_));

    if (absAccel_ < snap.minAbsAccel) {
      snap.minAbsAccel = absAccel_;
    }

    if (absAccel_ > snap.maxAbsAccel) {
      snap.maxAbsAccel = absAccel_;
    }

    snap.xAccel[k] = xAccel_;
    snap.yAccel[k] = yAccel_;
    snap.absAccel[k] = absAccel_;
    snap.xAccel[2 * snap.maxAccel - k] = yAccel_;
    snap.yAccel[2 * snap.maxAccel - k] = xAccel_;
    snap.absAccel[2 * snap.maxAccel - k] = absAccel_;
    snap.zones[k++] = snap.zones[j++];
    xAccel_ = snap.maxAccel - (j - bj);
  }

  // fill in the acceleration of the snapzone at 45deg since we only
  // searched for shortangles smaller than 45deg (= 8192)
  absAccel_ = sqrtf(2) * xnyAccel;

  if (absAccel_ < snap.minAbsAccel) {
    snap.minAbsAccel = absAccel_;
  }

  if (absAccel_ > snap.maxAbsAccel) {
    snap.maxAbsAccel = absAccel_;
  }

  snap.xAccel[k] = xnyAccel;
  snap.yAccel[k] = xnyAccel;
  snap.absAccel[k] = absAccel_;

  for (i = 0; i < snap.maxAccel; ++i) {
    snap.zones[snap.maxAccel + i] = 16383 - snap.zones[snap.maxAccel - 1 - i];
  }

  snap.zones[2 * snap.maxAccel] = snap.zones[0] + 16384;
}

void Snaphud::UpdateMaxSnapZones() {
  // calculate max number of snapzones in 1 quadrant
  // this needs to be dynamically calculated because
  // ps->speed can be modified by target_scale_velocity
  // default: 57 on ground, 7 in air (352 wishspeed)
  const int maxSnaphudZonesQ1 = static_cast<int>(std::roundf(snap.a) * 2 + 1);

  // need to add rounded xy clipped to normal gravity influence to frame accel
  // 11 snap zones for ~78deg slope, 9 zones for 60deg slope, etc.
  // const int maxSnaphudZonesQ1 =
  //    static_cast<int>(std::roundf(snap.a + gravityAccelxy) * 2 + 1);

  snap.zones.resize(maxSnaphudZonesQ1);
  snap.xAccel.resize(maxSnaphudZonesQ1);
  snap.yAccel.resize(maxSnaphudZonesQ1);
  snap.absAccel.resize(maxSnaphudZonesQ1);
}

bool Snaphud::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  pm = pmoveUtils->getPmove();

  // water and ladder movement are not important
  // since speed is capped anyway
  // check this only after we have a valid pmove
  if (pm->pmext->waterlevel > 1 || pm->pmext->ladder) {
    return false;
  }

  if (pmoveUtils->skipUpdate(lastUpdateTime)) {
    return true;
  }

  // show upmove influence?
  const float scale = etj_snapHUDTrueness.integer &
                              static_cast<int>(SnapTrueness::SNAP_JUMPCROUCH)
                          ? pm->pmext->scale
                          : pm->pmext->scaleAlt;

  // calculate wishspeed
  vec3_t wishvel;
  float wishspeed = pmoveUtils->getWishspeed(wishvel, scale, pm->pmext->forward,
                                             pm->pmext->right, pm->pmext->up);

  // set default wishspeed for drawing if no user input
  if (!pm->cmd.forwardmove && !pm->cmd.rightmove) {
    wishspeed = ps->speed * ps->sprintSpeedScale;
  }

  const int8_t uCmdScale = pmoveUtils->getUserCmdScale();
  InitSnaphud(wishvel, uCmdScale);

  // show true groundzones?
  const float accel =
      etj_snapHUDTrueness.integer & static_cast<int>(SnapTrueness::SNAP_GROUND)
          ? pm->pmext->accel
          : pm_airaccelerate;

  float a = accel * wishspeed * pm->pmext->frametime;

  // clamp the max value to match max scaling of target_scale_velocity
  // FIXME: magic number bad
  if (a > 85) {
    a = 85;
  }

  if (a != snap.a || a == 0.0f) {
    snap.a = a;
    UpdateMaxSnapZones();
    UpdateSnapState();
  }

  hudType = static_cast<HudType>(etj_drawSnapHUD.integer);

  switch (hudType) {
    case HudType::SNAP_EDGE:
      edgeThickness = std::clamp(etj_snapHUDEdgeThickness.integer, 0, 128);
      break;
    case HudType::SNAP_BORDER:
      borderOnly = true;
      borderThickness =
          std::clamp(etj_snapHUDBorderThickness.value, 0.1f,
                     std::min(etj_snapHUDHeight.value * 2, 10.0f));
      break;
    default:
      borderOnly = false;
      break;
  }

  PrepareDrawables();

  return true;
}

void Snaphud::PrepareDrawables() {
  isCurrentAlt = false;
  drawableSnaps.clear();

  for (int i = 0; i < 2 * snap.maxAccel; ++i) {
    for (int j = 0; j < 65536; j += 16384) {
      const int bSnap = snap.zones[i] + 1 + j;
      const int eSnap = snap.zones[i + 1] + j;
      const bool active = AngleNormalize65536(yaw - bSnap) <=
                          AngleNormalize65536(eSnap - bSnap);
      const bool isAlt = i % 2;
      isCurrentAlt |= active && isAlt;

      drawableSnaps.push_back(DrawableSnap{bSnap, eSnap, isAlt, active});
    }
  }
}

void Snaphud::render() const {
  float h = etj_snapHUDHeight.value;
  float y = 240 + etj_snapHUDOffsetY.value;

  float fov;
  if (!etj_snapHUDFov.value) {
    fov = cg.refdef.fov_x;
  } else {
    fov = std::clamp(etj_snapHUDFov.value, 1.0f, 179.0f);
  }

  for (const DrawableSnap &ds : drawableSnaps) {
    int8_t color = ds.alt ? 1 : 0;

    if (etj_snapHUDActiveIsPrimary.integer && isCurrentAlt) {
      color ^= 1;
    }

    if (etj_snapHUDHLActive.integer && ds.active) {
      color += 2;
    }

    if (hudType == HudType::SNAP_EDGE) {
      CG_FillAngleYaw(SHORT2RAD(ds.bSnap), SHORT2RAD(ds.bSnap + edgeThickness),
                      SHORT2RAD(yaw), y, h, fov, snaphudColors[color]);
      CG_FillAngleYaw(SHORT2RAD(ds.eSnap), SHORT2RAD(ds.eSnap - edgeThickness),
                      SHORT2RAD(yaw), y, h, fov, snaphudColors[color]);
    } else {
      CG_FillAngleYawExt(SHORT2RAD(ds.bSnap), SHORT2RAD(ds.eSnap),
                         SHORT2RAD(yaw), y, h, fov, snaphudColors[color],
                         borderOnly, borderThickness);
    }
  }
}

Snaphud::CurrentSnap Snaphud::getCurrentSnap(const playerState_t &ps,
                                             const pmove_t *pm,
                                             const bool upmoveTrueness) {
  static Snaphud s;
  CurrentSnap cs{};

  // get player yaw
  float yaw = ps.viewangles[YAW];

  const usercmd_t *cmd = pmoveUtils->getUserCmd();

  // determine whether strafestyle is "forwards"
  const bool forwards = CGaz::strafingForwards(ps, pm);

  // determine whether strafing to the right ("moving mouse rightwards")
  const bool rightStrafe =
      (forwards && cmd->rightmove > 0) ||
      (!forwards &&
       (cmd->rightmove < 0 || (cmd->forwardmove != 0 && cmd->rightmove == 0)));

  // get opt angle
  float opt = CGaz::getOptAngle(ps, pm, false);

  float frameAccel = pmoveUtils->getFrameAccel(upmoveTrueness);

  // clamp the max value to match max scaling of target_scale_velocity
  if (frameAccel > 85) {
    frameAccel = 85;
  }
  if (frameAccel != s.snap.a) {
    s.snap.a = frameAccel;
    s.UpdateMaxSnapZones();
    s.UpdateSnapState();
  }

  // early out if we have no snapzones yet, this can happen for a brief
  // moment when swapping teams since we're transitioning from
  // cg.snap->ps to cg.predictedPlayerstate
  if (s.snap.zones.size() == 0) {
    cs.snap = INVALID_SNAP_DIR;
    return cs;
  }

  // necessary 45 degrees shift to match snapzones
  if ((cmd->forwardmove != 0 && cmd->rightmove != 0) ||
      (cmd->forwardmove == 0 && cmd->rightmove == 0)) {
    yaw += 45;
    opt += 45;
  }

  // snapzones only cover (a bit more than) one fourth of all
  // viewangles, therefore crop yaw and opt to [0,90)
  yaw = std::fmod(AngleNormalize360(yaw), 90);
  opt = std::fmod(AngleNormalize360(opt), 90);

  // get number of snapzones
  auto snapCount = s.snap.zones.size() - 1;

  // get snapzone index which corresponds to the *next* snapzone
  // linear search is good enough here as snapCount is always relatively
  // small
  unsigned int i = 0;
  while (i < snapCount && opt >= SHORT2DEG(s.snap.zones[i])) {
    ++i;
  }

  // adjust snapzone index for rightStrafe
  if (rightStrafe) {
    i = (i == 0 ? snapCount : i - 1);
  }

  // get the snapzone
  const float snap = SHORT2DEG(s.snap.zones[i]);
  // snap now contains the yaw value corresponding to the start of the
  // next snapzone, or equivalently the end of the current snapzone

  cs.opt = opt;
  cs.yaw = yaw;
  cs.snap = snap;
  cs.rightStrafe = rightStrafe;
  return cs;
}

bool Snaphud::inMainAccelZone(const playerState_t &ps, const pmove_t *pm) {
  const CurrentSnap cs = getCurrentSnap(ps, pm, true);

  if (cs.snap == INVALID_SNAP_DIR) {
    return false;
  }

  const float yaw = cs.yaw;
  const float opt = cs.opt;
  const float snap = cs.snap;

  // return true if yaw is between opt angle and end of the current
  // snapzone also account for jumps at the boundary (e.g. 100 and 10
  // both have to be valid)
  if (cs.rightStrafe) {
    if (yaw < opt && yaw > snap) {
      return true;
    }
    // this is awkward because can not check for yaw >= 0 since
    // that is always true
    if (snap > 90 && (yaw > 90 - std::fmod(snap, 90) || yaw < opt)) {
      return true;
    }
  } else {
    if (yaw > opt && yaw < snap) {
      return true;
    }
    if (snap > 90 && yaw < std::fmod(snap, 90)) {
      return true;
    }
  }

  return false;
}

bool Snaphud::canSkipDraw() const {
  if (!etj_drawSnapHUD.integer) {
    return true;
  }

  if (ps->persistant[PERS_TEAM] == TEAM_SPECTATOR || ps->pm_type == PM_NOCLIP ||
      ps->pm_type == PM_DEAD) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  if (cg.zoomedBinoc || BG_IsScopedWeapon(weapnumForClient())) {
    return true;
  }

  if (BG_PlayerMounted(ps->eFlags) || ps->weapon == WP_MOBILE_MG42_SET ||
      ps->weapon == WP_MORTAR_SET) {
    return true;
  }

  return false;
}
} // namespace ETJump
