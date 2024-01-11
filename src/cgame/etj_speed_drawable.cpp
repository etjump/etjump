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
#include "etj_cgaz.h"
#include "etj_snaphud.h"
#include <string>
#include "../game/etj_string_utilities.h"
#include "../game/etj_numeric_utilities.h"

constexpr int ACCEL_COLOR_SMOOTHING_TIME = 250;
constexpr float ACCEL_FOR_SOLID_COLOR = 100;

ETJump::DisplaySpeed::DisplaySpeed() {
  parseColor(etj_speedColor.string, _color);
  checkShadow();
  startListeners();
}

ETJump::DisplaySpeed::~DisplaySpeed() {}

void ETJump::DisplaySpeed::parseColor(const std::string &color, vec4_t &out) {
  parseColorString(color, out);
  out[3] *= etj_speedAlpha.value;
}

void ETJump::DisplaySpeed::startListeners() {
  cvarUpdateHandler->subscribe(&etj_speedColor, [&](const vmCvar_t *cvar) {
    parseColor(etj_speedColor.string, _color);
  });

  cvarUpdateHandler->subscribe(&etj_speedAlpha, [&](const vmCvar_t *cvar) {
    parseColor(etj_speedColor.string, _color);
  });

  cvarUpdateHandler->subscribe(&etj_speedShadow,
                               [&](const vmCvar_t *cvar) { checkShadow(); });

  consoleCommandsHandler->subscribe(
      "resetmaxspeed",
      [&](const std::vector<std::string> &args) { resetMaxSpeed(); });
}

bool ETJump::DisplaySpeed::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  const playerState_t &ps = cg.predictedPlayerState;

  // get usercmd
  // cmdScale is only checked here to be 0 or !0
  // so we can just use CMDSCALE_DEFAULT
  const int8_t ucmdScale = CMDSCALE_DEFAULT;
  const usercmd_t cmd = PmoveUtils::getUserCmd(ps, ucmdScale);

  pm = PmoveUtils::getPmove(cmd);

  float speedx = cg.predictedPlayerState.velocity[0];
  float speedy = cg.predictedPlayerState.velocity[1];

  auto speed = std::sqrt(speedx * speedx + speedy * speedy);

  // check if current frame should count towards strafe quality
  // we check for framerate dependency here by comparing current time
  // to last update time, using commandTime for clients for 100%
  // accuracy and cg.time for spectators/demos as an approximation note:
  // this will be wrong for clients running < 125FPS... oh well
  const auto frameTime = (cg.snap->ps.pm_flags & PMF_FOLLOW || cg.demoPlayback)
                             ? cg.time
                             : pm->ps->commandTime;

  _maxSpeed = speed > _maxSpeed ? speed : _maxSpeed;

  if (etj_speedColorUsesAccel.integer) {
    _storedSpeeds.push_back({cg.time, speed});

    popOldStoredSpeeds();
  } else if (!_storedSpeeds.empty()) {
    _storedSpeeds.clear();
  }

  if (canSkipUpdate(cmd, frameTime)) {
    return true;
  }

  _lastUpdateTime = frameTime;
  
  _accelx = static_cast<int>(speedx - _lastSpeed[0]);
  _accely = static_cast<int>(speedy - _lastSpeed[1]);

  Vector2Copy(cg.predictedPlayerState.velocity, _lastSpeed);

  return true;
}

void ETJump::DisplaySpeed::resetMaxSpeed() {
  _maxSpeed = 0;
  cg.resetmaxspeed = qtrue; // fix me
}

void ETJump::DisplaySpeed::checkShadow() {
  _shouldDrawShadow = etj_speedShadow.integer > 0 ? true : false;
}

void ETJump::DisplaySpeed::render() const {
  float size = 0.1f * etj_speedSize.value;
  float x = etj_speedX.integer;
  float y = etj_speedY.integer;
  ETJump_AdjustPosition(&x);

  auto status = getStatus();

  float w;
  switch (etj_speedAlign.integer) {
    case 1: // left align
      w = 0;
      break;
    case 2: // right align
      w = CG_Text_Width_Ext(status.c_str(), size, 0, &cgs.media.limboFont2);
      break;
    default: // center align
      w = CG_Text_Width_Ext(status.c_str(), size, 0, &cgs.media.limboFont2) / 2;
      break;
  }

  if (etj_drawSpeed2.integer == 8) {
    w = 0;
  }

  int style =
      _shouldDrawShadow ? ITEM_TEXTSTYLE_SHADOWED : ITEM_TEXTSTYLE_NORMAL;

  float speedx = cg.predictedPlayerState.velocity[0];
  float speedy = cg.predictedPlayerState.velocity[1];

  // fix me
  vec4_t color;

  if (etj_speedColorUsesAccel.integer == 1) {
    float accel = calcAvgAccel();
    float *accelColor = colorGreen;

    if (accel < 0) {
      accelColor = colorRed;
      accel = -accel;
    }

    float frac = accel / ACCEL_FOR_SOLID_COLOR;
    frac = std::min(frac, 1.f);

    LerpColor(colorWhite, accelColor, color, frac);
  } else if (etj_speedColorUsesAccel.integer != 0) {
    const playerState_t ps = cg.predictedPlayerState;
    const int8_t ucmdScale = CMDSCALE_DEFAULT;
    const usercmd_t cmd = PmoveUtils::getUserCmd(ps, ucmdScale);

    const float scale = PmoveUtils::PM_SprintScale(&ps);
    
    const float accelAngle = RAD2DEG(std::atan2(-cmd.rightmove, cmd.forwardmove));
    const float accelAngleAlt =
        RAD2DEG(std::atan2(cmd.rightmove, cmd.forwardmove));

    // max acceleration possible per frame
    const float frameAccel = CGaz::getFrameAccel(ps, pm);
    if (_accelx || _accely) {
      // get opt angles on both sides of velocity vector
      const float optAngle = CGaz::getOptAngle(ps, pm);
      const float altOptAngle = CGaz::getAltOptAngle(ps, pm);

      // get accels for opt angle
      const int optAccelx =
          static_cast<int>(std::round(frameAccel * cos(DEG2RAD(accelAngle + optAngle)) * scale));
      const int optAccely =
          static_cast<int>(std::round(frameAccel * sin(DEG2RAD(accelAngle + optAngle)) * scale));

      // get accels for alt angle
      const int altOptAccelx =
          static_cast<int>(std::round(frameAccel * cos(DEG2RAD(accelAngleAlt + altOptAngle)) * scale));
      const int altOptAccely =
          static_cast<int>(std::round(frameAccel * sin(DEG2RAD(accelAngleAlt + altOptAngle)) * scale));

      // find max accel possible between opt and altOpt angles
      int maxAccelx{0}, maxAccely{0};
      bool isMovingx = false;

      if (abs(speedx) > abs(speedy)) {
        // we're advancing on x axis
        isMovingx = true;
        if (abs(optAccelx) >= abs(altOptAccelx)) {
          maxAccelx = optAccelx;
          maxAccely = optAccely;
        } else {
          maxAccelx = altOptAccelx;
          maxAccely = altOptAccely;
        }
      } else {
        isMovingx = false;
        // we're advancing on y axis
        if (abs(optAccely) >= abs(altOptAccely)) {
          maxAccelx = optAccelx;
          maxAccely = optAccely;
        } else {
          maxAccelx = altOptAccelx;
          maxAccely = altOptAccely;
        }
      }

      // Generate the color based on the average normalized acceleration
      // Interpolate between red and green based on the average normalized
      // acceleration
      if (isMovingx) {
        float frac{0};

        if ((maxAccelx > 0 && _accelx >= 0) ||
            (maxAccelx < 0 && _accelx <= 0)) {
          frac = std::min(
              std::max(1.0f -
                           (abs(maxAccelx) - abs(_accelx + maxAccelx) / 2.0f) -
                           (abs(_accely) -
                            abs(std::max(abs(optAccely), abs(altOptAccely)))),
                       0.0f),
              1.0f);
        }

        LerpColor(colorRed, colorGreen, color, frac);
      } else {
        float frac{0};

        if ((maxAccely > 0 && _accely >= 0) ||
            (maxAccely < 0 && _accely <= 0)) {
          frac = std::min(
              std::max(1.0f -
                           (abs(maxAccely) - abs(_accely + maxAccely) / 2.0f) -
                           (abs(_accelx) -
                            abs(std::max(abs(optAccelx), abs(altOptAccelx)))),
                       0.0f),
              1.0f);
        }

        LerpColor(colorRed, colorGreen, color, frac);
      }
    } else {
      Vector4Copy(_color, color);
    }

    // we want a solid color all the time, no dark tints
    if (color[0] && color[1]) { // if we have a mix of R & G
      size_t maxColorIndex = color[0] > color[1] ? 0 : 1;
      float maxShade = 1.0f; // min value to show per color
      float coef = maxShade / color[maxColorIndex];
      VectorScale(color, coef, color);
    }

    // LerpColor adjusts alpha, make sure we still respect etj_speedAlpha
    color[3] = etj_speedAlpha.value;

  }

  CG_Text_Paint_Ext(x - w, y, size, size, color, status.c_str(), 0, 0, style,
                    &cgs.media.limboFont1);
}

std::string ETJump::DisplaySpeed::getStatus() const {
  float speed = sqrt(cg.predictedPlayerState.velocity[0] *
                         cg.predictedPlayerState.velocity[0] +
                     cg.predictedPlayerState.velocity[1] *
                         cg.predictedPlayerState.velocity[1]);

  switch (etj_drawSpeed2.integer) {
    case 2:
      return stringFormat("%.0f %.0f", speed, _maxSpeed);
    case 3:
      return stringFormat("%.0f ^z%.0f", speed, _maxSpeed);
    case 4:
      return stringFormat("%.0f (%.0f)", speed, _maxSpeed);
    case 5:
      return stringFormat("%.0f ^z(%.0f)", speed, _maxSpeed);
    case 6:
      return stringFormat("%.0f ^z[%.0f]", speed, _maxSpeed);
    case 7:
      return stringFormat("%.0f | %.0f", speed, _maxSpeed);
    case 8:
      return stringFormat("Speed: %.0f", speed);
    // tens
    case 9:
      return stringFormat("%02i", static_cast<int>(speed) / 10 % 10 * 10);
    case 10:
      return stringFormat("%3d %3d", _accelx, _accely);
    default:
      return stringFormat("%.0f", speed);
  }
}

bool ETJump::DisplaySpeed::canSkipDraw() const {
  return !etj_drawSpeed2.integer || ETJump::showingScores();
}

bool ETJump::DisplaySpeed::canSkipUpdate(usercmd_t cmd, int frameTime) const {
  // only count this frame if it's relevant to pmove
  // this makes sure that if clients FPS > 125
  // we only count frames at pmove_msec intervals
  if (!pm->ps || _lastUpdateTime + pm->pmove_msec > frameTime)
  {
    return true;
  }

  return false;
}

void ETJump::DisplaySpeed::popOldStoredSpeeds() {
  do {
    auto &front = _storedSpeeds.front();

    if (cg.time - front.time > ACCEL_COLOR_SMOOTHING_TIME) { // too old
      _storedSpeeds.pop_front();
      continue;
    } else if (cg.time < front.time) { // we went back in time!
      _storedSpeeds.pop_front();
      continue;
    }
  } while (false);
}

float ETJump::DisplaySpeed::calcAvgAccel() const {
  if (_storedSpeeds.size() < 2) { // need 2 speed points to compute acceleration
    return 0;
  }

  float totalSpeedDelta = 0;
  auto iter = _storedSpeeds.begin();
  for (auto prevIter = iter++; iter != _storedSpeeds.end(); prevIter = iter++) {
    totalSpeedDelta += iter->speed - prevIter->speed;
  }

  float timeDeltaMs = _storedSpeeds.back().time - _storedSpeeds.front().time;
  float accel = totalSpeedDelta / (timeDeltaMs / 1000.f);

  return accel;
}
