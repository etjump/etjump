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

#include "etj_accelmeter_drawable.h"
#include "etj_cvar_update_handler.h"
#include "etj_pmove_utils.h"
#include "etj_utilities.h"
#include "../game/etj_string_utilities.h"

namespace ETJump {
AccelMeter::AccelMeter() {
  parseColor(etj_accelColor.string, accelColor);
  setTextStyle();
  setSize();
  setAccelColorStyle();
  startListeners();
}

void AccelMeter::startListeners() {
  cvarUpdateHandler->subscribe(&etj_accelColor, [&](const vmCvar_t *cvar) {
    parseColor(cvar->string, accelColor);
  });

  cvarUpdateHandler->subscribe(&etj_accelAlpha, [&](const vmCvar_t *cvar) {
    parseColor(etj_accelColor.string, accelColor);
  });

  cvarUpdateHandler->subscribe(&etj_accelSize,
                               [&](const vmCvar_t *cvar) { setSize(); });

  cvarUpdateHandler->subscribe(&etj_accelShadow,
                               [&](const vmCvar_t *cvar) { setTextStyle(); });

  cvarUpdateHandler->subscribe(
      &etj_accelColorUsesAccel,
      [&](const vmCvar_t *cvar) { setAccelColorStyle(); });
}

void AccelMeter::parseColor(const std::string &color, vec4_t &out) {
  parseColorString(color, out);
  out[3] *= etj_accelAlpha.value;
}

void AccelMeter::setTextStyle() {
  textStyle =
      etj_accelShadow.integer ? ITEM_TEXTSTYLE_SHADOWED : ITEM_TEXTSTYLE_NORMAL;
}

void AccelMeter::setSize() {
  size = 0.1f * etj_accelSize.value;
  halfW = static_cast<float>(
              CG_Text_Width_Ext("-88  -88", size, 0, &cgs.media.limboFont1)) *
          0.5f;
}

void AccelMeter::setAccelColorStyle() {
  if (!etj_accelColorUsesAccel.integer) {
    parseColor(etj_accelColor.string, accelColor);
  }

  accelColorStyle = etj_accelColorUsesAccel.integer;
}

bool AccelMeter::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  const playerState_t &ps = cg.predictedPlayerState;
  const int8_t ucmdScale = CMDSCALE_DEFAULT;
  const usercmd_t cmd = PmoveUtils::getUserCmd(ps, ucmdScale);

  pm = PmoveUtils::getPmove(cmd);

  // FIXME: this whole update check thing for non-pmove frames should be
  //  moved to PmoveUtils so it can be done everywhere it needs to be done
  //  with a single function call
  const int frameTime = (cg.snap->ps.pm_flags & PMF_FOLLOW || cg.demoPlayback)
                            ? cg.time
                            : pm->ps->commandTime;

  if (canSkipUpdate(frameTime)) {
    return true;
  }

  lastUpdateTime = frameTime;
  float currentSpeed = VectorLength2(ps.velocity);

  if (accelColorStyle == AccelColor::Style::Simple ||
      (accelColorStyle == AccelColor::Style::Advanced &&
       AccelColor::lowSpeedOnGround(currentSpeed, ps.groundEntityNum))) {
    storedSpeeds.push_back({cg.time, currentSpeed});
    AccelColor::popOldStoredSpeeds(storedSpeeds);
  } else if (!storedSpeeds.empty()) {
    storedSpeeds.clear();
  }

  VectorSubtract(ps.velocity, lastSpeed, accel);
  VectorCopy(ps.velocity, lastSpeed);

  if (pm->ps->pm_type == PM_NOCLIP) {
    VectorClear(accel);

    if (accelColorStyle) {
      Vector4Copy(colorWhite, accelColor);
    } else {
      parseColor(etj_accelColor.string, accelColor);
    }
  } else if (accelColorStyle) {
    AccelColor::setAccelColor(accelColorStyle, currentSpeed,
                              etj_accelAlpha.value, pm, storedSpeeds, accel,
                              accelColor);
  }

  y = etj_accelY.value;

  accelStr.clear();
  accelStr.emplace_back(stringFormat("%.0f", accel[0]));
  accelStr.emplace_back(stringFormat("%.0f", accel[1]));

  return true;
}

void AccelMeter::render() const {
  // this happens for a few frames when spawning in a map
  // because canSkipUpdate exits beforeRender early
  if (accelStr.empty()) {
    return;
  }

  float accelX = etj_accelX.value;
  ETJump_AdjustPosition(&accelX);

  vec4_t color;
  Vector4Copy(accelColor, color);

  switch (etj_accelAlign.integer) {
    case Alignment::Left:
      CG_Text_Paint_Ext(accelX, y, size, size, color, accelStr[0], 0, 0,
                        textStyle, &cgs.media.limboFont1);
      CG_Text_Paint_Ext(accelX + halfW, y, size, size, color, accelStr[1], 0, 0,
                        textStyle, &cgs.media.limboFont1);
      break;
    case Alignment::Right:
      CG_Text_Paint_RightAligned_Ext(accelX, y, size, size, color, accelStr[0],
                                     0, 0, textStyle, &cgs.media.limboFont1);
      CG_Text_Paint_RightAligned_Ext(accelX - halfW, y, size, size, color,
                                     accelStr[1], 0, 0, textStyle,
                                     &cgs.media.limboFont1);
      break;
    default: // center align
      CG_Text_Paint_Centred_Ext(accelX - (halfW * 0.5f), y, size, size, color,
                                accelStr[0], 0, 0, textStyle,
                                &cgs.media.limboFont1);
      CG_Text_Paint_Centred_Ext(accelX + (halfW * 0.5f), y, size, size, color,
                                accelStr[1], 0, 0, textStyle,
                                &cgs.media.limboFont1);
      break;
  }
}

bool AccelMeter::canSkipUpdate(int frameTime) const {
  // only count this frame if it's relevant to pmove
  // this makes sure that if clients FPS > 125
  // we only count frames at pmove_msec intervals
  if (!pm->ps || lastUpdateTime + pm->pmove_msec > frameTime) {
    return true;
  }

  return false;
}

bool AccelMeter::canSkipDraw() {
  if (!etj_drawAccel.integer) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
