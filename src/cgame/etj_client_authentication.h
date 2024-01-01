/*
 * MIT License
 *
 * Copyright (c) 2024 ETJump team <zero@etjump.com>
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
#include <functional>
#include "etj_client_commands_handler.h"

namespace ETJump {
class ClientAuthentication {
public:
  struct OperationResult {
    bool success;
    std::string message;
  };

  explicit ClientAuthentication(
      std::function<void(const std::string &)> sendClientCommand,
      std::function<void(const std::string &)> print,
      std::function<std::string()> getHwid,
      std::shared_ptr<ClientCommandsHandler> serverCommandsHandler);
  ~ClientAuthentication();

  void login();

private:
  std::function<void(const std::string &)> _sendClientCommand;
  std::function<void(const std::string &)> _print;
  std::function<std::string()> _getHwid;
  std::shared_ptr<ClientCommandsHandler> _serverCommandsHandler;
  const std::string GUID_FILE;
  std::string getGuid();
  std::string getHwid();
  std::string createGuid() const;
  OperationResult saveGuid(const std::string &guid) const;
};
} // namespace ETJump
