/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
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

#include "etj_fireteam_countdown.h"
#include "g_local.h"

namespace ETJump {
void FireteamCountdown::runFrame() {
  // we can't erase the completed commands in the middle of the loop
  // because it invalidates the iterator, so just add to-be-deleted
  // commands here and delete them once everything is processed
  std::vector<int> completed{};

  for (auto &[clientNum, cmd] : countdownCommands) {
    if (level.time > cmd.nextMessageTime) {
      sendCountdownMessage(clientNum);
      cmd.seconds--;
      cmd.nextMessageTime += 1000;

      if (cmd.seconds < 0) {
        completed.push_back(clientNum);
      }
    }
  }

  for (const auto &cmd : completed) {
    countdownCommands.erase(cmd);
  }
}

void FireteamCountdown::addCountdown(const int clientNum,
                                     const int8_t seconds) {
  countdownCommands[clientNum].seconds = seconds;
  countdownCommands[clientNum].nextMessageTime = level.time + level.frameTime;
}

void FireteamCountdown::removeCountdown(const int clientNum) {
  countdownCommands.erase(clientNum);
}

void FireteamCountdown::sendCountdownMessage(const int clientNum) {
  gentity_t *ent = g_entities + clientNum;
  char message[MAX_SAY_TEXT]{};

  Com_sprintf(message, sizeof(message),
              countdownCommands[clientNum].seconds > 0
                  ? std::to_string(countdownCommands[clientNum].seconds).c_str()
                  : "GO!");
  G_Say(ent, nullptr, SAY_BUDDY, qtrue, message);
}
} // namespace ETJump
