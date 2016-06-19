#include <boost/algorithm/string.hpp>
#include "etj_levels.h"
#include "etj_local.h"
#include "utilities.hpp"

Levels::Level::Level(int level, std::string const& name, std::string const& greeting, std::string const& commands)
{
	this->level    = level;
	this->name     = name;
	this->greeting = greeting;
	this->commands = commands;
}

Levels::Levels()
{
	dummyLevel_ = std::shared_ptr<Level>(new Level(0, "", "", ""));
}

Levels::~Levels()
{
}

bool Levels::Add(int level, std::string const name, std::string const commands, std::string const greeting)
{
	auto it = FindConst(level);
	if (it != levels_.end())
	{
		errorMessage = "level exists";
		return false;
	}

	std::shared_ptr<Level> levelPtr(new Level(level, name, greeting, commands));
	levels_.push_back(levelPtr);

	if (!WriteToConfig())
	{
		return false;
	}
	return true;
}

bool Levels::Delete(int level)
{
	auto it = Find(level);
	if (it != levels_.end())
	{
		levels_.erase(it);
		WriteToConfig();
		return true;
	}

	return false;
}

bool Levels::Edit(int level, std::string const& name, std::string const& commands, std::string const& greeting, int updated)
{
	auto it = FindConst(level);
	if (it == levels_.end())
	{
		errorMessage = "level does not exist";
		return false;
	}

	const auto CMDS_UPDATED     = 1;
	const auto GREETING_UPDATED = 2;
	const auto NAME_UPDATED     = 4;

	if (updated & CMDS_UPDATED)
	{
		it->get()->commands = commands;
	}

	if (updated & GREETING_UPDATED)
	{
		it->get()->greeting = greeting;

	}

	if (updated & NAME_UPDATED)
	{
		it->get()->name = name;

	}

	if (!WriteToConfig())
	{
		return false;
	}
	return true;
}

bool Levels::CreateDefaultLevels()
{
	levels_.clear();

	auto tempLevel = std::make_shared<Level>(0, "Visitor",
	                                         "Welcome Visitor [n]^7! Your last visit was on [t]!", "a");
	levels_.push_back(tempLevel);

	tempLevel = std::make_shared<Level>(1, "Friend",
	                                    "Welcome Friend [n]^7! Your last visit was [d] ago!", "a");
	levels_.push_back(tempLevel);

	tempLevel = std::make_shared<Level>(2, "Moderator",
	                                    "Welcome Moderator [n]^7!§", "*-As");
	levels_.push_back(tempLevel);

	tempLevel = std::make_shared<Level>(3, "Administrator",
	                                    "Welcome Administrator [n]^7!", "*");
	levels_.push_back(tempLevel);

	if (!WriteToConfig())
	{
		return false;
	}
	return true;
}

void WriteString(const char *toWrite, fileHandle_t& f)
{
	trap_FS_Write(toWrite, strlen(toWrite), f);
	trap_FS_Write("\n", 1, f);
}

void WriteInt(int toWrite, fileHandle_t& f)
{
	const int BUFSIZE = 32;
	char      buf[BUFSIZE];
	Com_sprintf(buf, sizeof(buf), "%d", toWrite);
	trap_FS_Write(buf, strlen(buf), f);
	trap_FS_Write("\n", 1, f);
}

bool Levels::SortByLevel(const std::shared_ptr<Level>& lhs, const std::shared_ptr<Level>& rhs)
{
	return lhs->level < rhs->level;
}

bool Levels::WriteToConfig()
{
	auto f = 0;
	trap_FS_FOpenFile(g_levelConfig.string, &f, FS_WRITE);

	sort(levels_.begin(), levels_.end(), SortByLevel);
	for (ConstIter it = levels_.begin(); it != levels_.end(); ++it)
	{
		trap_FS_Write("[level]\n", 8, f);
		trap_FS_Write("level = ", 8, f);
		WriteInt(it->get()->level, f);

		trap_FS_Write("name = ", 7, f);
		WriteString(it->get()->name.c_str(), f);

		trap_FS_Write("cmds = ", 7, f);
		WriteString(it->get()->commands.c_str(), f);

		trap_FS_Write("greeting = ", 11, f);
		WriteString(it->get()->greeting.c_str(), f);
		trap_FS_Write("\n", 1, f);
	}

	trap_FS_FCloseFile(f);
	return true;
}

Levels::Level const *Levels::GetLevel(int level)
{
	auto it  = levels_.begin();
	auto end = levels_.end();
	for (; it != end; ++it)
	{
		if (it->get()->level == level)
		{
			return it->get();
		}
	}


	return dummyLevel_.get();
}

void Levels::PrintLevelInfo(gentity_t *ent)
{
	auto it  = levels_.begin();
	auto end = levels_.end();

	ChatPrintTo(ent, "^3levelinfo: ^7check console for more information.");
	BeginBufferPrint();
	BufferPrint(ent, "Levels: ");
	for (; it != end; ++it)
	{
		if (it + 1 == end)
		{
			BufferPrint(ent, va("%d", (*it)->level));
			FinishBufferPrint(ent, true);
			return;
		}
		BufferPrint(ent, va("%d, ", it->get()->level));
	}
}

void Levels::PrintLevelInfo(gentity_t *ent, int level)
{
	auto it  = levels_.begin();
	auto end = levels_.end();

	for (; it != end; ++it)
	{
		if (it->get()->level == level)
		{
			ChatPrintTo(ent, "^3levelinfo: ^7check console for more information.");
			ConsolePrintTo(ent, va("^5Level: ^7%d\n^5Name: ^7%s\n^5Commands: ^7%s\n^5Greeting: ^7%s",
			                       level, it->get()->name.c_str(), it->get()->commands.c_str(), it->get()->greeting.c_str()));
			return;
		}
	}
	ChatPrintTo(ent, "^3levelinfo: ^7undefined level: " + ToString(level));
}

bool Levels::LevelExists(int level) const
{
	auto it = levels_.begin();

	for (; it != levels_.end(); ++it)
	{
		if (it->get()->level == level)
		{
			return true;
		}
	}
	return false;
}

void Levels::PrintLevels()
{
	auto it = levels_.begin();

	std::string level;

	for (; it != levels_.end(); ++it)
	{
		level += it->get()->level + "\n" +
		         it->get()->name + "\n" +
		         it->get()->commands + "\n" +
		         it->get()->greeting + "\n";

		G_LogPrintf(level.c_str());

		level = "";
	}
}

std::string Levels::ErrorMessage() const
{
	return errorMessage;
}

Levels::ConstIter Levels::FindConst(int level)
{
	ConstIter it = levels_.begin();
	for (; it != levels_.end(); ++it)
	{
		if (it->get()->level == level)
		{
			return it;
		}
	}
	return it;
}

Levels::Iter Levels::Find(int level)
{
	auto it = levels_.begin();
	for (; it != levels_.end(); ++it)
	{
		if (it->get()->level == level)
		{
			return it;
		}
	}
	return it;
}

void ReadInt(char **configFile, int& level)
{
	auto token = COM_ParseExt(configFile, qfalse);

	if (!Q_stricmp(token, "="))
	{
		token = COM_ParseExt(configFile, qfalse);
	}
	else
	{
		G_LogPrintf("readconfig: missing = before \"%s\" on line %d.",
		            token, COM_GetCurrentParseLine());
	}
	level = atoi(token);
}

void ReadString(char **configFile, std::string& str)
{
	auto token = COM_ParseExt(configFile, qfalse);

	if (!Q_stricmp(token, "="))
	{
		token = COM_ParseExt(configFile, qfalse);
	}
	else
	{
		G_LogPrintf("readconfig: missing = before \"%s\" on line %d.",
		            token, COM_GetCurrentParseLine());
	}
	str.clear();
	while (token[0])
	{
		str += token;
		str.push_back(' ');
		token = COM_ParseExt(configFile, qfalse);
	}

	boost::trim_right(str);
}

bool Levels::ReadFromConfig()
{
	auto f   = 0;
	auto len = trap_FS_FOpenFile(g_levelConfig.string, &f, FS_READ);
	if (len < 0)
	{
		CreateDefaultLevels();
		return true;
	}
	std::unique_ptr<char[]> file;
	try
	{
		file = std::unique_ptr<char[]>(new char[len + 1]);
	}
	catch (...)
	{
		G_Error("Failed to allocate memory to parse level config.");
		trap_FS_FCloseFile(f);
		return false;
	}

	trap_FS_Read(file.get(), len, f);
	file[len] = 0;
	trap_FS_FCloseFile(f);

	char                   *token    = nullptr;
	auto                   levelOpen = false;
	std::shared_ptr<Level> tempLevel;

	levels_.clear();

	auto file2 = file.get();

	token = COM_Parse(&file2);

	while (*token)
	{
		if (!Q_stricmp(token, "[level]"))
		{
			if (levelOpen)
			{
				levels_.push_back(tempLevel);
			}
			levelOpen = false;
		}
		else if (!Q_stricmp(token, "cmds"))
		{
			ReadString(&file2, tempLevel->commands);
		}
		else if (!Q_stricmp(token, "level"))
		{
			ReadInt(&file2, tempLevel->level);
		}
		else if (!Q_stricmp(token, "greeting"))
		{
			ReadString(&file2, tempLevel->greeting);
		}
		else if (!Q_stricmp(token, "name"))
		{
			ReadString(&file2, tempLevel->name);
		}
		else
		{
			G_LogPrintf("readconfig: parse error near %s on line %d",
			            token, COM_GetCurrentParseLine());
		}

		if (!Q_stricmp(token, "[level]"))
		{
			try
			{
				tempLevel = std::make_shared<Level>(0, "", "", "");
			}
			catch (...)
			{
				G_Error("Failed to allocate memory for a level.");
				return false;
			}

			levelOpen = true;
		}

		token = COM_Parse(&file2);
	}

	if (levelOpen)
	{
		levels_.push_back(tempLevel);
	}
	return true;
}
