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

#include "etj_speed_drawable.h"
#include "etj_utilities.h"
#include "etj_cvar_update_handler.h"
#include "etj_client_commands_handler.h"
#include "etj_pmove_utils.h"
#include "../game/etj_string_utilities.h"

namespace ETJump {

DrawSpeed::DrawSpeed() {
  parseColor(etj_speedColor.string, speedColor);
  setTextStyle();
  setAccelColorStyle();
  setSize();
  startListeners();
}

DrawSpeed::~DrawSpeed() {
  consoleCommandsHandler->unsubscribe("resetmaxspeed");
}

void DrawSpeed::parseColor(const std::string &color, vec4_t &out) {
  parseColorString(color, out);
  out[3] *= etj_speedAlpha.value;
}

void DrawSpeed::startListeners() {
  cvarUpdateHandler->subscribe(&etj_speedColor, [&](const vmCvar_t *cvar) {
    parseColor(etj_speedColor.string, speedColor);
  });

  cvarUpdateHandler->subscribe(&etj_speedAlpha, [&](const vmCvar_t *cvar) {
    parseColor(etj_speedColor.string, speedColor);
  });

  cvarUpdateHandler->subscribe(&etj_speedSize,
                               [&](const vmCvar_t *cvar) { setSize(); });

  cvarUpdateHandler->subscribe(&etj_speedShadow,
                               [&](const vmCvar_t *cvar) { setTextStyle(); });

  cvarUpdateHandler->subscribe(
      &etj_speedColorUsesAccel,
      [&](const vmCvar_t *cvar) { setAccelColorStyle(); });

  consoleCommandsHandler->subscribe(
      "resetmaxspeed",
      [&](const std::vector<std::string> &args) { resetMaxSpeed(); });
}

bool DrawSpeed::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  playing = cg.snap->ps.clientNum == cg.clientNum && !cg.demoPlayback;

  const playerState_t *ps = playing ? &cg.predictedPlayerState : &cg.snap->ps;
  const int8_t ucmdScale = CMDSCALE_DEFAULT;
  const usercmd_t cmd = PmoveUtils::getUserCmd(*ps, ucmdScale);

  pm = PmoveUtils::getPmove(cmd);

  // check if current frame should update speed meter
  // we check for framerate dependency here by comparing current time
  // to last update time, using commandTime for clients for 100%
  // accuracy and cg.time for spectators/demos as an approximation note:
  // this will be wrong for clients running < 125FPS... oh well
  const int frameTime = (cg.snap->ps.pm_flags & PMF_FOLLOW || cg.demoPlayback)
                            ? cg.time
                            : pm->ps->commandTime;

  if (canSkipUpdate(frameTime)) {
    return true;
  }

  currentSpeed = VectorLength2(pm->ps->velocity);
  lastUpdateTime = frameTime;
  maxSpeed = currentSpeed > maxSpeed ? currentSpeed : maxSpeed;

  if (accelColorStyle == AccelColor::Style::Simple ||
      (accelColorStyle == AccelColor::Style::Advanced &&
       AccelColor::lowSpeedOnGround(currentSpeed, pm->ps->groundEntityNum))) {
    storedSpeeds.push_back({frameTime, currentSpeed});
    AccelColor::popOldStoredSpeeds(storedSpeeds, frameTime);
  } else if (!storedSpeeds.empty()) {
    storedSpeeds.clear();
  }

  VectorSubtract(pm->ps->velocity, lastSpeed, accel);
  VectorCopy(pm->ps->velocity, lastSpeed);

  if (accelColorStyle) {
    if (!playing) {
      accelColorStyle = AccelColor::Style::Simple;
    }

    AccelColor::setAccelColor(accelColorStyle, currentSpeed,
                              etj_speedAlpha.value, pm, storedSpeeds, accel,
                              speedColor);
  }

  speedStr = getSpeedString();
  y = etj_speedY.value;

  // need to calculate this every frame because speed string changes
  switch (etj_speedAlign.integer) {
    case Alignment::Left:
      w = 0;
      break;
    case Alignment::Right:
      w = static_cast<float>(
          CG_Text_Width_Ext(speedStr, size, 0, &cgs.media.limboFont2));
      break;
    default: // center align
      w = static_cast<float>(
              CG_Text_Width_Ext(speedStr, size, 0, &cgs.media.limboFont2)) *
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

  CG_Text_Paint_Ext(speedX - w, y, size, size, color, speedStr, 0, 0, textStyle,
                    &cgs.media.limboFont1);
}

void DrawSpeed::resetMaxSpeed() {
  maxSpeed = 0;
  cg.resetmaxspeed = qtrue; // fix me
}

void DrawSpeed::setTextStyle() {
  textStyle =
      etj_speedShadow.integer ? ITEM_TEXTSTYLE_SHADOWED : ITEM_TEXTSTYLE_NORMAL;
}

void DrawSpeed::setAccelColorStyle() {
  if (!etj_speedColorUsesAccel.integer) {
    parseColor(etj_speedColor.string, speedColor);
  }

  accelColorStyle = etj_speedColorUsesAccel.integer;
}

void DrawSpeed::setSize() { size = 0.1f * etj_speedSize.value; }

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

bool DrawSpeed::canSkipUpdate(int frameTime) const {
  // only count this frame if it's relevant to pmove
  // this makes sure that if clients FPS > 125
  // we only count frames at pmove_msec intervals
  if (!pm->ps || lastUpdateTime + pm->pmove_msec > frameTime) {
    return true;
  }

  return false;
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
