#pragma once
#include <string>
#include <vector>

namespace ETJump
{
	// A persistent user that stays across multiple sessions / server restarts etc.
	class User
	{
	public:
		User(int64_t id, const std::string& guid);
		~User();
	private:
		int64_t _id;
		std::string _guid;
		int _level;
		int64_t _lastSeen;
		std::string _name;
		std::string _title;
		std::string _commands;
		std::string _greeting;
		std::vector<std::string> hardwareIds;
	};
}



