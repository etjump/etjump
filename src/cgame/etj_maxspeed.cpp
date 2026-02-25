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

#include "etj_maxspeed.h"
#include "cg_local.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_entity_events_handler.h"
#include "etj_utilities.h"

namespace ETJump {
DisplayMaxSpeed::DisplayMaxSpeed(
    const std::shared_ptr<EntityEventsHandler> &entityEvents,
    const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : entityEvents(entityEvents), cvarUpdate(cvarUpdate) {
  parseColor(etj_speedColor.string, color);
  startListeners();
}

DisplayMaxSpeed::~DisplayMaxSpeed() {
  entityEvents->unsubscribe(EV_LOAD_TELEPORT);

  cvarUpdate->unsubscribe(&etj_speedColor);
  cvarUpdate->unsubscribe(&etj_speedAlpha);
}

void DisplayMaxSpeed::startListeners() {
  entityEvents->subscribe(EV_LOAD_TELEPORT, [this](centity_t *) {
    if (cg.snap->ps.clientNum != cg.clientNum) {
      maxSpeed = 0;
      return;
    }

    displayMaxSpeed = maxSpeed;
    maxSpeed = 0;
    animationStartTime = cg.time;
  });

  cvarUpdate->subscribe(&etj_speedColor, [this](const vmCvar_t *cvar) {
    parseColor(cvar->string, color);
  });

  cvarUpdate->subscribe(&etj_speedAlpha, [this](const vmCvar_t *) {
    parseColor(etj_speedColor.string, color);
  });
}

void DisplayMaxSpeed::parseColor(const std::string &color, vec4_t &out) {
  cgame.utils.colorParser->parseColorString(color, out);
  out[3] *= etj_speedAlpha.value;
}

bool DisplayMaxSpeed::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  auto speed = sqrt(cg.snap->ps.velocity[0] * cg.snap->ps.velocity[0] +
                    cg.snap->ps.velocity[1] * cg.snap->ps.velocity[1]);

  if (speed >= maxSpeed) {
    maxSpeed = speed;
  }

  return true;
}

void DisplayMaxSpeed::render() const {

  vec4_t color = {1, 1, 1, 1};
  auto fade = CG_FadeAlpha(animationStartTime, etj_maxSpeedDuration.integer);
  Vector4Copy(color, color);
  color[3] *= fade;

  auto size = 0.1f;
  size *= etj_speedSize.value;

  auto str = va("%0.f", displayMaxSpeed);
  float w;
  switch (etj_speedAlign.integer) {
    case 1: // left align
      w = 0;
      break;
    case 2: // right align
      w = CG_Text_Width_Ext(str, size, 0, &cgs.media.limboFont2);
      break;
    default: // center align
      w = CG_Text_Width_Ext(str, size, 0, &cgs.media.limboFont2) / 2.0f;
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

bool DisplayMaxSpeed::canSkipDraw() {
  return !etj_drawMaxSpeed.integer || showingScores();
}
} // namespace ETJump
