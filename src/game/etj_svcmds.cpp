#include "etj_svcmds.h"

#include <stdexcept>

#include "etj_string_utilities.h"

void ETJump::ServerCommands::addCommand(CommandDefinition definition,
                                        Command command) {
  if (_commands.count(definition.name) > 0) {
    throw std::runtime_error(stringFormat(
        "`%s` has already been registered as a server side command.", definition.name));
  }

  _commandDefinitions[definition.name] = definition;
  _commands[definition.name] = command;
}

bool ETJump::ServerCommands::check(const std::string &name,
                                   const std::vector<std::string> &args) {
  if (_commands.count(name) == 0) {
    return false;
  }

  const auto *definition = &_commandDefinitions[name];
  const auto *command = &_commands[name];

  auto result = CommandParser(*definition, args).parse();

  if (result.helpRequested) {
    Printer::PrintLn(definition->help());
    return true;
  }

  if (!result.errors.empty()) {
    for (const auto & e : result.errors) {
      Printer::PrintLn(e);
    }
    return true;
  }

  (*command)(result);
  return true;
}
