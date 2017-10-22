#include <boost/algorithm/string.hpp>
#include "etj_admin_commands_registrar.h"
#include "etj_admin_commands_handler.h"
#include "etj_user_service.h"
#include "etj_level_service.h"
#include "etj_session_service.h"
#include "etj_printer.h"
#include "etj_result_set_formatter.h"
#include "etj_save.h"
#include "etj_messages.h"
#include "etj_time_utilities.h"

void ETJump::AdminCommandsRegistrar::registerAdminCommands()
{
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

		auto result = _levelService->add(level, name, commands, greeting);
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
		auto user = _sessionService->getUser(clientNum);
		auto level = _levelService->get(user.level);
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

		_levelService->remove(level);
		_sessionService->setLevelIfHasLevel(clientNum, level, newLevel);
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
		_levelService->edit(level, changes, changedFields);

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
		_sessionService->updateUser(clientNum, userId, changes, changedFields);
	});

	/**
	* findmap
	*/
	_adminCommandsHandler->subscribe('a', createCommandDefinition("findmap", "findmap", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* finduser
	*/
	_adminCommandsHandler->subscribe('A', createCommandDefinition("finduser", "finduser", {
		_requiredPlayerOption
	}, { _requiredPlayerOption }), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		_sessionService->listUsersByName(clientNum, command.options.at("player").text);
	});

	/**
	* finger
	*/
	_adminCommandsHandler->subscribe('f', createCommandDefinition("finger", "finger", {
		_requiredPlayerOption
	}, { _requiredPlayerOption }), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		auto targets = _sessionService->findUsersByName(command.options.find("player")->second.text);
		if (targets.size() == 0)
		{
			printCommandChatInfoMessage(clientNum, commandText, Error::NoConnectedClientsError);
			return;
		}
		printCommandChatInfoMessage(clientNum, commandText, Info::CheckConsoleForInfo);
		for (const auto & t : targets)
		{
			auto user = _sessionService->getUser(t);
			Printer::sendConsoleMessage(clientNum, (boost::format("^7name: %s\n^7id: %d^7\noriginal name: %s\n^7level: %d\n^7title: %s^7\n")
				% _sessionService->getName(clientNum)
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
		}
		else
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
	static const auto reasonOption = createOptionDefinition("reason", "Reason for kicking the player", CommandParser::OptionDefinition::Type::MultiToken, false);
	static const auto durationOption = createOptionDefinition("duration", "How long the player should be temporarily banned (not a real ban).", CommandParser::OptionDefinition::Type::Duration, false);
	_adminCommandsHandler->subscribe('k', createCommandDefinition("kick", "kick", {
		_requiredPlayerOption,
		reasonOption,
		durationOption
	}, {
		_requiredPlayerOption,
		durationOption
	}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		auto duration = getOptionalDuration(command, "duration", 0);
		auto reason = getOptionalText(command, "reason", "Player kicked");
		auto targets = _sessionService->findUsersByName(command.options.find("player")->second.text);
		if (targets.size() == 0)
		{
			printCommandChatInfoMessage(clientNum, commandText, Error::NoConnectedClientsError);
			return;
		}
		if (targets.size() > 1)
		{
			printCommandChatInfoMessage(clientNum, commandText, multipleMatchingNamesError(_sessionService->getNames(targets)));
			return;
		}
		_sessionService->dropClient(targets[0], reason, duration);
	});

	/**
	* leastplayed
	*/
	static const auto mapCount = createOptionDefinition("count", "How many maps to display", CommandParser::OptionDefinition::Type::Integer, false);
	_adminCommandsHandler->subscribe('a', createCommandDefinition("leastplayed", "leastplayed", {
		mapCount
	}, {mapCount}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		auto count = getOptionalInteger(command, "count", 10);
	});

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
		_requiredPlayerOption,
		muteDurationOption
	}, { _requiredPlayerOption, muteDurationOption }), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		auto targets = _sessionService->findUsersByName(command.options.find("player")->second.text);
		if (targets.size() == 0)
		{
			printCommandChatInfoMessage(clientNum, commandText, Error::NoConnectedClientsError);
			return;
		}
		if (targets.size() > 1)
		{
			printCommandChatInfoMessage(clientNum, commandText, multipleMatchingNamesError(_sessionService->getNames(targets)));
				return;
		}
		auto duration = getOptionalDuration(command, "duration", 0);

		if (!_sessionService->isEqualOrHigherLevel(clientNum, targets[0]))
		{
			printCommandChatInfoMessage(clientNum, commandText, "Cannot mute a fellow administrator.");
			return;
		}

		if (clientNum == targets[0])
		{
			printCommandChatInfoMessage(clientNum, commandText, "Cannot mute self.");
			return;
		}

		_sessionService->mute(targets[0], duration);
		if (duration > 0)
		{
			printCommandChatInfoMessage(clientNum, commandText, "Muted player " + _sessionService->getName(targets[0]) + " ^7until " + DateTime::toLocalTime(DateTime::now() + duration));
		}
		else
		{
			printCommandChatInfoMessage(clientNum, commandText, "Muted player " + _sessionService->getName(targets[0]));
		}
	});

	/**
	* noclip
	*/
	static const auto optionalCountOption = createOptionDefinition("count", "Number of times user is allowed to use noclip. -1 infinite.", CommandParser::OptionDefinition::Type::Integer, false);
	_adminCommandsHandler->subscribe('N', createCommandDefinition("noclip", "noclip", {
		_optionalPlayerOption,
		optionalCountOption
	}, { _optionalPlayerOption, optionalCountOption }), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
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
		}
		else
		{
			auto targets = _sessionService->findUsersByName(command.options.find("player")->second.text);
			if (targets.size() == 0)
			{
				printCommandChatInfoMessage(clientNum, commandText, Error::NoConnectedClientsError);
				return;
			}
			if (targets.size() > 1)
			{
				printCommandChatInfoMessage(clientNum, commandText, multipleMatchingNamesError(_sessionService->getNames(targets)));
				return;
			}
			auto ent = g_entities + targets[0];
			if (count >= 0)
			{
				printCommandChatInfoMessage(clientNum, commandText, stringFormat("%s ^7can use /noclip %d times.", ent->client->pers.netname, count));
				printCommandChatInfoMessage(targets[0], commandText, stringFormat("you can use noclip %d times.", count));
			}
			else {
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
		_requiredPlayerOption
	}, { _requiredPlayerOption }), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		auto targets = _sessionService->findUsersByName(command.options.find("player")->second.text);
		if (targets.size() == 0)
		{
			printCommandChatInfoMessage(clientNum, commandText, Error::NoConnectedClientsError);
			return;
		}
		if (targets.size() > 1)
		{
			printCommandChatInfoMessage(clientNum, commandText, multipleMatchingNamesError(_sessionService->getNames(targets)));
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
	
	static const auto newNameOption = createOptionDefinition("newname", "New name for target player", CommandParser::OptionDefinition::Type::MultiToken, true);
	static const auto newNamePositionalOption = createOptionDefinition("newname", "New name for target player", CommandParser::OptionDefinition::Type::Token, true);
	_adminCommandsHandler->subscribe('R', createCommandDefinition("rename", "rename", {
		_requiredPlayerOption,
		newNameOption
	}, {
		_requiredPlayerOption,
		newNamePositionalOption
	}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		auto targets = _sessionService->findUsersByName(command.options.find("player")->second.text);
		if (targets.size() == 0)
		{
			printCommandChatInfoMessage(clientNum, commandText, Error::NoConnectedClientsError);
			return;
		}
		if (targets.size() > 1)
		{
			printCommandChatInfoMessage(clientNum, commandText, multipleMatchingNamesError(_sessionService->getNames(targets)));
			return;
		}

		auto name = command.options.find("newname")->second.text;
		_sessionService->updateUserInfoValue(targets[0], "name", name);
		Printer::sendCommand(targets[0], stringFormat("set_name \"%s\"", name));
	});

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
	auto optionalLevelOption = createOptionDefinition("level", "level to set the player to", CommandParser::OptionDefinition::Type::Integer, false);
	auto optionalIdOption = createOptionDefinition("id", "user id", CommandParser::OptionDefinition::Type::Integer, false);
	_adminCommandsHandler->subscribe('s', createCommandDefinition("setlevel", "setlevel", { _optionalPlayerOption, optionalLevelOption, optionalIdOption }), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		int id = 0;
		bool idSpecified = false;
		std::string player;
		int level = 0;
		bool levelSpecified = false;
		if (hasOption(command, "id"))
		{
			id = command.options.at("id").integer;
			idSpecified = true;
		} 
		if (hasOption(command, "player"))
		{
			player = command.options.at("player").text;
		} 
		if (hasOption(command, "level"))
		{
			level = command.options.at("level").integer;
			levelSpecified = true;
		}
		if ((!idSpecified && player.length() == 0) || !levelSpecified)
		{
			// try to parse the command as 
			// !setlevel <player> <level>
			if (command.extraArgs.size() != 2)
			{
				printCommandChatInfoMessage(clientNum, commandText, "!setlevel <player> <level>");
				return;
			}

			player = command.extraArgs[0];
			try
			{
				level = std::stoi(command.extraArgs[1]);
			} catch (const std::out_of_range&)
			{
				printCommandChatInfoMessage(clientNum, commandText, 
					stringFormat("%s is out of range. Allowed level range is %d-%d", 
						command.extraArgs[1], 
						std::numeric_limits<int>::min(), 
						std::numeric_limits<int>::max()));
				return;
			} catch (const std::invalid_argument&)
			{
				printCommandChatInfoMessage(clientNum, commandText,
					stringFormat("%s is not a number", command.extraArgs[1]));
				return;
			}
		}

		auto targets = _sessionService->findUsersByName(player);
		if (targets.size() == 0)
		{
			printCommandChatInfoMessage(clientNum, commandText, Error::NoConnectedClientsError);
			return;
		}
		if (targets.size() > 1)
		{
			printCommandChatInfoMessage(clientNum, commandText, multipleMatchingNamesError(_sessionService->getNames(targets)));
			return;
		}
		if (!_sessionService->isCallerLevelEqualOrHigher(clientNum, level))
		{
			printCommandChatInfoMessage(clientNum, commandText, "you're not allowed to set level to a higher level than your own level.");
			return;
		}
		if (_levelService->get(level) == nullptr)
		{
			printCommandChatInfoMessage(clientNum, commandText, stringFormat("level %d does not exist", level));
			return;
		}
		if (!idSpecified)
		{
			_sessionService->setLevelByClientNum(clientNum, targets[0], level);
		} else
		{
			_sessionService->setLevelById(clientNum, id, level);
		}
	});

	/**
	* spectate
	*/
	_adminCommandsHandler->subscribe('a', createCommandDefinition("spectate", "spectate", { _optionalPlayerOption }, {_optionalPlayerOption}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {
		if (!isPlayer(clientNum))
		{
			return;
		}

		if (command.options.find("player") == end(command.options))
		{
			if ((g_entities + clientNum)->client->sess.sessionTeam != TEAM_SPECTATOR)
			{
				SetTeam((g_entities + clientNum), "spectator", qfalse, static_cast<weapon_t>(-1), static_cast<weapon_t>(-1), qfalse);
			}
			return;
		}

		auto targets = _sessionService->findUsersByName(command.options.find("player")->second.text);
		if (targets.size() == 0)
		{
			printCommandChatInfoMessage(clientNum, commandText, Error::NoConnectedClientsError);
			return;
		}
		if (targets.size() > 1)
		{
			printCommandChatInfoMessage(clientNum, commandText, multipleMatchingNamesError(_sessionService->getNames(targets)));
			return;
		}
	});

	/**
	* unban
	*/
	_adminCommandsHandler->subscribe('b', createCommandDefinition("unban", "unban", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});

	/**
	* unmute
	*/
	_adminCommandsHandler->subscribe('m', createCommandDefinition("unmute", "unmute", {
		_requiredPlayerOption,
	}, {
		_requiredPlayerOption,
	}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command)
	{
		auto targets = _sessionService->findUsersByName(command.options.find("player")->second.text);
		if (targets.size() == 0)
		{
			printCommandChatInfoMessage(clientNum, commandText, Error::NoConnectedClientsError);
			return;
		}
		if (targets.size() > 1)
		{
			printCommandChatInfoMessage(clientNum, commandText, multipleMatchingNamesError(_sessionService->getNames(targets)));
			return;
		}

		if (_sessionService->unmute(targets[0]))
		{
			printCommandChatInfoMessage(clientNum, commandText, "Unmuted player " + _sessionService->getName(targets[0]));
		}
		else
		{
			printCommandChatInfoMessage(clientNum, commandText, "Player " + _sessionService->getName(targets[0]) + "^7 is not muted.");
		}
	});

	/**
	* userinfo
	*/
	_adminCommandsHandler->subscribe('A', createCommandDefinition("userinfo", "userinfo", {}), [&](int clientNum, const std::string& commandText, const ETJump::CommandParser::Command& command) {});
}

bool ETJump::AdminCommandsRegistrar::isPlayer(int clientNum)
{
	return clientNum >= 0 && clientNum < Constants::Common::MAX_CONNECTED_CLIENTS && (g_entities + clientNum)->client;
}

std::pair<std::string, ETJump::CommandParser::OptionDefinition> ETJump::AdminCommandsRegistrar::createOptionDefinition(const std::string& name, const std::string& description, CommandParser::OptionDefinition::Type type, bool required)
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

ETJump::CommandParser::CommandDefinition ETJump::AdminCommandsRegistrar::createCommandDefinition(const std::string& name, const std::string& description, const std::map<std::string, CommandParser::OptionDefinition>& options, const std::vector<std::pair<std::string, ETJump::CommandParser::OptionDefinition>>& positionalOptions)
{
	std::vector<ETJump::CommandParser::OptionDefinition> transformedPositionalOptions;
	for (const auto & opt : positionalOptions)
	{
		transformedPositionalOptions.push_back(opt.second);
	}
	return{
		name,
		description,
		options,
		transformedPositionalOptions
	};
}

std::string ETJump::AdminCommandsRegistrar::getOptionalText(const ETJump::CommandParser::Command& command, const std::string& key, const std::string& defaultValue)
{
	auto optIter = command.options.find(key);
	if (optIter != end(command.options))
	{
		return optIter->second.text;
	}
	return defaultValue;
}

int ETJump::AdminCommandsRegistrar::getOptionalInteger(const ETJump::CommandParser::Command& command, const std::string& key, int defaultValue)
{
	auto optIter = command.options.find(key);
	if (optIter != end(command.options))
	{
		return optIter->second.integer;
	}
	return defaultValue;
}

long long ETJump::AdminCommandsRegistrar::getOptionalDuration(const ETJump::CommandParser::Command& command, const std::string& key, long long defaultValue)
{
	auto optIter = command.options.find(key);
	if (optIter != end(command.options))
	{
		return optIter->second.duration;
	}
	return defaultValue;
}

void ETJump::AdminCommandsRegistrar::printCommandChatInfoMessage(int clientNum, const std::string& command, const std::string& message)
{
	Printer::sendChatMessage(clientNum, "^3" + command + ": ^7" + message);
}

std::string ETJump::AdminCommandsRegistrar::multipleMatchingNamesError(const std::vector<std::string>& names)
{
	std::string buffer = ETJump::Error::MultipleMatchingPlayers + "\n";

	for (const auto & n : names)
	{
		buffer += ETJump::stringFormat("* %s\n", n);
	}

	return buffer;
}

bool ETJump::AdminCommandsRegistrar::hasOption(const CommandParser::Command& command, const std::string& option)
{
	return command.options.find(option) != end(command.options);
}
