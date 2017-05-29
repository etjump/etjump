#pragma once
#include <string>
#include "etj_user_new.h"
#include <boost/optional/optional.hpp>
#include <SQLiteCpp/Statement.h>

namespace ETJump
{
	class UserRepository
	{
	public:
		explicit UserRepository(const std::string& databaseFile, int timeout);
		~UserRepository();

		void createTables();

		void insert(const std::string& guid, const std::string& name, const std::string& ipAddress, const std::string& hardwareId);
		User get(const std::string& guid);
		void addHardwareId(int id, const std::string& hardwareId);
		void addAlias(int id, const std::string& alias);
	private:
		std::string _databaseFile;
		int _timeout;
	};
}




