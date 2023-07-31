#pragma once
#include <functional>
#include <string>

#include "etj_command_parser.h"
#include "etj_log.h"

namespace ETJump {
class ServerCommands {
public:
  using CommandDefinition = CommandParser::CommandDefinition;
  using Command = std::function<void(const CommandParser::Command &parameters)>;

  explicit ServerCommands(std::unique_ptr<Log> logger)
      : _logger(std::move(logger)) {}

  void addCommand(CommandDefinition definition, Command command);

  bool check(const std::string& name, const std::vector<std::string> &args);
private:
  std::map<std::string, CommandDefinition> _commandDefinitions;
  std::map<std::string, Command> _commands;
  std::unique_ptr<Log> _logger;
};
}
