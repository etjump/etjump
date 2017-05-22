#pragma once
#include <string>
#include "etj_user_new.h"

namespace ETJump
{
	class UserRepository
	{
	public:
		explicit UserRepository(const std::string& databaseFile, int timeout);
		~UserRepository();

		void insert(const std::string& guid, const std::string& name, const std::string& hardwareId);
		User get(const std::string& guid);
	private:
		std::string _databaseFile;
	};
}




