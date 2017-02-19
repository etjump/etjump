#pragma once
#include "etj_user2.h"

namespace ETJump
{
	class Client
	{
	public:
		Client(): _user(User()) {}
		explicit Client(User user): _user(user) {}
		const User& user() const
		{
			return _user;
		}
	private:
		User _user;
	};
}
