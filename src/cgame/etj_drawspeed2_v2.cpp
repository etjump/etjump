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

#include "etj_drawspeed2_v2.h"
#include "cg_local.h"
#include "etj_client_commands_handler.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_pmove_utils_v2.h"
#include "etj_utilities.h"

namespace ETJump {
inline constexpr float SPEED_SIZE_MIN = 0.1f;
inline constexpr float SPEED_SIZE_MAX = 10.0f;

DrawSpeed2::DrawSpeed2(
    const std::shared_ptr<CvarUpdateHandler> &cvarUpdate,
    const std::shared_ptr<ClientCommandsHandler> &consoleCommands)
    : cvarUpdate(cvarUpdate), consoleCommands(consoleCommands) {
  parseColor(etj_speedColor.string);
  setAccelColorStyle(etj_speedColorUsesAccel);
  setSize(etj_speedSize);
  startListeners();
}

DrawSpeed2::~DrawSpeed2() {
  cvarUpdate->unsubscribe(&etj_speedColor);
  cvarUpdate->unsubscribe(&etj_speedAlpha);
  cvarUpdate->unsubscribe(&etj_speedSize);
  cvarUpdate->unsubscribe(&etj_speedColorUsesAccel);

  consoleCommands->unsubscribe("resetmaxspeed");
}

void DrawSpeed2::startListeners() {
  cvarUpdate->subscribe(&etj_speedColor, [this](const vmCvar_t *cvar) {
    parseColor(cvar->string);
  });

  cvarUpdate->subscribe(&etj_speedAlpha, [this](const vmCvar_t *) {
    parseColor(etj_speedColor.string);
  });

  cvarUpdate->subscribe(&etj_speedSize,
                        [this](const vmCvar_t *cvar) { setSize(*cvar); });

  cvarUpdate->subscribe(&etj_speedColorUsesAccel, [this](const vmCvar_t *cvar) {
    setAccelColorStyle(*cvar);
  });

  consoleCommands->subscribe("resetmaxspeed",
                             [this](const std::vector<std::string> &) {
                               maxSpeed = 0;
                               // FIXME: this is for the regular speed meter
                               cg.resetmaxspeed = qtrue;
                             });
}

void DrawSpeed2::parseColor(const std::string &colorStr) {
  cgame.utils.colorParser->parseColorString(colorStr, color);
  color[3] = std::clamp(etj_speedAlpha.value, 0.0f, 1.0f);
}

void DrawSpeed2::setSize(const vmCvar_t &cvar) {
  size = CvarValueParser::parse<CvarValue::Size>(cvar, SPEED_SIZE_MIN,
                                                 SPEED_SIZE_MAX);
  size.x *= 0.1f;
  size.y *= 0.1f;
}

void DrawSpeed2::setAccelColorStyle(const vmCvar_t &cvar) {
  accelColorStyle =
      static_cast<AccelColorV2::Style>(std::clamp(cvar.integer, 0, 2));

  // re-parse the base color if we're turning off accel-based coloring
  if (accelColorStyle == AccelColorV2::Style::NONE) {
    parseColor(etj_speedColor.string);
  }
}

void DrawSpeed2::setupAccelColor(const PmoveUtilsV2::State &s,
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

  color[3] = std::clamp(etj_speedAlpha.value, 0.0f, 1.0f);
}

std::string DrawSpeed2::getSpeedString() const {
  switch (static_cast<Style>(etj_drawSpeed2.integer)) {
    case Style::SPEED_MAX:
      return StringUtils::format("%.0f %.0f", currentSpeed, maxSpeed);
    case Style::SPEED_MAX_COLOR:
      return StringUtils::format("%.0f ^z%.0f", currentSpeed, maxSpeed);
    case Style::SPEED_MAX_PARENTHESIS:
      return StringUtils::format("%.0f (%.0f)", currentSpeed, maxSpeed);
    case Style::SPEED_MAX_COLOR_PARENTHESIS:
      return StringUtils::format("%.0f ^z(%.0f)", currentSpeed, maxSpeed);
    case Style::SPEED_MAX_COLOR_BRACKETS:
      return StringUtils::format("%.0f ^z[%.0f]", currentSpeed, maxSpeed);
    case Style::SPEED_MAX_PIPE:
      return StringUtils::format("%.0f | %.0f", currentSpeed, maxSpeed);
    case Style::SPEED_TEXT:
      return StringUtils::format("Speed: %.0f", currentSpeed);
    case Style::SPEED_TENS:
      return StringUtils::format("%02i",
                                 static_cast<int>(currentSpeed) / 10 % 10 * 10);
    default:
      return StringUtils::format("%.0f", currentSpeed);
  }
}

bool DrawSpeed2::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  const PmoveUtilsV2::State &s = cgame.utils.pmoveV2->getState();

  // FIXME: TEMP SOLUTION - only correct at sv_fps/snaps 125 for now
  lastUpdateTime = cg.snap->serverTime;

  // store the old last update time so we can re-check it accel coloring
  int32_t oldLastUpdateTime = lastUpdateTime;

  currentSpeed = VectorLength2(s.pm.ps->velocity);
  maxSpeed = std::max(currentSpeed, maxSpeed);

  Vector2Subtract(s.pm.ps->velocity, lastSpeed, accelVec);
  Vector2Copy(s.pm.ps->velocity, lastSpeed);

  // spectators cannot use advanced accel coloring
  // because velocity isn't snapped
  // TODO: revisit with 'g_synchronousClients'
  if (s.pm.ps->clientNum != cg.clientNum &&
      accelColorStyle == AccelColorV2::Style::ADVANCED) {
    accelColorStyle = AccelColorV2::Style::SIMPLE;
  }

  if (accelColorStyle == AccelColorV2::Style::SIMPLE ||
      accelColorStyle == AccelColorV2::Style::ADVANCED &&
          AccelColorV2::lowSpeedOnGround(currentSpeed,
                                         s.pm.ps->groundEntityNum)) {
    storedSpeeds.push_back({lastUpdateTime, currentSpeed});
    AccelColorV2::popOldStoredSpeeds(storedSpeeds, lastUpdateTime);
  } else if (!storedSpeeds.empty()) {
    storedSpeeds.clear();
  }

  // FIXME: this calls color parsing every frame when noclip is active
  if (s.pm.ps->pm_type == PM_NOCLIP) {
    if (accelColorStyle != AccelColorV2::Style::NONE) {
      parseColor("white");
    } else {
      parseColor(etj_speedColor.string);
    }
  } else {
    // FIXME: should not be called every frame - only works correctly at 125fps
    setupAccelColor(s, currentSpeed, accelVec);
  }

  speedStr = getSpeedString();
  textStyle =
      etj_speedShadow.integer ? ITEM_TEXTSTYLE_SHADOWED : ITEM_TEXTSTYLE_NORMAL;

  x = std::clamp(etj_speedX.value, 0.0f, 640.0f);
  ETJump_AdjustPosition(&x);
  y = std::clamp(etj_speedY.value, 0.0f, static_cast<float>(SCREEN_HEIGHT));

  // TODO: A/B testing, remove
  y += 15;

  return true;
}

void DrawSpeed2::render() const {
  switch (static_cast<Align>(etj_speedAlign.integer)) {
    case Align::LEFT:
      CG_Text_Paint_Ext(x, y, size.x, size.y, color, speedStr, 0, 0, textStyle,
                        &cgs.media.limboFont1);
      break;
    case Align::RIGHT:
      CG_Text_Paint_RightAligned_Ext(x, y, size.x, size.y, color, speedStr, 0,
                                     0, textStyle, &cgs.media.limboFont1);
      break;
    default:
      CG_Text_Paint_Centred_Ext(x, y, size.x, size.y, color, speedStr, 0, 0,
                                textStyle, &cgs.media.limboFont1);
      break;
  }
}

bool DrawSpeed2::canSkipDraw() {
  if (!etj_drawSpeed2.integer) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
