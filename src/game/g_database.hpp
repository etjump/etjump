// This database stores all the users, levels and bans
// They are read from the a file on InitGame() and when Readconfig is 
// called. If g_synchronizedbs is set to 1, readconfig will be called
// It won't be protected from dirty reads etc. but considering how rarely
// things are written into it, it should be fine. 

#ifndef g_database_h__
#define g_database_h__

#include "g_local.hpp"
#include "boost/shared_ptr.hpp"
#include <map>
#include <vector>
#include <string>

struct Level {
	Level(int level, const std::string& name,
		const std::string& greeting, const std::string& commands);
	Level();
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

struct UserData {
    int id;
	int level;
	std::string name;
	std::string personalCommands;
	std::string personalGreeting;
	std::string personalTitle;
};

typedef boost::shared_ptr<UserData> UserDataPtr;
typedef boost::shared_ptr<Level> LevelPtr;

typedef std::map<std::string, UserDataPtr>::iterator UserIterator;
typedef std::map<std::string, UserDataPtr>::const_iterator ConstUserIterator;
typedef std::vector<LevelPtr>::const_iterator ConstLevelIterator;

// typedef const User* const UserData;

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

    bool UpdateUserLevel(const std::string& guid, int newLevel);

	// Get functions are used to get level, user & ban data
    bool GetLevel(int level, ConstLevelIterator& it) const;

    bool LevelExists(int level) const;

    std::string LevelName(int level) const;

	// returns false if user could not be found
	bool GetUser(const std::string& guid, ConstUserIterator& it) const;
    bool GetUser(int id, UserIterator& it);

	BanData				GetBan() const;
	bool ReadUserConfig(gentity_t *ent);
	bool ReadLevelConfig(gentity_t *ent);

    // For debugging
    void PrintUsers();
private:
	void DefaultLevels();
	void WriteUserConfig();
	void WriteLevelConfig();
	std::vector<LevelPtr> levels_;
	std::map<std::string, UserDataPtr> users_;
	std::vector<boost::shared_ptr<Ban> > bans_;

	bool usersSorted_;
};

#endif // g_database_h__


