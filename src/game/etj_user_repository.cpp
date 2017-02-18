#include "etj_user_repository.h"
#include <future>
#include "../SQLiteCpp/Database.h"
#include "../SQLiteCpp/Backup.h"
#include "../SQLiteCpp/Transaction.h"

ETJump::UserRepository::UserRepository(const std::string& database)
	: _activeCreateOrUpdateTasks{},
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

	_activeCreateOrUpdateTasks.push_back(ActiveTask<const std::shared_ptr<TaskResult<User>>>{
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

					id = db.getLastInsertRowid();
						
					// INSERT INTO users_names (name, user_id) VALUES (:name, :user_id);
					SQLite::Statement insertName(db, "INSERT INTO users_names (name, user_id) VALUES (:name, :user_id);");
					insertName.bind(":name", name);
					insertName.bind(":user_id", id);
					
					insertName.exec();

					// INSERT INTO users_hardware_ids (hardware_id, user_id) VALUES (:hardware_id, :user_id);
					SQLite::Statement insertHardwareId(db, "INSERT INTO users_hardware_ids (hardware_id, user_id) VALUES (:hardware_id, :user_id);");
					insertHardwareId.bind(":hardware_id", hardwareId);
					insertHardwareId.bind(":user_id", id);

					insertHardwareId.exec();

					// INSERT INTO users_ips (ip, user_id) VALUES (:ip, :user_id);
					SQLite::Statement insertIp(db, "INSERT INTO users_ips (ip, user_id) VALUES (:ip, :user_id);");
					insertIp.bind(":ip", ip);
					insertIp.bind(":user_id", id);

					insertIp.exec();

					tx.commit();
					User user(id, 0, 0, name, guid, "", "", "", { ip }, { name }, { hardwareId });
					return std::make_shared<TaskResult<User>>(user, "");
				}

				SQLite::Transaction tx(db);

				// INSERT OR IGNORE INTO users_names (name, user_id) VALUES (:name, :user_id);
				SQLite::Statement insertName(db, "INSERT OR IGNORE INTO users_names (name, user_id) VALUES (:name, :user_id);");
				insertName.bind(":name", name);
				insertName.bind(":user_id", id);
				
				insertName.exec();

				// INSERT OR IGNORE INTO users_hardware_ids (hardware_id, user_id) VALUES (:hardware_id, :user_id);
				SQLite::Statement insertHardwareId(db, "INSERT OR IGNORE INTO users_hardware_ids (hardware_id, user_id) VALUES (:hardware_id, :user_id);");
				insertHardwareId.bind(":hardware_id", hardwareId);
				insertHardwareId.bind(":user_id", id);

				insertHardwareId.exec();

				// INSERT OR IGNORE INTO users_ips (ip, user_id) VALUES (:ip, :user_id);
				SQLite::Statement insertIp(db, "INSERT OR IGNORE INTO users_ips (ip, user_id) VALUES (:ip, :user_id);");
				insertIp.bind(":ip", ip);
				insertIp.bind(":user_id", id);

				insertIp.exec();

				tx.commit();
/*
				SELECT
					hardware_id,
					user_id
				FROM users_hardware_ids WHERE user_id = :user_id;
*/
				SQLite::Statement hardwareIdsQuery(db,
					"SELECT "
						"hardware_id "
					"FROM users_hardware_ids WHERE user_id = :user_id;"
				);
				hardwareIdsQuery.bind(":user_id", id);
				std::vector<std::string> hardwareIds{};
				while(hardwareIdsQuery.executeStep())
				{
					hardwareIds.push_back(hardwareIdsQuery.getColumn(0));
				}
/*
				SELECT
					ip
				FROM users_ips WHERE user_id = :user_id;
*/
				SQLite::Statement ipsQuery(db,
					"SELECT "
						"ip "
					"FROM users_ips WHERE user_id = :user_id;"
				);
				ipsQuery.bind(":user_id", id);
				std::vector<std::string> ips;
				while (ipsQuery.executeStep())
				{
					ips.push_back(ipsQuery.getColumn(0));
				}
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
				namesQuery.bind(":user_id", id);
				std::vector<std::string> names;
				while (namesQuery.executeStep())
				{
					names.push_back(namesQuery.getColumn(0));
				}
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
				userQuery.bind(":id", id);
				userQuery.executeStep();

				std::string storedName = userQuery.getColumn(0);
				int level = userQuery.getColumn(1);
				int lastSeen = userQuery.getColumn(2);
				std::string storedGuid = userQuery.getColumn(3);
				std::string storedTitle = userQuery.getColumn(4);
				std::string storedCommands = userQuery.getColumn(5);
				std::string storedGreeting = userQuery.getColumn(6);

				User user(id, level, lastSeen, storedName, storedGuid, storedTitle, storedCommands, storedGreeting, ips, names, hardwareIds);

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
	for (auto & activeTask : _activeCreateOrUpdateTasks)
	{
		if (activeTask.result.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			activeTask.onCompletion(activeTask.result.get());
			activeTask.handled = true;
		}
	}
	std::vector<ActiveTask<const std::shared_ptr<TaskResult<User>>>> temp;
	for (auto & activeTask : _activeCreateOrUpdateTasks)
	{
		if (!activeTask.handled)
		{
			temp.push_back(std::move(activeTask));
		}
	}
	_activeCreateOrUpdateTasks = std::move(temp);
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
