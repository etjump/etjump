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

#include <algorithm>
#include "etj_areaindicator_drawable.h"

namespace ETJump {
AreaIndicator::AreaIndicator() {
  indicators.clear();

  indicators.emplace_back(&etj_drawSaveIndicator, &etj_saveIndicatorX,
                          &etj_saveIndicatorY, CONTENTS_NOSAVE,
                          BG_LEVEL_NO_SAVE, cgs.media.saveIcon);
  indicators.emplace_back(&etj_drawProneIndicator, &etj_proneIndicatorX,
                          &etj_proneIndicatorY, CONTENTS_NOPRONE,
                          BG_LEVEL_NO_PRONE, cgs.media.proneIcon);
  indicators.emplace_back(&etj_drawNoclipIndicator, &etj_noclipIndicatorX,
                          &etj_noclipIndicatorY, CONTENTS_NONOCLIP,
                          BG_LEVEL_NO_NOCLIP, cgs.media.noclipIcon);

  ci = nullptr;
  ps = nullptr;
  forbidShader = &cgs.media.friendShader;
  pronePressTime = 0;
  printProneMessage = false;
}

AreaIndicator::Indicator::Indicator(vmCvar_t *controlCvar,
                                    vmCvar_t *controlCvarX,
                                    vmCvar_t *controlCvarY, const int contents,
                                    const int shared,
                                    const qhandle_t iconShader) {
  draw = false;
  drawForbid = false;
  cvar = controlCvar;
  cvarX = controlCvarX;
  cvarY = controlCvarY;
  x = 0;
  y = 0;
  traceContents = contents;
  sharedValue = shared;
  shader = iconShader;
}

void AreaIndicator::checkPronePrint(trace_t &trace) {
  if (cgs.cheats || ci->team == TEAM_SPECTATOR) {
    printProneMessage = false;
    return;
  }

  if (cgs.shared & BG_LEVEL_NO_PRONE) {
    if (trace.fraction != 1.0f) {
      printProneMessage = false;
      return;
    }
  } else {
    if (trace.fraction == 1.0f) {
      printProneMessage = false;
      return;
    }
  }

  if (ps->stats[STAT_USERCMD_BUTTONS] & WBUTTON_PRONE) {
    if (cg.time > pronePressTime) {
      printProneMessage = true;
      pronePressTime = cg.time + 1000;
    }
  } else {
    printProneMessage = false;
  }
}

bool AreaIndicator::beforeRender() {
  ps = getValidPlayerState();
  ci = &cgs.clientinfo[ps->clientNum];

  if (canSkipDraw()) {
    return false;
  }

  trace_t trace;
  bool drawAny = false;

  for (auto &indicator : indicators) {
    indicator.draw = false;
    indicator.drawForbid = false;
    indicator.x = indicator.cvarX->value;
    indicator.y = indicator.cvarY->value;

    ETJump_AdjustPosition(&indicator.x);

    // this is a bit wasteful if none of the indicators are being drawn,
    // but we need to do a trace for prone print always,
    // and I'd rather not complicate the logic based around that
    CG_TraceCapsule(&trace, ps->origin, ps->mins, ps->maxs, ps->origin,
                    ps->clientNum, indicator.traceContents);

    if (indicator.traceContents == CONTENTS_NOPRONE) {
      checkPronePrint(trace);
    }

    switch (static_cast<DrawMode>(indicator.cvar->integer)) {
      case DrawMode::Always:
        indicator.draw = true;
        drawAny = true;

        if (((cgs.shared & indicator.sharedValue) && trace.fraction == 1.0f) ||
            (!(cgs.shared & indicator.sharedValue) && trace.fraction != 1.0f)) {
          indicator.drawForbid = true;
        }

        break;
      case DrawMode::Outside:
        if (trace.fraction == 1.0f) {
          indicator.draw = true;
          drawAny = true;

          if (cgs.shared & indicator.sharedValue) {
            indicator.drawForbid = true;
          }
        }

        break;
      case DrawMode::Inside:
        if (trace.fraction != 1.0f) {
          indicator.draw = true;
          drawAny = true;

          if (!(cgs.shared & indicator.sharedValue)) {
            indicator.drawForbid = true;
          }
        }

        break;
      default:
        indicator.draw = false;
        indicator.drawForbid = false;
        break;
    }
  }

  // user might not have indicators enabled at all,
  // but we still want to display the print if prone is not allowed
  return drawAny || printProneMessage;
}

void AreaIndicator::render() const {
  for (const auto &indicator : indicators) {
    if (indicator.draw) {
      drawPic(indicator.x, indicator.y, size, size, indicator.shader, color);

      if (indicator.drawForbid) {
        drawPic(indicator.x, indicator.y, size, size, *forbidShader);
      }
    }
  }

  if (printProneMessage) {
    CG_CenterPrint("^7You cannot ^3prone ^7inside this area.",
                   SCREEN_HEIGHT - 99, SMALLCHAR_WIDTH);
  }
}

bool AreaIndicator::canSkipDraw() {
  if (showingScores()) {
    return true;
  }

  if (cgs.demoCam.renderingFreeCam) {
    return true;
  }

  if (ci->team == TEAM_SPECTATOR && !(cg.snap->ps.pm_flags & PMF_FOLLOW)) {
    return true;
  }

  return false;
}
} // namespace ETJump
