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

#include "etj_accelmeter_drawable.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_pmove_utils.h"
#include "etj_utilities.h"

#include "../game/etj_string_utilities.h"

namespace ETJump {
AccelMeter::AccelMeter(const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : cvarUpdate(cvarUpdate) {
  parseColor(etj_accelColor.string, accelColor);
  setTextStyle(&etj_accelShadow);
  setSize(&etj_accelSize);
  setAccelColorStyle(&etj_accelColorUsesAccel);
  startListeners();

  // add dummy elements here, so we don't try to read an empty vector
  // at the very beginning of a map when we don't have a valid update yet
  accelStr.emplace_back("0");
  accelStr.emplace_back("0");
}

AccelMeter::~AccelMeter() {
  cvarUpdate->unsubscribe(&etj_accelColor);
  cvarUpdate->unsubscribe(&etj_accelAlpha);
  cvarUpdate->unsubscribe(&etj_accelSize);
  cvarUpdate->unsubscribe(&etj_accelShadow);
  cvarUpdate->unsubscribe(&etj_accelColorUsesAccel);
}

void AccelMeter::startListeners() {
  cvarUpdate->subscribe(&etj_accelColor, [this](const vmCvar_t *cvar) {
    parseColor(cvar->string, accelColor);
  });

  cvarUpdate->subscribe(&etj_accelAlpha, [this](const vmCvar_t *) {
    parseColor(etj_accelColor.string, accelColor);
  });

  cvarUpdate->subscribe(&etj_accelSize,
                        [this](const vmCvar_t *cvar) { setSize(cvar); });

  cvarUpdate->subscribe(&etj_accelShadow,
                        [this](const vmCvar_t *cvar) { setTextStyle(cvar); });

  cvarUpdate->subscribe(&etj_accelColorUsesAccel, [this](const vmCvar_t *cvar) {
    setAccelColorStyle(cvar);
  });
}

void AccelMeter::parseColor(const std::string &color, vec4_t &out) {
  cgame.utils.colorParser->parseColorString(color, out);
  out[3] *= etj_accelAlpha.value;
}

void AccelMeter::setTextStyle(const vmCvar_t *cvar) {
  textStyle = cvar->integer ? ITEM_TEXTSTYLE_SHADOWED : ITEM_TEXTSTYLE_NORMAL;
}

void AccelMeter::setSize(const vmCvar_t *cvar) {
  size = CvarValueParser::parse<CvarValue::Size>(*cvar, 1, 10);
  size.x *= 0.1f;
  size.y *= 0.1f;

  halfW = static_cast<float>(
              CG_Text_Width_Ext("-88  -88", size.x, 0, &cgs.media.limboFont1)) *
          0.5f;
}

void AccelMeter::setAccelColorStyle(const vmCvar_t *cvar) {
  if (!cvar->integer) {
    parseColor(etj_accelColor.string, accelColor);
  }

  accelColorStyle = cvar->integer;
}

bool AccelMeter::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  playing = cg.snap->ps.clientNum == cg.clientNum && !cg.demoPlayback;

  pm = cgame.utils.pmove->getPmove();

  if (cgame.utils.pmove->skipUpdate(lastUpdateTime, std::nullopt)) {
    return true;
  }

  float currentSpeed = VectorLength2(ps->velocity);

  int style = accelColorStyle;

  // force simple coloring instead of advanced on spec/demo
  // due to interpolation inaccuracy
  if (!playing && style == AccelColor::Style::Advanced) {
    style = static_cast<int>(AccelColor::Style::Simple);
  }

  if (style == AccelColor::Style::Simple ||
      (style == AccelColor::Style::Advanced &&
       AccelColor::lowSpeedOnGround(currentSpeed, ps->groundEntityNum))) {
    storedSpeeds.push_back({lastUpdateTime, currentSpeed});
    AccelColor::popOldStoredSpeeds(storedSpeeds, lastUpdateTime);
  } else if (!storedSpeeds.empty()) {
    storedSpeeds.clear();
  }

  VectorSubtract(ps->velocity, lastSpeed, accel);
  VectorCopy(ps->velocity, lastSpeed);

  if (ps->pm_type == PM_NOCLIP) {
    VectorClear(accel);

    if (style) {
      Vector4Copy(colorWhite, accelColor);
    } else {
      parseColor(etj_accelColor.string, accelColor);
    }
  } else if (style) {
    AccelColor::setAccelColor(style, currentSpeed, etj_accelAlpha.value, pm, ps,
                              storedSpeeds, accel, accelColor);
  }

  y = etj_accelY.value;

  accelStr.clear();
  accelStr.emplace_back(stringFormat("%.0f", accel[0]));
  accelStr.emplace_back(stringFormat("%.0f", accel[1]));

  return true;
}

void AccelMeter::render() const {
  float accelX = etj_accelX.value;
  ETJump_AdjustPosition(&accelX);

  switch (etj_accelAlign.integer) {
    case Left:
      CG_Text_Paint_Ext(accelX, y, size.x, size.y, accelColor, accelStr[0], 0,
                        0, textStyle, &cgs.media.limboFont1);
      CG_Text_Paint_Ext(accelX + halfW, y, size.x, size.y, accelColor,
                        accelStr[1], 0, 0, textStyle, &cgs.media.limboFont1);
      break;
    case Right:
      CG_Text_Paint_RightAligned_Ext(accelX, y, size.x, size.y, accelColor,
                                     accelStr[0], 0, 0, textStyle,
                                     &cgs.media.limboFont1);
      CG_Text_Paint_RightAligned_Ext(accelX - halfW, y, size.x, size.y,
                                     accelColor, accelStr[1], 0, 0, textStyle,
                                     &cgs.media.limboFont1);
      break;
    default: // center align
      CG_Text_Paint_Centred_Ext(accelX - (halfW * 0.5f), y, size.x, size.y,
                                accelColor, accelStr[0], 0, 0, textStyle,
                                &cgs.media.limboFont1);
      CG_Text_Paint_Centred_Ext(accelX + (halfW * 0.5f), y, size.x, size.y,
                                accelColor, accelStr[1], 0, 0, textStyle,
                                &cgs.media.limboFont1);
      break;
  }
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
