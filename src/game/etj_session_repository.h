#pragma once
#include <vector>
#include "etj_log.h"
#include <map>

namespace ETJump
{
	class SessionRepository
	{
	public:
		struct Session
		{
			int clientNum;
			std::map<std::string, std::string> values;
		};

		explicit SessionRepository(const std::string& database, int timeout, const std::string& serverId);
		~SessionRepository();

		void createTables();
		std::map<int, Session> loadSessions();
		void writeSessions(std::vector<Session> sessions);
		void clearSessions();
		void clearSession(int clientNum);

	private:
		std::string _database;
		int _timeout;
		Log _log;
		std::string _serverId;
	};
}
