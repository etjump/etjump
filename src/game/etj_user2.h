#pragma once
#include <vector>

namespace ETJump
{
	class User
	{
	public:
		static const long long INVALID_ID = -1;
	
		User(const long long id, const int level, const int lastSeen, const std::string& name, const std::string& guid, const std::string& title, const std::string& commands, const std::string& greeting, const std::vector<std::string>& ips, const std::vector<std::string>& hardwareIds)
			: _id(id),
			_level(level),
			_lastSeen(lastSeen),
			_name(name),
			_guid(guid),
			_title(title),
			_commands(commands),
			_greeting(greeting),
			_ips(ips),
			_hardwareIds(hardwareIds)
		{
		}

		User(): _id(INVALID_ID), _level(0), _lastSeen(0)
		{
		}

		long long id() const
		{
			return _id;
		}

		int level() const
		{
			return _level;
		}

		int lastSeen() const
		{
			return _lastSeen;
		}

		std::string name() const
		{
			return _name;
		}

		std::string guid() const
		{
			return _guid;
		}

		std::string title() const
		{
			return _title;
		}

		std::string commands() const
		{
			return _commands;
		}

		std::string greeting() const
		{
			return _greeting;
		}

		std::vector<std::string> ips() const
		{
			return _ips;
		}

		std::vector<std::string> hardwareIds() const
		{
			return _hardwareIds;
		}

	private:
		long long _id;
		int _level;
		int _lastSeen;
		std::string _name;
		std::string _guid;
		std::string _title;
		std::string _commands;
		std::string _greeting;
		std::vector<std::string> _ips;
		std::vector<std::string> _hardwareIds;
	};
}
