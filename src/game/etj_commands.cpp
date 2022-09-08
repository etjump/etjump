/*
 * MIT License
 * 
 * Copyright (c) 2022 ETJump team <zero@etjump.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <bitset>
#include "etj_commands.h"
#include "etj_local.h"
#include "etj_save_system.h"
#include "etj_session.h"
#include "etj_custom_map_votes.h"
#include "etj_timerun.h"
#include "g_local.h"
#include "etj_map_statistics.h"
#include "etj_utilities.h"
#include "etj_tokens.h"
#include "etj_string_utilities.h"

typedef std::function<bool (gentity_t *ent, Arguments argv)> Command;
typedef std::pair<std::function<bool (gentity_t *ent, Arguments argv)>, char> AdminCommandPair;
typedef std::map< std::string, std::function<bool (gentity_t *ent, Arguments argv)> >::const_iterator ConstCommandIterator;
typedef std::map< std::string, std::pair<std::function<bool (gentity_t *ent, Arguments argv)>, char> >::const_iterator ConstAdminCommandIterator;
typedef std::map< std::string, std::function<bool (gentity_t *ent, Arguments argv)> >::iterator CommandIterator;
typedef std::map< std::string, std::pair<std::function<bool (gentity_t *ent, Arguments argv)>, char> >::iterator AdminCommandIterator;

namespace CommandFlags
{
// Silent command execution via '/' flag
// Not listed here because it's not a command in itself

const char BAN = 'b';
// For everyone
const char BASIC       = 'a';
const char CANCELVOTE  = 'C';
// const char EBALL       = '8';
const char EDIT        = 'A';
const char FINGER      = 'f';
// const char HELP        = 'h';
const char KICK        = 'k';
const char LISTBANS    = 'L';
const char LISTPLAYERS = 'l';
const char MAP         = 'M';
const char MUTE        = 'm';
const char NOCLIP      = 'N';
const char NOGOTO      = 'K';
const char PASSVOTE    = 'P';
const char PUTTEAM     = 'p';
// const char READCONFIG  = 'G';
const char RENAME      = 'R';
const char RESTART     = 'r';
const char TOKENS      = 'V';
const char SAVESYSTEM  = 'T';
const char SETLEVEL    = 's';
const char MOVERSCALE  = 'v';
}

namespace ClientCommands
{

bool BackupLoad(gentity_t *ent, Arguments argv)
{
	ETJump::saveSystem->loadBackupPosition(ent);
	return true;
}

bool Load(gentity_t *ent, Arguments argv)
{
	ETJump::saveSystem->load(ent);
	return true;
}

bool Save(gentity_t *ent, Arguments argv)
{
	ETJump::saveSystem->save(ent);
	return true;
}

bool Unload(gentity_t *ent, Arguments argv)
{
	ETJump::saveSystem->unload(ent);
	return true;
}

bool ListInfo(gentity_t *ent, Arguments argv)
{
	if (argv->size() != 2)
	{
		std::string types = game.customMapVotes->ListTypes();
		ConsolePrintTo(ent, "^<List of custom map types: ^7" + types);
		return true;
	}

	const std::vector<std::string> *lines = game.customMapVotes->ListInfo(argv->at(1));
	if (lines->size() == 0)
	{
		ChatPrintTo(ent, "^<<listinfo: ^7list could not be found.");
		return false;
	}
	BeginBufferPrint();
	for (unsigned i = 0; i < lines->size(); i++)
	{
		BufferPrint(ent, lines->at(i));
	}
	FinishBufferPrint(ent, false);
	return true;
}

bool Records(gentity_t *ent, Arguments argv)
{
	std::string map = level.rawmapname, runName;

	if (argv->size() > 1)
	{
		runName = argv->at(1);
	}

	if (argv->size() > 2)
	{
		map = argv->at(2);
	}

	game.timerun->printRecords(ClientNum(ent), map, runName);
	return true;
}
}

void PrintManual(gentity_t *ent, const std::string& command)
{
	if (ent)
	{
		ChatPrintTo(ent, va("^3%s: ^7check console for more information.", command.c_str()));
		trap_SendServerCommand(ClientNum(ent), va("manual %s", command.c_str()));
	}
	else
	{
		int i   = 0;
		int len = sizeof(commandManuals) / sizeof(commandManuals[0]);
		for (i = 0; i < len; i++)
		{
			if (!Q_stricmp(commandManuals[i].cmd, command.c_str()))
			{
				G_Printf("%s\n\nUsage:\n%s\n\nDescription:\n%s\n",
				         commandManuals[i].cmd, commandManuals[i].usage,
				         commandManuals[i].description);
				return;
			}
		}
		if (i == len)
		{
			G_Printf("Couldn't find manual for command \"%s\"\n", command.c_str());
		}
	}
}

bool IsTargetHigherLevel(gentity_t *ent, gentity_t *target, bool equalIsHigher)
{
	if (!ent)
	{
		return false;
	}

	if (equalIsHigher)
	{
		return ETJump::session->GetLevel(ent) <= ETJump::session->GetLevel(target);
	}

	return ETJump::session->GetLevel(ent) < ETJump::session->GetLevel(target);
}

bool IsTargetHigherLevel(gentity_t *ent, unsigned id, bool equalIsHigher)
{
	if (equalIsHigher)
	{
		return ETJump::session->GetLevel(ent) <= ETJump::session->GetLevelById(id);
	}

	return ETJump::session->GetLevel(ent) < ETJump::session->GetLevelById(id);
}

namespace AdminCommands
{
const int CMDS_OPEN     = 1;
const int GREETING_OPEN = 2;
const int TITLE_OPEN    = 4;

namespace Updated
{
/* 
const unsigned NONE      = 0;
const unsigned LEVEL     = 0x00001;
const unsigned LAST_SEEN = 0x00002;
const unsigned NAME      = 0x00004;
*/
const unsigned TITLE     = 0x00008;
const unsigned COMMANDS  = 0x00010;
const unsigned GREETING  = 0x00020;
}

bool Admintest(gentity_t *ent, Arguments argv)
{
	if (!ent)
	{
		ChatPrintAll("^3admintest: ^7console is a level ? user.");
		return true;
	}

	ETJump::session->PrintAdmintest(ent);
	return true;
}

bool AddLevel(gentity_t *ent, Arguments argv)
{
	// !addlevel [level] -cmds [commands] -greeting [greeting] -title [title]
	if (argv->size() < 2)
	{
		PrintManual(ent, "addlevel");
		return false;
	}
	int open = 0;

	int         level = 0;
	std::string commands;
	std::string greeting;
	std::string title;

	if (!ToInt(argv->at(1), level))
	{
		ChatPrintTo(ent, va("^3system: ^7%d is not an integer.", level));
		return false;
	}

	if (argv->size() > 2)
	{
		ConstArgIter it = argv->begin() + 2;

		while (it != argv->end())
		{
			if (*it == "-cmds" && it + 1 != argv->end())
			{
				open = CMDS_OPEN;
			}
			else if (*it == "-greeting" && it + 1 != argv->end())
			{
				open = GREETING_OPEN;
			}
			else if (*it == "-title" && it + 1 != argv->end())
			{
				open = TITLE_OPEN;
			}
			else
			{
				switch (open)
				{
				case 0:
					ChatPrintTo(ent, va("^3addlevel: ^7ignored argument \"%s^7\".", it->c_str()));
					break;
				case CMDS_OPEN:
					commands += *it;
					break;
				case GREETING_OPEN:
					greeting += *it + " ";
					break;
				case TITLE_OPEN:
					title += *it + " ";
					break;
				default:
					break;
				}
			}

			it++;
		}

		greeting = ETJump::trimEnd(greeting);
		title = ETJump::trimEnd(title);
	}

	if (!game.levels->Add(level, title, commands, greeting))
	{
		ChatPrintTo(ent, "^3addlevel: ^7" + game.levels->ErrorMessage());
		return false;
	}

	ChatPrintTo(ent, va("^3addlevel: ^7added level %d.", level));

	return true;
}

bool Ball8(gentity_t *ent, Arguments argv)
{
	static const std::string Magical8BallResponses[] =
	{
		"It is certain",
		"It is decidedly so",
		"Without a doubt",
		"Yes - definitely",
		"You may rely on it",

		"As I see it, yes",
		"Most likely",
		"Outlook good",
		"Signs point to yes",
		"Yes",

		"Reply hazy, try again",
		"Ask again later",
		"Better not tell you now",
		"Cannot predict now",
		"Concentrate and ask again",

		"Don't count on it",
		"My reply is no",
		"My sources say no",
		"Outlook not so good",
		"Very doubtful"
	};
	constexpr int DELAY_8BALL = 3000;     // in milliseconds

	if (ent && ent->client->last8BallTime + DELAY_8BALL > level.time)
	{
		const int remainingSeconds = std::ceil((ent->client->last8BallTime + DELAY_8BALL - level.time) / 1000.0);
		ChatPrintTo(ent, "^3!8ball: ^7you must wait " + ETJump::getSecondsString(remainingSeconds) + " before using !8ball again.");
		return false;
	}

	if (argv->size() == 1)
	{
		PrintManual(ent, "8ball");
		return false;
	}

	int       random   = rand() % 20;
	const int POSITIVE = 10;
	const int NO_IDEA  = 15;
	if (random < POSITIVE)
	{
		ChatPrintAll("^3Magical 8 Ball: ^2" + Magical8BallResponses[random]);
	}
	else if (random < NO_IDEA)
	{
		ChatPrintAll("^3Magical 8 Ball: ^3" + Magical8BallResponses[random]);
	}
	else
	{
		ChatPrintAll("^3Magical 8 Ball: ^1" + Magical8BallResponses[random]);
	}

	if (ent)
	{
		ent->client->last8BallTime = level.time;
	}
	return true;
}

bool Ban(gentity_t *ent, Arguments argv)
{
	if (argv->size() == 1)
	{
		PrintManual(ent, "ban");
		return false;
	}
	// ban permanently
	time_t t;
	time(&t);
	std::string err;
	gentity_t   *player = PlayerGentityFromString(argv->at(1).c_str(), err);

	if (!player)
	{
		ChatPrintTo(ent, "^3ban: ^7no player with name " + argv->at(1));
		return false;
	}

	if (IsTargetHigherLevel(ent, player, true))
	{
		ChatPrintTo(ent, "^3ban: ^7you cannot ban a fellow admin.");
		return false;
	}

	unsigned    expires = 0;
	std::string reason  = "Banned by admin.";

	// !ban <name> <time> <reason>
	if (argv->size() >= 3)
	{
		if (!ToUnsigned(argv->at(2), expires))
		{
			// TODO: 10m, 2h etc.
			ChatPrintTo(ent, "^3ban: ^7time was not a number.");
			return false;
		}

		expires = static_cast<unsigned>(t) + expires;
	}

	if (argv->size() >= 4)
	{
		reason = "";
		for (size_t i = 3; i < argv->size(); i++)
		{
			if (i + 1 == argv->size())
			{
				reason += argv->at(i);
			}
			else
			{
				reason += argv->at(i) + " ";

			}

		}
	}
	if (!ETJump::session->Ban(ent, player, expires, reason))
	{
		ChatPrintTo(ent, "^3ban: ^7" + ETJump::session->GetMessage());
		return false;
	}

	trap_DropClient(ClientNum(player), "You are banned", 0);
	return true;
}

bool Cancelvote(gentity_t *ent, Arguments argv)
{
	if (level.voteInfo.voteTime)
	{
		level.voteInfo.voteCanceled = qtrue;
		ChatPrintAll("^3cancelvote: ^7vote has been canceled.");
	}
	else
	{
		ChatPrintTo(ent, "^3cancelvote: ^7no vote in progress.");
	}
	return true;
}

bool DeleteLevel(gentity_t *ent, Arguments argv)
{
	if (argv->size() != 2)
	{
		PrintManual(ent, "deletelevel");
		return false;
	}

	int level = 0;
	if (!ToInt(argv->at(1), level))
	{
		ChatPrintTo(ent, va("^3deletelevel: ^7%s is not an integer.", argv->at(1).c_str()));
		return false;
	}

	if (!game.levels->Delete(level))
	{
		ChatPrintTo(ent, "^3deletelevel: ^7couldn't find level.");
		return false;
	}

	int usersWithLevel = ETJump::session->LevelDeleted(level);

	ChatPrintTo(ent, "^3deletelevel: ^7deleted level. Set " + std::to_string(usersWithLevel) + " users to level 0.");

	return true;
}

bool EditCommands(gentity_t *ent, Arguments argv)
{
	// !editcommands level +command|-command +command|-command etc.
	if (argv->size() < 3)
	{
		PrintManual(ent, "editcommands");
		return false;
	}

	int level = 0;
	if (!ToInt(argv->at(1), level))
	{
		ChatPrintTo(ent, "^3editcommands: ^7defined level \"" + (*argv)[1] + "\" is not an integer.");
		return false;
	}

	if (!game.levels->LevelExists(level))
	{
		ChatPrintTo(ent, "^3editcommands: ^7level " + (*argv)[1] + " does not exist.");
		return false;
	}

	ConstArgIter it  = argv->begin() + 2;
	ConstArgIter end = argv->end();

	std::string currentPermissions = game.levels->GetLevel(level)->commands;

	bool        add = true;
	std::string currentCommand;
	std::string addCommands    = "+";
	std::string deleteCommands = "-";
	for (; it != end; it++)
	{
		if ((*it)[0] == '-')
		{
			add            = false;
			currentCommand = (*it).substr(1);
		}
		else if ((*it)[0] == '+')
		{
			add            = true;
			currentCommand = (*it).substr(1);
		}
		else
		{
			add            = true;
			currentCommand = (*it);
		}
		char flag = game.commands->FindCommandFlag(currentCommand);
		if (flag == 0)
		{
			ChatPrintTo(ent, "^3editcommands: ^7command \"" + currentCommand + "\" doesn't match any known command.");
			continue;
		}
		if (add)
		{
			addCommands += flag;
		}
		else
		{
			deleteCommands += flag;
		}
	}

	std::string duplicateFlags = "";
	for (size_t i = 0; i < addCommands.size(); i++)
	{
		if (deleteCommands.find(addCommands[i]) != std::string::npos)
		{
			ChatPrintTo(ent, va("^3editcommands: ^7ignoring command flag \"%c\". Are you trying to add or delete it?", addCommands[i]));
			duplicateFlags.push_back(addCommands[i]);
		}
	}

	if (duplicateFlags.length() > 0)
	{
		std::string temp;
		for (size_t i = 0; i < addCommands.length(); i++)
		{
			if (duplicateFlags.find(addCommands[i]) == std::string::npos)
			{
				temp += addCommands[i];
			}
		}
		addCommands = temp;

		temp.clear();
		for (size_t i = 0; i < deleteCommands.length(); i++)
		{
			if (duplicateFlags.find(deleteCommands[i]) == std::string::npos)
			{
				temp += deleteCommands[i];
			}
		}
		deleteCommands = temp;
	}

	// always has + in it
	if (addCommands.length() > 1)
	{
		currentPermissions += addCommands;
	}
	// always has - in it
	if (deleteCommands.length() > 1)
	{
		currentPermissions += deleteCommands;
	}
	game.levels->Edit(level, "", currentPermissions, "", 1);

	ChatPrintTo(ent, "^3editcommands: ^7edited level " + (*argv)[1] + " permissions. New permissions are: " + game.levels->GetLevel(level)->commands);

	return true;
}

bool EditLevel(gentity_t *ent, Arguments argv)
{
	if (argv->size() < 4)
	{
		PrintManual(ent, "editlevel");
		return false;
	}

	int updated = 0;
	int open    = 0;

	int         adminLevel = 0;
	std::string commands;
	std::string greeting;
	std::string title;

	if (!ToInt(argv->at(1), adminLevel))
	{
		ChatPrintTo(ent, va("^3editlevel: ^7%d is not an integer.", adminLevel));
		return false;
	}

	if (argv->size() > 2)
	{
		ConstArgIter it = argv->begin() + 2;

		while (it != argv->end())
		{
			if (*it == "-cmds" && it + 1 != argv->end())
			{
				open     = CMDS_OPEN;
				updated |= CMDS_OPEN;
			}
			else if (*it == "-greeting" && it + 1 != argv->end())
			{
				open     = GREETING_OPEN;
				updated |= GREETING_OPEN;
			}
			else if (*it == "-title" && it + 1 != argv->end())
			{
				open     = TITLE_OPEN;
				updated |= TITLE_OPEN;
			}
			else if (*it == "-clear" && it + 1 != argv->end())
			{
				ConstArgIter nextIt = it + 1;
				if (*nextIt == "cmds")
				{
					commands = "";
					updated |= CMDS_OPEN;

				}
				else if (*nextIt == "greeting")
				{
					greeting = "";
					updated |= GREETING_OPEN;
				}
				else if (*nextIt == "title")
				{
					title    = "";
					updated |= TITLE_OPEN;
				}
				else
				{
					it++;
				}
			}
			else
			{
				switch (open)
				{
				case 0:
					if (updated == 0)
					{
						ChatPrintTo(ent, va("^editlevel: ^7ignored argument \"%s^7\".", it->c_str()));
					}

					break;
				case CMDS_OPEN:
					commands += *it;
					break;
				case GREETING_OPEN:
					greeting += *it + " ";
					break;
				case TITLE_OPEN:
					title += *it + " ";
					break;
				default:
					break;
				}
			}

			it++;
		}

		greeting = ETJump::trimEnd(greeting);
		title = ETJump::trimEnd(title);
	}

	game.levels->Edit(adminLevel, title, commands, greeting, updated);

	for (int i = 0; i < level.numConnectedClients; i++)
	{
		int num = level.sortedClients[i];

		ETJump::session->ParsePermissions(num);
	}

	ChatPrintTo(ent, va("^3editlevel: ^7updated level %d.", adminLevel));

	return true;
}

bool EditUser(gentity_t *ent, Arguments argv)
{
	if (argv->size() < 4)
	{
		PrintManual(ent, "edituser");
		return false;
	}

	unsigned id = 0;
	if (!ToUnsigned(argv->at(1), id))
	{
		ChatPrintTo(ent, "^3edituser: ^7invalid id " + argv->at(1));
		return false;
	}

	if (!ETJump::database->UserExists(id))
	{
		ChatPrintTo(ent, "^3edituser: ^7user does not exist.");
		return false;
	}

	int updated = 0;
	int open    = 0;

	std::string commands;
	std::string greeting;
	std::string title;

	ConstArgIter it = argv->begin() + 2;
	while (it != argv->end())
	{
		if (*it == "-cmds" && it + 1 != argv->end())
		{
			open     = CMDS_OPEN;
			updated |= Updated::COMMANDS;
		}
		else if (*it == "-greeting" && it + 1 != argv->end())
		{
			open     = GREETING_OPEN;
			updated |= Updated::GREETING;
		}
		else if (*it == "-title" && it + 1 != argv->end())
		{
			open     = TITLE_OPEN;
			updated |= Updated::TITLE;
		}
		else if (*it == "-clear" && it + 1 != argv->end())
		{
			ConstArgIter nextIt = it + 1;
			if (*nextIt == "cmds")
			{
				commands = "";
				updated |= Updated::COMMANDS;
			}
			else if (*nextIt == "greeting")
			{
				greeting = "";
				updated |= Updated::GREETING;
			}
			else if (*nextIt == "title")
			{
				title    = "";
				updated |= Updated::TITLE;
			}
			it++;
		}
		else
		{
			switch (open)
			{
			case 0:
				ChatPrintTo(ent, va("^3edituser: ^7ignored argument \"%s^7\".", it->c_str()));
				break;
			case CMDS_OPEN:
				commands += *it;
				break;
			case GREETING_OPEN:
				greeting += *it + " ";
				break;
			case TITLE_OPEN:
				title += *it + " ";
				break;
			default:
				break;
			}
		}

		it++;
	}
		
	greeting = ETJump::trimEnd(greeting);
	title = ETJump::trimEnd(title);

	ChatPrintTo(ent, va("^3edituser: ^7updating user %d", id));
	return ETJump::database->UpdateUser(ent, id, commands, greeting, title, updated);
}

bool FindUser(gentity_t *ent, Arguments argv)
{
	if (argv->size() != 2)
	{
		ChatPrintTo(ent, "^3usage: ^7!finduser <name>");
		return false;
	}

	ETJump::database->FindUser(ent, argv->at(1));

	return true;
}

bool FindMap(gentity_t *ent, Arguments argv)
{
	if (argv->size() < 2)
	{
		ChatPrintTo(ent, "^3usage: ^7!findmap <map> [maps per row]");
		return false;
	}

	auto perRow = 3;
	if (argv->size() == 3)
	{
		try
		{
			perRow = std::stoi(argv->at(2));
		}
		catch (...)
		{
			perRow = 3;
		}
	}

	auto                     maps = game.mapStatistics->getMaps();
	std::vector<std::string> matching;
	for (auto &map : maps)
	{
		if (map.find(argv->at(1)) != std::string::npos)
		{
			matching.push_back(map);
		}
	}

	auto        mapsOnCurrentRow = 0;
	std::string buffer           = "^zFound " + std::to_string(matching.size()) + " maps:\n^7";
	for (auto& map : matching)
	{
		++mapsOnCurrentRow;
		if (mapsOnCurrentRow > perRow)
		{
			mapsOnCurrentRow = 1;
			buffer          += ETJump::stringFormat("\n%-22s", map);
		}
		else
		{
			buffer += ETJump::stringFormat("%-22s", map);
		}

	}

	buffer += "\n";

	Utilities::toConsole(ent, buffer);
	return true;
}

bool ListUserNames(gentity_t *ent, Arguments argv)
{
	if (argv->size() != 2)
	{
		ChatPrintTo(ent, "^3usage: ^7!listusernames <id>");
		return false;
	}

	int id;
	if (!ToInt(argv->at(1), id))
	{
		ChatPrintTo(ent, va("^3listusernames: ^7%s is not an id", argv->at(1).c_str()));
		return false;
	}

	ETJump::database->ListUserNames(ent, id);
	return true;
}

bool AdminCommands(gentity_t *ent, Arguments argv)
{
	ChatPrintTo(ent, "AdminCommands is not implemented.");
	return true;
}

bool Finger(gentity_t *ent, Arguments argv)
{
	if (argv->size() != 2)
	{
		PrintManual(ent, "finger");
		return false;
	}

	std::string err;
	gentity_t   *target = PlayerGentityFromString(argv->at(1), err);
	if (!target)
	{
		ChatPrintTo(ent, "^3finger: ^7" + err);
		return false;
	}


	ETJump::session->PrintFinger(ent, target);
	return true;
}

bool Help(gentity_t *ent, Arguments argv)
{
	if (argv->size() == 1)
	{
		game.commands->List(ent);
	}
	else
	{
		PrintManual(ent, argv->at(1));
	}

	return true;
}

bool Kick(gentity_t *ent, Arguments argv)
{
	const unsigned MIN_ARGS = 2;
	if (argv->size() < MIN_ARGS)
	{
		PrintManual(ent, "kick");
		return false;
	}

	std::string error   = "";
	gentity_t   *target = PlayerGentityFromString(argv->at(1), error);
	if (!target)
	{
		ChatPrintTo(ent, "^3kick: " + error);
		return false;
	}

	if (ent)
	{
		if (ent == target)
		{
			ChatPrintTo(ent, "^3kick: ^7you can't kick yourself.");
			return false;
		}

		if (IsTargetHigherLevel(ent, target, true))
		{
			ChatPrintTo(ent, "^3kick: ^7you can't kick a fellow admin.");
			return false;
		}
	}

	int timeout = 0;
	if (argv->size() >= 3)
	{
		if (!ToInt(argv->at(2), timeout))
		{
			ChatPrintTo(ent, "^3kick: ^7invalid timeout \"" + argv->at(2) + "\" specified.");
			return false;
		}
	}

	std::string reason;
	if (argv->size() >= 4)
	{
		reason = argv->at(3);
	}

	trap_DropClient(target - g_entities, reason.c_str(), timeout);
	return true;
}

std::string playedTimeFmtString(int seconds)
{
	auto minutes = seconds / 60;
	seconds -= minutes * 60;
	auto hours = minutes / 60;
	minutes -= hours * 60;
	auto days = hours / 24;
	hours -= days * 24;
	auto weeks = days / 7;
	days -= weeks * 7;

	std::string str;
	if (weeks)
	{
		str = ETJump::getWeeksString(weeks);
	}
	else if (days)
	{
		str = ETJump::getDaysString(days);
	}
	else if (hours || minutes || seconds)
	{
		str = ETJump::getHoursString(hours) + " " + ETJump::getMinutesString(minutes) + " " + ETJump::getSecondsString(seconds);
	}

	if (str.length() == 0)
	{
		str = "never";
	}
	return str;
}

bool LeastPlayed(gentity_t *ent, Arguments argv)
{
	auto mapsToList = 10;
	if (argv->size() == 2)
	{
		try
		{
			mapsToList = std::stoi((*argv)[1]);
		}
		catch (const std::invalid_argument&)
		{
			ChatPrintTo(ent, ETJump::stringFormat("^3Error: ^7%s^7 is not a number", argv->at(1)));
			return false;
		}
		catch (const std::out_of_range&)
		{
			mapsToList = 10;
		}
	}

	auto leastPlayed = game.mapStatistics->getLeastPlayed();

	auto        listedMaps = 0;
	std::string buffer     = "^zLeast played maps are:\n"
	                         "^gMap                    Played                         Last played       Times played\n";
	auto green = false;
	for (auto& map : leastPlayed)
	{
		if (listedMaps >= mapsToList)
		{
			break;
		}

		buffer += green ? "^g" : "^7";
		buffer += ETJump::stringFormat("%-22s %-30s %-17s     %d\n", map->name, playedTimeFmtString(map->secondsPlayed), Utilities::timestampToString(map->lastPlayed), map->timesPlayed);
		green   = !green;

		++listedMaps;
	}

	Utilities::toConsole(ent, buffer);

	return true;
}

bool LevelInfo(gentity_t *ent, Arguments argv)
{
	if (argv->size() == 1)
	{
		game.levels->PrintLevelInfo(ent);
	}
	else
	{
		int level = 0;
		if (!ToInt(argv->at(1), level))
		{
			ChatPrintTo(ent, "^3levelinfo: ^7 invalid level.");
			return false;
		}

		game.levels->PrintLevelInfo(ent, level);
	}
	return true;
}

bool ListBans(gentity_t *ent, Arguments argv)
{
	int page = 1;

	if (argv->size() > 1)
	{
		if (!ToInt(argv->at(1), page))
		{
			ChatPrintTo(ent, "^3listbans: ^7page is not a number.");
			return false;
		}
	}

	if (page < 1) page = 1;

	ETJump::database->ListBans(ent, page);

	return true;
}

bool ListFlags(gentity_t *ent, Arguments argv)
{
	game.commands->ListCommandFlags(ent);
	return true;
}

bool ListMaps(gentity_t *ent, Arguments argv)
{
	auto perRow = 3;

	if (argv->size() == 2)
	{
		try
		{
			perRow = std::stoi(argv->at(1), 0, 10);
		}
		catch (const std::invalid_argument&)
		{
			ChatPrintTo(ent, ETJump::stringFormat("^3listmaps: ^7%s^7 is not a number", argv->at(1)));
			return false;
		}
		catch (const std::out_of_range&)
		{
			perRow = 10;
		}

		if (perRow < 0)
		{
			ChatPrintTo(ent, "^3listmaps: ^7second argument must be over 0");
			return false;
		}

		if (perRow > 10)
		{
			perRow = 10;
		}
	}

	std::string buffer           = "^zListing all maps on server:\n^7";
	auto        maps             = game.mapStatistics->getMaps();
	auto        mapsOnCurrentRow = 0;
	for (auto& map : maps)
	{
		++mapsOnCurrentRow;
		if (mapsOnCurrentRow > perRow)
		{
			mapsOnCurrentRow = 1;
			buffer          += ETJump::stringFormat("\n%-22s", map);
		}
		else
		{
			buffer += ETJump::stringFormat("%-22s", map);
		}

	}

	buffer += "\n";

	buffer += ETJump::stringFormat("\n^zFound ^3%d ^zmaps on the server.\n", static_cast<int>(maps.size()));

	Utilities::toConsole(ent, buffer);

	return true;
}

bool ListUsers(gentity_t *ent, Arguments argv)
{
	int page = 1;

	if (argv->size() > 1)
	{
		if (!ToInt(argv->at(1), page))
		{
			ChatPrintTo(ent, "^3listusers: ^7page was not a number.");
			return false;
		}
	}

	ETJump::database->ListUsers(ent, page);
	return true;
}

bool ListPlayers(gentity_t *ent, Arguments argv)
{

	if (argv->size() == 1)
	{
		BeginBufferPrint();

		if (!level.numConnectedClients)
		{
			BufferPrint(ent, "^7There are currently no connected players.\n");
		}
		else
		{
			if (level.numConnectedClients > 1)
			{
				BufferPrint(ent, ETJump::stringFormat("^7There are currently %d connected players.\n", level.numConnectedClients));
			}
			else
			{
				BufferPrint(ent, "^7There is currently 1 connected player.\n");
			}
		}

		BufferPrint(ent, "#  ETJumpID  Level  Player\n");

		for (auto i = 0; i < level.numConnectedClients; i++)
		{
			auto clientNum = level.sortedClients[i];
			auto id        = ETJump::session->GetId(g_entities + clientNum);

			BufferPrint(ent, ETJump::stringFormat("^7%-2d %-9s %-6d %-s\n",
			                  clientNum,
			                  (id == -1 ? "-" : std::to_string(id)),
			                  ETJump::session->GetLevel(g_entities + clientNum),
			                  (g_entities + clientNum)->client->pers.netname));
		}

		FinishBufferPrint(ent);
	}

	return true;
}

bool Map(gentity_t *ent, Arguments argv)
{
	if (argv->size() != 2)
	{
		// PrintManual(ent, "map");
		return false;
	}

	std::string requestedMap = ETJump::StringUtil::toLowerCase(argv->at(1));

	if (!MapExists(requestedMap))
	{
		ChatPrintTo(ent, "^3map: ^7" + requestedMap + " is not on the server.");
		return false;
	}

	MapStatistics mapStats;

	if (strstr(mapStats.getBlockedMapsStr().c_str(), requestedMap.c_str()) != nullptr)
	{
		ChatPrintTo(ent, "^3map: ^7" + requestedMap + " cannot be played on this server.");
		return false;
	}

	trap_SendConsoleCommand(EXEC_APPEND, va("map %s\n", requestedMap.c_str()));
	return true;
}

bool MapInfo(gentity_t *ent, Arguments argv)
{
	auto mi         = game.mapStatistics->getMapInformation(argv->size() > 1 ? argv->at(1) : level.rawmapname);
	auto currentMap = game.mapStatistics->getCurrentMap();

	if (mi == nullptr)
	{
		ChatPrintTo(ent, "^3mapinfo: ^7Could not find the map");
		return false;
	}

	int seconds = mi->secondsPlayed;
	int minutes = seconds / 60;
	seconds = seconds - minutes * 60;
	int hours = minutes / 60;
	minutes = minutes - hours * 60;
	int days = hours / 24;
	hours = hours - days * 24;

	std::string message;
	if (mi == currentMap)
	{
		message = "^3mapinfo: ^7" + mi->name + " is the current map on the server. It has been played for a total of " + ETJump::getDaysString(days) + " " + ETJump::getHoursString(hours) + " " + ETJump::getMinutesString(minutes) + " " + ETJump::getSecondsString(seconds);
	}
	else
	{
		if (mi->lastPlayed == 0)
		{
			message = ETJump::stringFormat("^3mapinfo: ^7%s has never been played.", mi->name);
		}
		else
		{
			message = "^3mapinfo: ^7" + mi->name + " was last played on " + Utilities::timestampToString(mi->lastPlayed) + ". It has been played for a total of " + ETJump::getDaysString(days) + " " + ETJump::getHoursString(hours) + " " + ETJump::getMinutesString(minutes) + " " + ETJump::getSecondsString(seconds);
		}
	}
	ChatPrintTo(ent, message);

	return true;
}

bool MostPlayed(gentity_t *ent, Arguments argv)
{
	auto mapsToList = 10;
	if (argv->size() == 2)
	{
		try
		{
			mapsToList = std::stoi((*argv)[1]);
		}
		catch (const std::invalid_argument&)
		{
			ChatPrintTo(ent, ETJump::stringFormat("^3Error: ^7%s^7 is not a number", argv->at(1)));
			return false;
		}
		catch (const std::out_of_range&)
		{
			mapsToList = 1000;
		}
	}

	auto mostPlayed = game.mapStatistics->getMostPlayed();

	auto        listedMaps = 0;
	std::string buffer     = "^zMost played maps are:\n"
	                         "^gMap                    Played                         Last played       Times played\n";
	auto green = false;
	for (auto& map : mostPlayed)
	{
		if (listedMaps >= mapsToList)
		{
			break;
		}

		buffer += green ? "^g" : "^7";
		buffer += ETJump::stringFormat("%-22s %-30s %-17s     %d\n", map->name, playedTimeFmtString(map->secondsPlayed), Utilities::timestampToString(map->lastPlayed), map->timesPlayed);
		green   = !green;

		++listedMaps;
	}

	Utilities::toConsole(ent, buffer);

	return true;
}

void MutePlayer(gentity_t *target)
{
	target->client->sess.muted = qtrue;

	char userinfo[MAX_INFO_STRING] = "\0";
	char *ip                       = NULL;

	trap_GetUserinfo(target - g_entities, userinfo, sizeof(userinfo));
	ip = Info_ValueForKey(userinfo, "ip");

	G_AddIpMute(ip);
}

bool Mute(gentity_t *ent, Arguments argv)
{
	if (argv->size() != 2)
	{
		PrintManual(ent, "mute");
		return false;
	}

	std::string errorMsg;

	gentity_t *target = PlayerGentityFromString(argv->at(1), errorMsg);
	if (!target)
	{
		ChatPrintTo(ent, "^3!mute: ^7" + errorMsg);
		return false;
	}

	if (ent)
	{
		if (ent == target)
		{
			ChatPrintTo(ent, "^3mute: ^7you cannot mute yourself.");
			return false;
		}

		if (IsTargetHigherLevel(ent, target, true))
		{
			ChatPrintTo(ent, "^3mute: ^7you cannot mute a fellow admin.");
			return false;
		}
	}

	if (target->client->sess.muted == qtrue)
	{
		ChatPrintTo(ent, "^3mute: " + std::string(target->client->pers.netname) + " ^7is already muted.");
		return false;
	}

	MutePlayer(target);
	CPTo(target, "^5You've been muted");
	ChatPrintTo(ent, std::string(target->client->pers.netname) + " ^7has been muted.");
	return true;
}

bool Noclip(gentity_t *ent, Arguments argv)
{
	if (level.noNoclip)
	{
		ChatPrintTo(ent, "^3noclip: ^7noclip is disabled on this map.");
		return false;
	}

	if (argv->size() == 1)
	{
		if (!ent)
		{
			return false;
		}

		if (ent->client->sess.timerunActive)
		{
			ChatPrintTo(ent, "^3noclip: ^7cheats are disabled while timerun is active.");
			return false;
		}

		ent->client->noclip = ent->client->noclip ? qfalse : qtrue;
	}
	else
	{
		int count = 1;
		if (argv->size() == 3)
		{
			if (!ToInt(argv->at(2), count))
			{
				count = 1;
			}
		}
		std::string err;
		gentity_t   *other = PlayerGentityFromString(argv->at(1), err);
		if (!other)
		{
			ChatPrintTo(ent, "^3noclip: ^7" + err);
			return false;
		}

		if (other->client->sess.timerunActive)
		{
			ChatPrintTo(other, "^3noclip: ^7cheats are disabled while timerun is active.");
			return false;
		}

		if (count > 1)
		{
			ChatPrintTo(other, va("^3noclip: ^7you can use /noclip %d times.", count));
			ChatPrintTo(ent, va("^3noclip: ^7%s^7 can use /noclip %d times.", other->client->pers.netname, count));
		}
		else if (count < 0)
		{
			ChatPrintTo(other, "^3noclip: ^7you can use /noclip infinitely.");
			ChatPrintTo(ent, va("^3noclip: ^7%s^7 can use /noclip infinitely.", other->client->pers.netname));
		}
		else
		{
			ChatPrintTo(other, "^3noclip: ^7you can use /noclip once.");
			ChatPrintTo(ent, va("^3noclip: ^7%s^7 can use /noclip once.", other->client->pers.netname));
		}

		other->client->pers.noclipCount = count;
	}

	return true;
}

bool NoGoto(gentity_t *ent, Arguments argv)
{
	if (argv->size() != 2)
	{
		PrintManual(ent, "nogoto");
		return false;
	}

	std::string err;
	gentity_t   *target = NULL;
	target = PlayerGentityFromString(argv->at(1), err);
	if (!target)
	{
		ChatPrintTo(ent, "^3nocall: ^7" + err);
		return false;
	}

	if (target->client->sess.noGoto)
	{
		target->client->sess.noGoto = qfalse;
		ChatPrintTo(ent, va("^nogoto: ^7%s can use /goto now.", target->client->pers.netname));
		ChatPrintTo(target, "^3nogoto: ^7you can use /goto now.");
	}
	else
	{
		target->client->sess.noGoto = qtrue;
		ChatPrintTo(ent, va("^3nogoto: ^7%s can no longer use /goto.", target->client->pers.netname));
		ChatPrintTo(target, "^3nogoto: ^7you can no longer use /goto.");
	}

	return true;
}

bool NoSave(gentity_t *ent, Arguments argv)
{
	if (argv->size() != 2)
	{
		PrintManual(ent, "nosave");
		return false;
	}

	std::string err;
	gentity_t   *target = PlayerGentityFromString(argv->at(1), err);
	if (!target)
	{
		ChatPrintTo(ent, "^3nosave: ^7" + err);
		return false;
	}

	if (IsTargetHigherLevel(ent, target, true))
	{
		ChatPrintTo(ent, "^3nosave:^7 can't disable fellow admin's save and load.");
		return false;
	}

	if (target->client->sess.saveAllowed)
	{
		target->client->sess.saveAllowed = qfalse;
		ChatPrintTo(target, va("^3system:^7 %s^7 you are not allowed to save your position.", target->client->pers.netname));
		ChatPrintTo(ent, va("^3system:^7 %s^7 is not allowed to save their position.", target->client->pers.netname));
	}
	else
	{
		target->client->sess.saveAllowed = qtrue;
		ChatPrintTo(target, va("^3system:^7 %s^7 you are now allowed to save your position.", target->client->pers.netname));
		ChatPrintTo(ent, va("^3system:^7 %s^7 is now allowed to save their position.", target->client->pers.netname));
	}

	return true;
}

bool Passvote(gentity_t *ent, Arguments argv)
{
	if (level.voteInfo.voteTime)
	{
		level.voteInfo.forcePass = qtrue;
		ChatPrintAll("^3passvote:^7 vote has been passed.");
	}
	else
	{
		ChatPrintAll("^3passvote:^7 no vote in progress.");
	}
	return qtrue;
}

bool Putteam(gentity_t *ent, Arguments argv)
{
	if (argv->size() != 3)
	{
		PrintManual(ent, "putteam");
		return false;
	}

	std::string err;
	gentity_t   *target = PlayerGentityFromString(argv->at(1), err);
	if (!target)
	{
		ChatPrintTo(ent, "^3putteam: ^7" + err);
		return false;
	}

	if (IsTargetHigherLevel(ent, target, false))
	{
		ChatPrintTo(ent, "^3putteam: ^7you can't use putteam on a fellow admin.");
		return false;
	}

	const weapon_t w = static_cast<weapon_t>(-1);
	SetTeam(target, argv->at(2).c_str(), qfalse, w, w, qtrue);

	return true;
}

bool ReadConfig(gentity_t *ent, Arguments argv)
{
	ChatPrintTo(ent, "ReadConfig is not implemented.");
	return true;
}

bool RemoveSaves(gentity_t *ent, Arguments argv)
{
	if (argv->size() != 2)
	{
		PrintManual(ent, "rmsaves");
		return false;
	}

	std::string error;
	gentity_t   *target = PlayerGentityFromString(argv->at(1), error);
	if (!target)
	{
		ChatPrintTo(ent, "^3rmsaves: ^7" + error);
		return false;
	}

	if (IsTargetHigherLevel(ent, target, true))
	{
		ChatPrintTo(ent, "^3rmsaves: ^7can't remove fellow admin's saves.");
		return false;
	}

	ETJump::saveSystem->resetSavedPositions(target);
	ChatPrintTo(ent, va("^3system: ^7%s^7's saves were removed.", target->client->pers.netname));
	ChatPrintTo(target, "^3system: ^7your saves were removed");
	return true;
}

bool Rename(gentity_t *ent, Arguments argv)
{
	if (argv->size() != 3)
	{
		PrintManual(ent, "rename");
		return false;
	}

	std::string err;
	gentity_t   *target = PlayerGentityFromString(argv->at(1), err);
	if (!target)
	{
		ChatPrintTo(ent, "^3rename: ^7" + err);
		return false;
	}

	char userinfo[MAX_INFO_STRING] = "\0";
	int  cn                        = ClientNum(target);
	trap_GetUserinfo(cn, userinfo, sizeof(userinfo));

	const char *oldName = Info_ValueForKey(userinfo, "name");
	ChatPrintAll(va("^3rename: ^7%s^7 has been renamed to %s", oldName, argv->at(2).c_str()));
	Info_SetValueForKey(userinfo, "name", argv->at(2).c_str());
	trap_SetUserinfo(cn, userinfo);
	ClientUserinfoChanged(cn);
	trap_SendServerCommand(cn, va("set_name %s", argv->at(2).c_str()));
	return true;
}

bool Restart(gentity_t *ent, Arguments argv)
{
	Svcmd_ResetMatch_f(qfalse, qtrue);
	return true;
}

bool SetLevel(gentity_t *ent, Arguments argv)
{
	// !setlevel <player> <level>
	// !setlevel -id id level

	if (argv->size() == 3)
	{
		std::string err;
		gentity_t   *target = PlayerGentityFromString(argv->at(1), err);

		if (!target)
		{
			ChatPrintTo(ent, err);
			return false;
		}

		int level = 0;
		if (!ToInt(argv->at(2), level))
		{
			ChatPrintTo(ent, "^3setlevel: ^7invalid level " + argv->at(2));
			return false;
		}

		if (ent)
		{
			if (IsTargetHigherLevel(ent, target, false))
			{
				ChatPrintTo(ent, "^3setlevel: ^7you can't set the level of a fellow admin.");
				return false;
			}

			if (level > ETJump::session->GetLevel(ent))
			{
				ChatPrintTo(ent, "^3setlevel: ^7you're not allowed to setlevel higher than your own level.");
				return false;
			}
		}

		if (!game.levels->LevelExists(level))
		{
			ChatPrintTo(ent, "^3setlevel: ^7level does not exist.");
			return false;
		}

		if (!ETJump::session->SetLevel(target, level))
		{
			ChatPrintTo(ent, va("^3setlevel: ^7%s", ETJump::session->GetMessage().c_str()));
			return false;
		}

		ChatPrintTo(ent, va("^3setlevel: ^7%s^7 is now a level %d user.", target->client->pers.netname, level));
		ChatPrintTo(target, va("^3setlevel: ^7you are now a level %d user.", level));

		return true;
	}
	if (argv->size() == 4)
	{
		unsigned id = 0;
		if (!ToUnsigned(argv->at(2), id))
		{
			ChatPrintTo(ent, "^3setlevel: ^7invalid id " + argv->at(2));
			return false;
		}

		if (!ETJump::session->UserExists(id))
		{
			ChatPrintTo(ent, "^3setlevel: ^7user with id " + argv->at(2) + " doesn't exist.");
			return false;
		}

		int level = 0;
		if (!ToInt(argv->at(3), level))
		{
			ChatPrintTo(ent, "^3setlevel: ^7invalid level " + argv->at(2));
			return false;
		}

		if (ent)
		{
			if (IsTargetHigherLevel(ent, id, false))
			{
				ChatPrintTo(ent, "^3setlevel: ^7you can't set the level of a fellow admin.");
				return false;
			}

			if (level > ETJump::session->GetLevel(ent))
			{
				ChatPrintTo(ent, "^3setlevel: ^7you're not allowed to setlevel higher than your own level.");
				return false;
			}
		}

		if (!game.levels->LevelExists(level))
		{
			ChatPrintTo(ent, "^3setlevel: ^7level does not exist.");
			return false;
		}

		if (!ETJump::session->SetLevel(id, level))
		{
			ChatPrintTo(ent, va("^3setlevel: ^7%s", ETJump::session->GetMessage().c_str()));
			return false;
		}

		ChatPrintTo(ent, va("^3setlevel: ^7user with id %d is now a level %d user.", id, level));
	}
	else
	{

	}

	return true;
}

bool Spectate(gentity_t *ent, Arguments argv)
{
	if (!ent)
	{
		return qfalse;
	}

	if (argv->size() != 2)
	{
		if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
		{
			SetTeam(ent, "spectator", qfalse, static_cast<weapon_t>(-1), static_cast<weapon_t>(-1), qfalse);
		}

		return qtrue;
	}

	std::string error;
	//ETJump: match only players that are in game, filter out spectators
	gentity_t   *target = PlayerGentityFromString(argv->at(1), error, TEAM_SPECTATOR);

	if (!target)
	{
		ChatPrintTo(ent, "^3spectate: ^7" + error);
		return false;
	}

	if (target->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		ChatPrintTo(ent, "^3!spectate:^7 you can't spectate a spectator.");
		return qfalse;
	}

	if (!G_AllowFollow(ent, target))
	{
		ChatPrintTo(ent, va("^3!spectate: %s ^7is locked from spectators.", target->client->pers.netname));
		return qfalse;
	}

	if (ent->client->sess.sessionTeam != TEAM_SPECTATOR)
	{
		SetTeam(ent, "spectator", qfalse,
		        static_cast<weapon_t>(-1), static_cast<weapon_t>(-1), qfalse);
	}

	ent->client->sess.spectatorState  = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = target->client->ps.clientNum;
	return qtrue;
}

bool createToken(gentity_t *ent, Arguments argv)
{
	if (ent)
	{
		if (argv->size() != 3)
		{
			ChatPrintTo(ent, "^3usage: ^7!tokens create <easy (e)|medium (m)|hard (h)>");
			return false;
		}
	}
	else
	{
		if (argv->size() != 6)
		{
			ChatPrintTo(ent, "^3usage: ^7!tokens create <easy (e)|medium (m)|hard (h)> <x> <y> <z>");
			return false;
		}
	}

	std::array<float, 3> coordinates;
	if (argv->size() < 6)
	{
		VectorCopy(ent->r.currentOrigin, coordinates);
	}
	else
	{
		try
		{
			coordinates[0] = std::stof((*argv)[3]);
			coordinates[1] = std::stof((*argv)[4]);
			coordinates[2] = std::stof((*argv)[5]);
		}
		catch (const std::invalid_argument&)
		{
			ChatPrintTo(ent, "^3tokens: ^7coordinates are not numbers.");
			return false;
		}
		catch (const std::out_of_range&)
		{
			ChatPrintTo(ent, "^3tokens: ^7coordinates are out of range.");
			return false;
		}
		// Artificial limit, I don't think anyone needs it to be higher
		if (abs(coordinates[0]) > 100000)
		{
			ChatPrintTo(ent, "^3tokens: ^7x coordinate is out of range.");
			return false;
		}
		if (abs(coordinates[1]) > 100000)
		{
			ChatPrintTo(ent, "^3tokens: ^7y coordinate is out of range.");
			return false;
		}
		if (abs(coordinates[2]) > 100000)
		{
			ChatPrintTo(ent, "^3tokens: ^7z coordinate is out of range.");
			return false;
		}
	}

	Tokens::Difficulty difficulty;
	if ((*argv)[2] == "easy" || ((*argv)[2]) == "e")
	{
		difficulty = Tokens::Easy;
	}
	else if ((*argv)[2] == "medium" || ((*argv)[2]) == "m")
	{
		difficulty = Tokens::Medium;
	}
	else if ((*argv)[2] == "hard" || ((*argv)[2]) == "h")
	{
		difficulty = Tokens::Hard;
	}
	else
	{
		ChatPrintTo(ent, "^3tokens: ^7difficulty must be either easy (e), medium (m) or hard (h)");
		return false;
	}

	ChatPrintTo(ent, ETJump::stringFormat("Creating a token at (%f, %f, %f) for difficulty (%d)", coordinates[0], coordinates[1], coordinates[2], difficulty));
	auto result = game.tokens->createToken(difficulty, coordinates);
	if (!result.first)
	{
		ChatPrintTo(ent, "^3error: ^7" + result.second);
		return false;
	}
	return true;
}

bool moveToken(gentity_t *ent)
{
	if (!ent)
	{
		ChatPrintTo(ent, "^3usage: ^7!tokens move can only be used by players.");
		return false;
	}
	std::array<float, 3> coordinates;
	VectorCopy(ent->r.currentOrigin, coordinates);

	auto result = game.tokens->moveNearestToken(coordinates);
	if (!result.first)
	{
		ChatPrintTo(ent, "^3error: ^7" + result.second);
		return false;
	}

	ChatPrintTo(ent, "^3tokens: ^7" + result.second);

	return true;
}

bool deleteToken(gentity_t *ent, Arguments argv)
{
	if (!ent)
	{
		if (argv->size() != 4)
		{
			ChatPrintTo(ent, "^3usage: ^7!tokens <delete> <easy (e)|medium (m)|hard (h)> <1-6>");
			return false;
		}
	}

	if (argv->size() == 2)
	{
		std::array<float, 3> coordinates;
		VectorCopy(ent->r.currentOrigin, coordinates);
		auto result = game.tokens->deleteNearestToken(coordinates);
		if (!result.first)
		{
			ChatPrintTo(ent, "^3error: ^7" + result.second);
			return false;
		}

		ChatPrintTo(ent, "^3tokens: ^7" + result.second);

		return true;
	}

	if (argv->size() == 4)
	{
		Tokens::Difficulty difficulty;
		if ((*argv)[2] == "easy" || ((*argv)[2]) == "e")
		{
			difficulty = Tokens::Easy;
		}
		else if ((*argv)[2] == "medium" || ((*argv)[2]) == "m")
		{
			difficulty = Tokens::Medium;
		}
		else if ((*argv)[2] == "hard" || ((*argv)[2]) == "h")
		{
			difficulty = Tokens::Hard;
		}
		else
		{
			ChatPrintTo(ent, "^3tokens: ^7difficulty must be either easy (e), medium (m) or hard (h)");
			return false;
		}

		auto num = 1;
		try
		{
			num = std::stoi((*argv)[3]);
		}
		catch (const std::invalid_argument&)
		{
			ChatPrintTo(ent, "^3tokens: ^7" + (*argv)[3] + " is not a number.");
			return false;
		}
		catch (const std::out_of_range&)
		{
			ChatPrintTo(ent, "^3tokens: ^7" + (*argv)[3] + " is out of range (too large).");
			return false;
		}

		if (num < 1 || num > 6)
		{
			ChatPrintTo(ent, "^3tokens: ^7number should be between 1 and 6.");
			return false;
		}

		ChatPrintTo(ent, va("^3tokens: ^7deleting token %s #%d", (*argv)[2].c_str(), num));
		auto result = game.tokens->deleteToken(difficulty, num - 1);

		if (!result.first)
		{
			ChatPrintTo(ent, "^3error: ^7" + result.second);
			return false;
		}

		ChatPrintTo(ent, "^3tokens: ^7" + result.second);

		return true;
	}
	return true;
}

bool Tokens(gentity_t *ent, Arguments argv)
{
	if (!g_tokensMode.integer)
	{
		ChatPrintTo(ent, "^3tokens: ^7tokens mode is disabled. Set g_tokensMode \"1\" and restart map to enable tokens mode.");
		return false;
	}

	if (ent)
	{
		if (argv->size() < 2)
		{
			ChatPrintTo(ent, "^3usage: ^7check console for more information");
			Utilities::toConsole(ent,
			                     "^7!tokens create <easy (e)|medium (m)|hard (h)> ^9| Creates a new token\n"
			                     "^7!tokens move ^9| Moves nearest token to your location\n"
			                     "^7!tokens delete ^9| Deletes nearest token to your location\n"
			                     "^7!tokens delete <easy (e)|medium (m)|hard (h)> <1-6> ^9| Deletes specified token\n"
			                     );
			return false;
		}
	}
	else
	{
		if (argv->size() < 4)
		{
			Utilities::toConsole(ent,
			                     "^3usage: \n^7!tokens <easy (e)|medium (m)|hard (h)> <difficulty> <x> <y> <z>\n"
			                     "!tokens <delete> <easy (e)|medium (m)|hard (h)> <1-6>\n"
			                     );
			return false;
		}
	}

	if ((*argv)[1] == "create")
	{
		return createToken(ent, argv);
	}

	if ((*argv)[1] == "move")
	{
		return moveToken(ent);
	}

	if ((*argv)[1] == "delete")
	{
		return deleteToken(ent, argv);
	}

	return true;
}

bool Unban(gentity_t *ent, Arguments argv)
{

	if (argv->size() == 1)
	{
		PrintManual(ent, "unban");
		return false;
	}

	int id;
	if (!ToInt(argv->at(1), id))
	{
		ChatPrintTo(ent, "^3unban: ^7id is not a number.");
		return false;
	}

	if (!ETJump::database->Unban(ent, id))
	{
		ChatPrintTo(ent, "^3unban: ^7" + ETJump::database->GetMessage());
		return false;
	}

	ChatPrintTo(ent, "^3unban: ^7removed ban with id " + argv->at(1));
	return true;
}

bool Unmute(gentity_t *ent, Arguments argv)
{
	if (argv->size() != 2)
	{
		PrintManual(ent, "unmute");
		return false;
	}

	std::string error;
	gentity_t   *target = PlayerGentityFromString(argv->at(1), error);
	if (!target)
	{
		ChatPrintTo(ent, "^3unmute: ^7" + error);
		return false;
	}


	if (!target->client->sess.muted)
	{
		ChatPrintTo(ent, "^3unmute: ^7target is not muted.");
		return false;
	}

	target->client->sess.muted = qfalse;

	char *ip                       = NULL;
	char userinfo[MAX_INFO_STRING] = "\0";
	trap_GetUserinfo(ClientNum(target), userinfo, sizeof(userinfo));
	ip = Info_ValueForKey(userinfo, "ip");

	G_RemoveIPMute(ip);

	CPTo(target, "^5You've been unmuted.");
	ChatPrintAll(target->client->pers.netname + std::string(" ^7has been unmuted."));

	return true;
}

bool UserInfo(gentity_t *ent, Arguments argv)
{
	if (argv->size() == 1)
	{
		PrintManual(ent, "userinfo");
		return false;
	}

	int id;
	if (!ToInt(argv->at(1), id))
	{
		ChatPrintTo(ent, "^3userinfo: ^7id was not a number.");
		return false;
	}

	ETJump::database->UserInfo(ent, id);

	return true;
}

bool MoverScale(gentity_t *ent, Arguments argv)
{
	auto moverScaleValue = g_moverScale.value;
	if (argv->size() > 1)
	{
		if (!ToFloat(argv->at(1), moverScaleValue))
		{
			moverScaleValue = 1.0f;
		}
		// scale range 0.1 - 5.0
		if (moverScaleValue > 5.0f)
		{
			moverScaleValue = 5.0f;
		}
		else if (moverScaleValue < 0.1f)
		{
			moverScaleValue = 0.1f;
		}
		trap_Cvar_Set("g_moverScale", va("%f", moverScaleValue));
	}
	ChatPrintTo(ent, "^3Mover scale is set to: ^7" + std::to_string(moverScaleValue));

	return true;
}


}

Commands::Commands()
{
	//using AdminCommands::AdminCommand;
	//adminCommands_["addlevel"] = AdminCommand(AdminCommands::AddLevel, 'a');


	adminCommands_["addlevel"]    = AdminCommandPair(AdminCommands::AddLevel, CommandFlags::EDIT);
	adminCommands_["admintest"]   = AdminCommandPair(AdminCommands::Admintest, CommandFlags::BASIC);
	adminCommands_["8ball"]       = AdminCommandPair(AdminCommands::Ball8, CommandFlags::BASIC);
	adminCommands_["ban"]         = AdminCommandPair(AdminCommands::Ban, CommandFlags::BAN);
	adminCommands_["cancelvote"]  = AdminCommandPair(AdminCommands::Cancelvote, CommandFlags::CANCELVOTE);
	adminCommands_["deletelevel"] = AdminCommandPair(AdminCommands::DeleteLevel, CommandFlags::EDIT);
	//adminCommands_["deleteuser"] = AdminCommandPair(AdminCommands::DeleteUser, CommandFlags::EDIT);
	adminCommands_["editcommands"]  = AdminCommandPair(AdminCommands::EditCommands, CommandFlags::EDIT);
	adminCommands_["editlevel"]     = AdminCommandPair(AdminCommands::EditLevel, CommandFlags::EDIT);
	adminCommands_["edituser"]      = AdminCommandPair(AdminCommands::EditUser, CommandFlags::EDIT);
	adminCommands_["finduser"]      = AdminCommandPair(AdminCommands::FindUser, CommandFlags::EDIT);
	adminCommands_["listusernames"] = AdminCommandPair(AdminCommands::ListUserNames, CommandFlags::EDIT);
	adminCommands_["finger"]        = AdminCommandPair(AdminCommands::Finger, CommandFlags::FINGER);
	adminCommands_["findmap"]       = AdminCommandPair(AdminCommands::FindMap, CommandFlags::BASIC);
	adminCommands_["help"]          = AdminCommandPair(AdminCommands::Help, CommandFlags::BASIC);
	adminCommands_["kick"]          = AdminCommandPair(AdminCommands::Kick, CommandFlags::KICK);
	adminCommands_["leastplayed"]   = AdminCommandPair(AdminCommands::LeastPlayed, CommandFlags::BASIC);
	adminCommands_["levelinfo"]     = AdminCommandPair(AdminCommands::LevelInfo, CommandFlags::EDIT);
	adminCommands_["listbans"]      = AdminCommandPair(AdminCommands::ListBans, CommandFlags::LISTBANS);
	//adminCommands_["listcmds"] = AdminCommandPair(AdminCommands::ListCommands, CommandFlags::BASIC);
	adminCommands_["listflags"]   = AdminCommandPair(AdminCommands::ListFlags, CommandFlags::EDIT);
	adminCommands_["listmaps"]    = AdminCommandPair(AdminCommands::ListMaps, CommandFlags::BASIC);
	adminCommands_["listplayers"] = AdminCommandPair(AdminCommands::ListPlayers, CommandFlags::LISTPLAYERS);
	adminCommands_["listusers"]   = AdminCommandPair(AdminCommands::ListUsers, CommandFlags::EDIT);
	adminCommands_["map"]         = AdminCommandPair(AdminCommands::Map, CommandFlags::MAP);
	adminCommands_["mapinfo"]     = AdminCommandPair(AdminCommands::MapInfo, CommandFlags::BASIC);
	adminCommands_["mostplayed"]  = AdminCommandPair(AdminCommands::MostPlayed, CommandFlags::BASIC);
	adminCommands_["mute"]        = AdminCommandPair(AdminCommands::Mute, CommandFlags::MUTE);
	adminCommands_["noclip"]      = AdminCommandPair(AdminCommands::Noclip, CommandFlags::NOCLIP);
	adminCommands_["nogoto"]      = AdminCommandPair(AdminCommands::NoGoto, CommandFlags::NOGOTO);
	adminCommands_["nosave"]      = AdminCommandPair(AdminCommands::NoSave, CommandFlags::SAVESYSTEM);
	adminCommands_["passvote"]    = AdminCommandPair(AdminCommands::Passvote, CommandFlags::PASSVOTE);
	adminCommands_["putteam"]     = AdminCommandPair(AdminCommands::Putteam, CommandFlags::PUTTEAM);
//    adminCommands_["readconfig"] = AdminCommandPair(AdminCommands::ReadConfig, CommandFlags::READCONFIG);
	adminCommands_["rmsaves"]  = AdminCommandPair(AdminCommands::RemoveSaves, CommandFlags::SAVESYSTEM);
	adminCommands_["rename"]   = AdminCommandPair(AdminCommands::Rename, CommandFlags::RENAME);
	adminCommands_["restart"]  = AdminCommandPair(AdminCommands::Restart, CommandFlags::RESTART);
	adminCommands_["setlevel"] = AdminCommandPair(AdminCommands::SetLevel, CommandFlags::SETLEVEL);
	adminCommands_["spectate"] = AdminCommandPair(AdminCommands::Spectate, CommandFlags::BASIC);
	adminCommands_["tokens"]   = AdminCommandPair(AdminCommands::Tokens, CommandFlags::TOKENS);
	adminCommands_["unban"]    = AdminCommandPair(AdminCommands::Unban, CommandFlags::BAN);
	adminCommands_["unmute"]   = AdminCommandPair(AdminCommands::Unmute, CommandFlags::MUTE);
	adminCommands_["userinfo"] = AdminCommandPair(AdminCommands::UserInfo, CommandFlags::EDIT);
	adminCommands_["moverscale"] = AdminCommandPair(AdminCommands::MoverScale, CommandFlags::MOVERSCALE);

	commands_["backup"] = ClientCommands::BackupLoad;
	commands_["save"]   = ClientCommands::Save;
	commands_["load"]   = ClientCommands::Load;
	commands_["unload"] = ClientCommands::Unload;
//    commands_["race"] = ClientCommands::Race;
	commands_["listinfo"] = ClientCommands::ListInfo;
	commands_["records"]  = ClientCommands::Records;
	commands_["times"]    = ClientCommands::Records;
	commands_["ranks"]    = ClientCommands::Records;
}

bool Commands::ClientCommand(gentity_t *ent, const std::string& commandStr)
{
	G_DPrintf("Commands::ClientCommand called for %d\n", ClientNum(ent));

	ConstCommandIterator command = commands_.find(commandStr);
	if (command == commands_.end())
	{
		return false;
	}

	command->second(ent, GetArgs());

	return true;
}

bool Commands::List(gentity_t *ent)
{
	ConstAdminCommandIterator it  = adminCommands_.begin(),
	                          end = adminCommands_.end();

	BeginBufferPrint();
	ChatPrintTo(ent, "^3help: ^7check console for more information.");
	int              i    = 1;
	std::bitset<256> perm = ETJump::session->Permissions(ent);
	for (; it != end; it++)
	{
		if (perm[it->second.second] == false)
		{
			continue;
		}

		BufferPrint(ent, va("%-20s ", it->first.c_str()));
		if (i != 0 && i % 3 == 0)
		{
			BufferPrint(ent, "\n");
		}

		i++;
	}

	// Add a newline if last row is incomplete
	if (i % 3 != 1)
	{
		BufferPrint(ent, "\n");
	}

	// Let client know if they have access to silent commands
	if (ent && ETJump::session->HasPermission(ent, '/'))
	{
		BufferPrint(ent, "\n^7Use admin commands silently with ^3/!command");
	}

	FinishBufferPrint(ent, true);
	return true;
}

bool Commands::AdminCommand(gentity_t *ent)
{
	std::string command = "",
	            arg     = SayArgv(0);
	int skip            = 0;

	if (arg == "say" || arg == "enc_say")
	{
		arg  = SayArgv(1);
		skip = 1;
	}
	else
	{
		if (ent && !ETJump::session->HasPermission(ent, '/'))
		{
			return false;
		}
	}
	Arguments argv = GetSayArgs(skip);

	if (arg.length() == 0)
	{
		return false;
	}

	if (arg[0] == '!')
	{
		if (arg.length() == 1)
		{
			return false;
		}
		command = &arg[1];
	}
	else if (ent == NULL)
	{
		command = arg;
	}
	else
	{
		return false;
	}
	
	command = ETJump::StringUtil::toLowerCase(command);

	ConstAdminCommandIterator it = adminCommands_.lower_bound(command);

	if (it == adminCommands_.end())
	{
		return false;
	}
	std::bitset<256> permissions =
	    ETJump::session->Permissions(ent);
	std::vector<ConstAdminCommandIterator> foundCommands;
	while (it != adminCommands_.end() &&
	       it->first.compare(0, command.length(), command) == 0)
	{
		if (permissions[it->second.second])
		{
			if (it->first == command)
			{
				foundCommands.clear();
				foundCommands.push_back(it);
				break;
			}
			foundCommands.push_back(it);
		}
		++it;
	}

	if (foundCommands.size() == 1)
	{
		if (ent)
		{
			char nameBuf[MAX_NETNAME];
			Q_strncpyz(nameBuf, ent->client->pers.netname, sizeof(nameBuf));
			Q_CleanStr(nameBuf);
		}

		foundCommands[0]->second.first(ent, argv);
		return true;
	}

	if (foundCommands.size() > 1)
	{
		ChatPrintTo(ent, "^3server: ^7multiple matching commands found. Check console for more information");
		BeginBufferPrint();
		for (size_t i = 0; i < foundCommands.size(); i++)
		{
			BufferPrint(ent, va("* %s\n", foundCommands.at(i)->first.c_str()));
		}
		FinishBufferPrint(ent);
	}

	return false;
}

qboolean AdminCommandCheck(gentity_t *ent)
{
	return game.commands->AdminCommand(ent) ? qtrue : qfalse;
}

void Commands::ListCommandFlags(gentity_t *ent)
{
	ChatPrintTo(ent, "^3listflags: ^7check console for more information.");

	BeginBufferPrint();

	for (ConstAdminCommandIterator it = adminCommands_.begin(), end = adminCommands_.end(); it != end; it++)
	{
		BufferPrint(ent, ETJump::stringFormat("%c %s\n", it->second.second, it->first));
	}

	// Just manually print this since it's not an actual command
	BufferPrint(ent, "/ [silent commands]\n");

	FinishBufferPrint(ent);
}

char Commands::FindCommandFlag(const std::string &command)
{
	AdminCommandsIterator it  = adminCommands_.begin();
	AdminCommandsIterator end = adminCommands_.end();
	while (it != end)
	{
		if ((*it).first == command)
		{
			return (*it).second.second;
		}
		it++;
	}
	return 0;
}
