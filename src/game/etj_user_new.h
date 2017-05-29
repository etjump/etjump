#pragma once
#include <string>
#include <ctime>
#include <vector>

namespace ETJump
{
	struct User
	{
		long id;
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
	};
}
