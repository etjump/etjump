#pragma once
#include <string>
#include "etj_user.h"
#include <boost/optional/optional.hpp>
#include <SQLiteCpp/Database.h>
#include "etj_log.h"

namespace ETJump
{
	// user database can be accessed through the repository
	class UserRepository
	{
	public:
		static const int NEW_USER_ID = -1;
		static const int BUSY_TIMEOUT = 10000;
		
		explicit UserRepository(const std::string& databaseFile, int timeout);
		~UserRepository();

		void createTables();

		// thread safe
		void insert(const std::string& guid, const std::string& name, const std::string& ipAddress, const std::string& hardwareId) const;
		// thread safe
		User get(const std::string& guid) const;
		// thread safe
		void addHardwareId(int64_t id, const std::string& hardwareId) const;
		// thread safe
		void addAlias(int64_t id, const std::string& alias) const;
		// thread safe
		void update(int64_t id, MutableUserFields changes, int changedFields) const;
		// thread safe
		void addIpAddress(int64_t id, const std::string& ipAddress);
		// thread safe
		void updateLastSeen(int64_t id, time_t lastSeen);
		// thread safe
		// returns the count of updated users
		int setLevelIfHasLevel(int level, int newLevel);
	private:
		std::string _databaseFile;
		int _timeout;
		Log _log;
	};
}




