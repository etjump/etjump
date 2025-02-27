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

#pragma once
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <memory>

namespace ETJump {
class ClientCommandsHandler {
public:
  explicit ClientCommandsHandler(void(const char *));
  ~ClientCommandsHandler();

  // returns true if a match was found and function was called
  bool check(const std::string &command,
             const std::vector<std::string> &arguments);

  // registers a command handler that will be called if the command was
  // received from the server returns false if handler with the same
  // name already exists
  bool subscribe(const std::string &command,
                 std::function<void(const std::vector<std::string> &)> callback,
                 bool autocomplete = true);

  // unsubscribes the command handler
  // returns false if it does not exist
  bool unsubscribe(const std::string &command);

private:
  void (*_addToAutocompleteList)(const char *command);

  std::map<std::string, std::function<void(std::vector<std::string>)>>
      _callbacks;
};
} // namespace ETJump
