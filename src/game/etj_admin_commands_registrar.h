#pragma once

#include <memory>
#include "etj_command_parser.h"

namespace ETJump
{
	class AdminCommandsHandler;
	class LevelService;
	class SessionService;
	class UserService;

	class AdminCommandsRegistrar
	{
	public:
		AdminCommandsRegistrar(const std::shared_ptr<AdminCommandsHandler>& adminCommandsHandler, const std::shared_ptr<LevelService>& levelService, const std::shared_ptr<SessionService>& sessionService, const std::shared_ptr<UserService>& userService)
			: _adminCommandsHandler(adminCommandsHandler),
			_levelService(levelService),
			_sessionService(sessionService),
			_userService(userService),
			_requiredPlayerOption(createOptionDefinition("player", "Target player", CommandParser::OptionDefinition::Type::Token, true)),
			_optionalPlayerOption(createOptionDefinition("player", "Target player", CommandParser::OptionDefinition::Type::Token, false))
		{
		}
		~AdminCommandsRegistrar() {}

		void registerAdminCommands();

	private:
		bool isPlayer(int clientNum);

		std::shared_ptr<AdminCommandsHandler> _adminCommandsHandler;
		std::shared_ptr<LevelService> _levelService;
		std::shared_ptr<SessionService> _sessionService;
		std::shared_ptr<UserService> _userService;

		std::pair<std::string, CommandParser::OptionDefinition> _requiredPlayerOption;
		std::pair<std::string, CommandParser::OptionDefinition> _optionalPlayerOption;

		static std::pair<std::string, CommandParser::OptionDefinition> createOptionDefinition(
			const std::string& name, 
			const std::string& description, 
			CommandParser::OptionDefinition::Type type, 
			bool required
		);
		static CommandParser::CommandDefinition AdminCommandsRegistrar::createCommandDefinition(
			const std::string& name, 
			const std::string& description, 
			const std::map<std::string, CommandParser::OptionDefinition>& options = std::map<std::string, CommandParser::OptionDefinition>(), 
			const std::vector<std::pair<std::string, CommandParser::OptionDefinition>>& positionalOptions = std::vector<std::pair<std::string, CommandParser::OptionDefinition>>()
		);
		static std::string getOptionalText(const ETJump::CommandParser::Command& command, const std::string& key, const std::string& defaultValue = "");
		static int getOptionalInteger(const ETJump::CommandParser::Command& command, const std::string& key, int defaultValue = 0);
		static long long getOptionalDuration(const ETJump::CommandParser::Command& command, const std::string& key, long long defaultValue = 0);
		static void printCommandChatInfoMessage(int clientNum, const std::string& command, const std::string& message);
		static std::string multipleMatchingNamesError(const std::vector<std::string>& names);
		static bool hasOption(const CommandParser::Command& command, const std::string& option);
	};
}
