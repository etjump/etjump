// This database stores all the users, levels and bans
// They are read from the a file on InitGame() and when Readconfig is 
// called. If g_synchronizedbs is set to 1, readconfig will be called
// It won't be protected from dirty reads etc. but considering how rarely
// things are written into it, it should be fine. 

#ifndef g_database_h__
#define g_database_h__

#include "g_local.hpp"
#include "boost/shared_ptr.hpp"
#include <vector>
#include <string>

struct Level {
	int level;
	std::string name;
	std::string greeting;
	std::string commands;
};
typedef const Level* const LevelData;

struct User {
	int level;
	std::string name;
	std::string guid;
	std::string personalCommands;
	std::string personalGreeting;
	std::string personalTitle;
};
typedef const User* const UserData;

struct Ban {
	// TODO: bans
};
typedef const Ban* const BanData;

class Database {
public:
	Database();
	~Database();

	// New-commands are used to create a new level, user or ban to db
	// meaning creation and saving of them
	
	// NewLevel is used to add a new level to database.
	// ex: NewLevel(Level(5)); -> Add level 5 to DB.
	// just the integer is required, rest can be provided
	// later. Default value is empty for strings meaning level
	// has no greeting, no commands (except from previous level, which 
	// is possible to toggle on/off) and no name.
	bool NewLevel(Level level);

	// NewUser is used to add a new user to database.
	// ex: NewUser(User("Zero", "Zero's GUID", 5, "Personal commands"));
	// Just the GUID is required. Level is set to 0 if nothing is provided
	// Same with strings
	bool NewUser(User user);
	
	// NewBan is used to add a new ban to database.
	// TODO: entirely
	bool NewBan(Ban ban);

	// Save functions are used to save a level/user/ban to the
	// memory. Readconfig() uses these. 
	bool SaveLevel(const Level& level);
	bool SaveUser(const User& user);
	bool SaveBan(const Ban& ban);

	// Get functions are used to get level, user & ban data
	LevelData	GetLevel(int level) const;
	UserData	GetUser(const std::string& guid) const;
	BanData		GetBan() const;
private:
	std::vector<boost::shared_ptr<Level> > levels_;
	std::vector<boost::shared_ptr<User> > users_;
	std::vector<boost::shared_ptr<Ban> > bans_;

	bool usersSorted;
};

#endif // g_database_h__


