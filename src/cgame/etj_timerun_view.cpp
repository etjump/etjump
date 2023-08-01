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

#include "cg_local.h"
#include "etj_timerun_view.h"
#include "etj_utilities.h"
#include "etj_cvar_update_handler.h"
#include "etj_player_events_handler.h"
#include "../game/etj_string_utilities.h"
#include "../game/etj_container_utilities.h"
#include "../game/etj_numeric_utilities.h"

ETJump::TimerunView::TimerunView(
    std::shared_ptr<PlayerEventsHandler> playerEventsHandler)
  : Drawable(), _playerEventsHandler(playerEventsHandler) {
  for (auto &info : _playersTimerunInformation) {
    interrupt(info);
  }

  parseColorString(etj_runTimerInactiveColor.string, inactiveTimerColor);
  cvarUpdateHandler->subscribe(
      &etj_runTimerInactiveColor, [&](const vmCvar_t *cvar) {
        parseColorString(cvar->string, inactiveTimerColor);
      });
}

ETJump::TimerunView::~TimerunView() {
}

void ETJump::TimerunView::start() {
  auto clientNum = Q_atoi(CG_Argv(2));
  _playersTimerunInformation[clientNum].startTime = Q_atoi(CG_Argv(3));
  _playersTimerunInformation[clientNum].runName = CG_Argv(4);
  _playersTimerunInformation[clientNum].previousRecord = Q_atoi(CG_Argv(5));
  _playersTimerunInformation[clientNum].running = true;
  _playersTimerunInformation[clientNum].checkpoints.fill(
      TIMERUN_CHECKPOINT_NOT_SET);
  _playersTimerunInformation[clientNum].nextFreeCheckpointIdx = 0;
  _playersTimerunInformation[clientNum].numCheckpointsHit = 0;
  auto checkpointsStr = CG_Argv(6);
  if (strlen(checkpointsStr) > 0) {
    try {
      auto checkpoints =
          Container::map(StringUtil::split(CG_Argv(6), ","),
                         [](const std::string &i) { return std::stoi(i); });

      for (int i = 0; i < MAX_TIMERUN_CHECKPOINTS && i < checkpoints.size(); ++
           i) {
        _playersTimerunInformation[clientNum].previousRecordCheckpoints[i] =
            checkpoints[i];
      }
    } catch (const std::runtime_error &) {
      _playersTimerunInformation[clientNum].previousRecordCheckpoints =
          std::array<int, MAX_TIMERUN_CHECKPOINTS>();
      _playersTimerunInformation[clientNum].previousRecordCheckpoints.fill(
          TIMERUN_CHECKPOINT_NOT_SET);
    }
  }

  _playerEventsHandler->check(
      "timerun:record",
      {_playersTimerunInformation[clientNum].runName,
       std::to_string(_playersTimerunInformation[clientNum].startTime),
       std::to_string(_playersTimerunInformation[clientNum].previousRecord)});

  playerEventsHandler->check("timerun:stop", {CG_Argv(2), CG_Argv(1)});
}

void ETJump::TimerunView::stop() {
  auto clientNum = Q_atoi(CG_Argv(2));
  _playersTimerunInformation[clientNum].completionTime = Q_atoi(CG_Argv(3));
  _playersTimerunInformation[clientNum].running = false;
  _playersTimerunInformation[clientNum].lastRunTimer = cg.time;
}

void ETJump::TimerunView::checkpoint() {
  const int clientNum = Q_atoi(CG_Argv(2));
  const int checkpointIndex = Q_atoi(CG_Argv(3));
  const int checkpointTime = Q_atoi(CG_Argv(4));
  _playersTimerunInformation[clientNum].checkpoints[checkpointIndex] =
      checkpointTime;
  _playersTimerunInformation[clientNum].numCheckpointsHit = checkpointIndex + 1;
}

void ETJump::TimerunView::interrupt(
    PlayerTimerunInformation &playerTimerunInformation) {
  playerTimerunInformation.running = false;
  playerTimerunInformation.runName = "";
  playerTimerunInformation.completionTime = -1;
  playerTimerunInformation.previousRecord = 0;
  playerTimerunInformation.startTime = 0;
  playerTimerunInformation.lastRunTimer = cg.time;
}

void ETJump::TimerunView::interrupt() {
  auto clientNum = Q_atoi(CG_Argv(2));
  interrupt(_playersTimerunInformation[clientNum]);
}

const ETJump::PlayerTimerunInformation *
ETJump::TimerunView::currentRun() const {
  return &_playersTimerunInformation[cg.snap->ps.clientNum];
}

std::string ETJump::TimerunView::getTimerString(const int msec) {
  auto millis = msec;
  auto minutes = millis / 60000;
  millis -= minutes * 60000;
  auto seconds = millis / 1000;
  millis -= seconds * 1000;

  return ETJump::stringFormat("%02d:%02d.%03d", minutes, seconds, millis);
}

void ETJump::TimerunView::draw() {
  if (canSkipDraw()) {
    return;
  }

  auto run = currentRun();
  auto hasTimerun = ((cg.demoPlayback && (run->lastRunTimer || run->running)) ||
                     cg.hasTimerun);

  if (etj_drawRunTimer.integer == 0 || !hasTimerun) {
    return;
  }

  auto startTime = run->startTime;
  auto millis = 0;
  auto color = &colorWhite;
  const auto font = &cgs.media.limboFont1;

  if (run->running) {
    millis = cg.time - startTime;
  } else {
    millis = run->completionTime;
    if (millis == -1) {
      millis = 0;
    }
    color = &inactiveTimerColor;
  }

  vec4_t colorTemp;
  vec4_t colorWhite = {1.0f, 1.0f, 1.0f, 1.0f};
  vec4_t colorSuccess = {0.627f, 0.941f, 0.349f, 1.0f};
  vec4_t colorFail = {0.976f, 0.262f, 0.262f, 1.0f};

  auto range = getTransitionRange(run->previousRecord);
  auto style = ITEM_TEXTSTYLE_NORMAL;
  auto fadeOut = 2000;   // 2s fade out
  auto fadeStart = 5000; // 5s pause

  if (etj_runTimerShadow.integer > 0) {
    style = ITEM_TEXTSTYLE_SHADOWED;
  }

  if (run->previousRecord > 0) {
    if (millis > run->previousRecord) {
      color = &colorFail;
    }
    // add timer color transition when player gets closer to his
    // pb
    else if (millis + range >= run->previousRecord) {
      auto start = run->previousRecord - range;
      auto step = (millis - start) / (float)(run->previousRecord - start);

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

  float textWidth =
      CG_Text_Width_Ext(text.c_str(), 0.3, 0, font) / 2;
  auto x = etj_runTimerX.value;
  auto y = etj_runTimerY.value;

  // timer fading/hiding routine
  ETJump_AdjustPosition(&x);

  if (!run->running && etj_runTimerAutoHide.integer) {
    auto fstart = run->lastRunTimer + fadeStart;
    auto fend = fstart + fadeOut;

    if (fstart < cg.time && fend > cg.time) {

      vec4_t toColor;
      memcpy(&toColor, color, sizeof(toColor));
      toColor[3] = 0;

      auto step = (cg.time - fstart) / (float)(fend - fstart);

      ETJump_LerpColors(color, &toColor, &colorTemp, step);
      color = &colorTemp;

    } else if (cg.time > fend) {
      // dont draw timer once fading is done
      return;
    }

  }

  if (etj_drawCheckpoints.integer && etj_checkpointsCount.integer > 0) {
    // only adjust x/y if we're drawing checkpoints detached from runtimer
    if (etj_drawCheckpoints.integer == 2) {
      x = etj_checkpointsX.value;
      y = etj_checkpointsY.value;
      ETJump_AdjustPosition(&x);
    } else {
      // position the times below runtimer
      y += 20;
    }
    const float textSize = 0.1f * etj_checkpointsSize.value;
    const auto textStyle = etj_checkpointsShadow.integer
                             ? ITEM_TEXTSTYLE_SHADOWED
                             : ITEM_TEXTSTYLE_NORMAL;

    const int count = Numeric::clamp(etj_checkpointsCount.integer, 0, 5);
    const int startIndex = run->numCheckpointsHit;
    const int endIndex = run->numCheckpointsHit - count;

    for (int i = startIndex; i > 0 && i > endIndex; i--) {
      vec4_t *checkpointColor = &colorWhite;
      const int checkpointTime = run->checkpoints[i - 1];
      const int previousCheckpointTime = run->previousRecordCheckpoints[i - 1];

      if (previousCheckpointTime >= 0) {
        if (checkpointTime < previousCheckpointTime) {
          checkpointColor = &colorSuccess;
        } else {
          checkpointColor = &colorFail;
        }
      }

      /* if (etj_checkpointsStyle.integer == 1) {
        // absolute time
      } else {
        // relative time
      } */
      const auto timerStr = getTimerString(checkpointTime);

      textWidth =
          static_cast<float>(CG_Text_Width_Ext(timerStr, textSize, 0, font)) *
          0.5f;

      CG_Text_Paint_Ext(x - textWidth, y, textSize, textSize, *checkpointColor,
                        timerStr.c_str(), 0, 0, textStyle, font);
      y += 15;
    }
  }

  if (run->running) {

    if (run->previousRecord != -1 && ms > run->previousRecord) {
      pastRecordAnimation(color, text.c_str(), ms, run->previousRecord);
    }
  }

  CG_Text_Paint_Ext(x - textWidth, y, 0.3, 0.3, *color, text.c_str(), 0, 0,
                    style, font);
}

int ETJump::TimerunView::getTransitionRange(int previousRunTime) {
  auto range = 10000;

  if (3 * 1000 > previousRunTime) {
    range = 0;
  } else if (10 * 1000 > previousRunTime) {
    range = 500; // just for a nice short transition effect,
    // could be 0
  } else if (30 * 1000 > previousRunTime) {
    range = 2000;
  } else if (60 * 1000 > previousRunTime) {
    range = 3500;
  } else if (120 * 1000 > previousRunTime) {
    range = 5000;
  }

  return range;
}

void ETJump::TimerunView::pastRecordAnimation(vec4_t *color, const char *text,
                                              int timerTime, int record) {
  auto animationTime = 300;

  if (timerTime - record > animationTime) {
    return;
  }

  vec4_t toColor;
  vec4_t incolor;

  auto x = etj_runTimerX.value;
  auto y = etj_runTimerY.value;

  ETJump_AdjustPosition(&x);

  auto step = ((float)(timerTime - record) / animationTime);
  auto scale = 0.3 + 0.25 * step;

  auto originalTextHeight =
      CG_Text_Height_Ext(text, 0.3, 0, &cgs.media.limboFont1);
  auto textWidth = CG_Text_Width_Ext(text, scale, 0, &cgs.media.limboFont1) / 2;
  auto textHeight = (CG_Text_Height_Ext(text, scale, 0, &cgs.media.limboFont1) -
                     originalTextHeight) /
                    2;

  memcpy(&toColor, color, sizeof(toColor));
  toColor[3] = 0;

  ETJump_LerpColors(color, &toColor, &incolor, step);

  CG_Text_Paint_Ext(x - textWidth, y + textHeight, scale, scale, incolor, text,
                    0, 0, 0, &cgs.media.limboFont1);
}

bool ETJump::TimerunView::parseServerCommand() {
  auto argc = trap_Argc();

  if (argc == 1) {
    return false;
  }

  char cmd[MAX_TOKEN_CHARS]{};
  trap_Argv(1, cmd, sizeof(cmd));

  if (cmd == std::string("start")) {
    start();
  } else if (cmd == std::string("checkpoint")) {
    checkpoint();
  } else if (cmd == std::string("stop")) {
    stop();
  } else if (cmd == std::string("interrupt")) {
    interrupt();
  } else {
    return false;
  }

  return true;
}

bool ETJump::TimerunView::canSkipDraw() const {
  return ETJump::showingScores();
}
