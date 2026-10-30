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

#include "etj_draw_yaw.h"
#include "cg_local.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_pmove_utils_v2.h"
#include "etj_utilities.h"

namespace ETJump {
inline constexpr float DRAWYAW_SIZE_MIN = 1.0f;
inline constexpr float DRAWYAW_SIZE_MAX = 10.0f;

DrawYaw::DrawYaw(const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : cvarUpdate(cvarUpdate) {
  parseColor(etj_yawColor.string);
  setSize(etj_yawSize);
  startListeners();
}

DrawYaw::~DrawYaw() {
  cvarUpdate->unsubscribe(&etj_yawColor);
  cvarUpdate->unsubscribe(&etj_yawSize);
}

void DrawYaw::startListeners() {
  cvarUpdate->subscribe(&etj_yawColor, [this](const vmCvar_t *cvar) {
    parseColor(cvar->string);
  });

  cvarUpdate->subscribe(&etj_yawSize,
                        [this](const vmCvar_t *cvar) { setSize(*cvar); });
}

void DrawYaw::parseColor(const std::string &colorStr) {
  cgame.utils.colorParser->parseColorString(colorStr, color);
}

void DrawYaw::setSize(const vmCvar_t &cvar) {
  size = CvarValueParser::parse<CvarValue::Size>(cvar, DRAWYAW_SIZE_MIN,
                                                 DRAWYAW_SIZE_MAX);
  size.x *= 0.1f;
  size.y *= 0.1f;
}

bool DrawYaw::beforeRender() {
  if (canSkipDraw()) {
    hasLastYaw = false;
    moveLeft = moveRight = false;
    return false;
  }

  const PmoveUtilsV2::State &s = cgame.hudData.pmoveV2->getState();

  // never lerp this - updating at most once per pmove frame keeps the
  // arrows from strobing when rendering at a higher FPS than pmove cadence,
  // by holding the current arrow state until the next pmove frame
  if (cgame.hudData.pmoveV2->skipUpdate(lastUpdateTime, std::nullopt, s.pm)) {
    return moveLeft || moveRight;
  }

  // 'viewangles' are composed of the usercmd angles (mouse movement) plus
  // 'delta_angles', which absorbs server-side view changes (spawning,
  // teleporting, rotating movers etc.) - subtract it, so only mouse-induced
  // yaw changes are tracked
  const int yawAngle =
      (ANGLE2SHORT(s.pm.ps->viewangles[YAW]) - s.pm.ps->delta_angles[YAW]) &
      65535;

  // seed the reference angle on the first update and when the
  // spectated client changes, to avoid a spurious arrow flash
  if (!hasLastYaw || s.pm.ps->clientNum != lastClientNum) {
    lastYaw = yawAngle;
    lastClientNum = s.pm.ps->clientNum;
    hasLastYaw = true;
    moveLeft = moveRight = false;
    return false;
  }

  const int diff = yawAngle - lastYaw;
  constexpr int halfRange = 65536 / 2;
  moveLeft = ((diff > 0 && diff <= halfRange) || diff < -halfRange);
  moveRight = ((diff < 0 && diff >= -halfRange) || diff > halfRange);
  lastYaw = yawAngle;

  if (!moveLeft && !moveRight) {
    return false;
  }

  x = std::clamp(etj_yawX.value, 0.0f, 640.0f);
  const float halfWidth = std::clamp(etj_yawWidth.value, 0.0f, 640.0f) * 0.5f;
  x += moveLeft ? -halfWidth : halfWidth;
  ETJump_AdjustPosition(&x);
  y = std::clamp(etj_yawY.value, 0.0f, static_cast<float>(SCREEN_HEIGHT));

  str = moveLeft ? "<" : ">";

  textStyle =
      etj_yawShadow.integer ? ITEM_TEXTSTYLE_SHADOWED : ITEM_TEXTSTYLE_NORMAL;

  return true;
}

void DrawYaw::render() const {
  CG_Text_Paint_Centred_Ext(x, y, size.x, size.y, color, str, 0, 0, textStyle,
                            &cgs.media.limboFont1);
}

bool DrawYaw::canSkipDraw() {
  if (!etj_drawYaw.integer) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
