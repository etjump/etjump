#include <boost/algorithm/string.hpp>
#include "etj_admin_commands.h"
#include "etj_admin_commands_handler.h"
#include "etj_user_service.h"
#include "etj_level_service.h"
#include "etj_session_service.h"
#include "etj_printer.h"
#include "etj_result_set_formatter.h"

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

void ETJump::registerAdminCommands(std::shared_ptr<AdminCommandsHandler> injectedAdminCommandsHandler, std::shared_ptr<LevelService> injectedLevelService, std::shared_ptr<SessionService> injectedSessionService, std::shared_ptr<UserService> injectedUserService)
{
	// store them here as the ptrs will point to null otherwise
	static std::shared_ptr<AdminCommandsHandler> _adminCommandsHandler = injectedAdminCommandsHandler;
	static std::shared_ptr<LevelService> levelService = injectedLevelService;
	static std::shared_ptr<SessionService> sessionService = injectedSessionService;
	static std::shared_ptr<UserService> userService = injectedUserService;
	/**
	 * addlevel
	 */
	_adminCommandsHandler->subscribe('a', createCommandDefinition("addlevel", "Adds a level", {
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
	_adminCommandsHandler->subscribe('a', createCommandDefinition("admintest", "admintest", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* ban
	*/
	_adminCommandsHandler->subscribe('b', createCommandDefinition("ban", "ban", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* cancelvote
	*/
	_adminCommandsHandler->subscribe('C', createCommandDefinition("cancelvote", "cancelvote", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* deletelevel
	*/
	_adminCommandsHandler->subscribe('A', createCommandDefinition("deletelevel", "Deletes a level.", {
		createOptionDefinition("level", "Level to be deleted", CommandParser::OptionDefinition::Type::Integer, true),
		createOptionDefinition("newlevel", "Level that users with the deleted level will be set to. 0 by default.", CommandParser::OptionDefinition::Type::Integer, false)
	}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		if (command.errors.size() > 0)
		{
			Printer::sendChatMessage(clientNum, "^3Invalid parameters: ^7check console for more information.");
			Printer::sendConsoleMessage(clientNum, boost::join(command.errors, "\n"));
			return;
		}

		auto level = command.options.at("level").integer;
		auto newLevel = getOptionalInteger(command, "newlevel", 0);

		levelService->remove(level);
		sessionService->setLevelIfHasLevel(clientNum, level, newLevel);
	});

	/**
	* editcommands
	*/
	_adminCommandsHandler->subscribe('A', createCommandDefinition("editcommands", "editcommands", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* editlevel
	*/
	_adminCommandsHandler->subscribe('A', createCommandDefinition("editlevel", "editlevel", {
		createOptionDefinition("level", "The level that will be edited (integer)", ETJump::CommandParser::OptionDefinition::Type::Integer, true),
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
		auto commandsIter = command.options.find("commands");
		auto greetingIter = command.options.find("greeting");
		auto nameIter = command.options.find("name");
		LevelChanges changes;
		int changedFields;
		if (commandsIter != end(command.options))
		{
			changes.commands = commandsIter->second.text;
			changedFields |= static_cast<int>(LevelFields::Commands);
		}
		if (greetingIter != end(command.options))
		{
			changes.greeting = greetingIter->second.text;
			changedFields |= static_cast<int>(LevelFields::Greeting);
		}
		if (nameIter != end(command.options))
		{
			changes.name = nameIter->second.text;
			changedFields |= static_cast<int>(LevelFields::Name);
		}
		levelService->edit(level, changes, changedFields);

		Printer::sendChatMessage(clientNum, "^2ETJump: ^7updated level " + std::to_string(level));
	});

	/**
	* edituser
	*/
	_adminCommandsHandler->subscribe('A', createCommandDefinition("edituser", "edituser", {
		createOptionDefinition("id", "Target user ID", CommandParser::OptionDefinition::Type::Integer, true),
		createOptionDefinition("commands", "Allowed commands for the user", ETJump::CommandParser::OptionDefinition::Type::MultiToken, false),
		createOptionDefinition("greeting", "A greeting for the user", ETJump::CommandParser::OptionDefinition::Type::MultiToken, false),
		createOptionDefinition("title", "A title for the user", ETJump::CommandParser::OptionDefinition::Type::MultiToken, false),
	}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		if (command.errors.size() > 0)
		{
			Printer::sendChatMessage(clientNum, "^3Invalid parameters: ^7check console for more information.");
			Printer::sendConsoleMessage(clientNum, boost::join(command.errors, "\n"));
			return;
		}

		auto userId = command.options.at("id").integer;
		auto commandsIter = command.options.find("commands");
		auto greetingIter = command.options.find("greeting");
		auto titleIter = command.options.find("title");
		MutableUserFields changes;
		int changedFields;
		if (commandsIter != end(command.options))
		{
			changes.commands = commandsIter->second.text;
			changedFields |= static_cast<int>(UserFields::Commands);
		}
		if (greetingIter != end(command.options))
		{
			changes.greeting = greetingIter->second.text;
			changedFields |= static_cast<int>(UserFields::Greeting);
		}
		if (titleIter != end(command.options))
		{
			changes.title = titleIter->second.text;
			changedFields |= static_cast<int>(UserFields::Title);
		}
		sessionService->updateUser(clientNum, userId, changes, changedFields);
	});

	/**
	* findmap
	*/
	_adminCommandsHandler->subscribe('a', createCommandDefinition("findmap", "findmap", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* finduser
	*/
	_adminCommandsHandler->subscribe('A', createCommandDefinition("finduser", "finduser", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* finger
	*/
	_adminCommandsHandler->subscribe('f', createCommandDefinition("finger", "finger", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* help
	*/
	_adminCommandsHandler->subscribe('a', createCommandDefinition("help", "help", {
		createOptionDefinition("cols", "Number of columns that commands will be display on", CommandParser::OptionDefinition::Type::Integer, false)
	}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		if (command.extraArgs.size() == 0)
		{
			Printer::sendChatMessage(clientNum, "^3help: ^7check console for more information.");
			Utilities::ResultSetFormatter rsf;
			auto itemsOnCurrentRow = 0;
			const int numItemsPerRow = getOptionalInteger(command, "cols", 4);
			std::string buffer;
			for (const auto & c : _adminCommandsHandler->getSortedCommands())
			{
				if (itemsOnCurrentRow > 0 && itemsOnCurrentRow % numItemsPerRow == 0)
				{
					itemsOnCurrentRow = 0;
					buffer += "\n";
				} 
				buffer += (boost::format("%-20s") % c).str();
				++itemsOnCurrentRow;
			}
			Printer::sendConsoleMessage(clientNum, buffer);
		} else
		{
			auto targetCommand = command.extraArgs[0];
			auto definition = _adminCommandsHandler->getCommandDefinition(targetCommand);
			if (definition == nullptr)
			{
				Printer::sendChatMessage(clientNum, "^3help: ^7command " + targetCommand + " does not exist.");
				return;
			}
			Printer::sendChatMessage(clientNum, "^3help: ^7check console for more information.");

			std::string buffer = targetCommand + "\n\n" + definition->description + "\n\noptions";
			for (const auto & opt : definition->options)
			{
				buffer += "\n" + (boost::format("%-20s %-12s %-11s %s") % opt.second.name % ETJump::CommandParser::toString(opt.second.type) % (opt.second.required ? "(required)" : " ") % opt.second.description).str();
			}
			Printer::sendConsoleMessage(clientNum, buffer);
		}
	});

	/**
	* kick
	*/
	_adminCommandsHandler->subscribe('k', createCommandDefinition("kick", "kick", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* leastplayed
	*/
	_adminCommandsHandler->subscribe('a', createCommandDefinition("leastplayed", "leastplayed", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* levelinfo
	*/
	_adminCommandsHandler->subscribe('A', createCommandDefinition("levelinfo", "levelinfo", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* listbans
	*/
	_adminCommandsHandler->subscribe('L', createCommandDefinition("listbans", "listbans", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* listflags
	*/
	_adminCommandsHandler->subscribe('A', createCommandDefinition("listflags", "listflags", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* listmaps
	*/
	_adminCommandsHandler->subscribe('a', createCommandDefinition("listmaps", "listmaps", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* listplayers
	*/
	_adminCommandsHandler->subscribe('l', createCommandDefinition("listplayers", "listplayers", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* listusernames
	*/
	_adminCommandsHandler->subscribe('A', createCommandDefinition("listusernames", "listusernames", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* listusers
	*/
	_adminCommandsHandler->subscribe('A', createCommandDefinition("listusers", "listusers", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* map
	*/
	_adminCommandsHandler->subscribe('M', createCommandDefinition("map", "map", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* mapinfo
	*/
	_adminCommandsHandler->subscribe('a', createCommandDefinition("mapinfo", "mapinfo", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* mostplayed
	*/
	_adminCommandsHandler->subscribe('a', createCommandDefinition("mostplayed", "mostplayed", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* mute
	*/
	_adminCommandsHandler->subscribe('m', createCommandDefinition("mute", "mute", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* noclip
	*/
	_adminCommandsHandler->subscribe('N', createCommandDefinition("noclip", "noclip", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* nogoto
	*/
	_adminCommandsHandler->subscribe('K', createCommandDefinition("nogoto", "nogoto", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* nosave
	*/
	_adminCommandsHandler->subscribe('T', createCommandDefinition("nosave", "nosave", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* passvote
	*/
	_adminCommandsHandler->subscribe('P', createCommandDefinition("passvote", "passvote", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* putteam
	*/
	_adminCommandsHandler->subscribe('p', createCommandDefinition("putteam", "putteam", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* rename
	*/
	_adminCommandsHandler->subscribe('R', createCommandDefinition("rename", "rename", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* restart
	*/
	_adminCommandsHandler->subscribe('r', createCommandDefinition("restart", "restart", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* rmsaves
	*/
	_adminCommandsHandler->subscribe('T', createCommandDefinition("rmsaves", "rmsaves", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* setlevel
	*/
	_adminCommandsHandler->subscribe('s', createCommandDefinition("setlevel", "setlevel", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* spectate
	*/
	_adminCommandsHandler->subscribe('a', createCommandDefinition("spectate", "spectate", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* tokens
	*/
	_adminCommandsHandler->subscribe('V', createCommandDefinition("tokens", "tokens", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* unban
	*/
	_adminCommandsHandler->subscribe('b', createCommandDefinition("unban", "unban", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* unmute
	*/
	_adminCommandsHandler->subscribe('m', createCommandDefinition("unmute", "unmute", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* userinfo
	*/
	_adminCommandsHandler->subscribe('A', createCommandDefinition("userinfo", "userinfo", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});
}

