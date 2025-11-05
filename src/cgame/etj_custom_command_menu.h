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

#include "cg_local.h"

#include "../game/etj_json_utilities.h"

namespace ETJump {
inline constexpr int32_t MAX_CUSTOM_COMMANDS = 40;
// 9 and 0 are reserved for prev/next page navigation
inline constexpr uint8_t CUSTOM_COMMAND_MENU_PAGE_SIZE = 8;
inline constexpr uint8_t CUSTOM_COMMAND_MENU_MAX_PAGES =
    MAX_CUSTOM_COMMANDS / CUSTOM_COMMAND_MENU_PAGE_SIZE;

class CustomCommandMenu {
public:
  CustomCommandMenu();
  ~CustomCommandMenu() = default;

  struct CustomCommand {
    std::string name;
    std::string command;
  };

  [[nodiscard]] const std::map<uint8_t, std::vector<CustomCommand>> &
  getCustomCommands() const;

private:
  void startListeners();
  void parseCommands();
  void addCommand(const std::vector<std::string> &args);
  void deleteCommand(const std::vector<std::string> &args);
  void editCommand();
  void listCommands() const;
  [[nodiscard]] inline bool commandsFull() const;

  [[nodiscard]] static bool readFile(Json::Value &root);
  static bool writeFile(const Json::Value &root);

  std::map<uint8_t, std::vector<CustomCommand>> commands;
};
} // namespace ETJump
