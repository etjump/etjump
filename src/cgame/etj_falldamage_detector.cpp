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

#include "etj_falldamage_detector.h"
#include "cg_local.h"
#include "etj_pmove_utils_v2.h"
#include "etj_trace_utils.h"
#include "etj_utilities.h"

#include "../game/bg_local.h"

namespace ETJump {
int32_t FalldamageDetector::computeFallDamage(const float zVel,
                                              const float startHeight,
                                              const float endHeight,
                                              const float gravity,
                                              const int32_t waterlevel) {
  // mirrors the math in PM_CrashLand, solving the quadratic for the landing
  // velocity at 'endHeight', given the current vertical velocity and gravity.
  // this is the exact same gravity calculation as 'PM_SlideMove' does, so it
  // is framerate independent and works for both 'pmove_fixed' 0 and 1.
  const float acc = -gravity;
  const float a = acc / 2.0f;
  const float b = zVel;
  const float c = startHeight - endHeight;
  const float den = (b * b) - (4.0f * a * c);

  if (a == 0.0f || den < 0.0f) {
    return 0;
  }

  const float t = (-b - std::sqrt(den)) / (2.0f * a);
  float delta = zVel + (t * acc);
  delta *= delta * 0.0001f;

  if (waterlevel == 3) {
    return 0;
  }

  if (waterlevel == 2) {
    delta *= 0.25f;
  } else if (waterlevel == 1) {
    delta *= 0.5f;
  }

  if (delta > DELTA_FALL_NDIE) {
    return 9999;
  }

  if (delta > DELTA_FALL_DMG_50) {
    return 50;
  }

  if (delta > DELTA_FALL_DMG_25) {
    return 25;
  }

  if (delta > DELTA_FALL_DMG_15) {
    return 15;
  }

  if (delta > DELTA_FALL_DMG_10) {
    return 10;
  }

  return 0;
}

std::string FalldamageDetector::buildDamageString(const int32_t damage) const {
  if (damage > 50) {
    return "^1GIB";
  }

  const int32_t realDamage = ps->powerups[PW_ADRENALINE] ? damage / 2 : damage;

  if (ps->stats[STAT_HEALTH] <= realDamage) {
    return "^1" + std::to_string(realDamage) + "HP";
  }

  return std::to_string(realDamage) + "HP";
}

bool FalldamageDetector::wouldTakeFallDamage(const trace_t &trace) {
  // steep surfaces don't cause fall damage
  if (trace.plane.normal[2] < MIN_WALK_NORMAL) {
    return false;
  }

  return (cgame.sharedWSKeys.noFallDamage == NoFallDamageOpts::ON)
             ? (trace.surfaceFlags & SURF_NODAMAGE)
             : !(trace.surfaceFlags & SURF_NODAMAGE);
}

bool FalldamageDetector::beforeRender() {
  ps = getValidPlayerState();

  if (canSkipDraw()) {
    return false;
  }

  const float zVel = ps->velocity[2];
  const float startHeight = ps->origin[2] + ps->mins[2];
  const auto gravity = static_cast<float>(ps->gravity);

  vec3_t start{};
  vec3_t end{};

  trace_t trace{};

  damageBelow = 0;
  damageFall = 0;
  damageJump = 0;

  // copy playerstate so we can calculate the waterlevel at the destination
  playerState_t landingPs = *ps;

  pmove_t pmove{};
  pmove.ps = &landingPs;
  pmove.pointcontents = CG_PointContents;

  // detect fall damage below us
  if (etj_drawFallDamageDetector.integer == 2 &&
      (ps->groundEntityNum == ENTITYNUM_NONE || ps->pm_type == PM_NOCLIP)) {
    VectorCopy(ps->origin, start);
    start[2] = startHeight;
    VectorCopy(start, end);
    end[2] -= MAX_MAP_SIZE * 2;

    cgame.utils.trace->filteredTrace(ps->clientNum, &trace, start, nullptr,
                                     nullptr, end, ps->clientNum,
                                     (CONTENTS_SOLID | CONTENTS_PLAYERCLIP));

    if (trace.fraction != 1.0 && wouldTakeFallDamage(trace)) {
      VectorCopy(trace.endpos, landingPs.origin);
      landingPs.origin[2] = trace.endpos[2] - ps->mins[2];
      PmoveUtilsV2::setWaterLevel(pmove);

      damageBelow = computeFallDamage(zVel, startHeight, trace.endpos[2],
                                      gravity, pmove.waterlevel);
    }
  }

  VectorCopy(cg.refdef.vieworg, start);
  VectorMA(start, MAX_MAP_SIZE * 2, cg.refdef.viewaxis[0], end);

  cgame.utils.trace->filteredTrace(ps->clientNum, &trace, start, nullptr,
                                   nullptr, end, ps->clientNum,
                                   (CONTENTS_SOLID | CONTENTS_PLAYERCLIP));

  if (trace.fraction != 1.0f && wouldTakeFallDamage(trace)) {
    VectorCopy(trace.endpos, landingPs.origin);
    landingPs.origin[2] = trace.endpos[2] - ps->mins[2];
    PmoveUtilsV2::setWaterLevel(pmove);

    damageFall = computeFallDamage(zVel, startHeight, trace.endpos[2], gravity,
                                   pmove.waterlevel);

    if (ps->groundEntityNum != ENTITYNUM_NONE || ps->pm_type == PM_NOCLIP) {
      damageJump =
          computeFallDamage(zVel + JUMP_VELOCITY, startHeight, trace.endpos[2],
                            gravity, pmove.waterlevel);
    }
  }

  return (damageBelow > 0 || damageFall > 0 || damageJump > 0);
}

void FalldamageDetector::render() const {
  float x = std::clamp(etj_fallDamageDetectorX.value, 0.0f, 640.0f);
  ETJump_AdjustPosition(&x);
  const float y = std::clamp(etj_fallDamageDetectorY.value, 0.0f,
                             static_cast<float>(SCREEN_HEIGHT));

  if (damageFall > 0) {
    CG_DrawStringExt(x, y, va("F:%s", buildDamageString(damageFall).c_str()),
                     colorWhite, qfalse, qtrue, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
                     0);
  }

  if (damageJump > 0) {
    CG_DrawStringExt(
        x, y + 10, va("J:%s", buildDamageString(damageJump).c_str()),
        colorWhite, qfalse, qtrue, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);
  }

  if (damageBelow > 0) {
    CG_DrawStringExt(
        x, y + 20, va("B:%s", buildDamageString(damageBelow).c_str()),
        colorWhite, qfalse, qtrue, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);
  }
}

bool FalldamageDetector::canSkipDraw() const {
  if (!etj_drawFallDamageDetector.integer) {
    return true;
  }

  // can't take fall damage on this map
  if (cgame.sharedWSKeys.noFallDamage == NoFallDamageOpts::FORCE_ON) {
    return true;
  }

  if (cg.renderingThirdPerson) {
    return true;
  }

  if (ps->pm_type == PM_DEAD) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
