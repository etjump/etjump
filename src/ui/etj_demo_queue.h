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
#include <vector>
#include <array>
#include <functional>

namespace ETJump {
class DemoQueue {
public:
  DemoQueue();
  ~DemoQueue() = default;

  void commandHandler();
  void setManualSkip();

  bool manualSkip{};

private:
  std::vector<std::string> queue;
  std::string demoExt;

  void initCommands();
  void initProtocol();
  void initQueue();
  static void playDemo();

  void start() const;
  static void stop();
  void restart() const;
  void gotoDemo(const std::vector<std::string> &args);
  void printStatus();
  void printHelp(const std::string &cmd) const;

  int getCurrentDemoIndex();
  bool isValidCommand(const std::string &cmd) const;
  bool canStartPlayback() const;
  static bool playbackActive();
  static void disableManualSkip();

  struct DemoQueueCommand {
    std::string cmdString;
    std::string helpText;
    std::function<void()> callback;
  };

  static constexpr int NUM_DEMOQUEUE_COMMANDS = 8;
  std::array<DemoQueueCommand, NUM_DEMOQUEUE_COMMANDS> commands{};
};
} // namespace ETJump
