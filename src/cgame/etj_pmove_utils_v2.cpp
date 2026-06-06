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

#include "etj_pmove_utils_v2.h"
#include "cg_local.h"

namespace ETJump {
void PmoveUtilsV2::setupPmove(pmove_t &pm) {
  assert(pm.ps && pm.pmext);

  pm.trace = CG_TraceCapsule;
  pm.tracemask = pm.ps->pm_type == PM_DEAD ? MASK_PLAYERSOLID & ~CONTENTS_BODY
                                           : MASK_PLAYERSOLID;
  pm.pointcontents = CG_PointContents;
  pm.skill = cgs.clientinfo[pm.ps->clientNum].skill;
  pm.shared = cgs.shared;
  pm.pmove_msec = cgs.pmove_msec;

  // NOTE: incorrect in spec/demo playback, 'sprintTime' is local
  pm.pmext->sprintTime = cg.pmext.sprintTime;
  pm.pmext->jumpDelayBug = cg.jumpDelayBug;
  pm.pmext->autoSprint = cg.pmext.autoSprint;
}

void PmoveUtilsV2::setupUserCmd(const int8_t scale, pmove_t &pm) {
  if (!cg.demoPlayback && !(pm.ps->pm_flags & PMF_FOLLOW)) {
    const int32_t cmdNum = trap_GetCurrentCmdNumber();
    trap_GetUserCmd(cmdNum, &pm.cmd);
    return;
  }

  pm.cmd.forwardmove = static_cast<signed char>(
      scale * (!!(pm.ps->stats[STAT_USERCMD_MOVE] & UMOVE_FORWARD) -
               !!(pm.ps->stats[STAT_USERCMD_MOVE] & UMOVE_BACKWARD)));
  pm.cmd.rightmove = static_cast<signed char>(
      scale * (!!(pm.ps->stats[STAT_USERCMD_MOVE] & UMOVE_RIGHT) -
               !!(pm.ps->stats[STAT_USERCMD_MOVE] & UMOVE_LEFT)));
  pm.cmd.upmove = static_cast<signed char>(
      scale * (!!(pm.ps->stats[STAT_USERCMD_MOVE] & UMOVE_UP) -
               !!(pm.ps->stats[STAT_USERCMD_MOVE] & UMOVE_DOWN)));

  // store buttons too, so we get correct scale when sprint is held
  pm.cmd.buttons = pm.ps->stats[STAT_USERCMD_BUTTONS] >> 8;
  pm.cmd.wbuttons = pm.ps->stats[STAT_USERCMD_BUTTONS] & 0xff;

  pm.cmd.serverTime = cg.snap->serverTime;
}

PmoveUtilsV2::PmoveSingleResult
PmoveUtilsV2::pmoveSingle(pmove_t &pm, pml_t &pml,
                          const EnumBitset<PmoveDefaultInput> &defaultInput) {
  const int8_t scale = pm.ps->stats[STAT_USERCMD_BUTTONS] & BUTTON_WALKING << 8
                           ? CMDSCALE_WALK
                           : CMDSCALE_DEFAULT;

  PmoveUtilsV2::setupUserCmd(scale, pm);

  // because autosprint directly flips the sprint button bit in the players
  // 'usercmd_t', it is already accounted for it the stats that pmove
  // saves to the playerstate, so we don't care about it here
  // if we're in spec/demo playback
  if (!cg.demoPlayback && pm.ps->clientNum == cg.clientNum &&
      pm.pmext->autoSprint) {
    pm.cmd.buttons ^= BUTTON_SPRINT;
  }

  // clear all pmove local vars
  memset(&pml, 0, sizeof(pml));

  VectorClear(pml.forward);
  VectorClear(pml.right);
  VectorClear(pml.up);

  // save old velocity for crashlanding
  VectorCopy(pm.ps->velocity, pml.previous_velocity);

  AngleVectors(pm.ps->viewangles, pml.forward, pml.right, pml.up);

  if (pm.cmd.upmove < 10) {
    // not holding jump
    pm.ps->pm_flags &= ~PMF_JUMP_HELD;
  }

  if (pm.cmd.upmove > 0) {
    pm.cmd.upmove = CMDSCALE_DEFAULT;
  }

  if (pm.ps->pm_type >= PM_DEAD ||
      pm.ps->pm_flags & (PMF_LIMBO | PMF_TIME_LOCKPLAYER) ||
      BG_PlayerMounted(pm.ps->eFlags)) {
    pm.cmd.forwardmove = 0;
    pm.cmd.rightmove = 0;
    pm.cmd.upmove = 0;
  }

  // set default key combination if there's no user input
  if (!pm.cmd.forwardmove && !pm.cmd.rightmove && defaultInput) {
    setDefaultInput(pm, scale, defaultInput);
  }

  // set watertype, and waterlevel
  PmoveUtilsV2::setWaterLevel(pm);

  // set mins, maxs, and viewheight
  if (!PmoveUtilsV2::checkProne(pm)) {
    PmoveUtilsV2::checkDuck(pm);
  }

  // set groundentity
  PmoveUtilsV2::groundTrace(pm, pml);

  PmoveUtilsV2::checkLadderMove(pm, pml);

  PmoveSingleResult result{};

  if (pml.ladder) {
    result = PmoveSingleResult::LADDER;
  } else if (pm.waterlevel > 1 || pm.ps->pm_flags & PMF_TIME_WATERJUMP) {
    result = PmoveSingleResult::WATER;
  } else if (pm.ps->eFlags & EF_MOUNTEDTANK) {
    result = PmoveSingleResult::MOUNTED;
  } else if (pml.walking) {
    result = PmoveSingleResult::WALK;
  } else {
    result = PmoveSingleResult::AIR;
  }

  PmoveUtilsV2::sprint(pm);

  return result;
}

void PmoveUtilsV2::setWaterLevel(pmove_t &pm) {
  vec3_t point{};
  int32_t cont = 0;
  int32_t sample1 = 0;
  int32_t sample2 = 0;

  // get waterlevel, accounting for ducking
  pm.waterlevel = 0;
  pm.watertype = 0;

  // Ridah, modified this
  point[0] = pm.ps->origin[0];
  point[1] = pm.ps->origin[1];
  point[2] = pm.ps->origin[2] + pm.ps->mins[2] + 1;

  cont = pm.pointcontents(point, pm.ps->clientNum);

  if (!(cont & MASK_WATER)) {
    return;
  }

  sample2 = static_cast<int32_t>(static_cast<float>(pm.ps->viewheight) -
                                 pm.ps->mins[2]);
  sample1 = sample2 / 2;

  pm.watertype = cont;
  pm.waterlevel = 1;
  point[2] = pm.ps->origin[2] + pm.ps->mins[2] + static_cast<float>(sample1);

  cont = pm.pointcontents(point, pm.ps->clientNum);

  if (!(cont & MASK_WATER)) {
    return;
  }

  pm.waterlevel = 2;
  point[2] = pm.ps->origin[2] + pm.ps->mins[2] + static_cast<float>(sample2);

  cont = pm.pointcontents(point, pm.ps->clientNum);

  if (cont & MASK_WATER) {
    pm.waterlevel = 3;
  }
}

bool PmoveUtilsV2::canProne(const pmove_t &pm) {
  // can't go prone on ladders
  if (pm.ps->pm_flags & PMF_LADDER) {
    return false;
  }

  // no prone when using mg42's
  if (pm.ps->persistant[PERS_HWEAPON_USE] || pm.ps->eFlags & EF_MOUNTEDTANK) {
    return false;
  }

  if (pm.ps->weaponDelay && pm.ps->weapon == WP_PANZERFAUST) {
    return false;
  }

  if (pm.ps->weapon == WP_MORTAR_SET) {
    return false;
  }

  // can't go prone while swimming
  if (pm.waterlevel > 1) {
    return false;
  }

  return true;
}

bool PmoveUtilsV2::checkProne(pmove_t &pm) {
  trace_t trace{};
  pm.trace(&trace, pm.ps->origin, pm.ps->mins, pm.ps->maxs, pm.ps->origin,
           pm.ps->clientNum, CONTENTS_NOPRONE);

  if (!cgs.cheats && (pm.shared & BG_LEVEL_NO_PRONE) ? trace.fraction == 1.0f
                                                     : trace.fraction != 1.0f) {
    pm.ps->eFlags &= ~EF_PRONE;
    pm.ps->eFlags &= ~EF_PRONE_MOVING;
    return false;
  }

  if (!(pm.ps->eFlags & EF_PRONE)) {
    if (!canProne(pm)) {
      return false;
    }

    if (((pm.ps->pm_flags & PMF_DUCKED && pm.cmd.doubleTap == DT_FORWARD) ||
         (pm.cmd.wbuttons & WBUTTON_PRONE)) &&
        pm.cmd.serverTime - pm.pmext->proneTime > PRONE_DELAY_TIME) {
      trace_t trace;

      pm.mins[0] = pm.ps->mins[0];
      pm.mins[1] = pm.ps->mins[1];

      pm.maxs[0] = pm.ps->maxs[0];
      pm.maxs[1] = pm.ps->maxs[1];

      pm.mins[2] = pm.ps->mins[2];
      pm.maxs[2] = pm.ps->crouchMaxZ;

      pm.ps->eFlags |= EF_PRONE;
      traceAll(trace, pm.ps->origin, pm.ps->origin, pm);
      pm.ps->eFlags &= ~EF_PRONE;

      if (!trace.startsolid && !trace.allsolid) {
        // go prone
        pm.ps->pm_flags |= PMF_DUCKED; // crouched as well
        pm.ps->eFlags |= EF_PRONE;
        pm.pmext->proneTime = pm.cmd.serverTime; // timestamp 'go prone'
        pm.pmext->proneGroundTime = pm.cmd.serverTime;
      }
    }
  }

  if (pm.ps->eFlags & EF_PRONE) {
    if (pm.waterlevel > 1 || pm.ps->pm_type == PM_DEAD ||
        pm.ps->eFlags & EF_MOUNTEDTANK ||
        ((pm.cmd.doubleTap == DT_BACK || pm.cmd.upmove > 10 ||
          pm.cmd.wbuttons & WBUTTON_PRONE) &&
         pm.cmd.serverTime - pm.pmext->proneTime > PRONE_DELAY_TIME)) {
      trace_t trace;

      // see if we have the space to stop prone
      pm.mins[0] = pm.ps->mins[0];
      pm.mins[1] = pm.ps->mins[1];

      pm.maxs[0] = pm.ps->maxs[0];
      pm.maxs[1] = pm.ps->maxs[1];

      pm.mins[2] = pm.ps->mins[2];
      pm.maxs[2] = pm.ps->crouchMaxZ;

      pm.ps->eFlags &= ~EF_PRONE;
      traceAll(trace, pm.ps->origin, pm.ps->origin, pm);
      pm.ps->eFlags |= EF_PRONE;

      if (!trace.allsolid) {
        // crouch for a bit
        pm.ps->pm_flags |= PMF_DUCKED;

        // stop prone
        pm.ps->eFlags &= ~EF_PRONE;
        pm.ps->eFlags &= ~EF_PRONE_MOVING;
        pm.pmext->proneTime = pm.cmd.serverTime; // timestamp 'stop prone'

        // don't jump for a bit
        pm.pmext->jumpTime = pm.cmd.serverTime - PRONE_JUMP_DELAY_TIME;
        pm.ps->jumpTime = pm.cmd.serverTime - PRONE_JUMP_DELAY_TIME;
      }
    }
  }

  if (pm.ps->eFlags & EF_PRONE) {
    // See if we are moving
    const float spd = VectorLength(pm.ps->velocity);
    const bool userinput =
        std::abs(pm.cmd.forwardmove) + abs(pm.cmd.rightmove) > 10;

    if (userinput && spd > 40.f && !(pm.ps->eFlags & EF_PRONE_MOVING)) {
      pm.ps->eFlags |= EF_PRONE_MOVING;
    } else if (!userinput && spd < 20.0f && (pm.ps->eFlags & EF_PRONE_MOVING)) {
      pm.ps->eFlags &= ~EF_PRONE_MOVING;
    }

    pm.mins[0] = pm.ps->mins[0];
    pm.mins[1] = pm.ps->mins[1];

    pm.maxs[0] = pm.ps->maxs[0];
    pm.maxs[1] = pm.ps->maxs[1];

    pm.mins[2] = pm.ps->mins[2];

    pm.maxs[2] = pm.ps->maxs[2] - pm.ps->standViewHeight - PRONE_VIEWHEIGHT;
    pm.ps->viewheight = PRONE_VIEWHEIGHT;

    return true;
  }

  return false;
}

void PmoveUtilsV2::checkDuck(pmove_t &pm) {
  // Ridah, modified this for configurable bounding boxes
  pm.mins[0] = pm.ps->mins[0];
  pm.mins[1] = pm.ps->mins[1];

  pm.maxs[0] = pm.ps->maxs[0];
  pm.maxs[1] = pm.ps->maxs[1];

  pm.mins[2] = pm.ps->mins[2];

  if (pm.ps->pm_type == PM_DEAD) {
    // NOTE: must set death bounding box in game code
    pm.maxs[2] = pm.ps->maxs[2];
    pm.ps->viewheight = static_cast<int32_t>(pm.ps->deadViewHeight);
    return;
  }

  if ((pm.cmd.upmove < 0 && !(pm.ps->eFlags & EF_MOUNTEDTANK) &&
       !(pm.ps->pm_flags & PMF_LADDER)) ||
      pm.ps->weapon == WP_MORTAR_SET) {
    // duck
    pm.ps->pm_flags |= PMF_DUCKED;
  } else { // stand up if possible
    if (pm.ps->pm_flags & PMF_DUCKED) {
      trace_t trace;

      // try to stand up
      pm.maxs[2] = pm.ps->maxs[2];
      traceAll(trace, pm.ps->origin, pm.ps->origin, pm);

      if (!trace.allsolid) {
        pm.ps->pm_flags &= ~PMF_DUCKED;
      }
    }
  }

  if (pm.ps->pm_flags & PMF_DUCKED) {
    pm.maxs[2] = pm.ps->crouchMaxZ;
    pm.ps->viewheight = static_cast<int32_t>(pm.ps->crouchViewHeight);
  } else {
    pm.maxs[2] = pm.ps->maxs[2];
    pm.ps->viewheight = static_cast<int32_t>(pm.ps->standViewHeight);
  }
}

void PmoveUtilsV2::groundTrace(pmove_t &pm, pml_t &pml) {
  vec3_t point{};
  trace_t trace{};

  point[0] = pm.ps->origin[0];
  point[1] = pm.ps->origin[1];

  if (pm.ps->eFlags & EF_MG42_ACTIVE || pm.ps->eFlags & EF_AAGUN_ACTIVE) {
    point[2] = pm.ps->origin[2] - 1.f;
  } else {
    point[2] = pm.ps->origin[2] - 0.25f;
  }

  traceAllLegs(trace, &pm.pmext->proneLegsOffset, pm.ps->origin, point, pm);
  pml.groundTrace = trace;

  if (pm.shared & BG_LEVEL_NO_WALLBUG) {
    if (trace.allsolid && pm.ps->pm_type != PM_NOCLIP) {
      VectorClear(pm.ps->velocity);
    }
  }

  // do something corrective if the trace starts in a solid...
  if (trace.allsolid && !(pm.ps->eFlags & EF_MOUNTEDTANK)) {
    if (!PmoveUtilsV2::correctAllSolid(trace, pm, pml)) {
      return;
    }
  }

  // if the trace didn't hit anything, we are in free fall
  if (trace.fraction == 1.0) {
    groundTraceMissed(pm, pml);
    return;
  }

  // check if getting thrown off the ground
  if (pm.ps->velocity[2] > 0 &&
      DotProduct(pm.ps->velocity, trace.plane.normal) > 10) {
    pm.ps->groundEntityNum = ENTITYNUM_NONE;
    pml.groundPlane = qfalse;
    pml.walking = qfalse;
    return;
  }

  // slopes that are too steep will not be considered onground
  if (trace.plane.normal[2] < MIN_WALK_NORMAL) {
    // FIXME: if they can't slide down the slope, let them walk (sharp crevices)
    pm.ps->groundEntityNum = ENTITYNUM_NONE;
    pml.groundPlane = qtrue;
    pml.walking = qfalse;
    return;
  }

  pml.groundPlane = qtrue;
  pml.walking = qtrue;

  // hitting solid ground will end a waterjump
  if (pm.ps->pm_flags & PMF_TIME_WATERJUMP) {
    pm.ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
    pm.ps->pm_time = 0;
  }

  if (pm.ps->groundEntityNum == ENTITYNUM_NONE) {
    // just hit the ground
    // don't do landing time if we were just going down a slope
    if (pml.previous_velocity[2] < -200) {
      // don't allow another jump for a little while
      pm.ps->pm_flags |= PMF_TIME_LAND;
      if (pm.ps->pm_time < 250) {
        pm.ps->pm_time = 250;
      }
    }
  }

  pm.ps->groundEntityNum = trace.entityNum;
}

void PmoveUtilsV2::groundTraceMissed(pmove_t &pm, pml_t &pml) {
  // If we've never yet touched the ground, it's because we're spawning,
  // so don't set to "in air"
  if (pm.ps->groundEntityNum != -1) {
    // Signify that we're in mid-air
    pm.ps->groundEntityNum = ENTITYNUM_NONE;
  }

  pml.groundPlane = qfalse;
  pml.walking = qfalse;
}

void PmoveUtilsV2::traceAll(trace_t &trace, vec3_t start, vec3_t end,
                            const pmove_t &pm) {
  traceAllLegs(trace, nullptr, start, end, pm);
}

/* Traces all player bboxes -- body and legs */
void PmoveUtilsV2::traceAllLegs(trace_t &trace, float *legsOffset, vec3_t start,
                                vec3_t end, const pmove_t &pm) {
  pm.trace(&trace, start, pm.mins, pm.maxs, end, pm.ps->clientNum,
           pm.tracemask);

  /* legs */
  if (pm.ps->eFlags & EF_PRONE) {
    trace_t legtrace;

    PM_TraceLegs(&legtrace, legsOffset, start, end, &trace, pm.ps->viewangles,
                 pm.trace, pm.ps->clientNum, pm.tracemask);

    if (legtrace.fraction < trace.fraction || legtrace.startsolid ||
        legtrace.allsolid) {
      VectorSubtract(end, start, legtrace.endpos);
      VectorMA(start, legtrace.fraction, legtrace.endpos, legtrace.endpos);
      trace = legtrace;
    }
  }
}

bool PmoveUtilsV2::correctAllSolid(trace_t &trace, pmove_t &pm, pml_t &pml) {
  vec3_t point{};

  // jitter around
  for (int32_t i = -1; i <= 1; i++) {
    for (int32_t j = -1; j <= 1; j++) {
      for (int32_t k = -1; k <= 1; k++) {
        VectorCopy(pm.ps->origin, point);
        point[0] += static_cast<float>(i);
        point[1] += static_cast<float>(j);
        point[2] += static_cast<float>(k);
        traceAllLegs(trace, nullptr, point, point, pm);

        if (!trace.allsolid) {
          point[0] = pm.ps->origin[0];
          point[1] = pm.ps->origin[1];
          point[2] = pm.ps->origin[2] - 0.25f;

          traceAllLegs(trace, nullptr, pm.ps->origin, point, pm);
          pml.groundTrace = trace;
          return true;
        }
      }
    }
  }

  pm.ps->groundEntityNum = ENTITYNUM_NONE;
  pml.groundPlane = qfalse;
  pml.walking = qfalse;

  return false;
}

void PmoveUtilsV2::checkLadderMove(pmove_t &pm, pml_t &pml) {
  if (pm.ps->pm_time) {
    return;
  }

  float tracedist = 0;

  if (pml.walking) {
    tracedist = 1.0;
  } else {
    tracedist = TRACE_LADDER_DIST;
  }

  pml.ladder = qfalse;
  pm.ps->pm_flags &= ~PMF_LADDER; // clear ladder bit

  if (pm.ps->pm_type == PM_DEAD) {
    return;
  }

  // Can't climb ladders while prone
  if (pm.ps->eFlags & EF_PRONE) {
    return;
  }

  // check for ladder
  vec3_t flatforward{};
  flatforward[0] = pml.forward[0];
  flatforward[1] = pml.forward[1];
  flatforward[2] = 0;
  VectorNormalize(flatforward);

  vec3_t spot{};
  VectorMA(pm.ps->origin, tracedist, flatforward, spot);

  trace_t trace{};
  pm.trace(&trace, pm.ps->origin, pm.mins, pm.maxs, spot, pm.ps->clientNum,
           pm.tracemask);

  if (trace.fraction < 1 && (trace.surfaceFlags & SURF_LADDER)) {
    pml.ladder = qtrue;
  }

  vec3_t laddervec{};

  if (pml.ladder) {
    VectorCopy(trace.plane.normal, laddervec);
  }

  if (pml.ladder && !pml.walking && (trace.fraction * tracedist > 1.0)) {
    // if we are only just on the ladder, don't do this yet,
    // or it may throw us back off the ladder
    pml.ladder = qfalse;

    vec3_t mins{};
    VectorCopy(pm.mins, mins);
    mins[2] = -1;

    VectorMA(pm.ps->origin, -tracedist, laddervec, spot);

    pm.trace(&trace, pm.ps->origin, mins, pm.maxs, spot, pm.ps->clientNum,
             pm.tracemask);

    if ((trace.fraction < 1) && (trace.surfaceFlags & SURF_LADDER)) {
      pml.ladder = qtrue;
      pm.ps->pm_flags |= PMF_LADDER; // set ladder bit
    } else {
      pml.ladder = qfalse;
    }
  } else if (pml.ladder) {
    pm.ps->pm_flags |= PMF_LADDER; // set ladder bit
  }

  // create some up/down velocity if touching ladder
  // we are currently on the ground, only go up and
  // prevent X/Y if we are pushing forwards
  if (pml.ladder && pml.walking && pm.cmd.forwardmove <= 0) {
    pml.ladder = qfalse;
  }
}

bool PmoveUtilsV2::checkJump(pmove_t &pm, pml_t &pml, const bool isLerpFrame) {
  // no jumpin when prone
  if (pm.ps->eFlags & EF_PRONE) {
    return false;
  }

  // JPW NERVE -- jumping in multiplayer uses and requires sprint juice
  // (to prevent turbo skating, sprint + jumps) don't allow jump accel

  // rain - revert to using pmext for this since pmext is fixed now.
  // fix for #166
  // NOTE: do not check for jump delay if we're interpolating
  // between pmove frames ('com_maxFPS > 125'), otherwise the subsequent
  // checks that happen within the same pmove frame have jump delay,
  // and ground friction gets applied while we're in the air
  if (!isLerpFrame &&
      pm.cmd.serverTime - pm.pmext->jumpTime < JUMP_DELAY_TIME) {
    if (pm.pmext->proneTime - pm.pmext->jumpTime == PRONE_JUMP_DELAY_TIME) {
      return false;
    }

    if ((pm.shared & BG_LEVEL_NO_JUMPDELAY)
            ? (pml.groundTrace.surfaceFlags & SURF_NOJUMPDELAY)
            : !(pml.groundTrace.surfaceFlags & SURF_NOJUMPDELAY)) {
      return false;
    }
  }

  if (pm.ps->pm_flags & PMF_RESPAWNED) {
    return false; // don't allow jump until all buttons are up
  }

  if (pm.cmd.upmove < 10) {
    // not holding jump
    return false;
  }

  // must wait for jump to be released
  if (pm.ps->pm_flags & PMF_JUMP_HELD) {
    // clear upmove so cmdscale doesn't lower running speed
    pm.cmd.upmove = 0;
    return false;
  }

  pml.groundPlane = qfalse; // jumping away
  pml.walking = qfalse;
  pm.ps->pm_flags |= PMF_JUMP_HELD;

  pm.ps->groundEntityNum = ENTITYNUM_NONE;
  pm.ps->velocity[2] = JUMP_VELOCITY;

  return true;
}

void PmoveUtilsV2::sprint(pmove_t &pm) {
  if (pm.cmd.buttons & BUTTON_SPRINT &&
      (pm.cmd.forwardmove || pm.cmd.rightmove) &&
      !(pm.ps->pm_flags & PMF_DUCKED) && !(pm.ps->eFlags & EF_PRONE)) {
    if (pm.ps->powerups[PW_ADRENALINE]) {
      pm.pmext->sprintTime = SPRINTTIME;
    } else if (pm.ps->powerups[PW_NOFATIGUE]) {
      // take time from powerup before taking it from sprintTime
      pm.ps->powerups[PW_NOFATIGUE] -= 50;

      // (SA) go ahead and continue to recharge stamina at double rate with
      // stamina powerup even when exerting
      pm.pmext->sprintTime += 10;

      if (pm.pmext->sprintTime > SPRINTTIME) {
        pm.pmext->sprintTime = SPRINTTIME;
      }

      if (pm.ps->powerups[PW_NOFATIGUE] < 0) {
        pm.ps->powerups[PW_NOFATIGUE] = 0;
      }
    }
    // JPW NERVE -- sprint time tuned for multiplayer
    else {
      // JPW NERVE adjusted for framerate independence
      pm.pmext->sprintTime -= 5000 * PM_FRAMETIME;
    }
    // jpw

    if (pm.pmext->sprintTime < 0) {
      pm.pmext->sprintTime = 0;
    }

    if (!pm.ps->sprintExertTime) {
      pm.ps->sprintExertTime = 1;
    }
  } else {
    // JPW NERVE -- in multiplayer, recharge faster for top 75%
    // of sprint bar (for people that *just* use it for jumping,
    // not sprint) this code was mucked about with to eliminate
    // client-side framerate-dependancy in wolf single player
    if (pm.ps->powerups[PW_ADRENALINE]) {
      pm.pmext->sprintTime = SPRINTTIME;
    } else if (pm.ps->powerups[PW_NOFATIGUE]) {
      // (SA) recharge at  2x with stamina  powerup
      pm.pmext->sprintTime += 10;
    } else {
      int32_t rechargebase = 500;

      if (pm.skill[SK_BATTLE_SENSE] >= 2) {
        rechargebase =
            static_cast<int32_t>(static_cast<float>(rechargebase) * 1.06f);
      }

      // JPW NERVE adjusted for framerate independence
      pm.pmext->sprintTime += static_cast<float>(rechargebase) * PM_FRAMETIME;
      if (pm.pmext->sprintTime > 5000) {
        // JPW NERVE adjusted for framerate independence
        pm.pmext->sprintTime += static_cast<float>(rechargebase) * PM_FRAMETIME;
      }
      // jpw
    }
    if (pm.pmext->sprintTime > SPRINTTIME) {
      pm.pmext->sprintTime = SPRINTTIME;
    }

    pm.ps->sprintExertTime = 0;
  }
}

void PmoveUtilsV2::updateWishvel(vec2_t &wishvel, pmove_t &pm, pml_t &pml) {
  const float fmove = pm.cmd.forwardmove;
  const float smove = pm.cmd.rightmove;

  for (int32_t i = 0; i < 2; i++) {
    wishvel[i] = (pml.forward[i] * fmove) + (pml.right[i] * smove);
  }
}

float PmoveUtilsV2::cmdScale(const pmove_t &pm, const usercmd_t &cmd,
                             const bool upmove) {
  float total = 0;
  int32_t max = std::abs(cmd.forwardmove);

  if (std::abs(cmd.rightmove) > max) {
    max = abs(cmd.rightmove);
  }

  if (upmove && std::abs(cmd.upmove) > max) {
    max = abs(cmd.upmove);
  }

  if (!max) {
    return 0;
  }

  const auto fmoveSquared =
      static_cast<float>(cmd.forwardmove * cmd.forwardmove);
  const auto smoveSquared = static_cast<float>(cmd.rightmove * cmd.rightmove);

  if (upmove) {
    const auto umoveSquared = static_cast<float>(cmd.upmove * cmd.upmove);
    total = std::sqrt(fmoveSquared + smoveSquared + umoveSquared);
  } else {
    total = std::sqrt(fmoveSquared + smoveSquared);
  }

  float scale = static_cast<float>(pm.ps->speed) * static_cast<float>(max) /
                (127.0f * total);

  if (pm.cmd.buttons & BUTTON_SPRINT && pm.pmext->sprintTime > 50) {
    scale *= pm.ps->sprintSpeedScale;
  } else {
    scale *= pm.ps->runSpeedScale;
  }

  if (pm.ps->pm_type == PM_NOCLIP) {
    scale *= 3;
  }

  // trying some different balance for the FT
  if (pm.ps->weapon == WP_FLAMETHROWER && pm.cmd.buttons & BUTTON_ATTACK) {
    scale *= 0.7;
  }

  return scale;
}

bool PmoveUtilsV2::strafingForwards(const pmove_t &pm, const float wishspeed,
                                    const vec2_t wishvel) {
  // not strafing if speed is low, or no user input
  if (VectorLength2(pm.ps->velocity) < wishspeed ||
      (!pm.cmd.forwardmove && !pm.cmd.rightmove)) {
    return false;
  }

  // get angle between wishvel and player velocity
  const float wishvelAngle = RAD2DEG(std::atan2(wishvel[1], wishvel[0]));
  const float velAngle =
      RAD2DEG(std::atan2(pm.ps->velocity[1], pm.ps->velocity[0]));
  const float diffAngle = AngleDelta(wishvelAngle, velAngle);

  // return true if 'diffAngle' matches notion of "forwards"
  // fullbeat / halfbeat / invert (holding +moveleft) or
  // fullbeat / halfbeat / invert (holding +moveright) or
  // nobeat
  if ((pm.cmd.rightmove < 0 && diffAngle >= 0) ||
      (pm.cmd.rightmove > 0 && diffAngle < 0) ||
      (pm.cmd.forwardmove != 0 && diffAngle >= 0)) {
    return true;
  }

  return false;
}

void PmoveUtilsV2::setDefaultInput(
    pmove_t &pm, const int8_t scale,
    const EnumBitset<PmoveDefaultInput> &defaultInput) {
  if (defaultInput & PmoveDefaultInput::FORWARD) {
    pm.cmd.forwardmove = scale;
  }

  if (defaultInput & PmoveDefaultInput::SIDE) {
    pm.cmd.rightmove = scale;
  }

  if (defaultInput & PmoveDefaultInput::UP) {
    pm.cmd.upmove = scale;
  }

  // if we don't have sprint pressed, 'cmdScale' will return 'runSpeedScale'
  if (defaultInput & PmoveDefaultInput::SPRINT) {
    pm.ps->runSpeedScale = pm.ps->sprintSpeedScale;
  }
}
} // namespace ETJump
