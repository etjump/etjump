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
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace ETJump {
class ClientCommandsHandler;

class AwaitedCommandHandler {
public:
  struct AwaitedCommand {
    AwaitedCommand() : currentFrameCount(0), requiredFrameCount(0) {}
    int currentFrameCount;
    int requiredFrameCount;
    std::vector<std::string> commands;
  };

  AwaitedCommandHandler(
      std::shared_ptr<ClientCommandsHandler> consoleCommandsHandler,
      std::function<void(const char *)> sendConsoleCommand,
      std::function<void(const char *)> printToConsole);
  ~AwaitedCommandHandler();

  void runFrame();

private:
  void message(const std::string &message);
  void executeConsoleCommand(const std::string &command);
  void awaitCommand(const std::vector<std::string> &args);
  std::shared_ptr<ClientCommandsHandler> _consoleCommandsHandler;
  std::vector<std::unique_ptr<AwaitedCommand>> _awaitedCommands;
  std::function<void(const char *)> _sendConsoleCommand;
  std::function<void(const char *)> _printToConsole;
};
} // namespace ETJump
