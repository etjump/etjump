#pragma once
#include "etj_user_repository.h"
#include <memory>
#include <future>

namespace ETJump
{
	class UserService
	{
	public:

		explicit UserService(std::shared_ptr<UserRepository> userRepository);
		~UserService();

		std::future<User> getUser(const std::string& guid);
		std::future<User> insertUser(const std::string& guid, const std::string& name, const std::string& ipAddress, const std::string& hardwareId);
		// returns an int as Task<void> cannot be used (cannot future.get() a future<void>)
		std::future<int> updateUser(int64_t id, MutableUserFields changes, int changedFields);
		std::future<void> addHardwareId(int64_t id, const std::string& hardwareId);
		std::future<void> addAlias(int64_t id, const std::string& alias);
		std::future<void> addIpAddress(int64_t id, const std::string& cs);
		std::future<void> updateLastSeen(int64_t id, time_t lastSeen);
		std::future<int> setLevelIfHasLevel(int level, int newLevel);
		std::future<std::vector<User>> findUsersByName(const std::string& name);
        std::future<std::vector<User>> listUsers(int page, int rows);
	private:
		std::shared_ptr<UserRepository> _userRepository;
	};
}



