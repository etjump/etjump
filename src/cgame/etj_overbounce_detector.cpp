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

#include "etj_overbounce_detector.h"
#include "etj_overbounce_shared.h"
#include "etj_local.h"
#include "etj_utilities.h"

namespace ETJump {
bool OverbounceDetector::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  belowOverbounce = jumpOverbounce = fallOverbounce = stickyOverbounce = false;

  ps = getValidPlayerState();
  traceContents = getExtraTraceContents(ETJump::OB_DETECTOR) | CONTENTS_BODY;
  pmoveSec = static_cast<float>(cgs.pmove_msec) / 1000.0f;
  gravity = ps->gravity;
  zVel = ps->velocity[2];
  startHeight = ps->origin[2] + ps->mins[2];

  VectorSet(snap, 0, 0, gravity * pmoveSec);
  trap_SnapVector(snap);
  zVelSnapped = snap[2];

  x = etj_OBX.value;
  ETJump_AdjustPosition(&x);

  trace_t trace;

  if (ps->groundEntityNum == ENTITYNUM_NONE) {
    // below ob
    VectorCopy(ps->origin, start);
    start[2] = startHeight;
    VectorCopy(start, end);
    end[2] -= Overbounce::MAX_TRACE_DIST;

    cgame.utils.trace->filteredTrace(ps->clientNum, &trace, start, nullptr,
                                     nullptr, end, ps->clientNum,
                                     traceContents);

    if (trace.fraction != 1.0 && trace.plane.type == 2) {
      // something was hit and it's a floor
      endHeight = trace.endpos[2];

      // below ob
      if (Overbounce::isOverbounce(zVel, startHeight, endHeight, zVelSnapped,
                                   pmoveSec, gravity) &&
          Overbounce::surfaceAllowsOverbounce(&trace)) {
        belowOverbounce = true;
      }
    }
  }

  // use origin from playerState?
  VectorCopy(cg.refdef.vieworg, start);
  VectorMA(start, Overbounce::MAX_TRACE_DIST, cg.refdef.viewaxis[0], end);

  cgame.utils.trace->filteredTrace(ps->clientNum, &trace, start, nullptr,
                                   nullptr, end, ps->clientNum, traceContents);

  if (trace.fraction != 1.0 && trace.plane.type == 2) {
    // something was hit and it's a floor
    endHeight = trace.endpos[2];
    // CG_Printf("startHeight=%f, endHeight=%f\n", startHeight, endHeight);

    // fall ob
    if (Overbounce::isOverbounce(zVel, startHeight, endHeight, zVelSnapped,
                                 pmoveSec, gravity) &&
        Overbounce::surfaceAllowsOverbounce(&trace)) {
      fallOverbounce = true;
    }

    // jump ob
    if (ps->groundEntityNum != ENTITYNUM_NONE &&
        Overbounce::isOverbounce(zVel + JUMP_VELOCITY, startHeight, endHeight,
                                 zVelSnapped, pmoveSec, gravity) &&
        Overbounce::surfaceAllowsOverbounce(&trace)) {
      jumpOverbounce = true;
    }

    if (!jumpOverbounce && !fallOverbounce && startHeight != endHeight &&
        etj_drawOB.integer == 2) {
      startHeight += Overbounce::stickyOffset;
      // CG_Printf("startHeight=%f, endHeight=%f\n", startHeight, endHeight);

      // sticky fall ob
      if (Overbounce::isOverbounce(zVel, startHeight, endHeight, zVelSnapped,
                                   pmoveSec, gravity) &&
          Overbounce::surfaceAllowsOverbounce(&trace)) {
        fallOverbounce = true;
        stickyOverbounce = true;
      }

      // sticky jump ob
      if (ps->groundEntityNum != ENTITYNUM_NONE &&
          Overbounce::isOverbounce(zVel + JUMP_VELOCITY, startHeight, endHeight,
                                   zVelSnapped, pmoveSec, gravity) &&
          Overbounce::surfaceAllowsOverbounce(&trace)) {
        jumpOverbounce = true;
        stickyOverbounce = true;
      }
    }
  }

  return belowOverbounce || fallOverbounce || jumpOverbounce ||
         stickyOverbounce;
}

void OverbounceDetector::render() const {
  if (belowOverbounce) {
    CG_DrawStringExt(static_cast<int>(x + 10), etj_OBY.integer, "B", colorWhite,
                     qfalse, qtrue, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);
  }
  if (fallOverbounce) {
    CG_DrawStringExt(static_cast<int>(x - 10), etj_OBY.integer, "F", colorWhite,
                     qfalse, qtrue, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);
  }
  if (jumpOverbounce) {
    CG_DrawStringExt(static_cast<int>(x), etj_OBY.integer, "J", colorWhite,
                     qfalse, qtrue, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);
  }
  if (stickyOverbounce) {
    CG_DrawStringExt(static_cast<int>(x - 20), etj_OBY.integer, "S", colorWhite,
                     qfalse, qtrue, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);
  }
}

bool OverbounceDetector::canSkipDraw() {
  if (!etj_drawOB.integer || cg_thirdPerson.integer) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
