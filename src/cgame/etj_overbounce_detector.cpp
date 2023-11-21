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

#include "etj_overbounce_detector.h"
#include "etj_overbounce_shared.h"

namespace ETJump {
void OverbounceDetector::beforeRender() {
  belowOverbounce = jumpOverbounce = fallOverbounce = stickyOverbounce = false;

  ps = getValidPlayerState();
  traceContents = checkExtraTrace(ETJump::OB_DETECTOR);
  pmoveSec = static_cast<float>(cgs.pmove_msec) / 1000.0f;
  gravity = ps->gravity;
  v0 = ps->velocity[2];
  startHeight = ps->origin[2] + ps->mins[2];

  VectorSet(snap, 0, 0, gravity * pmoveSec);
  trap_SnapVector(snap);
  v0Snapped = snap[2];

  x = etj_OBX.value;
  ETJump_AdjustPosition(&x);

  trace_t trace;

  if (ps->groundEntityNum == ENTITYNUM_NONE) {
    // below ob
    VectorCopy(ps->origin, start);
    start[2] = startHeight;
    VectorCopy(start, end);
    end[2] -= Overbounce::MAX_TRACE_DIST;

    CG_Trace(&trace, start, vec3_origin, vec3_origin, end, ps->clientNum,
             traceContents);

    if (trace.fraction != 1.0 && trace.plane.type == 2) {
      // something was hit and it's a floor
      endHeight = trace.endpos[2];

      // below ob
      if (Overbounce::isOverbounce(v0, startHeight, endHeight, v0Snapped,
                                   pmoveSec, gravity) &&
          Overbounce::surfaceAllowsOverbounce(&trace)) {
        belowOverbounce = true;
      }
    }
  }

  // use origin from playerState?
  VectorCopy(cg.refdef.vieworg, start);
  VectorMA(start, Overbounce::MAX_TRACE_DIST, cg.refdef.viewaxis[0], end);

  CG_Trace(&trace, start, vec3_origin, vec3_origin, end, ps->clientNum,
           traceContents);

  if (trace.fraction != 1.0 && trace.plane.type == 2) {
    // something was hit and it's a floor
    endHeight = trace.endpos[2];
    // CG_Printf("startHeight=%f, endHeight=%f\n", startHeight, endHeight);

    // fall ob
    if (Overbounce::isOverbounce(v0, startHeight, endHeight, v0Snapped,
                                 pmoveSec, gravity) &&
        Overbounce::surfaceAllowsOverbounce(&trace)) {
      fallOverbounce = true;
    }

    // jump ob
    if (ps->groundEntityNum != ENTITYNUM_NONE &&
        Overbounce::isOverbounce(v0 + JUMP_VELOCITY, startHeight, endHeight,
                                 v0Snapped, pmoveSec, gravity) &&
        Overbounce::surfaceAllowsOverbounce(&trace)) {
      jumpOverbounce = true;
    }

    // don't predict sticky ob if there is an ob already or if
    // the sticky ob detection isn't requested
    if (jumpOverbounce || fallOverbounce || etj_drawOB.integer != 2) {
      return;
    }
    // don't display stickies on the same height we're currently
    // at since obviously it's possible and it's just distracting
    if (startHeight == endHeight) {
      return;
    }

    startHeight += Overbounce::stickyOffset;
    // CG_Printf("startHeight=%f, endHeight=%f\n", startHeight, endHeight);

    // sticky fall ob
    if (Overbounce::isOverbounce(v0, startHeight, endHeight, v0Snapped,
                                 pmoveSec, gravity) &&
        Overbounce::surfaceAllowsOverbounce(&trace)) {
      fallOverbounce = true;
      stickyOverbounce = true;
    }

    // sticky jump ob
    if (ps->groundEntityNum != ENTITYNUM_NONE &&
        Overbounce::isOverbounce(v0 + JUMP_VELOCITY, startHeight, endHeight,
                                 v0Snapped, pmoveSec, gravity) &&
        Overbounce::surfaceAllowsOverbounce(&trace)) {
      jumpOverbounce = true;
      stickyOverbounce = true;
    }
  }
}

void OverbounceDetector::render() const {
  if (canSkipDraw()) {
    return;
  }

  if (belowOverbounce) {
    DrawString(x + 10, etj_OBY.value, 0.25f, 0.25f, colorWhite, qfalse, "B", 0,
               ITEM_TEXTSTYLE_SHADOWED);
  }
  if (fallOverbounce) {
    DrawString(x - 10, etj_OBY.value, 0.25f, 0.25f, colorWhite, qfalse, "F", 0,
               ITEM_TEXTSTYLE_SHADOWED);
  }
  if (jumpOverbounce) {
    DrawString(x, etj_OBY.value, 0.25f, 0.25f, colorWhite, qfalse, "J", 0,
               ITEM_TEXTSTYLE_SHADOWED);
  }
  if (stickyOverbounce) {
    DrawString(x - 20, etj_OBY.value, 0.25f, 0.25f, colorWhite, qfalse, "S", 0,
               ITEM_TEXTSTYLE_SHADOWED);
  }
}

bool OverbounceDetector::canSkipDraw() {
  if (!etj_drawOB.integer || cg_thirdPerson.integer) {
    return true;
  }

  return false;
}
} // namespace ETJump