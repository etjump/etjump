#include "etj_user_repository.h"
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Transaction.h>


ETJump::UserRepository::UserRepository::UserRepository(const std::string& databaseFile, int timeout)
	: _databaseFile(databaseFile), _timeout(timeout)
{
}

ETJump::UserRepository::UserRepository::~UserRepository()
{
	
}

void ETJump::UserRepository::createTables()
{
	SQLite::Database db(_databaseFile, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE, _timeout);

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
		"CREATE TABLE IF NOT EXISTS ipAddresses ( "
		"  userId INTEGER NOT NULL, "
		"  ipAddress TEXT NOT NULL, "
		"  created INTEGER NOT NULL DEFAULT(STRFTIME('%s', 'NOW')), "
		"  FOREIGN KEY (userId) REFERENCES users(id) "
		"); "
	);

	db.exec(
		"CREATE TABLE IF NOT EXISTS aliases ( "
		"  userId INTEGER NOT NULL, "
		"  alias TEXT NOT NULL, "
		"  cleanAlias TEXT NOT NULL, "
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

void ETJump::UserRepository::UserRepository::insert(const std::string& guid, const std::string& name, const std::string& ipAddress, const std::string& hardwareId)
{
	SQLite::Database db(_databaseFile, SQLite::OPEN_READWRITE);

	SQLite::Transaction transaction(db);

	SQLite::Statement insertUser(db,
		"INSERT INTO users (guid, name) VALUES ( "
		"  ?, "
		"  ? "
		"); "
	);

	insertUser.bind(1, guid);
	insertUser.bind(2, name);
	insertUser.exec();

	long id = db.getLastInsertRowid();

	SQLite::Statement insertIpAddress(db,
		"INSERT INTO ipAddresses (userId, ipAddress) VALUES ( "
		"  ?, "
		"  ? "
		"); "
	);

	insertIpAddress.bind(1, id);
	insertIpAddress.bind(2, ipAddress);
	insertIpAddress.exec();

	SQLite::Statement insertAlias(db,
		"INSERT INTO aliases (userId, alias, cleanAlias) VALUES ( "
		"    ?, "
		"    ?, "
		"    ? "
		"); "
	);

	// TODO: remove color codes
	auto cleanName = name;

	insertAlias.bind(1, id);
	insertAlias.bind(2, name);
	insertAlias.bind(3, cleanName);
	insertAlias.exec();

	SQLite::Statement insertHardwareId(db,
		"INSERT INTO hardwareIds (userId, hardwareId) VALUES ( "
		"    ?, "
		"    ? "
		"); "
	);

	insertHardwareId.bind(1, id);
	insertHardwareId.bind(2, hardwareId);
	insertHardwareId.exec();

	transaction.commit();
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

		SQLite::Statement getUserHardwareIds(db,
			"SELECT "
			"  hardwareId "
			"FROM hardwareIds "
			"WHERE userId=?; "
		);

		getUserHardwareIds.bind(1, id);

		while (getUserHardwareIds.executeStep())
		{
			const char *hardwareId = getUserHardwareIds.getColumn(0);
			user.hardwareIds.push_back(hardwareId);
		}

		SQLite::Statement getUserAliases(db,
			"SELECT "
			"  alias "
			"FROM aliases "
			"WHERE userId=?; "
		);

		getUserAliases.bind(1, id);

		while (getUserAliases.executeStep())
		{
			const char *alias = getUserAliases.getColumn(0);
			user.aliases.push_back(alias);
		}

		return user;
	}
	
	User nonExistingUser;
	nonExistingUser.id = -1;
	return nonExistingUser;
}

void ETJump::UserRepository::addHardwareId(int id, const std::string& hardwareId)
{
}

void ETJump::UserRepository::addAlias(int id, const std::string& alias)
{
}
