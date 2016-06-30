#include "etj_user_repository.h"
#include "etj_user.h"
#include <boost/format.hpp>


ETJump::UserRepository::UserRepository(const std::string& directory, const std::string& file) : _filepath(directory + (file.length() > 0 ? file : "users.db"))
{
	SQLite::Database database(_filepath, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	createUsersTable(database);
	createHardwareIdsTable(database);
	createUsersNamesTable(database);
}

ETJump::UserRepository::~UserRepository()
{
}

std::future<ETJump::IUserRepository::Result> ETJump::UserRepository::getOrCreate(const std::string& guid, const std::string& hardwareId, const std::string& name)
{
	return std::async(std::launch::async, [=]()
	{
		try
		{
			SQLite::Database database(_filepath, SQLITE_OPEN_READWRITE);
			database.setBusyTimeout(10000);
			SQLite::Statement query(database, "SELECT id, level, lastSeen, name, title, commands, greeting FROM users_v2 WHERE guid=?;");
			query.bind(1, guid);
			if (query.executeStep())
			{
				int id = query.getColumn(0);
				int level = query.getColumn(1);
				int lastSeen = query.getColumn(2);
				std::string prevName = query.getColumn(3);
				std::string title = query.getColumn(4);
				std::string commands = query.getColumn(5);
				std::string greeting = query.getColumn(6);
				SQLite::Statement hardwareIdsQuery(database, "SELECT hardwareId FROM hardwareIds WHERE userId=?;");
				hardwareIdsQuery.bind(1, id);
				std::vector<std::string> hardwareIds;
				while (hardwareIdsQuery.executeStep())
				{
					hardwareIds.push_back(hardwareIdsQuery.getColumn(0));
				}
				if (find(begin(hardwareIds), end(hardwareIds), hardwareId) == end(hardwareIds))
				{
					hardwareIds.push_back(hardwareId);
					SQLite::Statement insertHardwareId(database, "INSERT INTO hardwareIds (hardwareId, userId) VALUES (?, ?);");
					insertHardwareId.bind(1, hardwareId);
					insertHardwareId.bind(2, id);
					insertHardwareId.exec();
				}
				return Result{ User(id, guid, level, lastSeen, prevName, title, commands, greeting, {}), "" };
			}
			else
			{
				SQLite::Statement insert(database, "INSERT INTO users_v2 (level, lastSeen, guid, name, title, commands, greeting) VALUES (?, ?, ?, ?, ?, ?, ?);");
				insert.bind(1, 0);
				insert.bind(2, 0);
				insert.bind(3, guid);
				insert.bind(4, name);
				insert.bind(5, "");
				insert.bind(6, "");
				insert.bind(7, "");
				insert.exec();
				auto id = database.getLastInsertRowid();

				SQLite::Statement insertHardwareId(database, "INSERT INTO hardwareIds (hardwareId, userId) VALUES (?, ?);");
				insertHardwareId.bind(1, hardwareId);
				insertHardwareId.bind(2, id);
				insertHardwareId.exec();
				return Result{ User(id, guid, 0, 0, "", "", "", "", {hardwareId}), "" };
			}
		}
		catch (const SQLite::Exception& exception)
		{
			return Result{ User(), (boost::format("Could not fetch user from database: %s.") % exception.what()).str() };
		}
	});
}

std::future<ETJump::IUserRepository::Result> ETJump::UserRepository::edit(User user)
{
	return std::async(std::launch::async, [=]()
	{
		return Result{ user, "" };
	});
}

void ETJump::UserRepository::createUsersTable(SQLite::Database& database)
{
	std::string createSql = "CREATE TABLE IF NOT EXISTS users_v2 ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"level INTEGER NOT NULL,"
		"lastSeen INTEGER NOT NULL,"
		"guid TEXT NOT NULL,"
		"name TEXT NOT NULL,"
		"title TEXT NOT NULL,"
		"commands TEXT NOT NULL,"
		"greeting TEXT NOT NULL"
		");";

	SQLite::Statement createUsers(database, createSql);

	try
	{
		createUsers.exec();
	}
	catch (const SQLite::Exception& exception)
	{
		throw RepositoryInitializationFailedException((boost::format("Could not create users_v2 table: %s.") % exception.what()).str());
	}
}

void ETJump::UserRepository::createHardwareIdsTable(SQLite::Database& database)
{
	std::string createSql = "CREATE TABLE IF NOT EXISTS hardwareIds ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"hardwareId TEXT NOT NULL,"
		"userId INTEGER NOT NULL,"
		"FOREIGN KEY(userId) REFERENCES users_v2(id)"
		");";

	try
	{
		SQLite::Statement createHardwareIds(database, createSql);
		createHardwareIds.exec();
	}
	catch (const SQLite::Exception& exception)
	{
		throw RepositoryInitializationFailedException((boost::format("Could not create hardwareIds table: %s.") % exception.what()).str());
	}
}

void ETJump::UserRepository::createUsersNamesTable(SQLite::Database& database)
{
	std::string createSql = "CREATE TABLE IF NOT EXISTS usersNames ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"userId INTEGER NOT NULL,"
		"cleanName TEXT NOT NULL,"
		"name TEXT NOT NULL,"
		"FOREIGN KEY(userId) REFERENCES users_v2(id)"
		");";
	SQLite::Statement createUsersNamesTable(database, createSql);

	try
	{
		createUsersNamesTable.exec();
	}
	catch (const SQLite::Exception& exception)
	{
		throw RepositoryInitializationFailedException((boost::format("Could not create users names table: %s.") % exception.what()).str());
	}
}
