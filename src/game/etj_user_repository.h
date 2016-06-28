#pragma once
#include "etj_iuser_repository.h"

namespace ETJump
{
	class UserRepository : public IUserRepository
	{
	public:
		UserRepository();
		~UserRepository();

		std::future<User> get(const std::string& guid) const override;
		std::future<User> create(User user) override;
		std::future<User> edit(User user) override;
	};
}



