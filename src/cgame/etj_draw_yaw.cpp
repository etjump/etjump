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

#include "etj_draw_yaw.h"
#include "etj_pmove_utils.h"
#include "etj_utilities.h"
#include "../game/etj_numeric_utilities.h"
#include "etj_cvar_update_handler.h"

namespace ETJump {

DrawYaw::DrawYaw() {
  Vector4Copy(colorWhite, color);
  moveLeft = false;
  moveRight = false;
  oldYawAngle = 0;

  startListeners();
}

void DrawYaw::startListeners() {
  cvarUpdateHandler->subscribe(&etj_drawYawColor,
                               [&](const vmCvar_t *cvar) { parseColor(); });
}

void DrawYaw::parseColor() { parseColorString(etj_drawYawColor.string, color); }

bool DrawYaw::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  const playerState_t *ps = getValidPlayerState();
  const int yawAngle = ANGLE2SHORT(ps->viewangles[YAW]);
  const int diff = yawAngle - oldYawAngle;
  constexpr int halfRange = 65536 / 2;
  moveLeft = ((diff > 0 && diff <= halfRange) || diff < -halfRange);
  moveRight = ((diff < 0 && diff >= -halfRange) || diff > halfRange);
  oldYawAngle = yawAngle;

  return true;
}

void DrawYaw::render() const {
  const float x = ETJump_AdjustPosition(etj_drawYawX.value);
  const float y = etj_drawYawY.value;
  const float scale = 0.1f * Numeric::clamp(etj_drawYawSize.value, 0.f, 10.f);
  const auto textStyle = (etj_drawYawShadow.integer ? ITEM_TEXTSTYLE_SHADOWED
                                                    : ITEM_TEXTSTYLE_NORMAL);

  if (moveLeft) {
    const char *c = "<";
    const int w = CG_Text_Width_Ext(c, scale, 0, &cgs.media.limboFont1);
    CG_Text_Paint_Centred_Ext(x - w, y, scale, scale, color, c, 0, 0, textStyle,
                              &cgs.media.limboFont1);
  }
  if (moveRight) {
    const char *c = ">";
    const int w = CG_Text_Width_Ext(c, scale, 0, &cgs.media.limboFont1);
    CG_Text_Paint_Centred_Ext(x + w, y, scale, scale, color, c, 0, 0, textStyle,
                              &cgs.media.limboFont1);
  }
}

bool DrawYaw::canSkipDraw() const {
  if (!etj_drawYaw.integer) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
