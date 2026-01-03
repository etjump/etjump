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

#include <string>
#include <utility>

#include "cg_local.h"
#include "etj_timerun_view.h"
#include "etj_utilities.h"
#include "etj_cvar_update_handler.h"
#include "etj_player_events_handler.h"
#include "../game/etj_string_utilities.h"

namespace ETJump {
inline constexpr int ANIMATION_TIME = 300;
inline constexpr int FADE_HOLD = 5000; // 5s pause before fade starts
inline constexpr int FADE_TIME = 2000; // 2s fade
inline constexpr int POPUP_FADE_TIME = 100;

TimerunView::TimerunView(std::shared_ptr<Timerun> timerun)
    : _timerun(std::move(timerun)), font(&cgs.media.limboFont1),
      autoHide(etj_runTimerAutoHide.integer) {
  parseColorString(etj_runTimerInactiveColor.string, inactiveTimerColor);
  setCheckpointSize(etj_checkpointsSize);
  setCheckpointPopupSize(etj_checkpointsPopupSize);

  cvarUpdateHandler->subscribe(
      &etj_runTimerInactiveColor, [&](const vmCvar_t *cvar) {
        parseColorString(cvar->string, inactiveTimerColor);
      });

  cvarUpdateHandler->subscribe(&etj_checkpointsSize, [&](const vmCvar_t *cvar) {
    setCheckpointSize(*cvar);
  });

  cvarUpdateHandler->subscribe(
      &etj_checkpointsPopupSize,
      [&](const vmCvar_t *cvar) { setCheckpointPopupSize(*cvar); });

  cvarUpdateHandler->subscribe(
      &etj_runTimerAutoHide,
      [&](const vmCvar_t *cvar) { autoHide = cvar->integer; });

  if (cg.demoPlayback) {
    demoSvFps = getSvFps();
  }
}

void TimerunView::setCheckpointSize(const vmCvar_t &cvar) {
  checkpointSize = CvarValueParser::parse<CvarValue::Size>(cvar, 0, 10);
}

void TimerunView::setCheckpointPopupSize(const vmCvar_t &cvar) {
  popupSize = CvarValueParser::parse<CvarValue::Size>(cvar, 0, 10);
}

const Timerun::PlayerTimerunInformation *TimerunView::currentRun() const {
  return _timerun->getTimerunInformationFor(cg.snap->ps.clientNum);
}

std::string TimerunView::getTimerString(int msec) {
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

void TimerunView::draw() {
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

  vec4_t *color = &colorDefault;

  // figure out the timing variable we can use for the run timer
  int timeVar;

  if (cg.demoPlayback) {
    timeVar = demoSvFps == 125 ? cg.snap->ps.commandTime : cg.time;
  } else if (isPlaying(cg.clientNum)) {
    timeVar = cg.predictedPlayerState.commandTime;
  } else {
    timeVar = cgs.sv_fps == 125 ? cg.snap->ps.commandTime : cg.time;
  }

  int millis;

  if (running) {
    millis = timeVar - run->startTime;
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

  // set green color for pb time, white for tied/new time
  if (!running && millis) {
    if (run->previousRecord == millis || run->previousRecord == -1) {
      color = &colorDefault;
    } else if (run->previousRecord > millis) {
      color = &colorSuccess;
    }
  }

  const std::string text = getTimerString(millis);

  auto x = etj_runTimerX.value;
  auto y = etj_runTimerY.value;
  ETJump_AdjustPosition(&x);

  (*color)[3] =
      getTimerAlpha(running, autoHide, lastRunTimer + FADE_HOLD, FADE_TIME);
  if ((*color)[3] == 0) {
    return;
  }

  CG_Text_Paint_Centred_Ext(x, y, 0.3f, 0.3f, *color, text, 0, 0, style, font);

  if (run->runHasCheckpoints &&
      (etj_drawCheckpoints.integer || etj_checkpointsPopup.integer)) {
    drawCheckpoints(x, y, timeVar, *run);
  }

  if (running && run->previousRecord != -1 && millis > run->previousRecord) {
    pastRecordAnimation(color, text.c_str(), millis, run->previousRecord);
  }
}

void TimerunView::drawCheckpoints(
    float x, float y, const int timeVar,
    const Timerun::PlayerTimerunInformation &run) {
  // only adjust x/y if we're drawing checkpoints detached from runtimer
  if (etj_drawCheckpoints.integer == 2) {
    x = etj_checkpointsX.value;
    y = etj_checkpointsY.value;
    ETJump_AdjustPosition(&x);
  } else {
    // position the times below runtimer
    y += 20;
  }

  const bool running = run.running;
  const int32_t lastRunTimer = run.lastRunTimer;

  const int32_t currentRunTime =
      running ? timeVar - run.startTime : run.completionTime;
  const float checkpointSizeX = 0.1f * checkpointSize.x;
  const float checkpointSizeY = 0.1f * checkpointSize.y;
  const auto textStyle = etj_checkpointsShadow.integer ? ITEM_TEXTSTYLE_SHADOWED
                                                       : ITEM_TEXTSTYLE_NORMAL;

  const int32_t count = std::clamp(etj_checkpointsCount.integer, 1, 5);
  const int32_t startIndex = run.numCheckpointsHit;
  const int32_t endIndex = run.numCheckpointsHit - count;

  // do not render checkpoints if we're not running and
  // did not just complete a run
  if (!running && run.completionTime <= 0) {
    return;
  }

  for (int32_t i = startIndex; i >= 0 && i > endIndex; i--) {
    const bool isLastCheckpoint = i == MAX_TIMERUN_CHECKPOINTS;

    // this will be either the previous best checkpoint time,
    // previous record or current run timer
    const int32_t comparisonTime =
        getCheckpointComparisonTime(currentRunTime, i, run);
    const int32_t currentCheckpointTime =
        isLastCheckpoint ? comparisonTime : run.checkpoints[i];

    const bool noRecordCheckpoint =
        comparisonTime == TIMERUN_CHECKPOINT_NOT_SET;
    const bool noCheckpointTimeSet =
        currentCheckpointTime == TIMERUN_CHECKPOINT_NOT_SET;

    const int32_t relativeTime =
        getRelativeCheckpointTimer(currentRunTime, comparisonTime, i, run);
    const int32_t absoluteTime = noCheckpointTimeSet || isLastCheckpoint
                                     ? currentRunTime
                                     : currentCheckpointTime;

    std::string dir;
    vec4_t *checkpointColor = nullptr;

    // if we don't have a next checkpoint set, we can't compare to anything
    // so render checkpoint as white
    if (noRecordCheckpoint || relativeTime == 0 ||
        comparisonTime == currentRunTime) {
      checkpointColor = &colorDefault;
    } else {
      bool isFasterCheckpoint = false;

      // if we've hit max checkpoints, checkpointTime will be 0,
      // so we need to compare currentTime against the comparisonTime,
      // which will be previously set record in this scenario
      if (isLastCheckpoint) {
        isFasterCheckpoint = currentRunTime < comparisonTime;
      } else {
        isFasterCheckpoint =
            (noCheckpointTimeSet ? currentRunTime : currentCheckpointTime) <
            comparisonTime;
      }

      checkpointColor = isFasterCheckpoint ? &colorSuccess : &colorFail;
      dir = isFasterCheckpoint ? "-" : "+";
    }

    const std::string checkpointStr =
        dir + getTimerString(etj_checkpointsStyle.integer == 1 ? absoluteTime
                                                               : relativeTime);

    // we do not need to check for alpha being 0 here because checkpoints
    // follow same fading as runtimer, and if runtimer alpha is 0,
    // we early out before ever reaching this part
    (*checkpointColor)[3] =
        getTimerAlpha(running, autoHide, lastRunTimer + FADE_HOLD, FADE_TIME);

    // check if we actually want to draw the regular checkpoint timer,
    // so we can only draw the popups if desired
    if (etj_drawCheckpoints.integer) {
      CG_Text_Paint_Centred_Ext(x, y, checkpointSizeX, checkpointSizeY,
                                *checkpointColor, checkpointStr, 0, 0,
                                textStyle, font);
    }

    if (etj_checkpointsPopup.integer && i == startIndex - 1) {
      vec4_t cpPopupColor;
      Vector4Copy(*checkpointColor, cpPopupColor);

      const int32_t popupTime =
          run.lastCheckpointTimestamp + etj_checkpointsPopupDuration.integer;
      const float popupSizeX = 0.1f * popupSize.x;
      const float popupSizeY = 0.1f * popupSize.y;
      const int32_t popupStyle = etj_checkpointsPopupShadow.integer
                                     ? ITEM_TEXTSTYLE_SHADOWED
                                     : ITEM_TEXTSTYLE_NORMAL;

      const float y2 = etj_checkpointsPopupY.value;
      float x2 = etj_checkpointsPopupX.value;
      ETJump_AdjustPosition(&x2);

      if (popupTime >= cg.time) {
        CG_Text_Paint_Centred_Ext(x2, y2, popupSizeX, popupSizeY, cpPopupColor,
                                  checkpointStr, 0, 0, popupStyle, font);
      } else if (popupTime + POPUP_FADE_TIME > cg.time) {
        // we want to always fade here so bypass running/autoHide
        cpPopupColor[3] =
            getTimerAlpha(false, true, popupTime, POPUP_FADE_TIME);

        CG_Text_Paint_Centred_Ext(x2, y2, popupSizeX, popupSizeY, cpPopupColor,
                                  checkpointStr, 0, 0, popupStyle, font);
      }
    }

    y += static_cast<float>(
        CG_Text_Height_Ext(checkpointStr, checkpointSizeY, 0, font) * 2);
  }
}

int32_t TimerunView::getCheckpointComparisonTime(
    const int currentTime, const int currentCPIndex,
    const Timerun::PlayerTimerunInformation &run) {
  if (currentCPIndex == MAX_TIMERUN_CHECKPOINTS) {
    return run.previousRecord >= 0 ? run.previousRecord : currentTime;
  }

  if (run.previousRecordCheckpoints[currentCPIndex] ==
      TIMERUN_CHECKPOINT_NOT_SET) {
    return run.previousRecord;
  }

  return run.previousRecordCheckpoints[currentCPIndex];
}

int32_t TimerunView::getRelativeCheckpointTimer(
    const int currentTime, const int comparisonTime, const int currentCPIndex,
    const Timerun::PlayerTimerunInformation &run) {
  // make sure we don't subtract -1 from timestamp if we don't
  // have a checkpoint time set at all
  const int relTimerOffset =
      comparisonTime == TIMERUN_CHECKPOINT_NOT_SET ? 0 : comparisonTime;

  if (currentCPIndex == MAX_TIMERUN_CHECKPOINTS) {
    return run.previousRecord >= 0 ? comparisonTime - currentTime
                                   : comparisonTime;
  }

  return run.checkpoints[currentCPIndex] == TIMERUN_CHECKPOINT_NOT_SET
             ? currentTime - relTimerOffset
             : run.checkpoints[currentCPIndex] - relTimerOffset;
}

float TimerunView::getTimerAlpha(bool running, bool autoHide, int fadeStart,
                                 int duration) {
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

int TimerunView::getTransitionRange(int previousTime) {
  const int range = previousTime / 10;

  // for very long runs/splits, cap the max transition range to 10s
  return range <= 10000 ? range : 10000;
}

void TimerunView::pastRecordAnimation(vec4_t *color, const char *text,
                                      int timerTime, int record) {
  if (timerTime - record > ANIMATION_TIME) {
    return;
  }

  vec4_t toColor;
  vec4_t inColor;

  auto x = etj_runTimerX.value;
  const auto y = etj_runTimerY.value;

  ETJump_AdjustPosition(&x);

  const auto step = static_cast<float>(timerTime - record) /
                    static_cast<float>(ANIMATION_TIME);
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

bool TimerunView::canSkipDraw() { return showingScores(); }
} // namespace ETJump
