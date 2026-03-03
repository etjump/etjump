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

#include "etj_pmove_utils.h"
#include "cg_local.h"
#include "etj_cvar_update_handler.h"

#include "../game/bg_local.h"

namespace ETJump {
PmoveUtils::PmoveUtils(const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : cvarUpdate(cvarUpdate) {
  initCvars();
  setupCallbacks();
  setPmoveStatus();
}

PmoveUtils::~PmoveUtils() {
  for (const auto &cvar : cvars) {
    cvarUpdate->unsubscribe(cvar);
  }
}

void PmoveUtils::initCvars() {
  cvars.emplace_back(&etj_drawSpeed2);
  cvars.emplace_back(&etj_drawCGaz);
  cvars.emplace_back(&etj_drawSnapHUD);
  cvars.emplace_back(&etj_drawAccel);
  cvars.emplace_back(&etj_drawStrafeQuality);
  cvars.emplace_back(&etj_drawUpmoveMeter);
}

void PmoveUtils::setupCallbacks() {
  for (const auto &cvar : cvars) {
    cvarUpdate->subscribe(cvar, [this](const vmCvar_t *) { setPmoveStatus(); });
  }
}

void PmoveUtils::setPmoveStatus() {
  for (const auto &cvar : cvars) {
    if (cvar->integer != 0) {
      doPmove = true;
      return;
    }
  }

  doPmove = false;
}

void PmoveUtils::setupUserCmd() {
  uCmdScale = ps.stats[STAT_USERCMD_BUTTONS] & BUTTON_WALKING << 8
                  ? CMDSCALE_WALK
                  : CMDSCALE_DEFAULT;

  if (cg.snap->ps.clientNum == cg.clientNum && !cg.demoPlayback) {
    const int cmdNum = trap_GetCurrentCmdNumber();
    trap_GetUserCmd(cmdNum, &cmd);
    return;
  }

  cmd.forwardmove = static_cast<signed char>(
      uCmdScale * (!!(ps.stats[STAT_USERCMD_MOVE] & UMOVE_FORWARD) -
                   !!(ps.stats[STAT_USERCMD_MOVE] & UMOVE_BACKWARD)));
  cmd.rightmove = static_cast<signed char>(
      uCmdScale * (!!(ps.stats[STAT_USERCMD_MOVE] & UMOVE_RIGHT) -
                   !!(ps.stats[STAT_USERCMD_MOVE] & UMOVE_LEFT)));
  cmd.upmove = static_cast<signed char>(
      uCmdScale * (!!(ps.stats[STAT_USERCMD_MOVE] & UMOVE_UP) -
                   !!(ps.stats[STAT_USERCMD_MOVE] & UMOVE_DOWN)));

  // store buttons too, so we get correct scale when sprint is held
  cmd.buttons = ps.stats[STAT_USERCMD_BUTTONS] >> 8;
  cmd.wbuttons = ps.stats[STAT_USERCMD_BUTTONS] & 0xff;

  // generate correct angles
  for (int i = 0; i < 3; i++) {
    cmd.angles[i] = ANGLE2SHORT(ps.viewangles[i]) - ps.delta_angles[i];
  }

  cmd.serverTime = cg.snap->serverTime;
}

void PmoveUtils::setupPmove() {
  if (cg.snap->ps.clientNum == cg.clientNum && !cg.demoPlayback) {
    pm = cg_pmove;
    pm.pmext = &cg.pmext;
    return;
  }

  pm.ps = &ps;
  pm.pmext = &pmext;
  pm.character =
      CG_CharacterForClientinfo(&cgs.clientinfo[cg.snap->ps.clientNum],
                                &cg_entities[cg.snap->ps.clientNum]);
  pm.trace = CG_TraceCapsule;
  pm.tracemask = cg.snap->ps.pm_type == PM_DEAD
                     ? MASK_PLAYERSOLID & ~CONTENTS_BODY
                     : MASK_PLAYERSOLID;
  pm.pointcontents = CG_PointContents;
  pm.skill = cgs.clientinfo[cg.snap->ps.clientNum].skill;
  pm.cmd = cmd;
  pm.pmove_msec = cgs.pmove_msec;
}

void PmoveUtils::runPmove() {
  // this should never be called before client prediction has run at least once
  assert(cg.validPPS);

  ps = cg.predictedPlayerState;
  setupUserCmd();
  setupPmove();

  // if not spectating/in demo playback, we have everything we need
  if (cg.snap->ps.clientNum == cg.clientNum && !cg.demoPlayback) {
    return;
  }

  PmoveSingle(&pm);
}

float PmoveUtils::getSprintScale() const {
  // based on PM_CmdScale from bg_pmove.cpp
  return ps.stats[STAT_USERCMD_BUTTONS] & BUTTON_SPRINT << 8 &&
                 cg.pmext.sprintTime > 50
             ? ps.sprintSpeedScale
             : ps.runSpeedScale;
}

float PmoveUtils::getWishspeed(vec3_t wishvel, const float scale,
                               vec3_t forward, vec3_t right, vec3_t up) const {
  updateWishvel(wishvel, forward, right, up, pm.cmd);

  float wishspeed = scale * VectorLength2(wishvel);

  // if walking, account for prone, crouch and water
  if (pm.pmext->walking) {
    // clamp the speed lower if prone
    if (pm.ps->eFlags & EF_PRONE) {
      if (wishspeed > pm.ps->speed * pm_proneSpeedScale) {
        wishspeed = pm.ps->speed * pm_proneSpeedScale;
      }
    }
    // clamp the speed lower if ducking
    else if (pm.ps->pm_flags & PMF_DUCKED) {
      if (wishspeed > pm.ps->speed * pm.ps->crouchSpeedScale) {
        wishspeed = pm.ps->speed * pm.ps->crouchSpeedScale;
      }
    }

    // clamp the speed lower if wading or walking on the bottom
    if (pm.pmext->waterlevel) {
      float waterScale = pm.pmext->waterlevel / 3.0f;
      if (pm.watertype == CONTENTS_SLIME) {
        waterScale = 1.0 - (1.0 - pm_slagSwimScale) * waterScale;
      } else {
        waterScale = 1.0 - (1.0 - pm_waterSwimScale) * waterScale;
      }

      if (wishspeed > pm.ps->speed * waterScale) {
        wishspeed = pm.ps->speed * waterScale;
      }
    }
  }

  return wishspeed;
}

void PmoveUtils::updateWishvel(vec3_t wishvel, vec3_t forward, vec3_t right,
                               vec3_t up, const usercmd_t &ucmd) const {
  AngleVectors(ps.viewangles, forward, right, up);

  // project moves down to flat plane
  forward[2] = 0;
  right[2] = 0;
  VectorNormalize(forward);
  VectorNormalize(right);

  for (uint8_t i = 0; i < 2; ++i) {
    wishvel[i] = ucmd.forwardmove * forward[i] + ucmd.rightmove * right[i];
  }
}

float PmoveUtils::getFrameAccel(const bool upmoveTrueness) {
  // no meaningful value if no user input
  if (cmd.forwardmove == 0 && cmd.rightmove == 0) {
    return 0;
  }

  vec3_t wishvel;
  const float scale = upmoveTrueness ? pm.pmext->scale : pm.pmext->scaleAlt;
  const float wishspeed = getWishspeed(wishvel, scale, pm.pmext->forward,
                                       pm.pmext->right, pm.pmext->up);

  return pm.pmext->accel * wishspeed * pm.pmext->frametime;
}

bool PmoveUtils::skipUpdate(int &lastUpdateTime,
                            std::optional<HUDLerpFlags> flag) const {
  // pmove hasn't run yet
  // this can happen for the first few frames of a game in localhost,
  // if the client is fast enough to load everything
  if (!pm.ps) {
    return true;
  }

  const int frameTime = cg.snap->ps.pm_flags & PMF_FOLLOW || cg.demoPlayback
                            ? cg.time
                            : ps.commandTime;
  const int now = frameTime - frameTime % pm.pmove_msec;

  // never skip updates if lerping is requested
  if (flag.has_value() &&
      !(etj_HUD_noLerp.integer & static_cast<int>(flag.value()))) {
    lastUpdateTime = now;
    return false;
  }

  if (lastUpdateTime + pm.pmove_msec > frameTime) {
    return true;
  }

  lastUpdateTime = now;
  return false;
}

bool PmoveUtils::check() const { return doPmove; }

const pmove_t *PmoveUtils::getPmove() const { return &pm; }

const usercmd_t *PmoveUtils::getUserCmd() const { return &cmd; }

int8_t PmoveUtils::getUserCmdScale() const { return uCmdScale; }
} // namespace ETJump
