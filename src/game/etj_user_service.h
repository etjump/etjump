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
		std::future<void> updateUser(int64_t id, MutableUserFields changes, int changedFields);
		std::future<void> addHardwareId(int64_t id, const std::string& hardwareId);
		std::future<void> addAlias(int64_t id, const std::string& alias);
		std::future<void> addIpAddress(int64_t id, const std::string& cs);
		std::future<void> updateLastSeen(int64_t id, time_t lastSeen);
	private:
		std::shared_ptr<UserRepository> _userRepository;
	};
}



