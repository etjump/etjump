#pragma once
#include <string>
#include <vector>

namespace ETJump
{
	enum class LevelFields
	{
		Name,
		Commands,
		Greeting
	};

	struct LevelChanges
	{
		std::string name;
		std::string commands;
		std::string greeting;
	};

	struct Level
	{
		Level(): level(0) {}
		Level(int level, const std::string& name, const std::string& commands, const std::string& greeting)
			: level(level), name(name), commands(commands), greeting(greeting)
		{
			
		}
		int level;
		std::string name;
		std::string commands;
		std::string greeting;
	};

	inline std::vector<Level> createDefaultLevels()
	{
		return std::vector<Level>
		{
			Level(0, "Visitor", "a", "Welcome Visitor [n]^7! Your last visit was on [t]!"),
			Level(1, "Friend", "a", "Welcome Friend [n]^7! Your last visit was [d] ago!"),
			Level(2, "Moderator", "*-As", "Welcome Moderator [n]^7!"),
			Level(3, "Administrator", "*", "Welcome Administrator [n]^7!"),
		};
	}
}
