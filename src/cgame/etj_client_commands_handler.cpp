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

#include "etj_client_commands_handler.h"
#include "../game/etj_string_utilities.h"

ETJump::ClientCommandsHandler::ClientCommandsHandler(
    void (*addToAutocompleteList)(const char *))
    : _addToAutocompleteList{addToAutocompleteList} {
  _callbacks.clear();
}

ETJump::ClientCommandsHandler::~ClientCommandsHandler() {}

bool ETJump::ClientCommandsHandler::check(
    const std::string &command, const std::vector<std::string> &arguments) {
  auto lowercasedCommand = ETJump::StringUtil::toLowerCase(command);
  auto match = _callbacks.find(lowercasedCommand);
  if (match != end(_callbacks)) {
    match->second(arguments);
    return true;
  }
  return false;
}

bool ETJump::ClientCommandsHandler::subscribe(
    const std::string &command,
    std::function<void(const std::vector<std::string> &)> callback,
    bool autocomplete) {
  auto lowercasedCommand = ETJump::StringUtil::toLowerCase(command);
  if (_callbacks.find(lowercasedCommand) != end(_callbacks)) {
    return false;
  }

  _callbacks[lowercasedCommand] = callback;
  if (_addToAutocompleteList != nullptr && autocomplete) {
    _addToAutocompleteList(command.c_str());
  }
  return true;
}

bool ETJump::ClientCommandsHandler::unsubscribe(const std::string &command) {
  auto lowercasedCommand = ETJump::StringUtil::toLowerCase(command);
  auto callback = _callbacks.find(lowercasedCommand);
  if (callback != end(_callbacks)) {
    return false;
  }

  _callbacks.erase(callback);
  return true;
}
