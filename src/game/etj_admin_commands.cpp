#include <boost/algorithm/string.hpp>
#include "etj_admin_commands.h"
#include "etj_admin_commands_handler.h"
#include "etj_level_service.h"
#include "etj_printer.h"

std::pair<std::string, ETJump::CommandParser::OptionDefinition> createOptionDefinition(const std::string& name, const std::string& description, ETJump::CommandParser::OptionDefinition::Type type, bool required)
{
	return{
		name,{
			name,
			description,
			type,
			required
		}
	};
}

ETJump::CommandParser::CommandDefinition createCommandDefinition(const std::string& name, const std::string& description, const std::map<std::string, ETJump::CommandParser::OptionDefinition>& options = std::map<std::string, ETJump::CommandParser::OptionDefinition>())
{
	return{
		name,
		description,
		options
	};
}

std::string getOptionalText(const ETJump::CommandParser::Command& command, const std::string& key, const std::string& defaultValue = "")
{
	auto optIter = command.options.find(key);
	if (optIter != end(command.options))
	{
		return optIter->second.text;
	}
	return defaultValue;
}

int getOptionalInteger(const ETJump::CommandParser::Command& command, const std::string& key, int defaultValue = 0)
{
	auto optIter = command.options.find(key);
	if (optIter != end(command.options))
	{
		return optIter->second.integer;
	}
	return defaultValue;
}

void ETJump::registerAdminCommands(std::shared_ptr<AdminCommandsHandler> adminCommandsHandler, std::shared_ptr<LevelService> levelService)
{
	/**
	 * addlevel
	 */
	adminCommandsHandler->subscribe('a', createCommandDefinition("addlevel", "Adds a level", {
		createOptionDefinition("level", "The level that will be added (integer)", ETJump::CommandParser::OptionDefinition::Type::Integer, true),
		createOptionDefinition("commands", "Allowed commands for the level", ETJump::CommandParser::OptionDefinition::Type::MultiToken, false),
		createOptionDefinition("greeting", "A greeting for the level", ETJump::CommandParser::OptionDefinition::Type::MultiToken, false),
		createOptionDefinition("name", "A name for the level", ETJump::CommandParser::OptionDefinition::Type::MultiToken, false),
	}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		if (command.errors.size() > 0)
		{
			Printer::sendChatMessage(clientNum, "^3Invalid parameters: ^7check console for more information.");
			Printer::sendConsoleMessage(clientNum, boost::join(command.errors, "\n"));
			return;
		}

		auto level = command.options.at("level").integer;
		std::string greeting = getOptionalText(command, "greeting");
		std::string commands = getOptionalText(command, "commands");
		std::string name = getOptionalText(command, "name");

		auto result = levelService->add(level, name, commands, greeting);
		if (!result.success)
		{
			Printer::sendChatMessage(clientNum, "^3addlevel: ^7" + result.message);
		}
	});
	/**
	* admintest
	*/
	adminCommandsHandler->subscribe('a', createCommandDefinition("a", "admintest", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* ban
	*/
	adminCommandsHandler->subscribe('b', createCommandDefinition("b", "ban", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* cancelvote
	*/
	adminCommandsHandler->subscribe('C', createCommandDefinition("C", "cancelvote", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* deletelevel
	*/
	adminCommandsHandler->subscribe('A', createCommandDefinition("A", "deletelevel", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* editcommands
	*/
	adminCommandsHandler->subscribe('A', createCommandDefinition("A", "editcommands", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* editlevel
	*/
	adminCommandsHandler->subscribe('A', createCommandDefinition("A", "editlevel", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* edituser
	*/
	adminCommandsHandler->subscribe('A', createCommandDefinition("A", "edituser", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* findmap
	*/
	adminCommandsHandler->subscribe('a', createCommandDefinition("a", "findmap", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* finduser
	*/
	adminCommandsHandler->subscribe('A', createCommandDefinition("A", "finduser", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* finger
	*/
	adminCommandsHandler->subscribe('f', createCommandDefinition("f", "finger", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* help
	*/
	adminCommandsHandler->subscribe('a', createCommandDefinition("a", "help", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* kick
	*/
	adminCommandsHandler->subscribe('k', createCommandDefinition("k", "kick", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* leastplayed
	*/
	adminCommandsHandler->subscribe('a', createCommandDefinition("a", "leastplayed", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* levelinfo
	*/
	adminCommandsHandler->subscribe('A', createCommandDefinition("A", "levelinfo", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* listbans
	*/
	adminCommandsHandler->subscribe('L', createCommandDefinition("L", "listbans", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* listflags
	*/
	adminCommandsHandler->subscribe('A', createCommandDefinition("A", "listflags", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* listmaps
	*/
	adminCommandsHandler->subscribe('a', createCommandDefinition("a", "listmaps", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* listplayers
	*/
	adminCommandsHandler->subscribe('l', createCommandDefinition("l", "listplayers", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* listusernames
	*/
	adminCommandsHandler->subscribe('A', createCommandDefinition("A", "listusernames", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* listusers
	*/
	adminCommandsHandler->subscribe('A', createCommandDefinition("A", "listusers", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* map
	*/
	adminCommandsHandler->subscribe('M', createCommandDefinition("M", "map", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* mapinfo
	*/
	adminCommandsHandler->subscribe('a', createCommandDefinition("a", "mapinfo", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* mostplayed
	*/
	adminCommandsHandler->subscribe('a', createCommandDefinition("a", "mostplayed", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* mute
	*/
	adminCommandsHandler->subscribe('m', createCommandDefinition("m", "mute", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* noclip
	*/
	adminCommandsHandler->subscribe('N', createCommandDefinition("N", "noclip", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* nogoto
	*/
	adminCommandsHandler->subscribe('K', createCommandDefinition("K", "nogoto", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* nosave
	*/
	adminCommandsHandler->subscribe('T', createCommandDefinition("T", "nosave", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* passvote
	*/
	adminCommandsHandler->subscribe('P', createCommandDefinition("P", "passvote", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* putteam
	*/
	adminCommandsHandler->subscribe('p', createCommandDefinition("p", "putteam", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* rename
	*/
	adminCommandsHandler->subscribe('R', createCommandDefinition("R", "rename", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* restart
	*/
	adminCommandsHandler->subscribe('r', createCommandDefinition("r", "restart", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* rmsaves
	*/
	adminCommandsHandler->subscribe('T', createCommandDefinition("T", "rmsaves", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* setlevel
	*/
	adminCommandsHandler->subscribe('s', createCommandDefinition("s", "setlevel", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* spectate
	*/
	adminCommandsHandler->subscribe('a', createCommandDefinition("a", "spectate", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* tokens
	*/
	adminCommandsHandler->subscribe('V', createCommandDefinition("V", "tokens", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* unban
	*/
	adminCommandsHandler->subscribe('b', createCommandDefinition("b", "unban", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* unmute
	*/
	adminCommandsHandler->subscribe('m', createCommandDefinition("m", "unmute", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* userinfo
	*/
	adminCommandsHandler->subscribe('A', createCommandDefinition("A", "userinfo", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});
}
