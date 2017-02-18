#pragma once
#include "etj_iuser_repository.h"
#include <future>
#include <map>

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
	private:

		template <typename T>
		struct ActiveTask
		{
			std::future<std::shared_ptr<TaskResult<User>>> result;
			std::function<void(T)> onCompletion;
			bool handled;
		};

		void createDatabaseTables();

		std::vector<ActiveTask<const std::shared_ptr<TaskResult<User>>>> _activeCreateOrUpdateTasks;
		std::map<std::string, int> _idCache;
		std::string _database;
	};
}
