#include "g_database.hpp"
#include <algorithm>

// Used to sort user database by guid. "LESS"
bool SortByGuid(const boost::shared_ptr<User>& lhs,
				const boost::shared_ptr<User>& rhs)
{
	return lhs->guid < rhs->guid;
}

bool SortByGuidStr(const boost::shared_ptr<User>& lhs,
				const std::string& rhs)
{
	return lhs->guid < rhs;
}

bool SortByGuidStr(const std::string& lhs, 
				const boost::shared_ptr<User>& rhs)
{
	return lhs < rhs->guid;
}

bool Database::NewLevel( Level level )
{
	G_LogPrintf("Database::NewLevel is not yet implemented.\n");
	return true;
}

bool Database::NewUser( User user )
{
	std::pair<std::string, UserData> newUser;
	newUser.first = user.guid;
	newUser.second.level = user.level;
	newUser.second.name = user.name;
	newUser.second.personalCommands = user.personalCommands;
	newUser.second.personalGreeting = user.personalGreeting;
	newUser.second.personalTitle = user.personalTitle;
	
	if(users_.insert(newUser).second == false) 
	{
		// User already exists
		return false;
	}

	// user is now on db
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
	return &dummyLevel;
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
	usersSorted_ = false;
}

bool Database::GetUser( const std::string& guid, ConstUserIterator& it ) const
{
	it = users_.find(guid);
	if(it != users_.end())
	{
		return true;
	}
	return false;
}

namespace configConstants {
	std::string 
		USER_BEGIN =	"[user] = ",
		LEVEL =			"level = ",
		NAME =			"name = ",
		GUID =			"guid = ",
		PCMDS =			"pcmds = ",
		PGRT =			"pgrt = ",
		PTITLE =		"ptitle = ";
}
void Database::WriteUserConfig()
{
	ConstUserIterator it = users_.begin();

	fileHandle_t f = -1;
	
	if(trap_FS_FOpenFile(g_userConfig.string, &f, FS_WRITE) < 0)
	{
		G_LogPrintf("WriteConfig: failed to open user config file.\n");
		return;
	}

	while(it != users_.end())
	{
		using namespace configConstants;
		trap_FS_Write(USER_BEGIN.c_str(), USER_BEGIN.length(), f);
		
	}
}
