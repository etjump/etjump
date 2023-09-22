/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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

  auto run = currentRun();
  auto hasTimerun =
      (cg.demoPlayback && (run->lastRunTimer || run->running)) || cg.hasTimerun;

  if (!etj_drawRunTimer.integer || !hasTimerun) {
    return;
  }

  auto startTime = run->startTime;
  auto millis = 0;
  auto color = &colorWhite;
  const auto font = &cgs.media.limboFont1;

  // ensure correct 8ms interval timer when playing
  // specs/demo playback get approximation from cg.time, so timer stays smooth
  // one day this can maybe be real commandTime for all scenarios
  // if we get to sv_fps 125 servers...
  const int timeVar = (isPlaying(cg.clientNum) && !cg.demoPlayback)
                          ? cg.predictedPlayerState.commandTime
                          : cg.time;

  if (run->running) {
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
  const auto style = etj_runTimerShadow.integer ? ITEM_TEXTSTYLE_SHADOWED
                                                : ITEM_TEXTSTYLE_NORMAL;

  if (run->previousRecord > 0) {
    if (millis > run->previousRecord) {
      color = &colorFail;
    }
    // add timer color transition when player gets closer to their pb
    else if (millis + range >= run->previousRecord) {
      auto start = run->previousRecord - range;
      auto step = static_cast<float>(millis - start) /
                  static_cast<float>(run->previousRecord - start);

      ETJump_LerpColors(&colorWhite, &colorFail, &colorTemp, step / 2);
      color = &colorTemp;
    }
  }

  // set green color for pb time
  if (!run->running && millis &&
      (run->previousRecord > millis || run->previousRecord == -1)) {
    color = &colorSuccess;
  }

  auto ms = millis;
  auto minutes = millis / 60000;
  millis -= minutes * 60000;
  auto seconds = millis / 1000;
  millis -= seconds * 1000;

  auto text = ETJump::stringFormat("%02d:%02d.%03d", minutes, seconds, millis);
  auto x = etj_runTimerX.value;
  auto y = etj_runTimerY.value;

  // timer fading/hiding routine
  ETJump_AdjustPosition(&x);

  (*color)[3] = getTimerAlpha(run->running, run->lastRunTimer, timeVar);
  if (!color[3]) {
    return;
  }

  CG_Text_Paint_Centred_Ext(x, y, 0.3, 0.3, *color, text, 0, 0, style, font);

  if (etj_drawCheckpoints.integer && run->runHasCheckpoints) {
    // only adjust x/y if we're drawing checkpoints detached from runtimer
    if (etj_drawCheckpoints.integer == 2) {
      x = etj_checkpointsX.value;
      y = etj_checkpointsY.value;
      ETJump_AdjustPosition(&x);
    } else {
      // position the times below runtimer
      y += 20;
    }
    const int currentTime =
        run->running ? timeVar - run->startTime : run->completionTime;
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
    if (!run->running && run->completionTime <= 0) {
      return;
    }

    for (int i = startIndex; i >= 0 && i > endIndex; i--) {
      vec4_t *checkpointColor = &colorWhite;
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
      // if we don't have a next checkpoint set, we can't compare to anything
      // so render checkpoint as white
      if (noRecordCheckpoint || relativeTime == 0 ||
          comparisonTime == currentTime) {
        checkpointColor = &colorWhite;
      } else {
        bool isFasterCheckpoint;
        // if we've hit max checkpoints, checkpointTime will be 0,
        // so we need to compare currentTime against the comparisonTime,
        // which will be previously set record in this scenario
        if (maxCheckpointsHit) {
          if (currentTime < comparisonTime) {
            isFasterCheckpoint = true;
          } else {
            isFasterCheckpoint = false;
          }
        } else {
          if ((noCheckpointTimeSet && currentTime < comparisonTime) ||
              (!noCheckpointTimeSet && checkpointTime < comparisonTime)) {
            isFasterCheckpoint = true;
          } else {
            isFasterCheckpoint = false;
          }
        }

        if (isFasterCheckpoint) {
          checkpointColor = &colorSuccess;
          dir = "-";
        } else {
          checkpointColor = &colorFail;
          dir = "+";
        }
      }

      const int absoluteTime = noCheckpointTimeSet || maxCheckpointsHit
                                   ? currentTime
                                   : checkpointTime;

      std::string timerStr = getTimerString(
          etj_checkpointsStyle.integer == 1 ? absoluteTime : relativeTime);

      timerStr.insert(0, dir);

      (*checkpointColor)[3] =
          getTimerAlpha(run->running, run->lastRunTimer, timeVar);
      if (!checkpointColor[3]) {
        return;
      }

      CG_Text_Paint_Centred_Ext(x, y, textSize, textSize, *checkpointColor,
                                timerStr, 0, 0, textStyle, font);
      y += static_cast<float>(2 *
                              CG_Text_Height_Ext(timerStr, textSize, 0, font));
    }
  }

  if (run->running) {
    if (run->previousRecord != -1 && ms > run->previousRecord) {
      pastRecordAnimation(color, text.c_str(), ms, run->previousRecord);
    }
  }
}

float ETJump::TimerunView::getTimerAlpha(bool running, int lastRunTimer,
                                         int timeVar) {
  const int fadeStart = lastRunTimer + fadeHold;
  const int fadeEnd = fadeStart + fadeOut;

  if (!running && etj_runTimerAutoHide.integer) {
    if (fadeStart < timeVar && fadeEnd > timeVar) {
      const float step =
          static_cast<float>(timeVar - fadeStart) / static_cast<float>(fadeOut);
      return 1.0f - step;
    } else if (timeVar >= fadeEnd) {
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
      CG_Text_Height_Ext(text, 0.3, 0, &cgs.media.limboFont1);
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
