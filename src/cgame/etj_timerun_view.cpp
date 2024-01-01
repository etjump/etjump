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

#include <string>
#include <utility>

#include "cg_local.h"
#include "etj_timerun_view.h"
#include "etj_utilities.h"
#include "etj_cvar_update_handler.h"
#include "etj_player_events_handler.h"
#include "../game/etj_string_utilities.h"
#include "../game/etj_numeric_utilities.h"

ETJump::TimerunView::TimerunView(std::shared_ptr<Timerun> timerun)
    : Drawable(), _timerun(std::move(timerun)) {
  parseColorString(etj_runTimerInactiveColor.string, inactiveTimerColor);
  cvarUpdateHandler->subscribe(
      &etj_runTimerInactiveColor, [&](const vmCvar_t *cvar) {
        parseColorString(cvar->string, inactiveTimerColor);
      });
}

ETJump::TimerunView::~TimerunView() = default;

const ETJump::Timerun::PlayerTimerunInformation *
ETJump::TimerunView::currentRun() const {
  return _timerun->getTimerunInformationFor(cg.snap->ps.clientNum);
}

std::string ETJump::TimerunView::getTimerString(int msec) {
  if (msec < 0) {
    msec *= -1;
  }

  auto millis = msec;
  auto minutes = millis / 60000;
  millis -= minutes * 60000;
  auto seconds = millis / 1000;
  millis -= seconds * 1000;

  return stringFormat("%02d:%02d.%03d", minutes, seconds, millis);
}

void ETJump::TimerunView::draw() {
  if (canSkipDraw()) {
    return;
  }

  const auto run = currentRun();
  const int lastRunTimer = run->lastRunTimer;
  const bool running = run->running;
  const bool hasTimerun =
      (cg.demoPlayback && (lastRunTimer || running)) || cg.hasTimerun;

  if (!etj_drawRunTimer.integer || !hasTimerun) {
    return;
  }

  const int startTime = run->startTime;
  const auto font = &cgs.media.limboFont1;
  const bool autoHide = etj_runTimerAutoHide.integer;
  vec4_t *color = &colorDefault;

  // ensure correct 8ms interval timer when playing
  // specs/demo playback get approximation from cg.time, so timer stays smooth
  // one day this can maybe be real commandTime for all scenarios
  // if we get to sv_fps 125 servers...
  const int timeVar = (isPlaying(cg.clientNum) && !cg.demoPlayback)
                          ? cg.predictedPlayerState.commandTime
                          : cg.time;

  int millis;

  if (running) {
    millis = timeVar - startTime;
  } else {
    millis = run->completionTime;
    if (millis == -1) {
      millis = 0;
    }
    color = &inactiveTimerColor;
  }

  vec4_t colorTemp;
  const auto range = getTransitionRange(run->previousRecord);
  const int style = etj_runTimerShadow.integer ? ITEM_TEXTSTYLE_SHADOWED
                                               : ITEM_TEXTSTYLE_NORMAL;

  if (run->previousRecord > 0) {
    if (millis > run->previousRecord) {
      color = &colorFail;
    }
    // add timer color transition when player gets closer to their pb
    else if (millis + range >= run->previousRecord) {
      const int start = run->previousRecord - range;
      const auto step = static_cast<float>(millis - start) /
                        static_cast<float>(run->previousRecord - start);

      ETJump_LerpColors(&colorDefault, &colorFail, &colorTemp, step / 2);
      color = &colorTemp;
    }
  }

  // set green color for pb time
  if (!running && millis &&
      (run->previousRecord > millis || run->previousRecord == -1)) {
    color = &colorSuccess;
  }

  const int ms = millis;
  const int minutes = millis / 60000;
  millis -= minutes * 60000;
  const int seconds = millis / 1000;
  millis -= seconds * 1000;

  const std::string text =
      ETJump::stringFormat("%02d:%02d.%03d", minutes, seconds, millis);

  auto x = etj_runTimerX.value;
  auto y = etj_runTimerY.value;
  ETJump_AdjustPosition(&x);

  (*color)[3] =
      getTimerAlpha(running, autoHide, lastRunTimer + fadeHold, fadeTime);
  if ((*color)[3] == 0) {
    return;
  }

  CG_Text_Paint_Centred_Ext(x, y, 0.3f, 0.3f, *color, text, 0, 0, style, font);

  if ((etj_drawCheckpoints.integer || etj_checkpointsPopup.integer) &&
      run->runHasCheckpoints) {
    // only adjust x/y if we're drawing checkpoints detached from runtimer
    if (etj_drawCheckpoints.integer == 2) {
      x = etj_checkpointsX.value;
      y = etj_checkpointsY.value;
      ETJump_AdjustPosition(&x);
    } else {
      // position the times below runtimer
      y += 20;
    }

    const int popupTime =
        run->lastCheckpointTimestamp + etj_checkpointsPopupDuration.integer;
    const float popupSize = 0.1f * etj_checkpointsPopupSize.value;
    const int popupStyle = etj_checkpointsPopupShadow.integer
                               ? ITEM_TEXTSTYLE_SHADOWED
                               : ITEM_TEXTSTYLE_NORMAL;
    const float y2 = etj_checkpointsPopupY.value;
    float x2 = etj_checkpointsPopupX.value;
    ETJump_AdjustPosition(&x2);

    const int currentTime = running ? timeVar - startTime : run->completionTime;
    const float textSize = 0.1f * etj_checkpointsSize.value;
    const auto textStyle = etj_checkpointsShadow.integer
                               ? ITEM_TEXTSTYLE_SHADOWED
                               : ITEM_TEXTSTYLE_NORMAL;

    const int count = Numeric::clamp(etj_checkpointsCount.integer, 1, 5);
    const int startIndex = run->numCheckpointsHit;
    const int endIndex = run->numCheckpointsHit - count;
    const bool previousRecordSet = run->previousRecord >= 0;

    // do not render checkpoints if we're not running and
    // did not just complete a run
    if (!running && run->completionTime <= 0) {
      return;
    }

    for (int i = startIndex; i >= 0 && i > endIndex; i--) {
      const int checkpointTime = run->checkpoints[i];
      const bool maxCheckpointsHit = i == MAX_TIMERUN_CHECKPOINTS;

      // this will be either the previous best checkpoint time,
      // previous record or current run timer
      int comparisonTime;

      if (maxCheckpointsHit) {
        comparisonTime = previousRecordSet ? run->previousRecord : currentTime;
      } else if (run->previousRecordCheckpoints[i] ==
                 TIMERUN_CHECKPOINT_NOT_SET) {
        comparisonTime = run->previousRecord;
      } else {
        comparisonTime = run->previousRecordCheckpoints[i];
      }

      const bool noRecordCheckpoint =
          comparisonTime == TIMERUN_CHECKPOINT_NOT_SET;
      const bool noCheckpointTimeSet =
          checkpointTime == TIMERUN_CHECKPOINT_NOT_SET;

      // make sure we don't subtract -1 from timestamp if we don't
      // have a checkpoint time set at all
      const int relTimerOffset = noRecordCheckpoint ? 0 : comparisonTime;
      int relativeTime;
      if (maxCheckpointsHit) {
        relativeTime =
            previousRecordSet ? comparisonTime - currentTime : comparisonTime;
      } else {
        relativeTime = noCheckpointTimeSet ? currentTime - relTimerOffset
                                           : checkpointTime - relTimerOffset;
      }

      std::string dir;
      vec4_t *checkpointColor;
      // if we don't have a next checkpoint set, we can't compare to anything
      // so render checkpoint as white
      if (noRecordCheckpoint || relativeTime == 0 ||
          comparisonTime == currentTime) {
        checkpointColor = &colorDefault;
      } else {
        bool isFasterCheckpoint;
        // if we've hit max checkpoints, checkpointTime will be 0,
        // so we need to compare currentTime against the comparisonTime,
        // which will be previously set record in this scenario
        if (maxCheckpointsHit) {
          isFasterCheckpoint = currentTime < comparisonTime;
        } else {
          isFasterCheckpoint =
              (noCheckpointTimeSet ? currentTime : checkpointTime) <
              comparisonTime;
        }

        checkpointColor = isFasterCheckpoint ? &colorSuccess : &colorFail;
        dir = isFasterCheckpoint ? "-" : "+";
      }

      const int absoluteTime = noCheckpointTimeSet || maxCheckpointsHit
                                   ? currentTime
                                   : checkpointTime;

      std::string timerStr = getTimerString(
          etj_checkpointsStyle.integer == 1 ? absoluteTime : relativeTime);

      timerStr.insert(0, dir);

      // we do not need to check for alpha being 0 here because checkpoints
      // follow same fading as runtimer, and if that is 0, we early out
      // before ever reaching this part
      (*checkpointColor)[3] =
          getTimerAlpha(running, autoHide, lastRunTimer + fadeHold, fadeTime);

      // we must check for cvar here to allow only checkpoint popups to display
      if (etj_drawCheckpoints.integer) {
        CG_Text_Paint_Centred_Ext(x, y, textSize, textSize, *checkpointColor,
                                  timerStr, 0, 0, textStyle, font);
      }

      if (etj_checkpointsPopup.integer && i == startIndex - 1) {
        vec4_t cpPopupColor;
        Vector4Copy(*checkpointColor, cpPopupColor);

        if (popupTime >= cg.time) {
          CG_Text_Paint_Centred_Ext(x2, y2, popupSize, popupSize, cpPopupColor,
                                    timerStr, 0, 0, popupStyle, font);
        } else if (popupTime + popupFadeTime > cg.time) {
          // we want to always fade here so bypass running/autoHide
          cpPopupColor[3] =
              getTimerAlpha(false, true, popupTime, popupFadeTime);

          CG_Text_Paint_Centred_Ext(x2, y2, popupSize, popupSize, cpPopupColor,
                                    timerStr, 0, 0, popupStyle, font);
        }
      }

      y += static_cast<float>(2 *
                              CG_Text_Height_Ext(timerStr, textSize, 0, font));
    }
  }

  if (running && run->previousRecord != -1 && ms > run->previousRecord) {
    pastRecordAnimation(color, text.c_str(), ms, run->previousRecord);
  }
}

float ETJump::TimerunView::getTimerAlpha(bool running, bool autoHide,
                                         int fadeStart, int duration) {
  const int fadeEnd = fadeStart + duration;
  const int now = cg.time;

  if (!running && autoHide) {
    if (fadeStart < now && fadeEnd > now) {
      return 1.0f -
             static_cast<float>(now - fadeStart) / static_cast<float>(duration);
    } else if (now >= fadeEnd) {
      return 0.0f;
    }
  }

  return 1.0f;
}

int ETJump::TimerunView::getTransitionRange(int previousTime) {
  const int range = previousTime / 10;

  // for very long runs/splits, cap the max transition range to 10s
  return range <= 10000 ? range : 10000;
}

void ETJump::TimerunView::pastRecordAnimation(vec4_t *color, const char *text,
                                              int timerTime, int record) {
  if (timerTime - record > animationTime) {
    return;
  }

  vec4_t toColor;
  vec4_t inColor;

  auto x = etj_runTimerX.value;
  const auto y = etj_runTimerY.value;

  ETJump_AdjustPosition(&x);

  const auto step = static_cast<float>(timerTime - record) /
                    static_cast<float>(animationTime);
  const auto scale = 0.3f + 0.25f * step;

  const auto originalTextHeight =
      CG_Text_Height_Ext(text, 0.3f, 0, &cgs.media.limboFont1);
  const auto textHeight =
      static_cast<float>(
          CG_Text_Height_Ext(text, scale, 0, &cgs.media.limboFont1) -
          originalTextHeight) *
      0.5f;

  memcpy(&toColor, color, sizeof(toColor));
  toColor[3] = 0;

  ETJump_LerpColors(color, &toColor, &inColor, step);

  CG_Text_Paint_Centred_Ext(x, y + textHeight, scale, scale, inColor, text, 0,
                            0, 0, &cgs.media.limboFont1);
}

bool ETJump::TimerunView::canSkipDraw() { return showingScores(); }
