#include "etj_user_repository.h"
#include <SQLiteCpp/Database.h>


ETJump::UserRepository::UserRepository::UserRepository(const std::string& databaseFile, int timeout)
	: _databaseFile(databaseFile)
{
	SQLite::Database db(databaseFile, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE, timeout);

	db.exec(
		"CREATE TABLE IF NOT EXISTS users ( "
		"  id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"  guid TEXT NOT NULL, "
		"  level INTEGER NOT NULL DEFAULT (0), "
		"  created INTEGER NOT NULL DEFAULT(STRFTIME('%s', 'NOW')), "
		"  modified INTEGER, "
		"  lastSeen INTEGER NOT NULL DEFAULT(STRFTIME('%s', 'NOW')), "
		"  name TEXT NOT NULL, "
		"  title TEXT, "
		"  commands TEXT, "
		"  greeting TEXT "
		"); "
	);

	db.exec(
		"CREATE TABLE IF NOT EXISTS hardwareIds ( "
		"  userId INTEGER NOT NULL, "
		"  hardwareId TEXT NOT NULL, "
		"  created INTEGER NOT NULL DEFAULT(STRFTIME('%s', 'NOW')), "
		"  FOREIGN KEY (userId) REFERENCES users(id) "
		"); "
	);

	db.exec(
		"CREATE TABLE IF NOT EXISTS aliases ( "
		"  userId INTEGER NOT NULL, "
		"  alias TEXT NOT NULL, "
		"  created INTEGER NOT NULL DEFAULT(STRFTIME('%s', 'NOW')), "
		"  FOREIGN KEY (userId) REFERENCES users(id), "
		"  UNIQUE (userId, alias) "
		"); "
	);

	db.exec(
		"CREATE TABLE IF NOT EXISTS usersLog ( "
		"  id INTEGER PRIMARY KEY AUTOINCREMENT, "
		"  userId INTEGER NOT NULL, "
		"  changes TEXT NOT NULL, "
		"  changedBy INTEGER NOT NULL, "
		"  timestamp INTEGER NOT NULL DEFAULT (STRFTIME('%s', 'NOW')), "
		"  FOREIGN KEY (userId) REFERENCES users(id), "
		"  FOREIGN KEY (changedBy) REFERENCES users(id) "
		"); "
	);
}

ETJump::UserRepository::UserRepository::~UserRepository()
{
}

void ETJump::UserRepository::UserRepository::insert(const std::string& guid, const std::string& name, const std::string& hardwareId)
{
	SQLite::Database db(_databaseFile, SQLite::OPEN_READWRITE);

	SQLite::Statement insertQuery(db,
		"INSERT INTO users (guid, name) VALUES ( "
		"  ?, "
		"  ? "
		"); "
	);

	insertQuery.bind(1, guid);
	insertQuery.bind(2, name);
	insertQuery.exec();
}

ETJump::User ETJump::UserRepository::get(const std::string& guid)
{
	SQLite::Database db(_databaseFile, SQLite::OPEN_READWRITE);

	SQLite::Statement getUserQuery(db,
		"SELECT "
		"  id, "
		"  level, "
		"  created, "
		"  modified, "
		"  lastSeen, "
		"  name, "
		"  title, "
		"  commands, "
		"  greeting "
		"FROM users "
		"WHERE guid=?; "
	);

	getUserQuery.bind(1, guid);
	if (getUserQuery.executeStep())
	{
		unsigned id = getUserQuery.getColumn(0);
		int level = getUserQuery.getColumn(1);
		std::time_t created = getUserQuery.getColumn(2);
		std::time_t modified = getUserQuery.getColumn(3);
		std::time_t lastSeen = getUserQuery.getColumn(4);
		const char *name = getUserQuery.getColumn(5);
		const char *title = getUserQuery.getColumn(6);
		const char *commands = getUserQuery.getColumn(7);
		const char *greeting = getUserQuery.getColumn(8);

		User user;
		user.id = id;
		user.level = level;
		user.created = created;
		user.modified = modified;
		user.lastSeen = lastSeen;
		user.name = name != nullptr ? name : "";
		user.title = title != nullptr ? title : "";
		user.commands = commands != nullptr ? commands : "";
		user.greeting = greeting != nullptr ? greeting : "";
		return user;
	}
	
	User nonExistingUser;
	nonExistingUser.id = -1;
	return nonExistingUser;
}
