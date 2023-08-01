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
#include <string>
#include "etj_drawable.h"

namespace ETJump {
struct PlayerTimerunInformation {
  PlayerTimerunInformation() {
    previousRecordCheckpoints.fill(TIMERUN_CHECKPOINT_NOT_SET);
    checkpoints.fill(TIMERUN_CHECKPOINT_NOT_SET);
  };
  int startTime{};
  int completionTime{};
  std::string runName;
  int previousRecord{};
  std::array<int, MAX_TIMERUN_CHECKPOINTS> previousRecordCheckpoints;
  bool running{};
  // used for fading
  int lastRunTimer{};
  int numCheckpointsHit{};
  std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints;
  int nextFreeCheckpointIdx{};
};

class TimerunView : public Drawable {
public:
  explicit TimerunView(std::shared_ptr<PlayerEventsHandler> playerEventsHandler);
  ~TimerunView();
  static const int MaxClients = 64;
  // whenever server sends any command that starts with
  // `timerun` this will be called
  bool parseServerCommand();

  // whenever the player starts a timerun
  void start();

  // whenever the player stops a timerun
  void stop();

  // whenever the player hits a checkpoint
  void checkpoint();

  // whenever the player's timerun is interrupted (not finished)
  void interrupt();
  static void interrupt(PlayerTimerunInformation &playerTimerunInformation);

  // draws the timer
  void draw();

  int getTransitionRange(int previousRunTime);

  void pastRecordAnimation(vec4_t *color, const char *text, int timerTime,
                           int record);

  // returns the currently active run if there's any
  // e.g. if player is running => return player's run,
  // else if player is running and we're speccing the player
  // => return that player's run
  const PlayerTimerunInformation *currentRun() const;

private:
  std::string getTimerString(const int msec);

  std::array<PlayerTimerunInformation, MaxClients> _playersTimerunInformation;
  vec4_t inactiveTimerColor;
  std::shared_ptr<PlayerEventsHandler> _playerEventsHandler;

  bool canSkipDraw() const;
};
} // namespace ETJump
