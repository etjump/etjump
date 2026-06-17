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

#include "etj_accelmeter_v2.h"
#include "cg_local.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_utilities.h"

namespace ETJump {
inline constexpr float ACCELMETER_SIZE_MIN = 1.0f;
inline constexpr float ACCELMETER_SIZE_MAX = 10.0f;

AccelMeterV2::AccelMeterV2(const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : cvarUpdate(cvarUpdate) {
  parseColor(etj_accelColor.string);
  setAccelColorStyle(etj_accelColorUsesAccel);
  setSize(etj_accelSize);
  startListeners();
}

AccelMeterV2::~AccelMeterV2() {
  cvarUpdate->unsubscribe(&etj_accelColor);
  cvarUpdate->unsubscribe(&etj_accelAlpha);
  cvarUpdate->unsubscribe(&etj_accelColorUsesAccel);
  cvarUpdate->unsubscribe(&etj_accelSize);
}

void AccelMeterV2::startListeners() {
  cvarUpdate->subscribe(&etj_accelColor, [this](const vmCvar_t *cvar) {
    parseColor(cvar->string);
  });

  cvarUpdate->subscribe(&etj_accelAlpha, [this](const vmCvar_t *) {
    parseColor(etj_accelColor.string);
  });

  cvarUpdate->subscribe(&etj_accelSize,
                        [this](const vmCvar_t *cvar) { setSize(*cvar); });

  cvarUpdate->subscribe(&etj_accelColorUsesAccel, [this](const vmCvar_t *cvar) {
    setAccelColorStyle(*cvar);
  });
}

void AccelMeterV2::parseColor(const std::string &colorStr) {
  cgame.utils.colorParser->parseColorString(colorStr, color);
  color[3] = std::clamp(etj_accelAlpha.value, 0.0f, 1.0f);
}

void AccelMeterV2::setSize(const vmCvar_t &cvar) {
  size = CvarValueParser::parse<CvarValue::Size>(cvar, ACCELMETER_SIZE_MIN,
                                                 ACCELMETER_SIZE_MAX);
  size.x *= 0.1f;
  size.y *= 0.1f;

  halfW = static_cast<float>(
      CG_Text_Width_Ext("-88 ", size.x, 0, &cgs.media.limboFont1));
}

void AccelMeterV2::setAccelColorStyle(const vmCvar_t &cvar) {
  accelColorStyle =
      static_cast<AccelColorV2::Style>(std::clamp(cvar.integer, 0, 2));

  // re-parse the base color if we're turning off accel-based coloring
  if (accelColorStyle == AccelColorV2::Style::NONE) {
    parseColor(etj_accelColor.string);
  }
}

void AccelMeterV2::setupAccelColor(const PmoveUtilsV2::State &s,
                                   const float speed, const vec2_t accelVec) {
  if (accelColorStyle == AccelColorV2::Style::NONE) {
    return;
  }

  if (accelColorStyle == AccelColorV2::Style::SIMPLE ||
      AccelColorV2::lowSpeedOnGround(speed, s.pm.ps->groundEntityNum)) {
    Vector4Copy(colorGreen, color);

    float avgAccel = AccelColorV2::calcAvgAccel(storedSpeeds);

    if (avgAccel < 0) {
      Vector4Copy(colorRed, color);
      avgAccel = -avgAccel;
    }

    const float frac =
        std::min(avgAccel / AccelColorV2::ACCEL_FOR_SOLID_COLOR, 1.0f);

    LerpColor(colorWhite, color, color, frac);
  } else if (accelColorStyle == AccelColorV2::Style::ADVANCED) {
    if (accelVec[0] != 0 || accelVec[1] != 0) {
      AccelColorV2::calcAdvancedAccelColor(s.pm, s.pml, accelVec, s.wishspeed,
                                           s.wishvel, s.velAngle, s.optAngle,
                                           s.a, color);
    } else {
      Vector4Copy(colorWhite, color);
    }
  }

  color[3] = std::clamp(etj_accelAlpha.value, 0.0f, 1.0f);
}

bool AccelMeterV2::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  const PmoveUtilsV2::State &s = cgame.utils.pmoveV2->getState();

  // pps must be accurate in order to draw this accurately
  // TODO: revisit with 'g_synchronousClients'
  if (!PmoveUtilsV2::ppsIsAccurate(s.pm)) {
    return false;
  }

  // never lerp this, we want real acceleration values always
  if (PmoveUtilsV2::skipUpdate(lastUpdateTime, std::nullopt, s.pm)) {
    return true;
  }

  if (accelColorStyle == AccelColorV2::Style::SIMPLE ||
      (accelColorStyle == AccelColorV2::Style::ADVANCED) &&
          AccelColorV2::lowSpeedOnGround(s.vf, s.pm.ps->groundEntityNum)) {
    storedSpeeds.push_back({lastUpdateTime, s.vf});
    AccelColorV2::popOldStoredSpeeds(storedSpeeds, lastUpdateTime);
  } else if (!storedSpeeds.empty()) {
    storedSpeeds.clear();
  }

  Vector2Subtract(s.pm.ps->velocity, lastSpeed, accelVec);
  Vector2Copy(s.pm.ps->velocity, lastSpeed);

  // FIXME: this calls color parsing every frame when noclip is active
  if (s.pm.ps->pm_type == PM_NOCLIP) {
    Vector2Set(accelVec, 0, 0);

    if (accelColorStyle != AccelColorV2::Style::NONE) {
      parseColor("white");
    } else {
      parseColor(etj_accelColor.string);
    }
  } else {
    setupAccelColor(s, s.vf, accelVec);
  }

  x = std::clamp(etj_accelX.value, 0.0f, 640.0f);
  ETJump_AdjustPosition(&x);
  y = std::clamp(etj_accelY.value, 0.0f, static_cast<float>(SCREEN_HEIGHT));

  textStyle =
      etj_accelShadow.integer ? ITEM_TEXTSTYLE_SHADOWED : ITEM_TEXTSTYLE_NORMAL;

  accelStr[0] = StringUtils::format("%.0f", accelVec[0]);
  accelStr[1] = StringUtils::format("%.0f", accelVec[1]);

  return true;
}

void AccelMeterV2::render() const {
  switch (static_cast<Align>(etj_accelAlign.integer)) {
    case Align::LEFT:
      CG_Text_Paint_Ext(x, y, size.x, size.y, color, accelStr[0], 0, 0,
                        textStyle, &cgs.media.limboFont1);
      CG_Text_Paint_Ext(x + halfW, y, size.x, size.y, color, accelStr[1], 0, 0,
                        textStyle, &cgs.media.limboFont1);
      break;
    case Align::RIGHT:
      CG_Text_Paint_RightAligned_Ext(x, y, size.x, size.y, color, accelStr[0],
                                     0, 0, textStyle, &cgs.media.limboFont1);
      CG_Text_Paint_RightAligned_Ext(x - halfW, y, size.x, size.y, color,
                                     accelStr[1], 0, 0, textStyle,
                                     &cgs.media.limboFont1);
      break;
    case Align::CENTER:
    default:
      CG_Text_Paint_Centred_Ext(x - (halfW * 0.5f), y, size.x, size.y, color,
                                accelStr[0], 0, 0, textStyle,
                                &cgs.media.limboFont1);
      CG_Text_Paint_Centred_Ext(x + (halfW * 0.5f), y, size.x, size.y, color,
                                accelStr[1], 0, 0, textStyle,
                                &cgs.media.limboFont1);
      break;
  }
}

bool AccelMeterV2::canSkipDraw() {
  if (!etj_drawAccel.integer) {
    return true;
  }

  // TODO: revisit with 'g_synchronousClients'
  if (cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}

} // namespace ETJump
