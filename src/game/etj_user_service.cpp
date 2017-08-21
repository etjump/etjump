#include "etj_user_service.h"


ETJump::UserService::UserService(std::shared_ptr<UserRepository> userRepository)
	: _userRepository(userRepository)
{
}

ETJump::UserService::~UserService()
{
}

std::future<int> ETJump::UserService::setLevelIfHasLevel(int level, int newLevel)
{
	return std::async(std::launch::async, [=]()
	{
		return _userRepository->setLevelIfHasLevel(level, newLevel);
	});
}

std::future<ETJump::User> ETJump::UserService::getUser(const std::string& guid)
{
	return std::async(std::launch::async, [=]()
	{
		return _userRepository->get(guid);
	});
}

std::future<ETJump::User> ETJump::UserService::insertUser(const std::string& guid, const std::string& name, const std::string& ipAddress, const std::string& hardwareId)
{
	return std::async(std::launch::async, [=]()
	{
		_userRepository->insert(guid, name, ipAddress, hardwareId);
		return _userRepository->get(guid);
	});
}

std::future<void> ETJump::UserService::updateUser(int64_t id, MutableUserFields changes, int changedFields)
{
	return std::async(std::launch::async, [=]()
	{
		_userRepository->update(id, changes, changedFields);
	});
}

std::future<void> ETJump::UserService::addHardwareId(int64_t id, const std::string& hardwareId)
{
	return std::async(std::launch::async, [=]()
	{
		_userRepository->addHardwareId(id, hardwareId);
	});
}

std::future<void> ETJump::UserService::addAlias(int64_t id, const std::string& alias)
{
	return std::async(std::launch::async, [=]()
	{
		_userRepository->addAlias(id, alias);
	});
}

std::future<void> ETJump::UserService::addIpAddress(int64_t id, const std::string& ipAddress)
{
	return std::async(std::launch::async, [=]()
	{
		_userRepository->addIpAddress(id, ipAddress);
	});
}

std::future<void> ETJump::UserService::updateLastSeen(int64_t id, time_t lastSeen)
{
	return std::async(std::launch::async, [=]()
	{
		_userRepository->updateLastSeen(id, lastSeen);
	});
}

