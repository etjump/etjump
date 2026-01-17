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

#include "etj_custom_command_menu.h"
#include "etj_client_commands_handler.h"

#include "../game/etj_filesystem.h"

namespace ETJump {
inline constexpr char CUSTOM_COMMAND_FILE[] = "custom_commands.dat";

CustomCommandMenu::CustomCommandMenu() {
  startListeners();
  parseCommands();
}

void CustomCommandMenu::startListeners() {
  consoleCommandsHandler->subscribe(
      "addCustomCommand",
      [this](const std::vector<std::string> &args) { addCommand(args); });

  consoleCommandsHandler->subscribe(
      "deleteCustomCommand",
      [this](const std::vector<std::string> &args) { deleteCommand(args); });

  consoleCommandsHandler->subscribe(
      "listCustomCommands",
      [this](const std::vector<std::string> &) { listCommands(); });

  consoleCommandsHandler->subscribe(
      "readCustomCommands",
      [this](const std::vector<std::string> &) { parseCommands(); });
}

void CustomCommandMenu::parseCommands() {
  // always clear the commands, so if the user deletes the file and
  // executes 'readCustomCommands' to refresh, the commands are gone
  commands.clear();

  if (!FileSystem::exists(CUSTOM_COMMAND_FILE)) {
    return;
  }

  Json::Value root;

  if (!readFile(root)) {
    return;
  }

  std::string err;
  int8_t currentPage = 0;

  for (const auto &obj : root) {
    CustomCommand cmd;

    if (!JsonUtils::parseValue(cmd.name, obj["name"], &err, "name") ||
        !JsonUtils::parseValue(cmd.command, obj["command"], &err, "command")) {
      CG_Printf("Failed to parse command from custom command file: %s\n",
                err.c_str());
      continue;
    }

    commands[currentPage].emplace_back(cmd);

    if (commands[currentPage].size() == CUSTOM_COMMAND_MENU_PAGE_SIZE) {
      currentPage++;
    }

    // failsafe in case user has added too many commands manually
    if (commandsFull()) {
      break;
    }
  }
}

// TODO: new command parser stuff from timeruns!
void CustomCommandMenu::addCommand(const std::vector<std::string> &args) {
  if (args.empty() || args.size() < 2) {
    CG_Printf("^3Usage: ^7addCustomCommand <name> <command>\nIf 'name' or "
              "'command' contains spaces or semicolons, enclose the entire "
              "argument in quotes.\n");
    return;
  }

  if (commandsFull()) {
    CG_Printf("You may not add more than ^3%i ^7custom commands.\n",
              MAX_CUSTOM_COMMANDS);
    return;
  }

  Json::Value root;

  // try to append to existing file if present
  if (FileSystem::exists(CUSTOM_COMMAND_FILE) && !readFile((root))) {
    return;
  }

  Json::Value cmd;

  cmd["name"] = args[0];
  cmd["command"] = args[1];

  root.append(cmd);

  if (writeFile(root)) {
    parseCommands();
  }
}

void CustomCommandMenu::deleteCommand(const std::vector<std::string> &args) {
  if (args.empty()) {
    CG_Printf("^3Usage: ^7deleteCustomCommand <number>\n"
              "See 'listCustomCommands' for command numbers.\n");
    return;
  }

  if (!FileSystem::exists(CUSTOM_COMMAND_FILE)) {
    CG_Printf("Unable to find ^3'%s' ^7file.\n", CUSTOM_COMMAND_FILE);
    return;
  }

  Json::Value root;

  if (!readFile(root)) {
    return;
  }

  const int32_t index = Q_atoi(args[0].c_str());
  const auto numCommands =
      std::min(static_cast<int32_t>(root.size()), MAX_CUSTOM_COMMANDS);

  if (index < 1 || index > numCommands) {
    CG_Printf("Invalid command number, valid range is ^31-%i\n", numCommands);
    return;
  }

  Json::Value removed;
  root.removeIndex(index - 1, &removed);

  if (writeFile(root)) {
    CG_Printf("Removed custom command ^3'%s'\n", removed["name"].asCString());
    parseCommands();
  }
}

void CustomCommandMenu::listCommands() const {
  if (commands.empty()) {
    CG_Printf("No commands saved.\n");
    return;
  }

  CG_Printf("Saved custom commands:\n\n");

  for (const auto &[page, cmds] : commands) {
    for (size_t i = 0; i < cmds.size(); i++) {
      CG_Printf("%2i.  ^3Name:    ^7%s\n"
                "     ^3Command: ^z%s\n",
                (i + 1) +
                    static_cast<size_t>(page * CUSTOM_COMMAND_MENU_PAGE_SIZE),
                cmds[i].name.c_str(), cmds[i].command.c_str());
    }
  }
}

inline bool CustomCommandMenu::commandsFull() const {
  return commands.size() == CUSTOM_COMMAND_MENU_MAX_PAGES &&
         commands.at(CUSTOM_COMMAND_MENU_MAX_PAGES - 1).size() ==
             CUSTOM_COMMAND_MENU_PAGE_SIZE;
}

const std::map<uint8_t, std::vector<CustomCommandMenu::CustomCommand>> &
CustomCommandMenu::getCustomCommands() const {
  return commands;
}

bool CustomCommandMenu::readFile(Json::Value &root) {
  std::string err;

  if (!JsonUtils::readFile(CUSTOM_COMMAND_FILE, root, &err)) {
    CG_Printf("Failed to read custom command file: %s\n", err.c_str());
    return false;
  }

  return true;
}

bool CustomCommandMenu::writeFile(const Json::Value &root) {
  std::string err;

  if (!JsonUtils::writeFile(CUSTOM_COMMAND_FILE, root, &err)) {
    CG_Printf("Failed to write custom command file: %s\n", err.c_str());
    return false;
  }

  return true;
}
} // namespace ETJump
