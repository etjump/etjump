#include "etj_user_repository.h"
#include "etj_user.h"


ETJump::UserRepository::UserRepository()
{
}

ETJump::UserRepository::~UserRepository()
{
}

std::future<ETJump::User> ETJump::UserRepository::get(const std::string& guid) const
{
	return std::async(std::launch::async, [=]()
	{
		std::this_thread::sleep_for(std::chrono::seconds(2));
		return User(1, guid);
	});
}

std::future<ETJump::User> ETJump::UserRepository::create(User user)
{
	return std::async(std::launch::async, [=]()
	{
		return user;
	});
}

std::future<ETJump::User> ETJump::UserRepository::edit(User user)
{
	return std::async(std::launch::async, [=]()
	{
		return user;
	});
}
