#pragma once
#include <future>

namespace ETJump
{
	class User;

	class IUserRepository
	{
	public:
		virtual ~IUserRepository() {}

		virtual std::future<User> get(const std::string& guid) const = 0;
		virtual std::future<User> create(User user) = 0;
		virtual std::future<User> edit(User user) = 0;
	};
}
