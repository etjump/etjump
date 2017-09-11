#include <boost/algorithm/string.hpp>
#include "etj_admin_commands.h"
#include "etj_admin_commands_handler.h"
#include "etj_user_service.h"
#include "etj_level_service.h"
#include "etj_session_service.h"
#include "etj_printer.h"
#include "etj_result_set_formatter.h"
#include "etj_save.h"
#include "etj_messages.h"
#include "etj_time_utilities.h"

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

	ETJump::CommandParser::CommandDefinition createCommandDefinition(const std::string& name, const std::string& description, const std::map<std::string, ETJump::CommandParser::OptionDefinition>& options = std::map<std::string, ETJump::CommandParser::OptionDefinition>(), const std::vector<ETJump::CommandParser::OptionDefinition>& positionalOptions = std::vector<ETJump::CommandParser::OptionDefinition>())
{
	return{
		name,
		description,
		options,
		positionalOptions
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

long long getOptionalDuration(const ETJump::CommandParser::Command& command, const std::string& key, long long defaultValue = 0)
{
	auto optIter = command.options.find(key);
	if (optIter != end(command.options))
	{
		return optIter->second.duration;
	}
	return defaultValue;
}

void printCommandChatInfoMessage(int clientNum, const std::string& command, const std::string& message)
{
	Printer::sendChatMessage(clientNum, "^3" + command + ": ^7" + message);
}

void ETJump::registerAdminCommands(std::shared_ptr<AdminCommandsHandler> injectedAdminCommandsHandler, std::shared_ptr<LevelService> injectedLevelService, std::shared_ptr<SessionService> injectedSessionService, std::shared_ptr<UserService> injectedUserService)
{
	// store them here as the ptrs will point to null otherwise
	static std::shared_ptr<AdminCommandsHandler> _adminCommandsHandler = injectedAdminCommandsHandler;
	static std::shared_ptr<LevelService> levelService = injectedLevelService;
	static std::shared_ptr<SessionService> sessionService = injectedSessionService;
	static std::shared_ptr<UserService> userService = injectedUserService;
	static const auto requiredPlayerOption = createOptionDefinition("player", "Target player", CommandParser::OptionDefinition::Type::Token, true);
	static const auto optionalPlayerOption = createOptionDefinition("player", "Target player", CommandParser::OptionDefinition::Type::Token, false);
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
		auto level = command.options.at("level").integer;
		std::string greeting = getOptionalText(command, "greeting");
		std::string commands = getOptionalText(command, "commands");
		std::string name = getOptionalText(command, "name");

		auto result = levelService->add(level, name, commands, greeting);
		if (!result.success)
		{
			printCommandChatInfoMessage(clientNum, commandText, result.message);
		}
	});
	/**
	* admintest
	*/
	_adminCommandsHandler->subscribe('a', createCommandDefinition("admintest", "admintest", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		if (clientNum == AdminCommandsHandler::CONSOLE_CLIENTNUM)
		{
			return;
		}
		auto user = sessionService->getUser(clientNum);
		auto level = levelService->get(user.level);
		Printer::broadcastChatMessage((boost::format("^3admintest: ^7%s^7 is a level %d user (%s^7).") 
			% (g_entities + clientNum)->client->pers.netname % user.level % level->name).str());
	});

	/**
	* ban
	*/
	_adminCommandsHandler->subscribe('b', createCommandDefinition("ban", "ban", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* cancelvote
	*/
	_adminCommandsHandler->subscribe('C', createCommandDefinition("cancelvote", "cancelvote", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		if (level.voteInfo.voteTime)
		{
			level.voteInfo.voteYes = 0;
			level.voteInfo.voteNo = level.numConnectedClients;
			Printer::broadcastChatMessage("^3cancelvote: ^7vote has been canceled");
		}
		else
		{
			printCommandChatInfoMessage(clientNum, commandText, "no vote in progress");
		}
	});

	/**
	* deletelevel
	*/
	_adminCommandsHandler->subscribe('A', createCommandDefinition("deletelevel", "Deletes a level.", {
		createOptionDefinition("level", "Level to be deleted", CommandParser::OptionDefinition::Type::Integer, true),
		createOptionDefinition("newlevel", "Level that users with the deleted level will be set to. 0 by default.", CommandParser::OptionDefinition::Type::Integer, false)
	}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		auto level = command.options.at("level").integer;
		auto newLevel = getOptionalInteger(command, "newlevel", 0);

		levelService->remove(level);
		sessionService->setLevelIfHasLevel(clientNum, level, newLevel);
	});

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

		printCommandChatInfoMessage(clientNum, commandText, "updated level" + std::to_string(level));
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
	_adminCommandsHandler->subscribe('f', createCommandDefinition("finger", "finger", {
		requiredPlayerOption
	}, { requiredPlayerOption.second }), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		auto targets = sessionService->findUsersByName(command.options.find("player")->second.text);
		if (targets.size() == 0)
		{
			printCommandChatInfoMessage(clientNum, commandText, Error::NoConnectedClientsError);
			return;
		}
		printCommandChatInfoMessage(clientNum, commandText, Info::CheckConsoleForInfo);
		for (const auto & t : targets)
		{
			auto user = sessionService->getUser(t);
			Printer::sendConsoleMessage(clientNum, (boost::format("^7name: %s\n^7id: %d^7\noriginal name: %s\n^7level: %d\n^7title: %s^7\n")
				% sessionService->getName(clientNum) 
				% user.id
				% user.name
				% user.level
				% user.title).str());
		}
	});

	/**
	* help
	*/
	_adminCommandsHandler->subscribe('a', createCommandDefinition("help", "help", {
		createOptionDefinition("cols", "Number of columns that commands will be display on", CommandParser::OptionDefinition::Type::Integer, false)
	}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		if (command.extraArgs.size() == 0)
		{
			printCommandChatInfoMessage(clientNum, commandText, Info::CheckConsoleForInfo);
			Utilities::ResultSetFormatter rsf;
			auto itemsOnCurrentRow = 0;
			const int numItemsPerRow = getOptionalInteger(command, "cols", 4);
			std::string buffer;
			for (const auto & c : _adminCommandsHandler->getAvailableCommands(clientNum))
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
			printCommandChatInfoMessage(clientNum, commandText, Info::CheckConsoleForInfo);

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
	_adminCommandsHandler->subscribe('k', createCommandDefinition("kick", "kick", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		
	});

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
	static const auto muteDurationOption = createOptionDefinition("duration", "How long to mute the player for", CommandParser::OptionDefinition::Type::Duration, false);
	_adminCommandsHandler->subscribe('m', createCommandDefinition("mute", "mute", {
		requiredPlayerOption,
		muteDurationOption
	}, { requiredPlayerOption.second, muteDurationOption.second }), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		auto targets = sessionService->findUsersByName(command.options.find("player")->second.text);
		if (targets.size() == 0)
		{
			printCommandChatInfoMessage(clientNum, commandText, Error::NoConnectedClientsError);
			return;
		}
		if (targets.size() > 1)
		{
			// TODO: list players
			printCommandChatInfoMessage(clientNum, commandText, Error::MultipleMatchingPlayers);
			return;
		}
		auto duration = getOptionalDuration(command, "duration", 0);
		
		if (!ETJump::sessionService->isEqualOrHigherLevel(clientNum, targets[0]))
		{
			printCommandChatInfoMessage(clientNum, commandText, "Cannot mute a fellow administrator.");
			return;
		}

		if (clientNum == targets[0])
		{
			printCommandChatInfoMessage(clientNum, commandText, "Cannot mute self.");
			return;
		}

		ETJump::sessionService->mute(targets[0], duration);
		if (duration > 0)
		{
			printCommandChatInfoMessage(clientNum, commandText, "Muted player " + ETJump::sessionService->getName(targets[0]) + " ^7until " + DateTime::toLocalTime(DateTime::now() + duration));
		} else
		{
			printCommandChatInfoMessage(clientNum, commandText, "Muted player " + ETJump::sessionService->getName(targets[0]));
		}
	});

	/**
	* noclip
	*/
	static const auto optionalCountOption = createOptionDefinition("count", "Number of times user is allowed to use noclip. -1 infinite.", CommandParser::OptionDefinition::Type::Integer, false);
	_adminCommandsHandler->subscribe('N', createCommandDefinition("noclip", "noclip", {
		optionalPlayerOption,
		optionalCountOption
	}, { optionalPlayerOption.second, optionalCountOption.second }), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		if (level.noNoclip)
		{
			printCommandChatInfoMessage(clientNum, commandText, "Noclip is disabled on this map.");
			return;
		}

		auto player = getOptionalText(command, "player", "");
		auto count = getOptionalInteger(command, "count", 1);
		if (player.length() == 0)
		{
			if (clientNum == AdminCommandsHandler::CONSOLE_CLIENTNUM)
			{
				return;
			}

			auto ent = (g_entities + clientNum);
			if (ent->client->sess.timerunActive)
			{
				printCommandChatInfoMessage(clientNum, commandText, "cheats are disabled while time run is active.");
				return;
			}

			(g_entities + clientNum)->client->noclip = (g_entities + clientNum)->client->noclip 
				? qfalse 
				: qtrue;
		} else
		{
			auto targets = sessionService->findUsersByName(command.options.find("player")->second.text);
			if (targets.size() == 0)
			{
				printCommandChatInfoMessage(clientNum, commandText, Error::NoConnectedClientsError);
				return;
			}
			if (targets.size() > 1)
			{
				// TODO: list players
				printCommandChatInfoMessage(clientNum, commandText, Error::MultipleMatchingPlayers);
				return;
			}
			auto ent = g_entities + targets[0];
			if (count >= 0)
			{
				printCommandChatInfoMessage(clientNum, commandText, stringFormat("%s ^7can use /noclip %d times.", ent->client->pers.netname, count));
				printCommandChatInfoMessage(targets[0], commandText, stringFormat("you can use noclip %d times.", count));
			} else {
				printCommandChatInfoMessage(clientNum, commandText, stringFormat("%s^7 can use /noclip infinitely.", ent->client->pers.netname));
				printCommandChatInfoMessage(targets[0], commandText, "you can use /noclip infinitely.");
			}
			ent->client->pers.noclipCount = count;
		}
	});

	/**
	* passvote
	*/
	_adminCommandsHandler->subscribe('P', createCommandDefinition("passvote", "passvote", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		if (level.voteInfo.voteTime)
		{
			level.voteInfo.voteNo = 0;
			level.voteInfo.voteYes = level.numConnectedClients;
			Printer::broadcastChatMessage("^3passvote:^7 vote has been passed.");
		}
		else
		{
			Printer::sendChatMessage(clientNum, "^3passvote:^7 no vote in progress.");
		}
		return qtrue;
	});

	/**
	* putspec
	*/
	_adminCommandsHandler->subscribe('p', createCommandDefinition("putspec", "putspec", {
		requiredPlayerOption
	}, { requiredPlayerOption.second }), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		auto targets = sessionService->findUsersByName(command.options.find("player")->second.text);
		if (targets.size() == 0)
		{
			printCommandChatInfoMessage(clientNum, commandText, Error::NoConnectedClientsError);
			return;
		}
		if (targets.size() > 1)
		{
			// TODO: list players
			printCommandChatInfoMessage(clientNum, commandText, Error::MultipleMatchingPlayers);
			return;
		}

		auto targetEnt = (g_entities + targets[0]);
		targetEnt->client->sess.lastTeamSwitch = level.time;

		const weapon_t w = static_cast<weapon_t>(-1);
		SetTeam(targetEnt, "s", qfalse, w, w, qtrue);
	});

	/**
	* rename
	*/
	_adminCommandsHandler->subscribe('R', createCommandDefinition("rename", "rename", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* restart
	*/
	_adminCommandsHandler->subscribe('r', createCommandDefinition("restart", "restart", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		Svcmd_ResetMatch_f(qfalse, qtrue);
	});

	/**
	* setlevel
	*/
	_adminCommandsHandler->subscribe('s', createCommandDefinition("setlevel", "setlevel", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
				
	});

	/**
	* spectate
	*/
	_adminCommandsHandler->subscribe('a', createCommandDefinition("spectate", "spectate", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* unban
	*/
	_adminCommandsHandler->subscribe('b', createCommandDefinition("unban", "unban", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* unmute
	*/
	_adminCommandsHandler->subscribe('m', createCommandDefinition("unmute", "unmute", {
		requiredPlayerOption,
	}, {
		requiredPlayerOption.second,
	}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		auto targets = sessionService->findUsersByName(command.options.find("player")->second.text);
		if (targets.size() == 0)
		{
			printCommandChatInfoMessage(clientNum, commandText, Error::NoConnectedClientsError);
			return;
		}
		if (targets.size() > 1)
		{
			// TODO: list players
			printCommandChatInfoMessage(clientNum, commandText, Error::MultipleMatchingPlayers);
			return;
		}

		if (ETJump::sessionService->unmute(targets[0]))
		{
			printCommandChatInfoMessage(clientNum, commandText, "Unmuted player " + ETJump::sessionService->getName(targets[0]));
		} else
		{
			printCommandChatInfoMessage(clientNum, commandText, "Player " + ETJump::sessionService->getName(targets[0]) + "^7 is not muted.");
		}
	});

	/**
	* userinfo
	*/
	_adminCommandsHandler->subscribe('A', createCommandDefinition("userinfo", "userinfo", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});
}

