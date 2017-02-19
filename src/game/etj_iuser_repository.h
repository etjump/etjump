#pragma once
#include <memory>
#include <functional>
#include <boost/optional.hpp>
#include "etj_user2.h"

namespace ETJump
{
	class IUserRepository
	{
	public:
		virtual ~IUserRepository() {}

		template <typename T>
		struct TaskResult
		{
			TaskResult(T result, std::string error): result(result), error(error) {}
			T result;
			std::string error;
		};

		struct UserUpdateModel
		{
			boost::optional<int> level;
			boost::optional<int> lastSeen;
			boost::optional<std::string> name;
			boost::optional<std::string> title;
			boost::optional<std::string> commands;
			boost::optional<std::string> greeting;
		};

		// Creates a new user based on guid & hardware id or updates an existing one
		// After completion onCompletionSync will be called synchronously from main thread
		virtual void createOrUpdateAsync(
			const std::string& name,
			const std::string& guid, 
			const std::string& hardwareId, 
			const std::string& ip,
			std::function<void(const std::shared_ptr<TaskResult<User>>)> onCompletionSync
		) = 0;

		// check for any completed tasks from main thread (to synchronously call)
		virtual void checkForCompletedTasks() = 0;

		virtual void updateAsync(long long userId, UserUpdateModel updateModel, std::function<void(const std::shared_ptr<TaskResult<User>>)> onCompletionSync) = 0;
	};
}
