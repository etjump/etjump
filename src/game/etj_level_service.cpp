#include "etj_level_service.h"
#include "etj_log.h"
#include "etj_file.h"
#include <boost/algorithm/string.hpp>
#include "etj_config_parser.h"

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
	try
	{
		File configFile(_levelsFile);
		auto config = configFile.read();

		auto parseResult = ConfigParser(config).getEntries();
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
	}
}

ETJump::OperationResult ETJump::LevelService::writeConfig()
{
	return OperationResult(true, "");
}
