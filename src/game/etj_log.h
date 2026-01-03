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
#include <mutex>
#include <vector>
#include <string>

#include "etj_string_utilities.h"

namespace ETJump {
/**
 * Thread safe implementation of a logger
 * All messages are queued and main thread will process them on next frame
 */
class Log {
public:
  explicit Log(std::string name) : _name(std::move(name)) {}

  template <typename... Targs>
  void info(const std::string &format, const Targs &...fargs) const {
    println("info", stringFormat(format, fargs...));
  }

  template <typename... Targs>
  void error(const std::string &format, const Targs &...fargs) const {
    println("error", stringFormat(format, fargs...));
  }

  static void processMessages();

private:
  std::string _name;

  void println(const std::string &level, const std::string &message) const;
  static std::mutex _messagesLock;
  static std::vector<std::string> _messages;
};
} // namespace ETJump
