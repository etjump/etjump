#include "users.h"
#include "g_utilities.h"

#include <algorithm>

UserDB::UserDB()
{
	isSorted = false;
	players_.clear();
	playersSortedByGuid_.clear();
}

UserDB::~UserDB()
{

}

UserDB::Client::Client()
{
	level = 0;
	dbUserID = 0;
	permissions.reset();
}

const char CREATE_USERS_TABLE[] =
	"CREATE TABLE IF NOT EXISTS users"
	"("
	"uid INTEGER PRIMARY KEY NOT NULL,"
	"name VARCHAR(36) NOT NULL DEFAULT 'ETPlayer',"
	"title VARCHAR(256) NOT NULL DEFAULT '',"
	"commands VARCHAR(256) NOT NULL DEFAULT '',"
	"greeting VARCHAR(256) NOT NULL DEFAULT '',"
	"guid VARCHAR(40) NOT NULL UNIQUE,"
	"hwid VARCHAR(40) NOT NULL,"
	"ip VARCHAR(16) NOT NULL,"
	"level INTEGER NOT NULL"
	");";

const char CREATE_ALIASES_TABLE[] =
	"CREATE TABLE IF NOT EXISTS aliases"
	"("
	"aliasid INTEGER PRIMARY KEY,"
	"alias VARCHAR(36) NOT NULL,"
	"uid INTEGER NOT NULL,"
	"FOREIGN KEY(uid) REFERENCES users(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
	"UNIQUE(uid, alias)"
	");";


// Called on game init. Will initialize the database connection
// and load data to memory
void UserDB::InitDatabase()
{
	// admin system is disabled
	if( !g_admin.integer )
	{
		return;
	}

	try
	{
		// Let's connect to the database
		db_.connect("etjump/etjump.db");
		
		// Create users table
		db_.execute(CREATE_USERS_TABLE);

		// Create aliases table
		db_.execute(CREATE_ALIASES_TABLE);
	} 
	catch ( sqlite3pp::database_error& e )
	{
		G_LogPrintf("SQLite3 ERROR: %s\n", e.what());
	}

	LoadUserDatabase();
}
// Called on game shutdown. Will close the database connection
// and free any reserved memory
void UserDB::Shutdown()
{
	// shared_ptr handles memory management
	players_.clear();
	playersSortedByGuid_.clear();
}

// Loads database to memory. We're dealing with a max of few thousand
// users, so reserving enough memory won't be a problem. SQLite perfor-
// mance might become one if we just used it for everything.
void UserDB::LoadUserDatabase()
{

	// Database connection is open. Load users into data structure
	sqlite3pp::query query(db_, "SELECT "
		"commands, greeting, guid, hwid, ip, name, title, level, uid"
		"FROM users;");

	Client tempClient;
	for(sqlite3pp::query::iterator it = query.begin();
		it != query.end(); it++)
	{
		// Create a temporary player from DB data
		CharPtrToString(it->get<const char*>(0), tempClient.commands);
		CharPtrToString(it->get<const char*>(1), tempClient.greeting);
		CharPtrToString(it->get<const char*>(2), tempClient.guid);
		CharPtrToString(it->get<const char*>(3), tempClient.hwid);
		CharPtrToString(it->get<const char*>(4), tempClient.ip);
		CharPtrToString(it->get<const char*>(5), tempClient.name);
		CharPtrToString(it->get<const char*>(6), tempClient.title);
		tempClient.level =		it->get<int>(7);
		tempClient.dbUserID =	static_cast<unsigned>(it->get<int>(8));

		// Add the player to database.
		AddExistingUserToDatabase(tempClient);
	}

	// Copy the player pointers to another vector which is sorted by
	// guids
	std::copy(players_.begin(), players_.end(), 
		std::back_inserter(playersSortedByGuid_));

	G_LogPrintf("ETJump: loaded %d users.\n", players_.size());
}

// Sets client guid to guid. Checks if guid has a level set, loads
// level data to client.
bool UserDB::SetGuid( gentity_t *ent, const std::string& guid )
{
	// Shouldn't happen but let's make sure either way
	if( guid.length() != GUID_LEN )
	{
		return false;
	}

	// set it on the temporary database
	clients_[ent->client->ps.clientNum].guid = guid;

	// Find a player with the same guid from db & load information
	ConstPlayerIter player = ConstFindPlayerByGUID(guid);
	// Found the player, get the data
	if(player != playersSortedByGuid_.end())
	{
		clients_[ent->client->ps.clientNum].commands
			= (*player)->commands;
		clients_[ent->client->ps.clientNum].dbUserID
			= (*player)->dbUserID;
		clients_[ent->client->ps.clientNum].greeting
			= (*player)->greeting;
		clients_[ent->client->ps.clientNum].hwid
			= (*player)->hwid;
		clients_[ent->client->ps.clientNum].level
			= (*player)->level;
		clients_[ent->client->ps.clientNum].name
			= (*player)->name;
		clients_[ent->client->ps.clientNum].title
			= (*player)->title;
	}
	// Couldn't find the player, add the data to db
	else 
	{
		Client tempClient;
		tempClient.commands = "";
		tempClient.dbUserID = players_.size() + 1;
		tempClient.greeting = "";
		tempClient.guid = guid;
		// FIXME: THIS IS NOT DONE!!!
	}

	return true;
}

// This function MUST BE used to add a client to database
// handles sorting stuff too
bool UserDB::AddExistingUserToDatabase( const Client& newUser )
{
	try
	{
		// We need to sort the database
		isSorted = false;

		boost::shared_ptr<Client> newUserPtr(new Client(newUser));

		players_.push_back(newUserPtr);
		playersSortedByGuid_.push_back(newUserPtr);
	}
	catch ( std::bad_alloc& e )
	{
		G_LogPrintf("UserDatabase: failed to allocate memory for"
			" a new user."
			" std::bad_alloc: %s\n", e.what());
		return false;
	}
	return true;
}

// This function MUST BE used to add a NEW user to database
bool UserDB::AddNewUserToDatabase( Client& newUser )
{
	try
	{
		// We need to sort the database after it was updated
		isSorted = false;

		// Set the database ID
		newUser.dbUserID = players_.size() + 1;

		boost::shared_ptr<Client> newUserPtr(new Client(newUser));

		players_.push_back(newUserPtr);
		playersSortedByGuid_.push_back(newUserPtr);

		// Add user to SQLite3 DB aswell
		try
		{
			sqlite3pp::command cmd(db_,
				"INSERT OR IGNORE INTO users (uid, guid, hwid, ip, "
				"level) VALUES (?1, ?2, ?3, ?4, ?5);");
			cmd.bind(1, static_cast<int>(newUserPtr->dbUserID));
			cmd.bind(2, newUserPtr->guid.c_str());
			cmd.bind(3, newUserPtr->hwid.c_str());
			cmd.bind(4, newUserPtr->ip.c_str());
			cmd.bind(5, newUserPtr->level);
			cmd.execute();
		}
		catch( sqlite3pp::database_error& dbError )
		{
			G_LogPrintf("SQLite3 ERROR: %s\n", dbError.what());
			return false;
		}
	}
	catch( std::bad_alloc& e )
	{
		G_LogPrintf("UserDatabase: failed to allocate memory for"
			" a new user."
			" std::bad_alloc: %s\n", e.what());
	}
}

// Used to sort clients based on db user id
bool UserDB::CompareUserIDs( const boost::shared_ptr<Client>& lhs, const unsigned& rhs )
{
	return lhs->dbUserID < rhs;
}

// Used to compare shared_ptr<Client> with a string
// GUIDs
bool UserDB::CompareGUIDs( const boost::shared_ptr<Client>& lhs, 
						   const std::string& rhs )
{ 
	return lhs->guid < rhs;
}

// Used to compare shared_ptr<Client> with another shared_ptr<Client>
// based on GUIDs
bool UserDB::CompareClientGUIDs( const boost::shared_ptr<Client>& lhs, 
								 const boost::shared_ptr<Client>& rhs )
{
	return lhs->guid < rhs->guid;
}

// Sorts database if it not sorted
void UserDB::SortDatabase()
{
 	if(!isSorted)
 	{
// 		std::sort(players_.begin(), players_.end(), CompareUserIDs);
		std::sort(playersSortedByGuid_.begin(), playersSortedByGuid_.end(),
 			CompareClientGUIDs);
 	}
	isSorted = true;
}

// Searches for the player from the database using binary search
// based on user id
// Returns end() iterator if player could not be found
UserDB::ConstPlayerIter UserDB::ConstFindPlayerByID( unsigned uid )
{
	// if the database is not sorted sort it now
	SortDatabase();
		
	// Database is now sorted and we can use binary search to find player
	ConstPlayerIter it = std::lower_bound(players_.begin(), 
		players_.end(), uid, CompareUserIDs);
	
	// Couldn't find player
	if((*it)->dbUserID != uid)
	{
		return players_.end();
	}
	
	// User must handle checking for valid values
	return it;
}

UserDB::PlayerIter UserDB::FindPlayerByID( unsigned uid )
{
	//if the database is not sorted sort it now
	SortDatabase();
	
	PlayerIter it = std::lower_bound(players_.begin(), 
		players_.end(), uid, CompareUserIDs);
	
	// Couldn't find player
	if((*it)->dbUserID != uid)
	{
		return players_.end();
	}
	
	return it;
}

// Searches for the player from the database using binary search
// based on guid
UserDB::ConstPlayerIter UserDB::ConstFindPlayerByGUID( const std::string& guid )
{
	// if the database is not sorted sort it now
	SortDatabase();

	ConstPlayerIter it = std::lower_bound(playersSortedByGuid_.begin(),
		playersSortedByGuid_.end(), guid, CompareGUIDs);

	// Couldn't find player
	if((*it)->guid != guid)
	{
		return playersSortedByGuid_.end();
	}

	return it;
}

UserDB::PlayerIter UserDB::FindPlayerByGUID( const std::string& guid )
{
	//if the database is not sorted sort it now
	SortDatabase();
	
	PlayerIter it = std::lower_bound(playersSortedByGuid_.begin(),
		playersSortedByGuid_.end(), guid, CompareGUIDs);
	
	// Couldn't find player
	if((*it)->guid != guid)
	{
		return playersSortedByGuid_.end();
	}

	return it;
}

