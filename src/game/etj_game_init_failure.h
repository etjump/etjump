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

#pragma once

#include <string>

namespace ETJump {
// A fatal error while loading a map isn't raised right away. The engine
// doesn't notify remote clients when it handles an error, so they would sit
// there until they time out. Instead, the clients are turned away with the
// error message first, and the error is raised a moment later, once the
// engine had the chance to deliver the disconnect messages.
class GameInitFailure {
  bool active = false;
  int failTime = 0;
  std::string message;
  std::string denialMessage;

public:
  // real time to wait before raising the error, the disconnect messages
  // are delivered to the dropped clients in the next server frames
  static constexpr int RAISE_DELAY = 1000;

  // the text of unexpected errors (unhandled exceptions) might contain
  // internal details, so clients only get a generic message for those
  static constexpr char UNEXPECTED_ERROR_DENIAL[] =
      "Server failed to load the map due to an internal error.";

  bool isActive() const;
  void activate(const char *errorMessage, bool unexpected, int time);

  // leaves the message intact, it's still needed to raise the error
  void reset();

  const char *getMessage() const;

  // reason given to clients that try to connect
  const char *getDenialMessage() const;

  bool shouldRaise(int time) const;
};
} // namespace ETJump
