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

#pragma once
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "cg_local.h"
#include "../game/q_shared.h"
#include "../game/etj_shared.h"

namespace ETJump {
namespace TimerunCommands {
class Completion;
class Record;
class Start;
class Checkpoint;
class Interrupt;
class Stop;
} // namespace TimerunCommands

class PlayerEventsHandler;

class Timerun {
public:
  Timerun();

  struct PlayerTimerunInformation {
    PlayerTimerunInformation() {
      previousRecordCheckpoints.fill(TIMERUN_CHECKPOINT_NOT_SET);
      checkpoints.fill(TIMERUN_CHECKPOINT_NOT_SET);
    };
    int startTime{};
    int completionTime{};
    std::string runName;
    int previousRecord{};
    std::array<int, MAX_TIMERUN_CHECKPOINTS> previousRecordCheckpoints{};
    bool running{};
    // used for fading
    int lastRunTimer{};
    int numCheckpointsHit{};
    std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints{};
    int nextFreeCheckpointIdx{};
  };

  explicit Timerun(int clientNum,
                   std::shared_ptr<PlayerEventsHandler> playerEventsHandler)
      : _clientNum(clientNum), _playerEventsHandler(playerEventsHandler) {}

  void onStop(const TimerunCommands::Stop *stop);
  void onInterrupt(const TimerunCommands::Interrupt *interrupt);
  void onCheckpoint(const TimerunCommands::Checkpoint *cp);
  void onStart(const TimerunCommands::Start *start);
  void printMessage(const std::string &message, int shader);
  void onRecord(const TimerunCommands::Record *record);
  void onCompletion(const TimerunCommands::Completion *completion);
  void parseServerCommand(const std::vector<std::string> &args);
  const PlayerTimerunInformation *getTimerunInformationFor(int clientNum);
  static int getNumCheckpointsHit(
      const std::array<int, MAX_TIMERUN_CHECKPOINTS> currentRunCheckpoints);

private:
  int _clientNum;

  std::string createCompletionMessage(const clientInfo_t &player,
                                      const std::string &runName,
                                      int completionTime,
                                      ETJump::opt<int> previousTime);

  std::array<PlayerTimerunInformation, MAX_CLIENTS> _playersTimerunInformation;
  std::shared_ptr<PlayerEventsHandler> _playerEventsHandler;
};
} // namespace ETJump
