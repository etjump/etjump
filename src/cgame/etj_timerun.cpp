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

#include "etj_timerun.h"

#include "cg_local.h"
#include "etj_client_commands_handler.h"
#include "etj_player_events_handler.h"
#include "../game/etj_timerun_shared.h"
#include "../game/etj_time_utilities.h"

namespace ETJump {
Timerun::Timerun(
    const std::shared_ptr<PlayerEventsHandler> &playerEventsHandler,
    const std::shared_ptr<ClientCommandsHandler> &serverCommandsHandler)
    : playerEventsHandler(playerEventsHandler),
      serverCommandsHandler(serverCommandsHandler) {
  registerListeners();
}

void Timerun::registerListeners() {
  // this is awkward, but the timerun command deserializer *really* wants
  // the first argument to be 'timerun', but we don't get that here
  serverCommandsHandler->subscribe(
      "timerun",
      [this](const auto &args) {
        std::vector<std::string> params;
        params.reserve(args.size() + 1);
        params.emplace_back("timerun");
        params.insert(params.cend(), args.cbegin(), args.cend());

        parseServerCommand(params);
      },
      false);

  playerEventsHandler->subscribe("timerun:start", [](const auto &args) {
    if (args.empty()) {
      return;
    }

    if (Q_atoi(args[0]) == cg.clientNum) {
      execCmdOnRunStart();
    }
  });

  const auto runEnd = [](const auto &args) {
    if (args.empty()) {
      return;
    }

    if (Q_atoi(args[0]) == cg.clientNum) {
      execCmdOnRunEnd();
    }
  };

  playerEventsHandler->subscribe("timerun:stop", runEnd);
  playerEventsHandler->subscribe("timerun:interrupt", runEnd);
}

void Timerun::reset() {
  playersTimerunInformation[cg.clientNum] = {};
  // to reset the fade timestamp for 'etj_runtimerAutoHide'
  playersTimerunInformation[cg.clientNum].lastRunTimer = cg.time;
}

void Timerun::onStop(const TimerunCommands::Stop *stop) {
  playersTimerunInformation[stop->clientNum].completionTime =
      stop->completionTime;
  playersTimerunInformation[stop->clientNum].running = false;
  playersTimerunInformation[stop->clientNum].lastRunTimer = cg.time;

  playerEventsHandler->check("timerun:stop",
                             {std::to_string(stop->clientNum), stop->runName,
                              std::to_string(stop->completionTime)});
}

void Timerun::onInterrupt(const TimerunCommands::Interrupt *interrupt) {
  playersTimerunInformation[interrupt->clientNum].running = false;
  playersTimerunInformation[interrupt->clientNum].runName = "";
  playersTimerunInformation[interrupt->clientNum].completionTime = -1;
  playersTimerunInformation[interrupt->clientNum].previousRecord = 0;
  playersTimerunInformation[interrupt->clientNum].startTime = 0;
  playersTimerunInformation[interrupt->clientNum].lastRunTimer = cg.time;

  playerEventsHandler->check("timerun:interrupt",
                             {std::to_string(interrupt->clientNum)});
}

void Timerun::onCheckpoint(const TimerunCommands::Checkpoint *cp) {
  playersTimerunInformation[cp->clientNum].checkpoints[cp->checkpointNum] =
      cp->checkpointTime;
  playersTimerunInformation[cp->clientNum].numCheckpointsHit =
      cp->checkpointNum + 1;
  playersTimerunInformation[cp->clientNum].lastCheckpointTimestamp = cg.time;
  playersTimerunInformation[cp->clientNum]
      .checkpointIndicesHit[cp->checkpointIndex] = true;
}

void Timerun::onStart(const TimerunCommands::Start *start) {
  playersTimerunInformation[start->clientNum].startTime = start->startTime;
  playersTimerunInformation[start->clientNum].runName = start->runName;
  playersTimerunInformation[start->clientNum].previousRecord =
      start->previousRecord.value_or(-1);
  playersTimerunInformation[start->clientNum].running = true;
  playersTimerunInformation[start->clientNum].checkpoints =
      start->currentRunCheckpoints;
  playersTimerunInformation[start->clientNum].runHasCheckpoints =
      start->runHasCheckpoints;
  playersTimerunInformation[start->clientNum].numCheckpointsHit =
      Timerun::getNumCheckpointsHit(start->currentRunCheckpoints);
  playersTimerunInformation[start->clientNum].previousRecordCheckpoints =
      start->checkpoints;
  playersTimerunInformation[start->clientNum].checkpointIndicesHit.fill(false);

  playerEventsHandler->check(
      "timerun:start", {std::to_string(start->clientNum), start->runName,
                        std::to_string(start->startTime),
                        std::to_string(start->previousRecord.value_or(-1))});
}

void Timerun::printMessage(const std::string &message, int shader) {
  CG_AddPMItem(PM_MESSAGE, message.c_str(), shader);
}

void Timerun::onRecord(const TimerunCommands::Record *record) {
  std::string message = createCompletionMessage(
      cgs.clientinfo[record->clientNum], record->runName,
      record->completionTime, record->previousRecordTime);
  const qhandle_t shader = !record->previousRecordTime.has_value()
                               ? cgs.media.stopwatchIcon
                               : cgs.media.stopwatchIconGreen;
  printMessage(message, shader);

  playerEventsHandler->check(
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

  playerEventsHandler->check("timerun:completion",
                             {std::to_string(completion->clientNum),
                              completion->runName,
                              std::to_string(completion->completionTime)});
}

void Timerun::parseServerCommand(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return;
  }

  if (args[1] == "start" || args[1] == "saveposstart") {
    auto startOpt = TimerunCommands::Start::deserialize(args);
    if (!startOpt.has_value()) {
      return;
    }
    const auto start = &startOpt.value();

    onStart(start);
    return;
  }
  if (args[1] == "checkpoint") {
    auto checkpointOpt = TimerunCommands::Checkpoint::deserialize(args);
    if (!checkpointOpt.has_value()) {
      return;
    }

    const auto cp = &checkpointOpt.value();

    onCheckpoint(cp);
    return;
  }
  if (args[1] == "stop") {
    auto stopOpt = TimerunCommands::Stop::deserialize(args);
    if (!stopOpt.has_value()) {
      return;
    }

    const auto stop = &stopOpt.value();
    onStop(stop);
    return;
  }

  if (args[1] == "interrupt") {
    auto interruptOpt = TimerunCommands::Interrupt::deserialize(args);
    if (!interruptOpt.has_value()) {
      return;
    }

    const auto interrupt = &interruptOpt.value();
    onInterrupt(interrupt);
    return;
  }

  if (args[1] == "record") {
    auto recordOpt = TimerunCommands::Record::deserialize(args);
    if (!recordOpt.has_value()) {
      return;
    }

    const auto record = &recordOpt.value();
    onRecord(record);
    return;
  }

  if (args[1] == "completion") {
    auto completionOpt = TimerunCommands::Completion::deserialize(args);
    if (!completionOpt.has_value()) {
      return;
    }

    const auto completion = &completionOpt.value();
    onCompletion(completion);
  }
}

const Timerun::PlayerTimerunInformation *
Timerun::getTimerunInformationFor(int clientNum) {
  return &playersTimerunInformation[clientNum];
}

std::string Timerun::createCompletionMessage(const clientInfo_t &player,
                                             const std::string &runName,
                                             int completionTime,
                                             std::optional<int> previousTime) {
  std::string who{(player.clientNum == cg.clientNum) ? "You" : player.name};
  std::string timeFinished{millisToString(completionTime)};
  std::string timeDifference{""};

  if (previousTime.has_value()) {
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

void Timerun::execCmdOnRunStart() {
  if (etj_onRunStart.string[0]) {
    trap_SendConsoleCommand(va("%s\n", etj_onRunStart.string));
  }
}

void Timerun::execCmdOnRunEnd() {
  if (etj_onRunEnd.string[0]) {
    trap_SendConsoleCommand(va("%s\n", etj_onRunEnd.string));
  }
}
} // namespace ETJump
