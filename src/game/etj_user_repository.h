#pragma once
#include "etj_iuser_repository.h"
#include <future>
#include <map>

namespace SQLite {
	class Database;
}

namespace ETJump
{
	class UserRepository : public IUserRepository
	{
	public:
		explicit UserRepository(const std::string& database);
		~UserRepository();

		void createOrUpdateAsync(
			const std::string& name,
			const std::string& guid, 
			const std::string& hardwareId, 
			const std::string& ip,
			std::function<void(const std::shared_ptr<TaskResult<User>>)> onCompletionSync
		) override;
		void checkForCompletedTasks() override;
		void updateAsync(long long userId, UserUpdateModel updateModel, std::function<void(const std::shared_ptr<TaskResult<User>>)> onCompletionSync) override;

	private:

		template <typename T>
		struct ActiveTask
		{
			std::future<std::shared_ptr<TaskResult<User>>> result;
			std::function<void(T)> onCompletion;
			bool handled;
		};

		void createDatabaseTables();
		///////////
		// Commands
		///////////
		static long long insertUser(SQLite::Database& db, const std::string& name, const std::string& guid);
		// insert a name for user. Throws if name already exists for user
		static void insertName(SQLite::Database& db, const std::string& name, long long userId);
		// insert a name for user. Does nothing if name already exists for user
		static void insertOrIgnoreName(SQLite::Database& db, const std::string& name, long long userId);
		// insert a hardware ID for user. Throws if hardware ID already exists for user
		static void insertHardwareId(SQLite::Database& db, const std::string& hardwareId, long long userId);
		// insert a hardware ID for user. Does nothing if hardware ID already exists for user.
		static void insertOrIgnoreHardwareId(SQLite::Database& db, const std::string& hardwareId, long long userId);
		// insert an IP for user. Throws if IP already exist for user
		static void insertIp(SQLite::Database& db, const std::string& ip, long long userId);
		// insert an IP for user. Does nothing if IP already exists for user.
		static void insertOrIgnoreIp(SQLite::Database& db, const std::string& ip, long long userId);
		////////////
		// Queries
		////////////
		static std::vector<std::string> getHardwareIdsForUser(SQLite::Database& db, long long userId);
		static std::vector<std::string> getIpsForUser(SQLite::Database& db, long long userId);
		static std::vector<std::string> getNamesForUser(SQLite::Database& db, long long userId);
		static User getUser(SQLite::Database& db, long long userId);

		std::vector<ActiveTask<const std::shared_ptr<TaskResult<User>>>> _userResultTasks;
		std::map<std::string, long long> _idCache;
		std::string _database;
	};
}
