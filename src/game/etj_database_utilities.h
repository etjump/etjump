#pragma once
#include <SQLiteCpp/Database.h>

namespace ETJump
{
	inline SQLite::Database openDatabase(const std::string& file, int flags = SQLite::OPEN_READWRITE, int timeout = 10000)
	{
		auto database = SQLite::Database(file, flags);
		database.setBusyTimeout(timeout);
		return std::move(database);
	}
}
