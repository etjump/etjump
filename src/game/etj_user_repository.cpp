#include "etj_user_repository.h"
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Transaction.h>
#include <boost/algorithm/string/join.hpp>
#include "etj_log.h"


ETJump::UserRepository::UserRepository::UserRepository(const std::string& databaseFile, int timeout)
	: _databaseFile(databaseFile), _timeout(timeout), _log(Log("UserRepository"))
{
	_log.infoLn("Using database file " + databaseFile);
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

void ETJump::UserRepository::UserRepository::insert(const std::string& guid, const std::string& name, const std::string& ipAddress, const std::string& hardwareId) const
{
	SQLite::Database db(_databaseFile, SQLite::OPEN_READWRITE, _timeout);

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

	auto id = db.getLastInsertRowid();

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

ETJump::User ETJump::UserRepository::get(const std::string& guid) const
{
	try
	{
		SQLite::Database db(_databaseFile, SQLite::OPEN_READWRITE, _timeout);
		db.setBusyTimeout(BUSY_TIMEOUT);

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
			user.guid = guid;
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

			SQLite::Statement getUserIpAddresses(db,
				"SELECT "
				"  ipAddress "
				"FROM ipAddresses "
				"WHERE userId=?; "
			);

			getUserIpAddresses.bind(1, id);

			while (getUserIpAddresses.executeStep())
			{
				const char *ipAddress = getUserIpAddresses.getColumn(0);
				user.ipAddresses.push_back(ipAddress);
			}

			return user;
		}
	} catch (const SQLite::Exception& e)
	{
		_log.errorLn("querying for user failed: (" + std::to_string(e.getErrorCode()) + ") " + e.getErrorStr());
	}
	User nonExistingUser;
	nonExistingUser.id = NEW_USER_ID;
	return nonExistingUser;
}

void ETJump::UserRepository::addHardwareId(int64_t id, const std::string& hardwareId) const
{
	try
	{
		SQLite::Database db(_databaseFile, SQLite::OPEN_READWRITE, _timeout);

		SQLite::Statement insertHardwareId(db,
			"INSERT INTO hardwareIds (userId, hardwareId) VALUES ( "
			"  ?, "
			"  ? "
			"); "
		);
		insertHardwareId.bind(1, id);
		insertHardwareId.bind(2, hardwareId);
		insertHardwareId.exec();
	} catch (const SQLite::Exception& e)
	{
		_log.errorLn("adding a hardware id " + hardwareId + " for user " + std::to_string(id) + " failed: (" + std::to_string(e.getErrorCode()) + ") " + e.getErrorStr());
	}
}

void ETJump::UserRepository::addAlias(int64_t id, const std::string& alias) const
{
	try
	{
		SQLite::Database db(_databaseFile, SQLite::OPEN_READWRITE, _timeout);

		SQLite::Statement insertAlias(db,
			"INSERT INTO aliases (userId, alias, cleanAlias) VALUES ( "
			"  ?, "
			"  ?, "
			"  ? "
			"); "
		);

		insertAlias.bind(1, id);
		insertAlias.bind(2, alias);
		insertAlias.bind(3, alias);
		insertAlias.exec();
	} catch (const SQLite::Exception& e)
	{
		_log.errorLn("adding an alias " + alias + " for user " + std::to_string(id) + " failed: (" + std::to_string(e.getErrorCode()) + ") " + e.getErrorStr());
	}
	
}

void ETJump::UserRepository::update(int64_t id, MutableUserFields changes, int changedFields) const
{
	try
	{
		if (changedFields == 0)
		{
			return;
		}

		std::string editUserQuery =
			"UPDATE users SET modified=STRFTIME('%s', 'NOW'), ";
		std::vector<std::string> changedColumns;
		if (changedFields & static_cast<int>(UserFields::Level))
		{
			changedColumns.push_back("level");
		}
		if (changedFields & static_cast<int>(UserFields::LastSeen))
		{
			changedColumns.push_back("lastSeen");
		}
		if (changedFields & static_cast<int>(UserFields::Name))
		{
			changedColumns.push_back("name");
		}
		if (changedFields & static_cast<int>(UserFields::Title))
		{
			changedColumns.push_back("title");
		}
		if (changedFields & static_cast<int>(UserFields::Commands))
		{
			changedColumns.push_back("commands");
		}
		if (changedFields & static_cast<int>(UserFields::Greeting))
		{
			changedColumns.push_back("greeting");
		}

		for (int i = 0, len = changedColumns.size(); i < len; ++i)
		{
			editUserQuery += " " + changedColumns[i] + "=:" + changedColumns[i];
			if (i != len - 1)
			{
				editUserQuery += ", ";
			}
		}

		editUserQuery += " WHERE id=:id";

		SQLite::Database db(_databaseFile, SQLite::OPEN_READWRITE, _timeout);
		SQLite::Statement updateStmt(db, editUserQuery);

		updateStmt.bind(":id", id);

		for (int i = 0, len = changedColumns.size(); i < len; ++i)
		{
			if (changedColumns[i] == "level")
			{
				updateStmt.bind(":" + changedColumns[i], changes.level);
			}
			else if (changedColumns[i] == "lastSeen")
			{
				updateStmt.bind(":" + changedColumns[i], static_cast<long long>(changes.lastSeen));
			}
			else if (changedColumns[i] == "name")
			{
				updateStmt.bind(":" + changedColumns[i], changes.name);
			}
			else if (changedColumns[i] == "title")
			{
				updateStmt.bind(":" + changedColumns[i], changes.title);
			}
			else if (changedColumns[i] == "commands")
			{
				updateStmt.bind(":" + changedColumns[i], changes.commands);
			}
			else if (changedColumns[i] == "greeting")
			{
				updateStmt.bind(":" + changedColumns[i], changes.greeting);
			}
			else
			{
				updateStmt.bind(":", "");
			}
		}

		updateStmt.exec();
	} catch (const SQLite::Exception& e)
	{
		_log.errorLn("updating user " + std::to_string(id) + " failed: (" + std::to_string(e.getErrorCode()) + ") " + e.getErrorStr());
	}
}

void ETJump::UserRepository::updateLastSeen(int64_t id, time_t lastSeen)
{
	try
	{
		SQLite::Database db(_databaseFile, SQLite::OPEN_READWRITE, _timeout);

		SQLite::Statement updateStmt(db,
			"UPDATE users SET lastSeen=? WHERE id=?;"
		);

		updateStmt.bind(1, static_cast<long long>(lastSeen));
		updateStmt.bind(2, id);

		updateStmt.exec();
	} catch (const SQLite::Exception& e)
	{
		_log.errorLn("updating last seen for user " + std::to_string(id) + " failed: (" + std::to_string(e.getErrorCode()) + ") " + e.getErrorStr());
	}
	
}

int ETJump::UserRepository::setLevelIfHasLevel(int level, int newLevel)
{
	try
	{
		SQLite::Database db(_databaseFile, SQLite::OPEN_READWRITE, _timeout);

		SQLite::Statement updateStmt(db,
			"UPDATE users SET level=? WHERE level=?;"
		);

		updateStmt.bind(1, newLevel);
		updateStmt.bind(2, level);
		
		updateStmt.exec();
		return db.getTotalChanges();
	} catch (const SQLite::Exception& e)
	{
		_log.errorLn("setting all users with level " + std::to_string(level) + " to level " + std::to_string(newLevel) + " failed: (" + std::to_string(e.getErrorCode()) + ") " + e.getErrorStr());
	}
	return 0;
}

void ETJump::UserRepository::addIpAddress(int64_t id, const std::string& ipAddress)
{
	try
	{

		SQLite::Database db(_databaseFile, SQLite::OPEN_READWRITE, _timeout);

		SQLite::Statement insertAlias(db,
			"INSERT INTO ipAddresses (userId, ipAddress) VALUES ( "
			"  ?, "
			"  ? "
			"); "
		);

		insertAlias.bind(1, id);
		insertAlias.bind(2, ipAddress);
		insertAlias.exec();
	}
	catch (const SQLite::Exception& e)
	{
		_log.errorLn("adding an ip address " + ipAddress + " for user " + std::to_string(id) + " failed: (" + std::to_string(e.getErrorCode()) + ") " + e.getErrorStr());
	}
}

