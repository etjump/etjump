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
#include "etj_timerun.h"
#include "../game/etj_string_utilities.h"

Timerun::Timerun(int clientNum) { _clientNum = clientNum; }

void Timerun::startTimerun(const std::string &runName, int startTime,
                           int previousRecord) {
  _running = true;
  _startTime = startTime;
  _currentTimerun = runName;
  _runningPlayerClientNum = _clientNum;

  auto playerTimes = _fastestTimes.find(_clientNum);
  if (playerTimes == _fastestTimes.end()) {
    _fastestTimes[_clientNum] = std::map<std::string, int>();
    playerTimes = _fastestTimes.find(_clientNum);
  }

  playerTimes->second[runName] = previousRecord;
  _fastestTime = previousRecord;
}

void Timerun::startSpectatorTimerun(int clientNum, const std::string &runName,
                                    int startTime, int previousRecord) {
  if (clientNum == _clientNum) {
    return;
  }

  _running = true;
  _currentTimerun = runName;
  _startTime = startTime;
  _runningPlayerClientNum = clientNum;

  auto playerTimes = _fastestTimes.find(clientNum);
  if (playerTimes == _fastestTimes.end()) {
    _fastestTimes[clientNum] = std::map<std::string, int>();
    playerTimes = _fastestTimes.find(clientNum);
  }

  playerTimes->second[runName] = previousRecord;
  _fastestTime = previousRecord;
}

void Timerun::interrupt() {
  _running = false;
  _currentTimerun = "";
  _startTime = 0;
  _runningPlayerClientNum = 0;
  _fastestTime = -1;
}

void Timerun::stopTimerun(int completionTime) {
  _running = false;
  _completionTime = completionTime;
}

void Timerun::record(int clientNum, std::string runName, int completionTime) {

  int previousTime = NO_PREVIOUS_RECORD;

  auto playerTimes = _fastestTimes.find(clientNum);
  if (playerTimes != _fastestTimes.end()) {
    auto previousTimeIter = playerTimes->second.find(runName);
    if (previousTimeIter != playerTimes->second.end()) {
      previousTime = previousTimeIter->second;
    }
  }
  std::string message = createCompletionMessage(
      cgs.clientinfo[clientNum], runName, completionTime, previousTime);
  int shader = previousTime == NO_PREVIOUS_RECORD
                   ? cgs.media.stopwatchIcon
                   : cgs.media.stopwatchIconGreen;
  printMessage(message, shader);
}

void Timerun::completion(int clientNum, std::string runName,
                         int completionTime) {
  int previousRecord = NO_PREVIOUS_RECORD;
  auto playerTimes = _fastestTimes.find(clientNum);
  if (playerTimes != _fastestTimes.end()) {
    auto previousTimeIter = playerTimes->second.find(runName);
    if (previousTimeIter != playerTimes->second.end()) {
      previousRecord = previousTimeIter->second;
    }
  }

  // if we're the player / spectating the player, print the message
  if (clientNum == cg.snap->ps.clientNum) {
    std::string message = createCompletionMessage(
        cgs.clientinfo[clientNum], runName, completionTime, previousRecord);
    printMessage(message, cgs.media.stopwatchIconRed);
  }
}

void Timerun::stopSpectatorTimerun(int clientNum, int completionTime,
                                   const std::string &currentRun) {
  _running = false;
  _completionTime = completionTime;
}

Timerun::Time Timerun::createTimeFromTimestamp(int timestamp) {
  int millis = timestamp;

  int minutes = millis / static_cast<int>(Time::Duration::Minute);
  millis -= minutes * static_cast<int>(Time::Duration::Minute);

  int seconds = millis / static_cast<int>(Time::Duration::Second);
  millis -= seconds * static_cast<int>(Time::Duration::Second);

  return {minutes, seconds, millis, timestamp};
}

std::string Timerun::createCompletionMessage(clientInfo_t &player,
                                             std::string &runName,
                                             int completionTime,
                                             int previousTime) {
  Time now = createTimeFromTimestamp(completionTime);
  std::string who{(player.clientNum == _clientNum) ? "You" : player.name};
  std::string timeFinished{createTimeString(now)};
  std::string timeDifference{""};
  auto postfix = '!';

  if (previousTime != NO_PREVIOUS_RECORD) {
    Time diff = createTimeFromTimestamp(abs(previousTime - completionTime));
    std::string timeDir;
    if (previousTime > completionTime) {
      // faster
      timeDir = "-^2";
    } else if (previousTime < completionTime) {
      // slower
      timeDir = "+^1";
    } else {
      // tied
      timeDir = "+^7";
    }
    timeDifference =
        ETJump::stringFormat("^7(%s%s^7)", timeDir, createTimeString(diff));
    postfix = (previousTime > completionTime) ? '!' : '.';
  }

  std::string message =
      ETJump::stringFormat("^7%s ^7completed %s ^7in %s%c %s", who, runName,
                           timeFinished, postfix, timeDifference);

  return message;
}

std::string Timerun::createTimeString(Time &time) {
  return ETJump::stringFormat("%02d:%02d.%03d", time.minutes, time.seconds,
                              time.ms);
}

void Timerun::printMessage(std::string &message, int shaderIcon) {
  CG_AddPMItem(PM_MESSAGE, message.c_str(), shaderIcon);
}
