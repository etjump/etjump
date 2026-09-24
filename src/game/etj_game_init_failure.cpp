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

#include "etj_game_init_failure.h"

namespace ETJump {
bool GameInitFailure::isActive() const { return active; }

void GameInitFailure::activate(const char *errorMessage, const bool unexpected,
                               const int time) {
  message = errorMessage;
  denialMessage = unexpected ? UNEXPECTED_ERROR_DENIAL : message;
  failTime = time;
  active = true;
}

void GameInitFailure::reset() { active = false; }

const char *GameInitFailure::getMessage() const { return message.c_str(); }

const char *GameInitFailure::getDenialMessage() const {
  return denialMessage.c_str();
}

bool GameInitFailure::shouldRaise(const int time) const {
  return active && time - failTime >= RAISE_DELAY;
}
} // namespace ETJump
