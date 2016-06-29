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
		User();
		~User();

		friend std::ostream& operator<<(std::ostream& os, const User& obj)
		{
			return os
				<< "_id: " << obj._id
				<< " _guid: " << obj._guid
				<< " _level: " << obj._level
				<< " _lastSeen: " << obj._lastSeen
				<< " _name: " << obj._name
				<< " _title: " << obj._title
				<< " _commands: " << obj._commands
				<< " _greeting: " << obj._greeting;
		}

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



