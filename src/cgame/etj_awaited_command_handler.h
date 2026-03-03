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

#include <memory>
#include <string>
#include <vector>

namespace ETJump {
class ClientCommandsHandler;
class PlayerEventsHandler;

class AwaitedCommandHandler {
public:
  struct AwaitedCommand {
    int currentFrameCount{};
    int requiredFrameCount{};
    std::vector<std::string> commands;
  };

  AwaitedCommandHandler(
      const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
      const std::shared_ptr<PlayerEventsHandler> &playerEvents);
  ~AwaitedCommandHandler();

  void runFrame();

private:
  void startListeners();
  static void message(const std::string &message);
  static void executeConsoleCommand(const std::string &command);
  void awaitCommand(const std::vector<std::string> &args);

  std::vector<std::unique_ptr<AwaitedCommand>> awaitedCommands;

  std::shared_ptr<ClientCommandsHandler> consoleCommands;
  std::shared_ptr<PlayerEventsHandler> playerEvents;
};
} // namespace ETJump
