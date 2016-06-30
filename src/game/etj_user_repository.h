#pragma once
#include "etj_iuser_repository.h"
#include <SQLiteCpp/Database.h>

namespace ETJump
{
	class RepositoryInitializationFailedException : public std::runtime_error
	{
	public:
		explicit RepositoryInitializationFailedException(const std::string& message)
			: runtime_error(message)
		{
		}
	};

	class UserRepository : public IUserRepository
	{
	public:
		explicit UserRepository(const std::string& directory, const std::string& file);
		~UserRepository();

		std::future<Result> getOrCreate(const std::string& guid, const std::string& hardwareId) override;
		std::future<Result> edit(User user) override;
	private:
		const std::string _filepath;

		static void createUsersTable(SQLite::Database& database);
		static void createHardwareIdsTable(SQLite::Database& database);
		static void createUsersNamesTable(SQLite::Database& database);
	};
}



