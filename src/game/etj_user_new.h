#pragma once
#include <string>
#include <ctime>
#include <vector>

namespace ETJump
{
	enum class UserFields
	{
		Level = 1 << 0,
		LastSeen = 1 << 1,
		Name = 1 << 2,
		Title = 1 << 3,
		Commands = 1 << 4,
		Greeting = 1 << 5
	};

	struct MutableUserFields
	{
		int level;
		std::time_t lastSeen;
		std::string name;
		std::string title;
		std::string commands;
		std::string greeting;
	};

	struct User
	{
		int64_t id;
		std::string guid;
		int level;
		std::time_t lastSeen;
		std::time_t created;
		std::time_t modified;
		std::string name;
		std::string title;
		std::string commands;
		std::string greeting;
		std::vector<std::string> hardwareIds;
		std::vector<std::string> aliases;
		std::vector<std::string> ipAddresses;
	};
}
