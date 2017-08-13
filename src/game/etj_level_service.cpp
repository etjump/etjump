#include "etj_level_service.h"
#include "etj_log.h"
#include "etj_file.h"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "etj_config_serializer.h"

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
	std::vector<std::string> serializationErrors;
	std::vector<std::string> parsingErrors;
	bool anyLevelsExist = false;
	try
	{
		_levels.clear();
		File configFile(_levelsFile);
		auto config = configFile.read();

		auto serializer = ConfigSerializer(config);
		auto parseResult = serializer.deserialize();
		serializationErrors = serializer.getErrors();

		for (const auto & entry : parseResult)
		{
			if (entry.name != "level")
			{
				continue;
			}

			Level level;
			level.name = getValue(entry.values, "name");
			level.commands = getValue(entry.values, std::vector<std::string>({ "commands", "cmds" }));
			level.greeting = getValue(entry.values, "greeting");
			auto levelStr = getValue(entry.values, "level");
			if (levelStr.length() == 0)
			{
				// skip definitions without level field
				parsingErrors.push_back((boost::format("level block with name: \"%s\", commands: \"%s\", greeting: \"%s\" ignored due to an empty or non-existing level field.") % level.name % level.commands % level.greeting).str());
				continue;
			}

			try
			{
				auto lvl = std::stoi(levelStr);
				level.level = lvl;
			} catch (const std::invalid_argument&)
			{
				parsingErrors.push_back((boost::format("level block with name: \"%s\", commands: \"%s\", greeting: \"%s\" ignored due to a non-integer level: \"%s\". Levels must be numbers.") % level.name % level.commands % level.greeting % levelStr).str());
				continue;
			} catch (const std::out_of_range&)
			{
				parsingErrors.push_back((boost::format("level block with name: \"%s\", commands: \"%s\", greeting: \"%s\" ignored due to a too high level: \"%s\". Levels must be between %d and %d") % level.name % level.commands % level.greeting % levelStr % std::numeric_limits<int>::min() % std::numeric_limits<int>::max()).str());
				continue;
			}
			_levels[level.level] = level;
			anyLevelsExist = true;
		}
	}
	catch (File::FileNotFoundException)
	{
		anyLevelsExist = false;
	}

	std::string errBuf;
	if (serializationErrors.size() > 0)
	{
		errBuf = 
			"Found " + std::to_string(serializationErrors.size()) + " errors while reading " + _levelsFile + "\n" +
			boost::join(serializationErrors, "\n");
	}

	if (parsingErrors.size() > 0)
	{
		if (errBuf.size() > 0)
		{
			errBuf += "\n";
		}
		errBuf +=
			"Found " + std::to_string(parsingErrors.size()) + " errors while parsing " + _levelsFile + "\n" +
			boost::join(parsingErrors, "\n");
	}

	if (errBuf.size())
	{
		_log.warnLn(errBuf);
	}

	if (!anyLevelsExist)
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
	return OperationResult(true, "");
}

ETJump::OperationResult ETJump::LevelService::writeConfig()
{
	std::vector<ConfigEntry> entries;
	for (const auto & level : _levels)
	{
		ConfigEntry entry;
		entry.name = "level";
		entry.values["level"] = std::to_string(level.second.level);
		entry.values["name"] = level.second.name;
		entry.values["commands"] = level.second.commands;
		entry.values["greeting"] = level.second.greeting;
		entries.push_back(entry);
	}
	File f(_levelsFile, File::Mode::Write);
	try
	{
		f.write(ConfigSerializer::serialize(entries));
	} catch (const std::exception& e)
	{
		// doesn't really matter if it's a logic error or an exception, either way 
		// we need to log it
		return OperationResult(false, e.what());
	}

	return OperationResult(true, "");
}

std::string ETJump::LevelService::getValue(const std::map<std::string, std::string>& dictionary, const std::string& key) const
{
	auto match = dictionary.find(key);
	if (match == end(dictionary))
	{
		return std::string();
	}
	return match->second;
}

std::string ETJump::LevelService::getValue(const std::map<std::string, std::string>& dictionary, const std::vector<std::string>& keys) const
{
	for (const auto & key : keys)
	{
		auto match = dictionary.find(key);
		if (match == end(dictionary))
		{
			continue;
		}
		return match->second;
	}
	return std::string();
}
