#include "etj_session_repository.h"
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Transaction.h>
#include "etj_log.h"

ETJump::SessionRepository::SessionRepository(const std::string& database, int timeout)
	: _database(database), _timeout(timeout), _log(Log("SessionRepository"))
{
}

ETJump::SessionRepository::~SessionRepository()
{
}

void ETJump::SessionRepository::createTables()
{
	SQLite::Database db(_database, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE, _timeout);

	db.exec(
		"CREATE TABLE IF NOT EXISTS sessions ( "
		"  clientNum INTEGER NOT NULL, "
		"  key TEXT NOT NULL, "
		"  value TEXT NOT NULL "
		"); "
	);
}

std::map<int, ETJump::SessionRepository::Session> ETJump::SessionRepository::loadSessions()
{
	SQLite::Database db(_database, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE, _timeout);

	SQLite::Statement selectSessionsStmt(db,
		"SELECT "
		"  clientNum, "
		"  key, "
		"  value "
		"FROM sessions; "
	);

	std::map<int, Session> sessions;
	while (selectSessionsStmt.executeStep())
	{
		auto clientNum = selectSessionsStmt.getColumn(0);
		std::string key = selectSessionsStmt.getColumn(1);
		std::string value = selectSessionsStmt.getColumn(2);

		auto session = sessions.find(clientNum);
		if (session == end(sessions))
		{
			auto newSession = Session();
			newSession.clientNum = clientNum;
			newSession.values[key] = value;
			sessions[clientNum] = newSession;
		} else
		{
			session->second.values[key] = value;
		}
	}

	_log.infoLn("Loaded " + std::to_string(sessions.size()) + " sessions");

	return sessions;
}

void ETJump::SessionRepository::writeSessions(std::vector<Session> sessions)
{
	SQLite::Database db(_database, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE, _timeout);

	SQLite::Statement insertSessionStmt(db,
		"INSERT INTO sessions (clientNum, key, value) VALUES ( "
		"  ?, "
		"  ?, "
		"  ? "
		"); "
	);

	SQLite::Transaction transaction(db);
	
	for (const auto & s : sessions)
	{
		for (const auto & keyValue : s.values)
		{
			insertSessionStmt.bind(1, s.clientNum);
			insertSessionStmt.bind(2, keyValue.first);
			insertSessionStmt.bind(3, keyValue.second);
			insertSessionStmt.exec();

			insertSessionStmt.reset();
			insertSessionStmt.clearBindings();
		}
	}

	transaction.commit();
	_log.infoLn("Saved " + std::to_string(sessions.size()) + " sessions");
}

void ETJump::SessionRepository::clearSessions()
{
	SQLite::Database db(_database, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE, _timeout);

	db.exec("DELETE FROM sessions;");

	_log.infoLn("Cleared session data.");
}
