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

// HUGE thanks to Jelvan1 for CGaz 1 code
// https://github.com/Jelvan1/cgame_proxymod/

#include "etj_cgaz.h"
#include "etj_snaphud.h"
#include "etj_utilities.h"
#include "etj_pmove_utils.h"
#include "../game/etj_numeric_utilities.h"
#include "etj_cvar_update_handler.h"

namespace ETJump {
CGaz::state_t state;
float CGaz::drawMin{};
float CGaz::drawOpt{};
float CGaz::drawMaxCos{};
float CGaz::drawMax{};
float CGaz::drawVel{};
float CGaz::yaw{};
float CGaz::drawSnap{};

CGaz::CGaz() {
  // CGaz 1
  parseColorString(etj_CGaz1Color1.string, CGaz1Colors[0]);
  parseColorString(etj_CGaz1Color2.string, CGaz1Colors[1]);
  parseColorString(etj_CGaz1Color3.string, CGaz1Colors[2]);
  parseColorString(etj_CGaz1Color4.string, CGaz1Colors[3]);

  // CGaz 2
  parseColorString(etj_CGaz2Color1.string, CGaz2Colors[0]);
  parseColorString(etj_CGaz2Color2.string, CGaz2Colors[1]);

  startListeners();
}

void CGaz::startListeners() {
  // CGaz 1
  cvarUpdateHandler->subscribe(&etj_CGaz1Color1, [&](const vmCvar_t *cvar) {
    parseColorString(etj_CGaz1Color1.string, CGaz1Colors[0]);
  });
  cvarUpdateHandler->subscribe(&etj_CGaz1Color2, [&](const vmCvar_t *cvar) {
    parseColorString(etj_CGaz1Color2.string, CGaz1Colors[1]);
  });
  cvarUpdateHandler->subscribe(&etj_CGaz1Color3, [&](const vmCvar_t *cvar) {
    parseColorString(etj_CGaz1Color3.string, CGaz1Colors[2]);
  });
  cvarUpdateHandler->subscribe(&etj_CGaz1Color4, [&](const vmCvar_t *cvar) {
    parseColorString(etj_CGaz1Color4.string, CGaz1Colors[3]);
  });

  // CGaz 2
  cvarUpdateHandler->subscribe(&etj_CGaz2Color1, [&](const vmCvar_t *cvar) {
    parseColorString(etj_CGaz2Color1.string, CGaz2Colors[0]);
  });
  cvarUpdateHandler->subscribe(&etj_CGaz2Color2, [&](const vmCvar_t *cvar) {
    parseColorString(etj_CGaz2Color2.string, CGaz2Colors[1]);
  });
}

void CGaz::UpdateCGaz1(vec3_t wishvel, int8_t uCmdScale, usercmd_t cmd) {
  // set default key combination if no user input
  if (!cmd.forwardmove && !cmd.rightmove) {
    cmd.forwardmove = uCmdScale;

    // recalculate wishvel with defaulted forwardmove
    PmoveUtils::PM_UpdateWishvel(wishvel, cmd, pm->pmext->forward,
                                 pm->pmext->right, pm->pmext->up, *ps);
  }

  yaw = atan2f(wishvel[1], wishvel[0]) - drawVel;
}

void CGaz::UpdateCGaz2() {
  drawVel = AngleNormalize180(ps->viewangles[YAW] -
                              AngleNormalize180(RAD2DEG(drawVel)));
  drawVel = DEG2RAD(drawVel);
}

void CGaz::UpdateDraw(float wishspeed, const playerState_t *ps,
                      const pmove_t *pm) {
  // this can happen when running > 125fps, set default wishspeed to
  // avoid div by 0 later
  if (wishspeed == 0) {
    wishspeed = static_cast<float>(ps->speed) * ps->sprintSpeedScale;
  }

  const float accel = pm->pmext->accel;

  state.gSquared = GetSlickGravity(ps, pm);
  state.vSquared = VectorLengthSquared2(pm->pmext->previous_velocity);
  state.vfSquared = VectorLengthSquared2(pm->pmext->velocity);
  state.wishspeed = wishspeed;
  state.a = accel * state.wishspeed * pm->pmext->frametime;
  state.aSquared = powf(state.a, 2);
  // show true ground zones?
  if (!(etj_CGazTrueness.integer &
        static_cast<int>(CGazTrueness::CGAZ_GROUND)) ||
      state.vSquared - state.vfSquared >=
          2 * state.a * state.wishspeed - state.aSquared) {
    state.vSquared = state.vfSquared;
  }

  state.v = sqrtf(state.vSquared);
  state.vf = sqrtf(state.vfSquared);

  drawMin = UpdateDrawMin(&state);
  drawOpt = UpdateDrawOpt(&state);
  drawMaxCos = UpdateDrawMaxCos(&state);
  drawMax = UpdateDrawMax(&state);
  drawVel = atan2f(pm->pmext->velocity[1], pm->pmext->velocity[0]);
}

float CGaz::UpdateDrawSnap(const playerState_t *ps, pmove_t *pm) {
  // don't highlight snapzone on very low velocities,
  // or if drawing isn't requested
  if (!etj_CGaz1DrawSnapZone.integer || !(etj_drawCGaz.integer & 1) ||
      state.vf < state.wishspeed) {
    return Snaphud::INVALID_SNAP_DIR;
  }

  const Snaphud::CurrentSnap cs = Snaphud::getCurrentSnap(
      *ps, pm,
      etj_CGazTrueness.integer &
          static_cast<int>(CGazTrueness::CGAZ_JUMPCROUCH));

  if (cs.snap == Snaphud::INVALID_SNAP_DIR) {
    return Snaphud::INVALID_SNAP_DIR;
  }

  float snap = cs.snap;

  // edge case, solution from snaphud code
  if (cs.rightStrafe && snap > 90.0f) {
    snap = 90.0f - std::fmod(snap, 90.0f);
  }

  // snaps are in absolute angles, calculate it relative to yaw
  const float viewOffset =
      AngleNormalize180(cs.yaw - static_cast<float>(RAD2DEG(yaw)));

  float snapInCgazAngles =
      std::fmod(!cs.rightStrafe ? snap - viewOffset : viewOffset - snap, 90.0f);

  if (snapInCgazAngles < 0.0f) {
    snapInCgazAngles += 90.0f;
  }

  return DEG2RAD(snapInCgazAngles);
}

float CGaz::UpdateDrawMin(state_t const *state) {
  float const num_squared = state->wishspeed * state->wishspeed -
                            state->vSquared + state->vfSquared +
                            state->gSquared;
  float const num = sqrtf(num_squared);
  return num >= state->vf ? 0 : acosf(num / state->vf);
}

float CGaz::UpdateDrawOpt(state_t const *state) {
  float const num = state->wishspeed - state->a;
  return num >= state->vf ? 0 : acosf(num / state->vf);
}

float CGaz::UpdateDrawMaxCos(state_t const *state) {
  float const num = sqrtf(state->vSquared - state->gSquared) - state->vf;
  float drawMaxCos = num >= state->a ? 0 : acosf(num / state->a);
  return drawMaxCos;
}

float CGaz::UpdateDrawMax(state_t const *state) {
  float const num =
      state->vSquared - state->vfSquared - state->aSquared - state->gSquared;
  float const den = 2 * state->a * state->vf;
  if (num >= den) {
    return 0;
  } else if (-num >= den) {
    return (float)M_PI;
  }
  float drawMax = acosf(num / den);
  return drawMax;
}

float CGaz::GetSlickGravity(const playerState_t *ps, const pmove_t *pm) {
  if ((pm->pmext->groundTrace.surfaceFlags & SURF_SLICK) ||
      (ps->pm_flags & PMF_TIME_KNOCKBACK)) {
    return powf(static_cast<float>(ps->gravity) * pm->pmext->frametime, 2);
  }

  return 0;
}

bool CGaz::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  const auto uCmdScale = static_cast<int8_t>(ps->stats[STAT_USERCMD_BUTTONS] &
                                                     (BUTTON_WALKING << 8)
                                                 ? CMDSCALE_WALK
                                                 : CMDSCALE_DEFAULT);
  const usercmd_t cmd = PmoveUtils::getUserCmd(*ps, uCmdScale);

  // get correct pmove state
  pm = PmoveUtils::getPmove(cmd);

  // water and ladder movement are not important
  // since speed is capped anyway
  // check this only after we have a valid pmove
  if (pm->pmext->waterlevel > 1 || pm->pmext->ladder) {
    return false;
  }

  if (PmoveUtils::skipUpdate(lastUpdateTime, pm, ps)) {
    return true;
  }

  // show upmove influence?
  const float scale =
      etj_CGazTrueness.integer & static_cast<int>(CGazTrueness::CGAZ_JUMPCROUCH)
          ? pm->pmext->scale
          : pm->pmext->scaleAlt;

  vec3_t wishvel;
  float wishspeed =
      PmoveUtils::PM_GetWishspeed(wishvel, scale, cmd, pm->pmext->forward,
                                  pm->pmext->right, pm->pmext->up, *ps, pm);

  // set default wishspeed for drawing if no user input
  if (!cmd.forwardmove && !cmd.rightmove) {
    wishspeed = static_cast<float>(ps->speed) * ps->sprintSpeedScale;
  }

  UpdateDraw(wishspeed, ps, pm);

  // cgaz1snapzone
  drawSnap = UpdateDrawSnap(ps, pm);

  if (etj_drawCGaz.integer & 1) {
    UpdateCGaz1(wishvel, uCmdScale, cmd);
  }
  if (etj_drawCGaz.integer & 2) {
    UpdateCGaz2();
  }

  return true;
}

void CGaz::render() const {
  // DeFRaG proxymod CGaz by Jelvan1
  if (etj_drawCGaz.integer & 1) {
    const auto y =
        static_cast<float>(etj_CGazY.integer > 0 ? etj_CGazY.integer % 480 : 0);
    const float h = etj_CGazHeight.value > 0 ? etj_CGazHeight.value : 0;

    float fov;
    if (etj_CGazFov.value == 0) {
      fov = cg.refdef.fov_x;
    } else {
      fov = Numeric::clamp(etj_CGazFov.value, 1, 179);
    }

    const float zone1 = drawMin;

    float zone2 = drawOpt;

    if (drawSnap != Snaphud::INVALID_SNAP_DIR) {
      // if snap < min angle, the accel zone fills the whole snapzone
      zone2 = drawSnap < drawMin ? drawMaxCos : drawSnap;
    }

    const float zone3 = std::max(zone2, drawMaxCos);

    const float zone4 =
        drawMax == 0 || drawMax == (float)M_PI || drawMax >= zone3 ? drawMax
                                                                   : zone3;

    // No accel zone
    CG_FillAngleYaw(-zone1, +zone1, yaw, y, h, fov, CGaz1Colors[0]);

    // Min angle
    CG_FillAngleYaw(+zone1, +zone2, yaw, y, h, fov, CGaz1Colors[1]);
    CG_FillAngleYaw(-zone2, -zone1, yaw, y, h, fov, CGaz1Colors[1]);

    // Accel/snap zone
    CG_FillAngleYaw(+zone2, +zone3, yaw, y, h, fov, CGaz1Colors[2]);
    CG_FillAngleYaw(-zone3, -zone2, yaw, y, h, fov, CGaz1Colors[2]);

    // Max angle
    CG_FillAngleYaw(+zone3, +zone4, yaw, y, h, fov, CGaz1Colors[3]);
    CG_FillAngleYaw(-zone4, -zone3, yaw, y, h, fov, CGaz1Colors[3]);
  }

  // Dzikie Weze's 2D-CGaz
  if (etj_drawCGaz.integer & 2) {
    const usercmd_t cmd = pm->cmd;
    float scx = SCREEN_CENTER_X - 0.5f; // -0.5 since thickness is 1px
    const float scy = SCREEN_CENTER_Y - 0.5f;

    if (etj_stretchCgaz.integer) {
      ETJump_EnableWidthScale(false);
      scx -= SCREEN_OFFSET_X;
    }

    // draw movement keys direction
    if (cmd.rightmove || cmd.forwardmove) {
      float mult = 1.0f;

      if (etj_CGaz2WishDirFixedSpeed.value > 0) {
        // scale to get same lengths as minline fixed speed
        constexpr float wishDirScale = 2.0f * 5.0f * 127.0f;
        mult = etj_CGaz2WishDirFixedSpeed.value / wishDirScale;
      }

      if (etj_CGaz2WishDirUniformLength.integer && cmd.rightmove &&
          cmd.forwardmove) {
        constexpr float isqrt2 = 0.70710678118;
        mult *= isqrt2;
      }

      DrawLine(scx, scy, scx + mult * static_cast<float>(cmd.rightmove),
               scy - mult * static_cast<float>(cmd.forwardmove),
               CGaz2Colors[1]);
    }

    // When under wishspeed velocity, most accel happens when
    // you move straight towards your current velocity, so skip
    // drawing the "wings" on the sides
    const bool drawSides = state.vf > state.wishspeed;

    // minline length, either fixed or from current speed
    float velSize;

    if (etj_CGaz2FixedSpeed.value > 0) {
      velSize = etj_CGaz2FixedSpeed.value / 5.0f;
    } else {
      velSize = std::min(state.vf / 5.0f, SCREEN_HEIGHT / 2.0f);
    }

    if (!etj_CGaz2NoVelocityDir.integer ||
        (!drawSides && etj_CGaz2NoVelocityDir.integer == 2)) {
      float dirSize = velSize;

      if (!drawSides && etj_CGaz2FixedSpeed.value > 0) {
        // prevent comically long velocity direction lines on fixed speeds
        dirSize = std::min(127.0f, dirSize);
      }

      DrawLine(scx, scy, scx + dirSize * std::sin(drawVel),
               scy - dirSize * std::cos(drawVel), CGaz2Colors[0]);
    }

    if (drawSides) {
      velSize /= 2;
      DrawLine(scx, scy, scx + velSize * std::sin(drawVel + drawOpt),
               scy - velSize * std::cos(drawVel + drawOpt), CGaz2Colors[0]);
      DrawLine(scx, scy, scx + velSize * std::sin(drawVel - drawOpt),
               scy - velSize * std::cos(drawVel - drawOpt), CGaz2Colors[0]);
    }

    if (etj_stretchCgaz.integer) {
      ETJump_EnableWidthScale(true);
    }
  }
}

bool CGaz::strafingForwards(const playerState_t &ps, const pmove_t *pm) {
  // "forwards" means player viewangles naturally match keys pressed,
  // i.e. looking ahead with +forward and looking backwards with +back,
  // except for nobeat: looking to the left with +forward only and
  // looking to the right with +back only

  // get player speed
  const float speed = VectorLength2(ps.velocity);

  // get sprint scale
  const float scale = PmoveUtils::PM_SprintScale(&ps);

  // get usercmd
  const auto ucmdScale =
      static_cast<int8_t>(ps.stats[STAT_USERCMD_BUTTONS] & (BUTTON_WALKING << 8)
                              ? CMDSCALE_WALK
                              : CMDSCALE_DEFAULT);
  const usercmd_t cmd = PmoveUtils::getUserCmd(ps, ucmdScale);

  // not strafing if speed lower than ground speed or no user input
  if (speed < static_cast<float>(ps.speed) * scale ||
      (cmd.forwardmove == 0 && cmd.rightmove == 0)) {
    return false;
  }

  // get wishvel
  vec3_t wishvel;
  PmoveUtils::PM_UpdateWishvel(wishvel, cmd, pm->pmext->forward,
                               pm->pmext->right, pm->pmext->up, ps);

  // get angle between wishvel and player velocity
  const float wishvelAngle = RAD2DEG(std::atan2(wishvel[1], wishvel[0]));
  const float velAngle = RAD2DEG(std::atan2(ps.velocity[1], ps.velocity[0]));
  const float diffAngle = AngleDelta(wishvelAngle, velAngle);

  // return true if diffAngle matches notion of "forwards"
  // fullbeat / halfbeat / invert (holding +moveleft) or
  // fullbeat / halfbeat / invert (holding +moveright) or
  // nobeat
  if ((cmd.rightmove < 0 && diffAngle >= 0) ||
      (cmd.rightmove > 0 && diffAngle < 0) ||
      (cmd.forwardmove != 0 && diffAngle >= 0)) {
    return true;
  }

  return false;
}

float CGaz::getOptAngle(const playerState_t &ps, const pmove_t *pm,
                        bool alternate) {
  const auto uCmdScale =
      static_cast<int8_t>(ps.stats[STAT_USERCMD_BUTTONS] & (BUTTON_WALKING << 8)
                              ? CMDSCALE_WALK
                              : CMDSCALE_DEFAULT);
  const usercmd_t cmd = PmoveUtils::getUserCmd(ps, uCmdScale);

  // get correct pmove state
  pm = PmoveUtils::getPmove(cmd);

  // water and ladder movement are not important
  // since speed is capped anyway
  // check this only after we have a valid pmove
  if (pm->pmext->waterlevel > 1 || pm->pmext->ladder) {
    return false;
  }

  // show upmove influence?
  const float scale =
      etj_CGazTrueness.integer & static_cast<int>(CGazTrueness::CGAZ_JUMPCROUCH)
          ? pm->pmext->scale
          : pm->pmext->scaleAlt;

  vec3_t wishvel;
  float wishspeed =
      PmoveUtils::PM_GetWishspeed(wishvel, scale, cmd, pm->pmext->forward,
                                  pm->pmext->right, pm->pmext->up, ps, pm);

  // set default wishspeed for drawing if no user input
  if (!cmd.forwardmove && !cmd.rightmove) {
    wishspeed = static_cast<float>(ps.speed) * ps.sprintSpeedScale;
  }

  UpdateDraw(wishspeed, &ps, pm);

  // no meaningful value if speed lower than ground speed or no user input
  if (state.vf < state.wishspeed ||
      (cmd.forwardmove == 0 && cmd.rightmove == 0)) {
    return 0;
  }

  // determine whether strafing "forwards"
  const bool forwards = strafingForwards(ps, pm);

  // get variables associated with optimal angle
  const float accelAngle = RAD2DEG(
      std::atan2(alternate ? cmd.rightmove : -cmd.rightmove, cmd.forwardmove));

  float perAngle = RAD2DEG(drawOpt);
  float velAngle = RAD2DEG(drawVel);

  if (!forwards) {
    perAngle *= -1;
  }

  if (alternate) {
    perAngle *= -1;
  }

  // shift yaw to optimal angle for all strafe styles
  float opt = ps.viewangles[YAW];

  if (cmd.rightmove < 0) {
    // fullbeat / halfbeat / invert (holding +moveleft)
    opt = velAngle + perAngle - accelAngle;
  } else if (cmd.rightmove > 0) {
    // fullbeat / halfbeat / invert (holding +moveright)
    opt = velAngle - perAngle - accelAngle;
  } else if (cmd.forwardmove != 0) {
    // nobeat
    opt = velAngle + perAngle;
  }

  // return minimum angle for which you still gain the highest accel
  return AngleNormalize180(opt);
}

bool CGaz::canSkipDraw() const {
  if (!etj_drawCGaz.integer) {
    return true;
  }

  if (VectorLengthSquared2(ps->velocity) == 0) {
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
