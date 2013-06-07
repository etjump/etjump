#include "g_database.hpp"

// Used to sort user database by guid. "LESS"
bool SortByGuid(const boost::shared_ptr<User>& lhs,
				const boost::shared_ptr<User>& rhs)
{
	return lhs->guid < rhs->guid;
}

bool Database::NewLevel( Level level )
{
	G_LogPrintf("Database::NewLevel is not yet implemented.\n");
	return true;
}

bool Database::NewUser( User user )
{
	boost::shared_ptr<User> newUser(new User(user));

	if(!usersSorted)
	{
		std::sort(users_.begin(), users_.end(), SortByGuid);
	}

	return true;
}

bool Database::NewBan( Ban ban )
{
	G_LogPrintf("Database::NewBan is not yet implemented.\n");
	return true;
}

bool Database::SaveLevel( const Level& level )
{
	G_LogPrintf("Database::SaveLevel is not yet implemented.\n");
	return true;
}

bool Database::SaveUser( const User& user )
{
	G_LogPrintf("Database::SaveUser is not yet implemented.\n");
	return true;
}

bool Database::SaveBan( const Ban& ban )
{
	G_LogPrintf("Database::SaveBan is not yet implemented.\n");
	return true;
}

LevelData Database::GetLevel( int level ) const
{
	static Level dummyLevel;
	dummyLevel.name = "Dummy level";
	dummyLevel.greeting = "Hello [n]^7!";
	dummyLevel.level = 0;
	dummyLevel.commands = "abc";
	return 0;
}

UserData Database::GetUser( const std::string& guid ) const
{
	return 0;
}

BanData Database::GetBan() const
{
	return 0;
}

Database::~Database()
{

}

Database::Database()
{
	usersSorted = false;
}
