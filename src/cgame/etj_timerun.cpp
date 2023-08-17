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

#include "etj_timerun.h"

#include "cg_local.h"
#include "etj_player_events_handler.h"
#include "../game/etj_timerun_shared.h"
#include "../game/etj_time_utilities.h"

namespace ETJump {
Timerun::Timerun() {
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    auto interrupt = TimerunCommands::Interrupt(i);
    onInterrupt(&interrupt);
  }
}

void Timerun::onStop(const TimerunCommands::Stop *stop) {
  auto clientNum = stop->clientNum;
  _playersTimerunInformation[clientNum].completionTime = stop->completionTime;
  _playersTimerunInformation[clientNum].running = false;
  _playersTimerunInformation[clientNum].lastRunTimer = cg.time;

  _playerEventsHandler->check("timerun:stop",
                              {std::to_string(stop->clientNum), stop->runName,
                               std::to_string(stop->completionTime)});
}

void Timerun::onInterrupt(const TimerunCommands::Interrupt *interrupt) {
  _playersTimerunInformation[interrupt->clientNum].running = false;
  _playersTimerunInformation[interrupt->clientNum].runName = "";
  _playersTimerunInformation[interrupt->clientNum].completionTime = -1;
  _playersTimerunInformation[interrupt->clientNum].previousRecord = 0;
  _playersTimerunInformation[interrupt->clientNum].startTime = 0;
  _playersTimerunInformation[interrupt->clientNum].lastRunTimer = cg.time;

  _playerEventsHandler->check("timerun:interrupt",
                              {std::to_string(interrupt->clientNum)});
}

void Timerun::onCheckpoint(const TimerunCommands::Checkpoint *cp) {
  _playersTimerunInformation[cp->clientNum].checkpoints[cp->checkpointIndex] =
      cp->checkpointTime;
  _playersTimerunInformation[cp->clientNum].numCheckpointsHit =
      cp->checkpointIndex + 1;
}

void Timerun::onStart(const TimerunCommands::Start *start) {
  auto clientNum = start->clientNum;
  _playersTimerunInformation[clientNum].startTime = start->startTime;
  _playersTimerunInformation[clientNum].runName = start->runName;
  _playersTimerunInformation[clientNum].previousRecord =
      start->previousRecord.valueOr(-1);
  _playersTimerunInformation[clientNum].running = true;
  _playersTimerunInformation[clientNum].checkpoints =
      start->currentRunCheckpoints;
  _playersTimerunInformation[clientNum].nextFreeCheckpointIdx = 0;
  _playersTimerunInformation[clientNum].numCheckpointsHit =
      Timerun::getNumCheckpointsHit(start->currentRunCheckpoints);
  _playersTimerunInformation[clientNum].previousRecordCheckpoints =
      start->checkpoints;
  _playerEventsHandler->check(
      "timerun:start", {std::to_string(start->clientNum), start->runName,
                        std::to_string(start->startTime),
                        std::to_string(start->previousRecord.valueOr(-1))});
}

void Timerun::printMessage(const std::string &message, int shader) {
  CG_AddPMItem(PM_MESSAGE, message.c_str(), shader);
}

void Timerun::onRecord(const TimerunCommands::Record *record) {
  if (record->clientNum == cg.snap->ps.clientNum) {
    std::string message = createCompletionMessage(
        cgs.clientinfo[record->clientNum], record->runName,
        record->completionTime, record->previousRecordTime);
    int shader = !record->previousRecordTime.hasValue()
                     ? cgs.media.stopwatchIcon
                     : cgs.media.stopwatchIconGreen;
    printMessage(message, shader);
  }

  _playerEventsHandler->check(
      "timerun:record", {std::to_string(record->clientNum), record->runName,
                         std::to_string(record->completionTime)});
}

void Timerun::onCompletion(const TimerunCommands::Completion *completion) {
  if (completion->clientNum == cg.snap->ps.clientNum) {
    std::string message = createCompletionMessage(
        cgs.clientinfo[completion->clientNum], completion->runName,
        completion->completionTime, completion->previousRecordTime);
    printMessage(message, cgs.media.stopwatchIconRed);
  }

  _playerEventsHandler->check("timerun:completion",
                              {std::to_string(completion->clientNum),
                               completion->runName,
                               std::to_string(completion->completionTime)});
}

void Timerun::parseServerCommand(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return;
  }

  if (args[1] == "start") {
    auto startOpt = TimerunCommands::Start::deserialize(args);
    if (!startOpt.hasValue()) {
      return;
    }
    const auto start = &startOpt.value();

    onStart(start);
    return;
  }
  if (args[1] == "checkpoint") {
    auto checkpointOpt = TimerunCommands::Checkpoint::deserialize(args);
    if (!checkpointOpt.hasValue()) {
      return;
    }

    const auto cp = &checkpointOpt.value();

    onCheckpoint(cp);
    return;
  }
  if (args[1] == "stop") {
    auto stopOpt = TimerunCommands::Stop::deserialize(args);
    if (!stopOpt.hasValue()) {
      return;
    }

    const auto stop = &stopOpt.value();
    onStop(stop);
    return;
  }

  if (args[1] == "interrupt") {
    auto interruptOpt = TimerunCommands::Interrupt::deserialize(args);
    if (!interruptOpt.hasValue()) {
      return;
    }

    const auto interrupt = &interruptOpt.value();
    onInterrupt(interrupt);
    return;
  }

  if (args[1] == "record") {
    auto recordOpt = TimerunCommands::Record::deserialize(args);
    if (!recordOpt.hasValue()) {
      return;
    }

    const auto record = &recordOpt.value();
    onRecord(record);
    return;
  }

  if (args[1] == "completion") {
    auto completionOpt = TimerunCommands::Completion::deserialize(args);
    if (!completionOpt.hasValue()) {
      return;
    }

    const auto completion = &completionOpt.value();
    onCompletion(completion);
  }
}

const Timerun::PlayerTimerunInformation *
Timerun::getTimerunInformationFor(int clientNum) {
  return &_playersTimerunInformation[clientNum];
}

std::string Timerun::createCompletionMessage(const clientInfo_t &player,
                                             const std::string &runName,
                                             int completionTime,
                                             opt<int> previousTime) {
  std::string who{(player.clientNum == _clientNum) ? "You" : player.name};
  std::string timeFinished{millisToString(completionTime)};
  std::string timeDifference{""};

  if (previousTime.hasValue()) {
    timeDifference =
        +"^7(" + diffToString(completionTime, previousTime.value()) + "^7)";
  }

  std::string message =
      ETJump::stringFormat("^7%s ^7completed %s ^7in %s ^7%s^7", who, runName,
                           timeFinished, timeDifference);

  return message;
}
int Timerun::getNumCheckpointsHit(
    const std::array<int, MAX_TIMERUN_CHECKPOINTS> currentRunCheckpoints) {
  int numCheckpointsHit;
  for (numCheckpointsHit = 0; numCheckpointsHit < MAX_TIMERUN_CHECKPOINTS;
       numCheckpointsHit++) {
    if (currentRunCheckpoints[numCheckpointsHit] ==
        TIMERUN_CHECKPOINT_NOT_SET) {
      break;
    }
  }
  return numCheckpointsHit;
}
} // namespace ETJump
