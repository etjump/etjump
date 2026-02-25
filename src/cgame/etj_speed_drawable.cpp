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

#include "etj_speed_drawable.h"
#include "etj_local.h"
#include "etj_utilities.h"

#include "../game/etj_string_utilities.h"

namespace ETJump {

DrawSpeed::DrawSpeed(
    const std::shared_ptr<CvarUpdateHandler> &cvarUpdate,
    const std::shared_ptr<ClientCommandsHandler> &consoleCommands)
    : cvarUpdate(cvarUpdate), consoleCommands(consoleCommands) {
  parseColor(etj_speedColor.string, speedColor);
  setTextStyle(&etj_speedShadow);
  setAccelColorStyle(&etj_speedColorUsesAccel);
  setSize(&etj_speedSize);
  startListeners();
}

DrawSpeed::~DrawSpeed() {
  cvarUpdate->unsubscribe(&etj_speedColor);
  cvarUpdate->unsubscribe(&etj_speedAlpha);
  cvarUpdate->unsubscribe(&etj_speedSize);
  cvarUpdate->unsubscribe(&etj_speedShadow);
  cvarUpdate->unsubscribe(&etj_speedColorUsesAccel);

  consoleCommands->unsubscribe("resetmaxspeed");
}

void DrawSpeed::parseColor(const std::string &color, vec4_t &out) {
  cgame.utils.colorParser->parseColorString(color, out);
  out[3] *= etj_speedAlpha.value;
}

void DrawSpeed::startListeners() {
  cvarUpdate->subscribe(&etj_speedColor, [this](const vmCvar_t *cvar) {
    parseColor(cvar->string, speedColor);
  });

  cvarUpdate->subscribe(&etj_speedAlpha, [this](const vmCvar_t *) {
    parseColor(etj_speedColor.string, speedColor);
  });

  cvarUpdate->subscribe(&etj_speedSize,
                        [this](const vmCvar_t *cvar) { setSize(cvar); });

  cvarUpdate->subscribe(&etj_speedShadow,
                        [this](const vmCvar_t *cvar) { setTextStyle(cvar); });

  cvarUpdate->subscribe(&etj_speedColorUsesAccel, [this](const vmCvar_t *cvar) {
    setAccelColorStyle(cvar);
  });

  consoleCommands->subscribe(
      "resetmaxspeed",
      [this](const std::vector<std::string> &) { resetMaxSpeed(); });
}

bool DrawSpeed::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  playing = cg.snap->ps.clientNum == cg.clientNum && !cg.demoPlayback;

  pm = cgame.utils.pmove->getPmove();

  // store the old last update time so we can re-check it accel coloring
  int oldLastUpdateTime = lastUpdateTime;

  if (cgame.utils.pmove->skipUpdate(lastUpdateTime, HUDLerpFlags::DRAWSPEED2)) {
    return true;
  }

  currentSpeed = VectorLength2(ps->velocity);
  maxSpeed = currentSpeed > maxSpeed ? currentSpeed : maxSpeed;

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

  if (accelColorStyle &&
      !cgame.utils.pmove->skipUpdate(oldLastUpdateTime, std::nullopt)) {
    AccelColor::setAccelColor(style, currentSpeed, etj_speedAlpha.value, pm, ps,
                              storedSpeeds, accel, speedColor);
  }

  speedStr = getSpeedString();
  y = etj_speedY.value;

  // need to calculate this every frame because speed string changes
  switch (etj_speedAlign.integer) {
    case Left:
      w = 0;
      break;
    case Right:
      w = static_cast<float>(
          CG_Text_Width_Ext(speedStr, size.x, 0, &cgs.media.limboFont2));
      break;
    default: // center align
      w = static_cast<float>(
              CG_Text_Width_Ext(speedStr, size.x, 0, &cgs.media.limboFont2)) *
          0.5f;
      break;
  }

  return true;
}

void DrawSpeed::render() const {
  float speedX = etj_speedX.value;
  ETJump_AdjustPosition(&speedX);

  vec4_t color;
  Vector4Copy(speedColor, color);

  CG_Text_Paint_Ext(speedX - w, y, size.x, size.y, color, speedStr, 0, 0,
                    textStyle, &cgs.media.limboFont1);
}

void DrawSpeed::resetMaxSpeed() {
  maxSpeed = 0;
  cg.resetmaxspeed = qtrue; // fix me
}

void DrawSpeed::setTextStyle(const vmCvar_t *cvar) {
  textStyle = cvar->integer ? ITEM_TEXTSTYLE_SHADOWED : ITEM_TEXTSTYLE_NORMAL;
}

void DrawSpeed::setAccelColorStyle(const vmCvar_t *cvar) {
  if (!cvar->integer) {
    parseColor(etj_speedColor.string, speedColor);
  }

  accelColorStyle = cvar->integer;
}

void DrawSpeed::setSize(const vmCvar_t *cvar) {
  size = CvarValueParser::parse<CvarValue::Size>(*cvar, 1, 10);
  size.x *= 0.1f;
  size.y *= 0.1f;
}

std::string DrawSpeed::getSpeedString() const {
  switch (etj_drawSpeed2.integer) {
    case 2:
      return stringFormat("%.0f %.0f", currentSpeed, maxSpeed);
    case 3:
      return stringFormat("%.0f ^z%.0f", currentSpeed, maxSpeed);
    case 4:
      return stringFormat("%.0f (%.0f)", currentSpeed, maxSpeed);
    case 5:
      return stringFormat("%.0f ^z(%.0f)", currentSpeed, maxSpeed);
    case 6:
      return stringFormat("%.0f ^z[%.0f]", currentSpeed, maxSpeed);
    case 7:
      return stringFormat("%.0f | %.0f", currentSpeed, maxSpeed);
    case 8:
      return stringFormat("Speed: %.0f", currentSpeed);
    // tens
    case 9:
      return stringFormat("%02i",
                          static_cast<int>(currentSpeed) / 10 % 10 * 10);
    default:
      return stringFormat("%.0f", currentSpeed);
  }
}

bool DrawSpeed::canSkipDraw() {
  if (!etj_drawSpeed2.integer) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
