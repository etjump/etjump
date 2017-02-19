#include "etj_user_repository.h"
#include <boost/format.hpp>
#include <future>
#include "../SQLiteCpp/Database.h"
#include "../SQLiteCpp/Backup.h"
#include "../SQLiteCpp/Transaction.h"
#include <boost/algorithm/string/join.hpp>

ETJump::UserRepository::UserRepository(const std::string& database)
	: _userResultTasks{},
	_idCache{},
	_database(database)
{
	createDatabaseTables();
}

ETJump::UserRepository::~UserRepository()
{
}

void ETJump::UserRepository::createOrUpdateAsync(const std::string& name, const std::string& guid, const std::string& hardwareId, const std::string& ip, std::function<void(const std::shared_ptr<TaskResult<User>>)> onCompletionSync)
{
	auto cachedId = User::INVALID_ID;
	auto idIterator = _idCache.find(guid);
	if (idIterator != end(_idCache))
	{
		cachedId = idIterator->second;
	}

	_userResultTasks.push_back(ActiveTask<const std::shared_ptr<TaskResult<User>>>{
		std::async(std::launch::async, [=]()
		{
			try
			{
				// if we have a valid id, we know the user is in database => we can perform an update
				// if we don't have a valid id, we need to check if user is in database. If servers are 
				// sharing database, it is possible that user has an entry already.
				SQLite::Database db(_database, SQLite::OPEN_READWRITE);

				/*
				SELECT
					id
				FROM users
				WHERE guid = ? ;
				*/
				auto id = cachedId;
				if (cachedId == User::INVALID_ID)
				{
					SQLite::Statement query(db,
						"SELECT "
							"id "	
						"FROM users "
						"WHERE guid = ?;"
					);

					query.bind(1, guid);

					if (query.executeStep())
					{
						id = query.getColumn(0);
					}
				}
/*
				INSERT INTO users(name, level, last_seen, guid, title, commands, greeting) VALUES(
					:name,
					: level,
					: last_seen,
					: guid,
					: title,
					: commands,
					: greeting
				);
*/
				// if we still don't have a valid ID, insert a new one
				if (id == User::INVALID_ID)
				{
					SQLite::Transaction tx(db);

					id = insertUser(db, name, guid);
						
					insertName(db, name, id);
					
					insertHardwareId(db, hardwareId, id);
					
					insertIp(db, ip, id);

					tx.commit();
					User user(id, 0, 0, name, guid, "", "", "", { ip }, { name }, { hardwareId });
					return std::make_shared<TaskResult<User>>(user, "");
				}

				SQLite::Transaction tx(db);

				insertOrIgnoreName(db, name, id);

				insertOrIgnoreHardwareId(db, hardwareId, id);
				
				insertOrIgnoreIp(db, ip, id);

				tx.commit();

				auto user = getUser(db, id);

				return std::make_shared<TaskResult<User>>(user, std::string(""));
			} catch (const std::exception& exception)
			{
				return std::make_shared<TaskResult<User>>(User(), exception.what());
			}
		}),
		onCompletionSync,
		false
	});
}

void ETJump::UserRepository::checkForCompletedTasks()
{
	for (auto & activeTask : _userResultTasks)
	{
		if (activeTask.result.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			activeTask.onCompletion(activeTask.result.get());
			activeTask.handled = true;
		}
	}
	std::vector<ActiveTask<const std::shared_ptr<TaskResult<User>>>> temp;
	for (auto & activeTask : _userResultTasks)
	{
		if (!activeTask.handled)
		{
			temp.push_back(std::move(activeTask));
		}
	}
	_userResultTasks = std::move(temp);
}

void ETJump::UserRepository::updateAsync(long long userId, UserUpdateModel updateModel, std::function<void(const std::shared_ptr<TaskResult<User>>)> onCompletionSync)
{
	if (!updateModel.name && !updateModel.commands && !updateModel.greeting && !updateModel.lastSeen && !updateModel.level && !updateModel.title)
	{
		onCompletionSync(std::make_shared<TaskResult<User>>(User(), "No changes were specified"));
		return;
	}

	_userResultTasks.push_back(ActiveTask<const std::shared_ptr<TaskResult<User>>>{
		std::async(std::launch::async, [=]()
		{
			try
			{
				SQLite::Database db(_database, SQLite::OPEN_READWRITE);

				boost::format updateSqlFmt("UPDATE users SET %s WHERE id=:user_id;");
				std::vector<std::string> updatedFields;
				if (updateModel.name)
				{
					updatedFields.push_back("name=:name");
				}
				if (updateModel.title)
				{
					updatedFields.push_back("title=:title");
				}
				if (updateModel.commands)
				{
					updatedFields.push_back("commands=:commands");
				}
				if (updateModel.greeting)
				{
					updatedFields.push_back("greeting=:greeting");
				}
				if (updateModel.lastSeen)
				{
					updatedFields.push_back("last_seen=:last_seen");
				}
				if (updateModel.level)
				{
					updatedFields.push_back("level=:level");
				}

				auto updateFieldsSql = (updateSqlFmt % boost::join(updatedFields, ", ")).str();

				SQLite::Statement updateStmt(db, updateFieldsSql);

				if (updateModel.name)
				{
					updateStmt.bind(":name", *updateModel.name);
				}
				if (updateModel.title)
				{
					updateStmt.bind(":title", *updateModel.title);
				}
				if (updateModel.commands)
				{
					updateStmt.bind(":commands", *updateModel.commands);
				}
				if (updateModel.greeting)
				{
					updateStmt.bind(":greeting", *updateModel.greeting);
				}
				if (updateModel.lastSeen)
				{
					updateStmt.bind(":last_seen", *updateModel.lastSeen);
				}
				if (updateModel.level)
				{
					updateStmt.bind(":level", *updateModel.level);
				}
				updateStmt.bind(":user_id", userId);

				updateStmt.exec();

				return std::make_shared<TaskResult<User>>(getUser(db, userId), "");
			}
			catch (const std::exception& exception)
			{
				return std::make_shared<TaskResult<User>>(User(), exception.what());
			}
		}),
		onCompletionSync,
		false
	});
}

//CREATE TABLE IF NOT EXISTS users (
//  id INTEGER PRIMARY KEY AUTOINCREMENT,
//  name TEXT NOT NULL,
//  level INTEGER NOT NULL DEFAULT (0),
//  last_seen INTEGER NOT NULL DEFAULT (0),
//  guid TEXT NOT NULL UNIQUE,
//  title TEXT NOT NULL,
//  commands TEXT NOT NULL,
//  greeting TEXT NOT NULL
//);
//
//CREATE TABLE IF NOT EXISTS users_hardware_ids (
//  hardware_id TEXT NOT NULL,
//  user_id INTEGER NOT NULL,
//  UNIQUE(hardware_id, user_id),
//  FOREIGN KEY (user_id) REFERENCES users(id)
//);
//
//CREATE TABLE IF NOT EXISTS users_ips (
//  ip TEXT NOT NULL,
//  user_id INTEGER NOT NULL,
//  UNIQUE(ip, user_id),
//  FOREIGN KEY (user_id) REFERENCES users(id)
//);
//
//CREATE TABLE IF NOT EXISTS users_names (
//  name TEXT NOT NULL,
//  user_id INTEGER NOT NULL,
//  UNIQUE(name, user_id),
//  FOREIGN KEY (user_id) REFERENCES users(id)
//);

void ETJump::UserRepository::createDatabaseTables()
{
	SQLite::Database db(_database, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

	db.exec(
		"CREATE TABLE IF NOT EXISTS users ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"name TEXT NOT NULL,"
			"level INTEGER NOT NULL DEFAULT(0),"
			"last_seen INTEGER NOT NULL DEFAULT(0),"
			"guid TEXT NOT NULL UNIQUE,"
			"title TEXT NOT NULL,"
			"commands TEXT NOT NULL,"
			"greeting TEXT NOT NULL"
		"); "
	);

	db.exec(
		"CREATE TABLE IF NOT EXISTS users_hardware_ids("
			"hardware_id TEXT NOT NULL,"
			"user_id INTEGER NOT NULL,"
			"UNIQUE(hardware_id, user_id),"
			"FOREIGN KEY(user_id) REFERENCES users(id)"
		");"
	);

	db.exec(
		"CREATE TABLE IF NOT EXISTS users_ips("
			"ip TEXT NOT NULL,"
			"user_id INTEGER NOT NULL,"
			"UNIQUE(ip, user_id),"
			"FOREIGN KEY(user_id) REFERENCES users(id)"
		");"
	);

	db.exec(
		"CREATE TABLE IF NOT EXISTS users_names("
			"name TEXT NOT NULL,"
			"user_id INTEGER NOT NULL,"
			"UNIQUE(name, user_id),"
			"FOREIGN KEY(user_id) REFERENCES users(id)"
		");"
	);
}

long long ETJump::UserRepository::insertUser(SQLite::Database& db, const std::string& name, const std::string& guid)
{
	SQLite::Statement insertUser(db,
		"INSERT INTO users(name, level, last_seen, guid, title, commands, greeting) VALUES("
		":name,"
		":level,"
		":last_seen,"
		":guid,"
		":title,"
		":commands,"
		":greeting"
		");"
	);
	insertUser.bind(":name", name);
	insertUser.bind(":level", 0);
	insertUser.bind(":last_seen", 0);
	insertUser.bind(":guid", guid);
	insertUser.bind(":title", "");
	insertUser.bind(":commands", "");
	insertUser.bind(":greeting", "");

	insertUser.exec();

	return db.getLastInsertRowid();
}

void ETJump::UserRepository::insertName(SQLite::Database& db, const std::string& name, long long userId)
{
	// INSERT INTO users_names (name, user_id) VALUES (:name, :user_id);
	SQLite::Statement insertName(db, "INSERT INTO users_names (name, user_id) VALUES (:name, :user_id);");
	insertName.bind(":name", name);
	insertName.bind(":user_id", userId);

	insertName.exec();
}

void ETJump::UserRepository::insertOrIgnoreName(SQLite::Database& db, const std::string& name, long long userId)
{
	// INSERT OR IGNORE INTO users_names (name, user_id) VALUES (:name, :user_id);
	SQLite::Statement insertName(db, "INSERT OR IGNORE INTO users_names (name, user_id) VALUES (:name, :user_id);");
	insertName.bind(":name", name);
	insertName.bind(":user_id", userId);

	insertName.exec();
}

void ETJump::UserRepository::insertOrIgnoreHardwareId(SQLite::Database& db, const std::string& hardwareId, long long userId)
{
	// INSERT OR IGNORE INTO users_hardware_ids (hardware_id, user_id) VALUES (:hardware_id, :user_id);
	SQLite::Statement insertHardwareId(db, "INSERT OR IGNORE INTO users_hardware_ids (hardware_id, user_id) VALUES (:hardware_id, :user_id);");
	insertHardwareId.bind(":hardware_id", hardwareId);
	insertHardwareId.bind(":user_id", userId);

	insertHardwareId.exec();
}

void ETJump::UserRepository::insertOrIgnoreIp(SQLite::Database& db, const std::string& ip, long long userId)
{
	// INSERT OR IGNORE INTO users_ips (ip, user_id) VALUES (:ip, :user_id);
	SQLite::Statement insertIp(db, "INSERT OR IGNORE INTO users_ips (ip, user_id) VALUES (:ip, :user_id);");
	insertIp.bind(":ip", ip);
	insertIp.bind(":user_id", userId);

	insertIp.exec();
}

/*
SELECT
	hardware_id
FROM users_hardware_ids WHERE user_id = :user_id;
*/
std::vector<std::string> ETJump::UserRepository::getHardwareIdsForUser(SQLite::Database& db, long long userId)
{
	SQLite::Statement hardwareIdsQuery(db,
		"SELECT "
		"hardware_id "
		"FROM users_hardware_ids WHERE user_id = :user_id;"
	);
	hardwareIdsQuery.bind(":user_id", userId);
	std::vector<std::string> hardwareIds{};
	while (hardwareIdsQuery.executeStep())
	{
		hardwareIds.push_back(hardwareIdsQuery.getColumn(0));
	}
	return hardwareIds;
}

/*
SELECT
	ip
FROM users_ips WHERE user_id = :user_id;
*/
std::vector<std::string> ETJump::UserRepository::getIpsForUser(SQLite::Database& db, long long userId)
{
	SQLite::Statement ipsQuery(db,
		"SELECT "
		"ip "
		"FROM users_ips WHERE user_id = :user_id;"
	);
	ipsQuery.bind(":user_id", userId);
	std::vector<std::string> ips;
	while (ipsQuery.executeStep())
	{
		ips.push_back(ipsQuery.getColumn(0));
	}
	return ips;
}

std::vector<std::string> ETJump::UserRepository::getNamesForUser(SQLite::Database& db, long long userId)
{
	/*
	SELECT
	name
	FROM users_names WHERE user_id = :user_id;
	*/
	SQLite::Statement namesQuery(db,
		"SELECT "
		"name "
		"FROM users_names WHERE user_id = :user_id;"
	);
	namesQuery.bind(":user_id", userId);
	std::vector<std::string> names;
	while (namesQuery.executeStep())
	{
		names.push_back(namesQuery.getColumn(0));
	}
	return names;
}

ETJump::User ETJump::UserRepository::getUser(SQLite::Database& db, long long userId)
{
	/*
	SELECT
	id,
	name,
	level,
	last_seen,
	guid,
	title,
	commands,
	greeting
	FROM users WHERE id = :id;
	*/
	SQLite::Statement userQuery(db,
		"SELECT "
		"name, "
		"level, "
		"last_seen, "
		"guid, "
		"title, "
		"commands, "
		"greeting "
		"FROM users WHERE id = :id;"
	);
	userQuery.bind(":id", userId);
	userQuery.executeStep();

	std::string storedName = userQuery.getColumn(0);
	int level = userQuery.getColumn(1);
	int lastSeen = userQuery.getColumn(2);
	std::string storedGuid = userQuery.getColumn(3);
	std::string storedTitle = userQuery.getColumn(4);
	std::string storedCommands = userQuery.getColumn(5);
	std::string storedGreeting = userQuery.getColumn(6);

	auto hardwareIds = getHardwareIdsForUser(db, userId);

	auto ips = getIpsForUser(db, userId);

	auto names = getNamesForUser(db, userId);

	User user(userId, level, lastSeen, storedName, storedGuid, storedTitle, storedCommands, storedGreeting, ips, names, hardwareIds);
	return user;
}

void ETJump::UserRepository::insertHardwareId(SQLite::Database& db, const std::string& hardwareId, long long userId)
{
	// INSERT INTO users_hardware_ids (hardware_id, user_id) VALUES (:hardware_id, :user_id);
	SQLite::Statement insertHardwareId(db, "INSERT INTO users_hardware_ids (hardware_id, user_id) VALUES (:hardware_id, :user_id);");
	insertHardwareId.bind(":hardware_id", hardwareId);
	insertHardwareId.bind(":user_id", userId);

	insertHardwareId.exec();
}

void ETJump::UserRepository::insertIp(SQLite::Database& db, const std::string& ip, long long userId)
{
	// INSERT INTO users_ips (ip, user_id) VALUES (:ip, :user_id);
	SQLite::Statement insertIp(db, "INSERT INTO users_ips (ip, user_id) VALUES (:ip, :user_id);");
	insertIp.bind(":ip", ip);
	insertIp.bind(":user_id", userId);

	insertIp.exec();
}
