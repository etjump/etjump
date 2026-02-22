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

#include <toml11/types.hpp>

#include "etj_client_commands_handler.h"
#include "etj_cvar_update_handler.h"

#include "../game/etj_command_parser.h"

namespace ETJump {
inline constexpr int32_t MAX_CUSTOM_COMMANDS = 40;
// 9 and 0 are reserved for prev/next page navigation
inline constexpr uint8_t CUSTOM_COMMAND_MENU_PAGE_SIZE = 8;
inline constexpr uint8_t CUSTOM_COMMAND_MENU_MAX_PAGES =
    MAX_CUSTOM_COMMANDS / CUSTOM_COMMAND_MENU_PAGE_SIZE;

class CustomCommandMenu {
public:
  CustomCommandMenu(
      const std::shared_ptr<CvarUpdateHandler> &cvarUpdateHandler,
      const std::shared_ptr<ClientCommandsHandler> &consoleCommandsHandler);
  ~CustomCommandMenu();

  struct CustomCommand {
    std::string name;
    std::string command;
  };

  [[nodiscard]] const std::map<
      uint8_t, std::array<CustomCommand, CUSTOM_COMMAND_MENU_PAGE_SIZE>> &
  getCustomCommands() const;

private:
  void startListeners();
  void parseCommands();
  void setFilename(const vmCvar_t *cvar);

  void addCommand(const std::vector<std::string> &args);
  void deleteCommand(const std::vector<std::string> &args);
  void editCommand(const std::vector<std::string> &args);
  void moveCommand(const std::vector<std::string> &args);
  void listCommands(const std::vector<std::string> &args) const;

  bool validateAddCommand(const CommandParser::Command &optCommand,
                          std::string &name, std::string &cmd,
                          std::optional<uint8_t> &page,
                          std::optional<uint8_t> &slot) const;
  bool validateDeleteCommand(const CommandParser::Command &optCommand,
                             std::optional<uint8_t> &page,
                             std::optional<uint8_t> &slot) const;
  [[nodiscard]] bool
  validateEditCommand(const CommandParser::Command &optCommand, uint8_t page,
                      uint8_t slot) const;

  [[nodiscard]] bool
  validateMoveCommand(const CommandParser::Command &optCommand,
                      uint8_t fromPage, uint8_t fromSlot, uint8_t toPage,
                      std::optional<uint8_t> &toSlot) const;

  [[nodiscard]] inline bool commandsFull() const;
  [[nodiscard]] inline bool pageIsFull(uint8_t page) const;

  // returns '0' if no free pages are left
  [[nodiscard]] uint8_t findFreePage() const;
  // returns '0' if no free slots are found on the page
  [[nodiscard]] uint8_t findFreeSlot(uint8_t page) const;
  // returns '0' if page is invalid, or no commands are found
  [[nodiscard]] uint8_t findSlotForDeletion(uint8_t page) const;

  static void sortTable(toml::ordered_value &table, const std::string &key);

  static void generateExampleFile(const std::vector<std::string> &args);

  [[nodiscard]] bool readFile(toml::ordered_value &table) const;
  [[nodiscard]] bool writeFile(const toml::ordered_value &table) const;

  std::string customCommandMenuFile;
  std::map<uint8_t, std::array<CustomCommand, CUSTOM_COMMAND_MENU_PAGE_SIZE>>
      commands;
  std::shared_ptr<CvarUpdateHandler> cvarUpdateHandler;
  std::shared_ptr<ClientCommandsHandler> consoleCommandsHandler;
};
} // namespace ETJump
