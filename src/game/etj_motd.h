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

/*
 * Prints a message of the day when client connects to server.
 * g_motdFile is the file where the motd is defined. The format is
 * in json.
 * Two messages are sent: the actual motd and also a message in chat
 * to inform that there's a motd in the console.
 * Example format:
 * {
 *   "chat_message": "Welcome to the server! Check out console for more
 * information.", "console_message": "Hi!"
 * }
 */

#pragma once

#include <string>
#include "g_local.h"
#include "etj_log.h"

namespace ETJump {
class Motd {
  bool initialized;
  std::string chatMotd;
  std::string consoleMotd;

  std::unique_ptr<Log> logger;
  std::string errors;

public:
  explicit Motd(std::unique_ptr<Log> log);
  ~Motd() = default;

  void initialize();
  void generateMotdFile();
  void printMotd(const gentity_t *ent) const;
};
} // namespace ETJump
