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

#include <cstdio>

#include "etj_game_failure.h"

namespace ETJump {
GameFailure::GameFailure() { reset(); }

bool GameFailure::isActive() const { return active; }

void GameFailure::activate(const char *errorMessage, const bool unexpected,
                           const bool dropClients, const int time) {
  std::snprintf(message.data(), message.size(), "%s", errorMessage);
  std::snprintf(denialMessage.data(), denialMessage.size(), "%s",
                unexpected ? GAME_FAILURE_GENERIC_DENIAL : errorMessage);

  // the engine sends the reason as 'disconnect "<reason>"', so quotes and
  // line breaks would cut it short, and some clients use it as a format
  // string, so a '%' could crash them
  for (char *c = denialMessage.data(); *c; ++c) {
    if (*c == '"') {
      *c = '\'';
    } else if (*c == '\n' || *c == '\r' || *c == '%') {
      *c = ' ';
    }
  }

  dropsPending = dropClients;
  dropTime = time;
  shutdownQueued = false;
  localClientDropped = false;
  active = true;
}

void GameFailure::reset() {
  active = false;
  localClientDropped = false;
  message[0] = '\0';

  // e.g. for a failing connect nested inside a syscall, which is denied
  // without the game going inert
  std::snprintf(denialMessage.data(), denialMessage.size(), "%s",
                GAME_FAILURE_GENERIC_DENIAL);
}

const char *GameFailure::getDenialMessage() const {
  return denialMessage.data();
}

const char *GameFailure::getMessage() const { return message.data(); }

bool GameFailure::takeClientDrops(const int time) {
  if (!active || !dropsPending) {
    return false;
  }

  dropsPending = false;
  dropTime = time;
  return true;
}

void GameFailure::setLocalClientDropped() { localClientDropped = true; }

bool GameFailure::takeShutdown(const int time) {
  if (!active || dropsPending || shutdownQueued ||
      time - dropTime < GAME_FAILURE_SHUTDOWN_DELAY) {
    return false;
  }

  shutdownQueued = true;
  return true;
}

bool GameFailure::shutsDownThroughError(const bool dedicated) const {
  return !dedicated && !localClientDropped;
}
} // namespace ETJump
