/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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
#include "../game/bg_local.h"

// etj_pmove_utils.cpp - helper functions for pmove related calculations

namespace ETJump {
static pmove_t pmove;
static pmoveExt_t pmext;
static playerState_t temp_ps;

usercmd_t PmoveUtils::getUserCmd(const playerState_t &ps, int8_t uCmdScale) {
  usercmd_t cmd{};

  if (!cg.demoPlayback && !(cg.snap->ps.pm_flags & PMF_FOLLOW)) {
    int32_t const cmdNum = trap_GetCurrentCmdNumber();
    trap_GetUserCmd(cmdNum, &cmd);
  }
  // generate fake userCmd for following spectators and demos
  else {
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
    for (auto i = 0; i < 3; i++) {
      cmd.angles[i] = ANGLE2SHORT(ps.viewangles[i]) - ps.delta_angles[i];
    }

    cmd.serverTime = cg.snap->serverTime;
  }
  return cmd;
}

pmove_t *PmoveUtils::getPmove(usercmd_t cmd) {
  if (cg.snap->ps.clientNum == cg.clientNum && !cg.demoPlayback) {
    cg_pmove.pmext = &cg.pmext;
    return &cg_pmove;
  }

  temp_ps = cg.predictedPlayerState;
  pmove.ps = &temp_ps;
  pmove.pmext = &pmext;
  pmove.character =
      CG_CharacterForClientinfo(&cgs.clientinfo[cg.snap->ps.clientNum],
                                &cg_entities[cg.snap->ps.clientNum]);
  pmove.trace = CG_TraceCapsule;
  pmove.tracemask = cg.snap->ps.pm_type == PM_DEAD
                        ? MASK_PLAYERSOLID & ~CONTENTS_BODY
                        : MASK_PLAYERSOLID;
  pmove.pointcontents = CG_PointContents;
  pmove.skill = cgs.clientinfo[cg.snap->ps.clientNum].skill;
  pmove.cmd = cmd;
  pmove.pmove_msec = cgs.pmove_msec;
  PmoveSingle(&pmove);
  return &pmove;
}

float PmoveUtils::PM_SprintScale(const playerState_t *ps) {
  // based on PM_CmdScale from bg_pmove.c
  float scale = ps->stats[STAT_USERCMD_BUTTONS] & (BUTTON_SPRINT << 8) &&
                        cg.pmext.sprintTime > 50
                    ? ps->sprintSpeedScale
                    : ps->runSpeedScale;
  return scale;
}

float PmoveUtils::PM_GetWishspeed(vec3_t wishvel, float scale, usercmd_t cmd,
                                  vec3_t forward, vec3_t right, vec3_t up,
                                  const playerState_t &ps, pmove_t *pm) {
  PM_UpdateWishvel(wishvel, cmd, forward, right, up, ps);

  float wishspeed = scale * VectorLength2(wishvel);

  // if walking, account for prone, crouch and water
  if (pm->pmext->walking) {
    // clamp the speed lower if prone
    if (pm->ps->eFlags & EF_PRONE) {
      if (wishspeed > pm->ps->speed * pm_proneSpeedScale) {
        wishspeed = pm->ps->speed * pm_proneSpeedScale;
      }
    }
    // clamp the speed lower if ducking
    else if (pm->ps->pm_flags & PMF_DUCKED) {
      if (wishspeed > pm->ps->speed * pm->ps->crouchSpeedScale) {
        wishspeed = pm->ps->speed * pm->ps->crouchSpeedScale;
      }
    }

    // clamp the speed lower if wading or walking on the bottom
    if (pm->pmext->waterlevel) {
      float waterScale = pm->pmext->waterlevel / 3.0f;
      if (pm->watertype == CONTENTS_SLIME) {
        waterScale = 1.0 - (1.0 - pm_slagSwimScale) * waterScale;
      } else {
        waterScale = 1.0 - (1.0 - pm_waterSwimScale) * waterScale;
      }

      if (wishspeed > pm->ps->speed * waterScale) {
        wishspeed = pm->ps->speed * waterScale;
      }
    }
  }

  return wishspeed;
}

void PmoveUtils::PM_UpdateWishvel(vec3_t wishvel, usercmd_t cmd, vec3_t forward,
                                  vec3_t right, vec3_t up,
                                  const playerState_t &ps) {
  AngleVectors(ps.viewangles, forward, right, up);

  // project moves down to flat plane
  forward[2] = 0;
  right[2] = 0;
  VectorNormalize(forward);
  VectorNormalize(right);

  for (uint8_t i = 0; i < 2; ++i) {
    wishvel[i] = cmd.forwardmove * forward[i] + cmd.rightmove * right[i];
  }
}
} // namespace ETJump
