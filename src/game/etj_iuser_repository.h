#pragma once
#include <future>
#include "etj_user.h"

namespace ETJump
{
	class IUserRepository
	{
	public:
		virtual ~IUserRepository() {}

		struct Result
		{
			User user;
			std::string errorMessage;
		};

		virtual std::future<Result> getOrCreate(const std::string& guid, const std::string& hardwareId, const std::string& name) = 0;
		virtual std::future<Result> edit(User user) = 0;
	};
}
