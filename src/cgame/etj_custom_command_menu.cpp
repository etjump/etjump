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
#include "etj_consolecommands.h"
#include "etj_cvar_update_handler.h"
#include "etj_client_commands_handler.h"

#include "../game/etj_filesystem.h"
#include "../game/etj_toml_utilities.h"

namespace ETJump {
inline constexpr char DEFAULT_CUSTOM_COMMAND_FILE[] = "custom_commands.dat";

CustomCommandMenu::CustomCommandMenu(
    const std::shared_ptr<CvarUpdateHandler> &cvarUpdateHandler,
    const std::shared_ptr<ClientCommandsHandler> &consoleCommandsHandler)
    : cvarUpdateHandler(cvarUpdateHandler),
      consoleCommandsHandler(consoleCommandsHandler) {

  startListeners();
  setFilename(&etj_ccMenu_filename);

  parseCommands();
}

CustomCommandMenu::~CustomCommandMenu() {
  cvarUpdateHandler->unsubscribe(&etj_ccMenu_filename);

  consoleCommandsHandler->unsubscribe("addCustomCommand");
  consoleCommandsHandler->unsubscribe("deleteCustomCommand");
  consoleCommandsHandler->unsubscribe("listCustomCommands");
  consoleCommandsHandler->unsubscribe("readCustomCommands");
}

void CustomCommandMenu::startListeners() {
  cvarUpdateHandler->subscribe(
      &etj_ccMenu_filename,
      [this](const vmCvar_t *cvar) { setFilename(cvar); });

  consoleCommandsHandler->subscribe(
      "addCustomCommand",
      [this](const std::vector<std::string> &args) { addCommand(args); });

  consoleCommandsHandler->subscribe(
      "deleteCustomCommand",
      [this](const std::vector<std::string> &args) { deleteCommand(args); });

  consoleCommandsHandler->subscribe(
      "listCustomCommands",
      [this](const std::vector<std::string> &args) { listCommands(args); });

  consoleCommandsHandler->subscribe(
      "readCustomCommands",
      [this](const std::vector<std::string> &) { parseCommands(); });
}

void CustomCommandMenu::setFilename(const vmCvar_t *cvar) {
  if (cvar->string[0] != '\0') {
    customCommandMenuFile = cvar->string;
    customCommandMenuFile += ".dat";
  } else {
    customCommandMenuFile = DEFAULT_CUSTOM_COMMAND_FILE;
  }

  parseCommands();
}

void CustomCommandMenu::parseCommands() {
  // always clear the commands, so if the user deletes the file and
  // executes 'readCustomCommands' to refresh, the commands are gone
  commands.clear();

  if (!FileSystem::exists(customCommandMenuFile)) {
    return;
  }

  toml::ordered_value table;

  if (!readFile(table)) {
    return;
  }

  for (uint8_t i = 1; i <= CUSTOM_COMMAND_MENU_MAX_PAGES; i++) {
    try {
      if (!table.contains("page-" + std::to_string(i))) {
        continue;
      }

      const auto &page = table.at("page-" + std::to_string(i));

      for (uint8_t j = 1; j <= CUSTOM_COMMAND_MENU_PAGE_SIZE; j++) {
        std::optional<std::string> name;
        std::optional<std::string> cmd;

        name = toml::find<std::optional<std::string>>(
            page, "name-" + std::to_string(j));
        cmd = toml::find<std::optional<std::string>>(
            page, "command-" + std::to_string(j));

        if (!name.has_value() || !cmd.has_value()) {
          continue;
        }

        CustomCommand command{};
        command.name = name.value();
        command.command = cmd.value();

        // we are looping 1-indexed to match the field names
        commands[i][j - 1] = command;
      }
    } catch (const toml::type_error &e) {
      CG_Printf("Skipping custom command parsing for page ^3%i^7: %s\n", i,
                TOMLUtils::getError(e.what()).c_str());
      continue;
    }
  }
}

void CustomCommandMenu::addCommand(const std::vector<std::string> &args) {
  if (commandsFull()) {
    CG_Printf("You may not add more than ^3%i ^7custom commands.\n",
              MAX_CUSTOM_COMMANDS);
    return;
  }

  constexpr char desc[] = R"(Adds a new custom command
    /addCustomCommand --name <name> --command <command> --page <page> --slot <slot>

    Has a shorthand format of
    /addCustomCommand <name> <command>
    /addCustomCommand <name> <command> <page>
    /addCustomCommand <name> <command> <page> <slot>)";

  const auto optCommand = ConsoleCommands::getOptCommand(
      "addCustomCommand",
      CommandParser::CommandDefinition::create("addCustomCommand", desc)
          .addOption("name", "n",
                     "Name of the command, displayed in the custom command "
                     "menu. Maximum displayed length is 28 characters.",
                     CommandParser::OptionDefinition::Type::MultiToken, false)
          .addOption(
              "command", "c",
              "The command to execute. If chaining multiple commands with "
              "semicolon, ensure the entire command is inside quotes.",
              CommandParser::OptionDefinition::Type::MultiToken, false)
          .addOption("page", "p",
                     "The page to add the command to. Default is the first "
                     "page with a free slot.",
                     CommandParser::OptionDefinition::Type::Integer, false)
          .addOption("slot", "s",
                     "The slot on the page to add the command to. Default is "
                     "the first free slot on a page. You must also specify "
                     "'page' option for this to work.",
                     CommandParser::OptionDefinition::Type::Integer, false),
      args);

  if (!optCommand.has_value()) {
    return;
  }

  std::string name;
  std::string cmd;
  std::optional<uint8_t> page;
  std::optional<uint8_t> slot;

  if (!validateAddCommand(optCommand.value(), name, cmd, page, slot)) {
    return;
  }

  toml::ordered_value table;

  // try to append to existing file if present
  if (FileSystem::exists(customCommandMenuFile) && !readFile(table)) {
    return;
  }

  const std::string slotName = "name-" + std::to_string(slot.value());
  const std::string slotCmd = "command-" + std::to_string(slot.value());

  table["page-" + std::to_string(page.value())][slotName] = name;
  table["page-" + std::to_string(page.value())][slotCmd] = cmd;

  if (writeFile(table)) {
    parseCommands();
  }
}

bool CustomCommandMenu::validateAddCommand(
    const CommandParser::Command &optCommand, std::string &name,
    std::string &cmd, std::optional<uint8_t> &page,
    std::optional<uint8_t> &slot) const {
  const auto optName = optCommand.getOptional("name");
  const auto optCmd = optCommand.getOptional("command");
  const auto optPage = optCommand.getOptional("page");
  const auto optSlot = optCommand.getOptional("slot");

  const auto opFailedChatMsg = []() {
    CG_AddToTeamChat("^3addCustomCommand: ^7operation failed. Check console "
                     "for more information.",
                     TEAM_SPECTATOR);
  };

  if (optCommand.extraArgs.size() > 1) {
    switch (optCommand.extraArgs.size()) {
      case 2:
        name = optCommand.extraArgs[0];
        cmd = optCommand.extraArgs[1];
        break;
      case 3:
        name = optCommand.extraArgs[0];
        cmd = optCommand.extraArgs[1];
        page = static_cast<uint8_t>(Q_atoi(optCommand.extraArgs[2]));
        break;
      default: // >= 4
        name = optCommand.extraArgs[0];
        cmd = optCommand.extraArgs[1];
        page = static_cast<uint8_t>(Q_atoi(optCommand.extraArgs[2]));
        slot = static_cast<uint8_t>(Q_atoi(optCommand.extraArgs[3]));
        break;
    }
  }

  if (name.empty()) {
    if (!optName.has_value()) {
      opFailedChatMsg();
      CG_Printf("Required option `name` was not specified.\n");
      return false;
    }

    name = optName.value().text;
  }

  if (cmd.empty()) {
    if (!optCmd.has_value()) {
      opFailedChatMsg();
      CG_Printf("Required option `command` was not specified.\n");
      return false;
    }

    cmd = optCmd.value().text;
  }

  // if the user gave slot but no page, the command is invalid
  if (optSlot.has_value() && !optPage.has_value()) {
    opFailedChatMsg();
    CG_Printf(
        "Parameter `slot` requires parameter `page` to be explicitly set.\n");
    return false;
  }

  if (!page.has_value()) {
    if (!optPage.has_value()) {
      // we've already checked for commands being full,
      // this will always return a valid page
      page = findFreePage();
    } else {
      page = static_cast<uint8_t>(optPage.value().integer);

      if (page.value() == 0 || page.value() > CUSTOM_COMMAND_MENU_MAX_PAGES) {
        opFailedChatMsg();
        CG_Printf("Parameter `page` out of range (1-5).\n");
        return false;
      }
    }
  }

  if (commands.find(page.value()) != commands.cend() &&
      pageIsFull(page.value())) {
    opFailedChatMsg();
    CG_Printf("Requested page ^3%i ^7is full.\n", page.value());
    return false;
  }

  if (!slot.has_value()) {
    if (!optSlot.has_value()) {
      slot = findFreeSlot(page.value());
    } else {
      slot = static_cast<uint8_t>(optSlot.value().integer);
    }
  }

  if (slot.value() == 0 || slot.value() > CUSTOM_COMMAND_MENU_PAGE_SIZE) {
    opFailedChatMsg();
    CG_Printf("Parameter `slot` out of range (1-8).\n");
    return false;
  }

  if (commands.find(page.value()) != commands.cend() &&
      (!commands.at(page.value())[slot.value() - 1].name.empty() &&
       !commands.at(page.value())[slot.value() - 1].command.empty())) {
    opFailedChatMsg();
    CG_Printf("Slot ^3%i ^7on page ^3%i ^7already contains a command.\n",
              slot.value(), page.value());
    return false;
  }

  return true;
}

void CustomCommandMenu::deleteCommand(const std::vector<std::string> &args) {
  constexpr char desc[] = R"(Deletes a command from the custom command file.
    /deleteCustomCommand --page <page> --slot <slot>

    Has a shorthand format of:
    /deleteCustomCommand <page>
    /deleteCustomCommand <page> <slot>)";

  const auto optCommand = ConsoleCommands::getOptCommand(
      "deleteCustomCommand",
      CommandParser::CommandDefinition::create("deleteCustomCommand", desc)
          .addOption("page", "p", "Page to delete the command from.",
                     CommandParser::OptionDefinition::Type::Integer, true, 0)
          .addOption("slot", "s",
                     "Slot to delete the command from. Defaults to last, "
                     "non-empty slot.",
                     CommandParser::OptionDefinition::Type::Integer, false, 1),
      args);

  if (!optCommand.has_value()) {
    return;
  }

  std::optional<uint8_t> page;
  std::optional<uint8_t> slot;

  if (!validateDeleteCommand(optCommand.value(), page, slot)) {
    return;
  }

  if (!FileSystem::exists(customCommandMenuFile)) {
    CG_Printf("Unable to find ^3'%s' ^7file.\n", customCommandMenuFile.c_str());
    return;
  }

  toml::ordered_value table;

  if (!readFile(table)) {
    return;
  }

  try {
    const std::string pageStr = "page-" + std::to_string(page.value());
    const std::string nameStr = "name-" + std::to_string(slot.value());
    const std::string cmdStr = "command-" + std::to_string(slot.value());

    table.at(pageStr).as_table().erase(nameStr);
    table.at(pageStr).as_table().erase(cmdStr);

    // delete the table if it's empty
    if (table.at(pageStr).as_table().empty()) {
      table.as_table().erase(pageStr);
    }
  } catch (const toml::type_error &e) {
    CG_Printf("Failed to delete command: %s",
              TOMLUtils::getError(e.what()).c_str());
  } catch (const std::out_of_range &e) {
    CG_Printf("Failed to delete command: %s",
              TOMLUtils::getError(e.what()).c_str());
  }

  if (writeFile(table)) {
    parseCommands();
  }
}

bool CustomCommandMenu::validateDeleteCommand(
    const CommandParser::Command &optCommand, std::optional<uint8_t> &page,
    std::optional<uint8_t> &slot) const {
  const auto optPage = optCommand.getOptional("page");
  const auto optSlot = optCommand.getOptional("slot");

  const auto opFailedChatMsg = []() {
    CG_AddToTeamChat("^3deleteCustomCommand: ^7operation failed. Check "
                     "console for more information.",
                     TEAM_SPECTATOR);
  };

  if (optPage.value().integer == 0 ||
      optPage.value().integer > CUSTOM_COMMAND_MENU_MAX_PAGES) {
    opFailedChatMsg();
    CG_Printf("Parameter `page` is out of range (1-5).\n");
    return false;
  }

  page = static_cast<uint8_t>(optPage.value().integer);

  if (commands.find(page.value()) == commands.cend()) {
    opFailedChatMsg();
    // technically the page "exists" from users perspective, so it's more
    // clear to tell the user that there's nothing on that page
    CG_Printf("No commands found on page ^3%i^7.\n", page.value());
    return false;
  }

  const auto isValidSlot = [this, &opFailedChatMsg, &page, &slot]() {
    if (slot.value() == 0 || slot.value() > CUSTOM_COMMAND_MENU_PAGE_SIZE) {
      opFailedChatMsg();
      CG_Printf("Parameter `slot` is out of range (1-8).\n");
      return false;
    }

    // make sure we actually have a command at this slot
    // we want to do this here, because the deletion fails silently otherwise
    if (commands.find(page.value()) != commands.cend() &&
        commands.at(page.value())[slot.value() - 1].name.empty() &&
        commands.at(page.value())[slot.value() - 1].command.empty()) {
      opFailedChatMsg();
      CG_Printf("No command found on page ^3%i ^7in slot ^3%i^7.\n",
                page.value(), slot.value());
      return false;
    }

    return true;
  };

  slot = optSlot.has_value() ? static_cast<uint8_t>(optSlot.value().integer)
                             : findSlotForDeletion(page.value());

  if (!isValidSlot()) {
    return false;
  }

  return true;
}

void CustomCommandMenu::listCommands(
    const std::vector<std::string> &args) const {
  if (commands.empty()) {
    CG_Printf("No commands saved.\n");
    return;
  }

  constexpr char desc[] = R"(Lists saved custom commands.
    /listCustomCommands --page <page>

    Has shorthand format of:
    /listcustomCommands <page>)";

  const auto optCommand = ConsoleCommands::getOptCommand(
      "listCustomCommands",
      CommandParser::CommandDefinition::create("listCustomCommands", desc)
          .addOption(
              "page", "p",
              "Page to list commands from. If not specified, lists all pages.",
              CommandParser::OptionDefinition::Type::Integer, false, 0),
      args);

  if (!optCommand.has_value()) {
    return;
  }

  const auto optPage = optCommand.value().getOptional("page");
  uint8_t page = 0;

  if (optPage.has_value()) {
    if (optPage.value().integer == 0 ||
        optPage.value().integer > CUSTOM_COMMAND_MENU_MAX_PAGES) {
      CG_AddToTeamChat("^3listCustomCommands: ^7operation failed. Check "
                       "console for more information.",
                       TEAM_SPECTATOR);
      CG_Printf("Parameter `page` is out of range (1-5).\n");
      return;
    }

    page = static_cast<uint8_t>(optPage.value().integer);
  }

  const auto printCommands =
      [](const uint8_t page,
         const std::array<CustomCommand, CUSTOM_COMMAND_MENU_PAGE_SIZE> &cmds) {
        CG_Printf("^7[ Page %i ]\n", page);

        for (size_t i = 0; i < cmds.size(); i++) {
          if (cmds[i].name.empty() || cmds[i].command.empty()) {
            continue;
          }

          CG_Printf("^7%i. %s\n   ^7%s\n", static_cast<int32_t>(i + 1),
                    cmds[i].name.c_str(), cmds[i].command.c_str());
        }

        CG_Printf("\n");
      };

  if (page == 0) {
    CG_Printf("Saved custom commands:\n\n");

    for (const auto &[savedPage, cmds] : commands) {
      printCommands(savedPage, cmds);
    }
  } else {
    if (commands.find(page) == commands.cend()) {
      CG_Printf("No commands on page ^3%i^7.\n", page);
      return;
    }

    CG_Printf("Saved custom commands on page ^3%i:\n\n", page);
    printCommands(page, commands.at(page));
  }
}

inline bool CustomCommandMenu::commandsFull() const {
  return commands.size() >= CUSTOM_COMMAND_MENU_MAX_PAGES &&
         std::all_of(
             commands.cbegin(), commands.cend(),
             [this](const auto &page) { return pageIsFull(page.first); });
}

inline bool CustomCommandMenu::pageIsFull(const uint8_t page) const {
  // this should not be called with an invalid page!
  assert(page > 0 && page <= CUSTOM_COMMAND_MENU_MAX_PAGES);

  for (uint8_t i = 0; i < CUSTOM_COMMAND_MENU_PAGE_SIZE; i++) {
    if (commands.at(page)[i].name.empty() &&
        commands.at(page)[i].command.empty()) {
      return false;
    }
  }

  return true;
}

uint8_t CustomCommandMenu::findFreePage() const {
  for (uint8_t i = 1; i <= CUSTOM_COMMAND_MENU_MAX_PAGES; i++) {
    if (commands.find(i) == commands.cend() || findFreeSlot(i)) {
      return i;
    }
  }

  // no free pages
  return 0;
}

uint8_t CustomCommandMenu::findFreeSlot(const uint8_t page) const {
  // if the page does not exist, it's fully free
  if (commands.find(page) == commands.cend()) {
    return 1;
  }

  for (uint8_t i = 0; i < CUSTOM_COMMAND_MENU_PAGE_SIZE; i++) {
    const auto &cmd = commands.at(page)[i];

    if (cmd.name.empty() && cmd.command.empty()) {
      return i + 1;
    }
  }

  // no free slots
  return 0;
}

uint8_t CustomCommandMenu::findSlotForDeletion(const uint8_t page) const {
  // we might call this with an invalid page, so return invalid slot
  // if the page is also invalid
  if (page == 0 || page > CUSTOM_COMMAND_MENU_MAX_PAGES) {
    return 0;
  }

  for (uint8_t i = CUSTOM_COMMAND_MENU_PAGE_SIZE; i > 0; i--) {
    const auto &cmd = commands.at(page)[i - 1];

    if (!cmd.name.empty() && !cmd.command.empty()) {
      return i;
    }
  }

  // no populated slots
  return 0;
}

const std::map<uint8_t, std::array<CustomCommandMenu::CustomCommand,
                                   CUSTOM_COMMAND_MENU_PAGE_SIZE>> &
CustomCommandMenu::getCustomCommands() const {
  return commands;
}

bool CustomCommandMenu::readFile(toml::ordered_value &table) const {
  std::string err;

  if (!TOMLUtils::readFile(customCommandMenuFile, table, &err)) {
    CG_Printf("%s\n", err.c_str());
    return false;
  }

  return true;
}

bool CustomCommandMenu::writeFile(const toml::ordered_value &table) const {
  std::string err;

  if (!TOMLUtils::writeFile(customCommandMenuFile, table, &err)) {
    CG_Printf("%s\n", err.c_str());
    return false;
  }

  return true;
}
} // namespace ETJump
