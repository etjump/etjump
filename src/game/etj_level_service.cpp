#include "etj_level_service.h"
#include "etj_log.h"
#include "etj_file.h"
#include <boost/algorithm/string.hpp>

ETJump::LevelService::LevelService(const std::string& levelsFile)
	:_dummyLevel(0, "", "", ""), _log(Log("LevelService")), _levelsFile(levelsFile)
{
}

ETJump::LevelService::~LevelService()
{
}

const ETJump::Level* ETJump::LevelService::get(int level)
{
	auto levelIter = _levels.find(level);
	if (levelIter == end(_levels))
	{
		return &_dummyLevel;
	}
	return &(_levels[level]);
}

ETJump::OperationResult ETJump::LevelService::add(int level, const std::string& name, const std::string& commands, const std::string& greeting)
{
	auto levelIter = _levels.find(level);
	if (levelIter != end(_levels))
	{
		return OperationResult(false, "Level " + std::to_string(level) + "(" + levelIter->second.name + ")" + " already exists.");
	}
	_levels[level] = Level(level, name, commands, greeting);
	return OperationResult(true, "Successfully added level " + std::to_string(level));
}

ETJump::OperationResult ETJump::LevelService::edit(int level, const LevelChanges& changes, int changedFields)
{
	if (changedFields == 0)
	{
		return OperationResult(false, "No changes specified.");
	}

	auto levelIter = _levels.find(level);
	if (levelIter == end(_levels))
	{
		return OperationResult(false, "Level " + std::to_string(level) + " does not exist.");
	}
	if (changedFields & static_cast<int>(LevelFields::Name))
	{
		levelIter->second.name = changes.name;
	}
	if (changedFields & static_cast<int>(LevelFields::Commands))
	{
		levelIter->second.commands = changes.commands;
	}
	if (changedFields & static_cast<int>(LevelFields::Greeting))
	{
		levelIter->second.greeting = changes.greeting;
	}

	auto writeConfigResult = writeConfig();
	if (!writeConfigResult.success)
	{
		return writeConfigResult;
	}
	return OperationResult(true, "Successfully updated level.");
}

ETJump::OperationResult ETJump::LevelService::remove(int level)
{
	auto levelIter = _levels.find(level);
	if (levelIter == end(_levels))
	{
		return OperationResult(false, "Level " + std::to_string(level) + " does not exist.");
	}
	_levels.erase(level);
	auto writeConfigResult = writeConfig();
	if (!writeConfigResult.success)
	{
		return writeConfigResult;
	}
	return OperationResult(true, "Successfully deleted level.");
}

ETJump::OperationResult ETJump::LevelService::readConfig()
{
	auto f = 0;
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

	char                   *token = nullptr;
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
		_levels.push_back(tempLevel);
	}
	return true;

	for (const auto & l : createDefaultLevels())
	{
		_levels[l.level] = l;
	}
	return OperationResult(true, "");
	/*try
	{
		File configFile(_levelsFile);
		auto config = configFile.read();

		auto parseResult = ConfigParser(config).parse();
	}
	catch (File::FileNotFoundException)
	{
		auto defaultLevels = createDefaultLevels();
		for (auto & level : defaultLevels)
		{
			_levels[level.level] = level;
		}
		auto writeConfigResult = writeConfig();
		if (!writeConfigResult.success)
		{
			return writeConfigResult;
		}
		return OperationResult(true, "Could not find levels config file " + _levelsFile + ". Creating default levels.");
	}*/
}

ETJump::OperationResult ETJump::LevelService::writeConfig()
{
	return OperationResult(true, "");
}
