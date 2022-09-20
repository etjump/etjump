/*
 * MIT License
 *
 * Copyright (c) 2022 ETJump team <zero@etjump.com>
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

#include "etj_entity_events_handler.h"
#include "etj_maxspeed.h"
#include "cg_local.h"
#include <string>
#include "etj_utilities.h"
#include "etj_cvar_update_handler.h"

ETJump::DisplayMaxSpeed::DisplayMaxSpeed(
    EntityEventsHandler *entityEventsHandler)
    : _entityEventsHandler{entityEventsHandler} {
  if (!entityEventsHandler) {
    CG_Error("DisplayMaxSpeed: clientCommandsHandler is null.\n");
    return;
  }

  entityEventsHandler->subscribe(EV_LOAD_TELEPORT, [&](centity_t *cent) {
    if (cg.snap->ps.clientNum != cg.clientNum) {
      _maxSpeed = 0;
      return;
    }

    _displayMaxSpeed = _maxSpeed;
    _maxSpeed = 0;
    _animationStartTime = cg.time;
  });

  parseColor(etj_speedColor.string, _color);
  cvarUpdateHandler->subscribe(&etj_speedColor, [&](const vmCvar_t *cvar) {
    parseColor(etj_speedColor.string, _color);
  });
  cvarUpdateHandler->subscribe(&etj_speedAlpha, [&](const vmCvar_t *cvar) {
    parseColor(etj_speedColor.string, _color);
  });
}

ETJump::DisplayMaxSpeed::~DisplayMaxSpeed() {
  _entityEventsHandler->unsubcribe(EV_LOAD_TELEPORT);
}

void ETJump::DisplayMaxSpeed::parseColor(const std::string &color,
                                         vec4_t &out) {
  parseColorString(color, out);
  out[3] *= etj_speedAlpha.value;
}

void ETJump::DisplayMaxSpeed::beforeRender() {
  auto speed = sqrt(cg.snap->ps.velocity[0] * cg.snap->ps.velocity[0] +
                    cg.snap->ps.velocity[1] * cg.snap->ps.velocity[1]);

  if (speed >= _maxSpeed) {
    _maxSpeed = speed;
  }
}

void ETJump::DisplayMaxSpeed::render() const {
  if (canSkipDraw()) {
    return;
  }

  vec4_t color;
  auto fade = CG_FadeAlpha(_animationStartTime, etj_maxSpeedDuration.integer);
  Vector4Copy(_color, color);
  color[3] *= fade;

  auto size = 0.1f;
  size *= etj_speedSize.value;

  auto str = va("%0.f", _displayMaxSpeed);
  float w;
  switch (etj_speedAlign.integer) {
    case 1: // left align
      w = 0;
      break;
    case 2: // right align
      w = CG_Text_Width_Ext(str, size, 0, &cgs.media.limboFont2);
      break;
    default: // center align
      w = CG_Text_Width_Ext(str, size, 0, &cgs.media.limboFont2) / 2;
      break;
  }

  auto x = etj_maxSpeedX.value;
  auto y = etj_maxSpeedY.value;

  ETJump_AdjustPosition(&x);

  auto style = ITEM_TEXTSTYLE_NORMAL;

  if (etj_speedShadow.integer > 0) {
    style = ITEM_TEXTSTYLE_SHADOWED;
  }

  CG_Text_Paint_Ext(x - w, y, size, size, color, str, 0, 0, style,
                    &cgs.media.limboFont1);
}

bool ETJump::DisplayMaxSpeed::canSkipDraw() const {
  return !etj_drawMaxSpeed.integer || ETJump::showingScores();
}
