#ifndef users_h__
#define users_h__

#include <string>
#include <vector>
#include <bitset>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <sqlite3pp.h>

#include "g_local.hpp"

// const unsigned MAX_CMDS = 256;
const unsigned GUID_LEN = 40;

class UserDB : public boost::noncopyable
{
public:
	UserDB();
	~UserDB();

	// Called on game init. Will initialize the database connection
	void InitDatabase();

	// Called on game shutdown. Will close the database connection
	// and free any reserved memory
	void Shutdown();

	// Sets client guid to guid. Checks if guid has a level set, loads
	// level data to client.
	bool SetGuid(gentity_t *ent, const std::string& guid);

private:

	struct Client 
	{
		Client();
		// Name of the player when he was first seen/last set
		std::string name;
		std::string guid;
		std::string hwid;
		std::string ip;
		// personal title instead of level title
		std::string title;
		// personal commands in addition to level commands
		std::string commands;
		// personal greeting instead of level greeting
		std::string greeting;
		// id to recognize user & perform updates to db
		unsigned dbUserID;
		// user level
		int level;
		// User permissions that include both personal & level
		std::bitset<MAX_CMDS> permissions;
	};

	typedef std::vector<boost::shared_ptr<Client> >::const_iterator 
		ConstPlayerIter;
	typedef std::vector<boost::shared_ptr<Client> >::iterator PlayerIter;

	// Used to sort clients based on db user id
	static bool CompareUserIDs(const boost::shared_ptr<Client>& lhs, const unsigned& rhs);
	// Used to compare shared_ptr<Client> with a string
	// GUIDs
	static bool CompareGUIDs( const boost::shared_ptr<Client>& lhs,
							  const std::string& rhs);
	// Used to compare shared_ptr<Client> with another shared_ptr<Client>
	// based on GUIDs
	static bool CompareClientGUIDs(const boost::shared_ptr<Client>& lhs,
		const boost::shared_ptr<Client>& rhs);

	// Currently active/inactive clients on server
	Client clients_[MAX_CLIENTS];

	// Sorts database if it not sorted
	void SortDatabase();

	// This function MUST BE used to add a client to database
	// handles sorting stuff too
	bool AddExistingUserToDatabase(const Client& User);

	// This function MUST BE used to add a NEW user to database
	bool AddNewUserToDatabase( Client& newUser );

	// Searches for the player from the database using binary search
	// based on user id
	ConstPlayerIter ConstFindPlayerByID(unsigned uid);
	PlayerIter		FindPlayerByID(unsigned uid);

	// Searches for the player from the database using binary search
	// based on guid
	ConstPlayerIter ConstFindPlayerByGUID(const std::string& guid);
	PlayerIter		FindPlayerByGUID(const std::string& guid);

	// if databases are sorted, don't need to sort them
	// check this before performing any searches to database
	bool isSorted;

	// Database of all players. Sorted by db id
	std::vector<boost::shared_ptr<Client> > players_;

	// Indices of all players sorted by GUID
	std::vector<boost::shared_ptr<Client> > playersSortedByGuid_;

	// Database handle that is opened on InitDatabase()
	sqlite3pp::database db_;

	// Loads database to memory. We're dealing with a max of few thousand
	// users, so reserving enough memory won't be a problem. SQLite perfor-
	// mance might become one if we just used it for everything.
	void LoadUserDatabase();

};

#endif // users_h__
