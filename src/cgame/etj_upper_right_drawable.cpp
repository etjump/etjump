/*
 * MIT License
 *
 * Copyright (c) 2022 ETJump team <zero@etjump.com>
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

#include "etj_upper_right_drawable.h"
#include "../game/etj_time_utilities.h"

ETJump::UpperRight::UpperRight() {
  FPSIndex = FPSInit = 0;
  FPSLastUpdate = FPSLastTime = fps = 0;
}

void ETJump::UpperRight::beforeRender() {
  // FPS meter
  const long long currentTime = getCurrentTimestamp();
  if (FPSInit < FPSFrames + 1) {
    ++FPSInit;
  }

  // skip first frame because dealing with deltas
  // and run up to FPSFrames+1 instead
  if (FPSInit != 1) {
    FPSFrameTimes[FPSIndex] = currentTime - FPSLastTime;
    FPSIndex = (FPSIndex + 1) % FPSFrames;

    if (currentTime - FPSLastUpdate >= etj_FPSMeterUpdateInterval.integer) {
      FPSLastUpdate = currentTime;
      fps = 0;
      for (std::size_t i = 0; i < FPSInit - 1; ++i) {
        fps += FPSFrameTimes[i];
      }
      fps = static_cast<double>(FPSInit - 1) / fps * 1000;
    }
  }

  FPSLastTime = currentTime;
}

void ETJump::UpperRight::render() const {
  if (cg_paused.integer || ETJump::showingScores()) {
    return;
  }

  if (etj_HUD_fireteam.integer && CG_IsOnFireteam(cg.clientNum)) {
    rectDef_t rect = {10, 10, 100, 100};
    CG_DrawFireTeamOverlay(&rect);
  }

  if (!(cg.snap->ps.pm_flags & PMF_LIMBO) &&
      (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR) &&
      (cgs.autoMapExpanded ||
       (!cgs.autoMapExpanded && (cg.time - cgs.autoMapExpandTime < 250.f)))) {
    return;
  }

  float y = y0;
  if (cg_drawRoundTimer.integer) {
    DrawTimer(y);
  }

  if (etj_drawClock.integer) {
    DrawTime(y);
  }

  if (cg_drawFPS.integer) {
    DrawFPS(y);
  }

  if (etj_drawspeed.integer) {
    DrawSpeed(y);
  }

  if (cg_drawSnapshot.integer) {
    DrawSnapshot(y);
  }
}

void ETJump::UpperRight::DrawTimer(float &y) const {
  const int msec = cg.time - cgs.levelStartTime;
  auto time = ETJump::toClock(msec, true);

  const char *s = time.hours > 0
                      ? va("%02d:%02d:%02d", time.hours, time.min, time.sec)
                      : va("%02d:%02d", time.min, time.sec);
  const int w = CG_Text_Width_Ext(s, textScale, 0, &cgs.media.limboFont1);

  CG_FillRect(upperRight_x - w - 2, y, w + 5, 12 + 2, backgroundColor);
  CG_DrawRect_FixedBorder(upperRight_x - w - 2, y, w + 5, 12 + 2, 1,
                          borderColor);
  CG_Text_Paint_Ext(upperRight_x - w, y + 11, textScale, textScale, textColor,
                    s, 0, 0, 0, &cgs.media.limboFont1);

  y += 16;
}

void ETJump::UpperRight::DrawTime(float &y) const {
  char displayTime[12];
  qtime_t tm;

  trap_RealTime(&tm);
  displayTime[0] = '\0';

  if (etj_drawClock.integer == 1) {
    Q_strcat(displayTime, sizeof(displayTime),
             va("%d:%02d", tm.tm_hour, tm.tm_min));
    Q_strcat(displayTime, sizeof(displayTime), va(":%02d", tm.tm_sec));
  } else {
    Q_strcat(displayTime, sizeof(displayTime),
             va("%d:%02d",
                ((tm.tm_hour == 0 || tm.tm_hour == 12) ? 12 : tm.tm_hour % 12),
                tm.tm_min));
    Q_strcat(displayTime, sizeof(displayTime), va(":%02d", tm.tm_sec));
    Q_strcat(displayTime, sizeof(displayTime),
             (tm.tm_hour < 12) ? " am" : " pm");
  }

  const int w =
      CG_Text_Width_Ext(displayTime, textScale, 0, &cgs.media.limboFont1);

  CG_FillRect(upperRight_x - w - 2, y, w + 5, 12 + 2, backgroundColor);
  CG_DrawRect_FixedBorder(upperRight_x - w - 2, y, w + 5, 12 + 2, 1,
                          borderColor);
  CG_Text_Paint_Ext(upperRight_x - w, y + 11, textScale, textScale, textColor,
                    displayTime, 0, 0, 0, &cgs.media.limboFont1);

  y += 16;
}

void ETJump::UpperRight::DrawFPS(float &y) const {
  const std::string s = std::to_string(fps) + " FPS";
  const int w =
      CG_Text_Width_Ext(s.c_str(), textScale, 0, &cgs.media.limboFont1);

  CG_FillRect(upperRight_x - w - 2, y, w + 5, 12 + 2, backgroundColor);
  CG_DrawRect_FixedBorder(upperRight_x - w - 2, y, w + 5, 12 + 2, 1,
                          borderColor);
  CG_Text_Paint_Ext(upperRight_x - w, y + 11, textScale, textScale, textColor,
                    s.c_str(), 0, 0, 0, &cgs.media.limboFont1);

  y += 16;
}

void ETJump::UpperRight::DrawSpeed(float &y) const {
#define SPEED_US_TO_KPH 14.56f
#define SPEED_US_TO_MPH 23.44f
  static vec_t highestSpeed, speed;
  static int lasttime;

  if (cg.resetmaxspeed) {
    highestSpeed = 0;
    cg.resetmaxspeed = qfalse;
  }

  const int thistime = trap_Milliseconds();

  if (thistime > (lasttime + etj_speedinterval.integer)) {

    if (etj_speedXYonly.integer) {
      speed = sqrt(cg.predictedPlayerState.velocity[0] *
                       cg.predictedPlayerState.velocity[0] +
                   cg.predictedPlayerState.velocity[1] *
                       cg.predictedPlayerState.velocity[1]);
    } else {
      speed = VectorLength(cg.predictedPlayerState.velocity);
    }

    if (speed > highestSpeed) {
      highestSpeed = speed;
    }

    lasttime = thistime;
  }

  char *s;
  switch (etj_drawspeed.integer) {
    default:
    case 1:
      // kw: swapped 1 and 2 to match etpro's
      // b_speedunit
      switch (etj_speedunit.integer) {
        default:
        case 0:
          // Units per second
          s = va("%.1f UPS", speed);
          break;
        case 1:
          // Miles per hour
          s = va("%.1f MPH", (speed / SPEED_US_TO_MPH));
          break;
        case 2:
          // Kilometers per hour
          s = va("%.1f KPH", (speed / SPEED_US_TO_KPH));
          break;
      }
      break;

    case 2:
      switch (etj_speedunit.integer) {
        default:
        case 0:
          // Units per second
          s = va("%.1f UPS (%.1f MAX)", speed, highestSpeed);
          break;
        case 1:
          // Miles per hour
          s = va("%.1f MPH (%.1f MAX)", (speed / SPEED_US_TO_MPH),
                 (highestSpeed / SPEED_US_TO_MPH));
          break;
        case 2:
          // Kilometers per hour
          s = va("%.1f KPH (%.1f MAX)", (speed / SPEED_US_TO_KPH),
                 (highestSpeed / SPEED_US_TO_KPH));
          break;
      }
      break;
  }

  const int w = CG_Text_Width_Ext(s, textScale, 0, &cgs.media.limboFont1);

  CG_FillRect(upperRight_x - w - 2, y, w + 5, 12 + 2, backgroundColor);
  CG_DrawRect_FixedBorder(upperRight_x - w - 2, y, w + 5, 12 + 2, 1,
                          borderColor);
  CG_Text_Paint_Ext(upperRight_x - w, y + 11, textScale, textScale, textColor,
                    s, 0, 0, 0, &cgs.media.limboFont1);

  y += 16;
}

void ETJump::UpperRight::DrawSnapshot(float &y) const {
  const char *s = va("time:%i snap:%i cmd:%i", cg.snap->serverTime,
                     cg.latestSnapshotNum, cgs.serverCommandSequence);
  const int w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;

  CG_DrawBigString(upperRight_x - w, y + 2, s, 1.0F);

  y += BIGCHAR_HEIGHT + 4;
}
