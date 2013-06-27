#include "g_local.hpp"
#include "g_sessiondb.hpp"
#include "g_utilities.h"
#include "g_database.hpp"
#include <boost/algorithm/string/replace.hpp>

// We have just one static clientDB in the game module. 
// It stores authentication information like GUID & HWID
// of clients.
SessionDB sessionDB;

// Same thing with User, Level & BanDB
Database adminDB;

void PrintGreeting( gentity_t * ent ) 
{
	std::string greeting = sessionDB.Greeting(ent);

	if(greeting.length() == 0)
	{
		return;
	}

	boost::replace_all(greeting, "[n]", ent->client->pers.netname);    

	ChatPrintAll(greeting);
}

bool ValidGuid(const std::string& guid);
// Called when "etguid" command is executed by the client
// Parses the command, checks for valid values and saves the 
// GUID to db.
void GuidReceived( gentity_t * ent ) 
{
	Arguments argv = GetArgs();
	// Get the userinfo for IP information etc.
	char userinfo[MAX_INFO_STRING] = "\0";
	trap_GetUserinfo(ent->client->ps.clientNum, 
		userinfo, sizeof(userinfo));
	// For userinfo value
	char *value = NULL;

	value = Info_ValueForKey(userinfo, "ip");

	// Command vector contains just etguid GUID, nothing else,
	// so let's not accept anything else.
	if(argv->size() != 2)
	{
		G_LogPrintf("Possible GUID spoof attempt(no GUID) by: %s. IP: %s.\n",
			ent->client->pers.netname, value);
		return;

	}

	// Make sure the GUID is valid
	if(!ValidGuid(argv->at(1)))
	{
		G_LogPrintf("Possible GUID spoof attempt(invalid GUID) by: %s. IP %s.\n",
			ent->client->pers.netname, value);
		return;

	}

	// Let's hash the GUID again.
	char *hashedGuid = G_SHA1(argv->at(1).c_str());
	if(!hashedGuid) 
	{
		G_LogPrintf("Error while trying to hash GUID.\n");
		return;

	}
	std::string guid = hashedGuid;

	// We now know that GUID is valid so we can happily save it to
	// client db
	// When we receive a GUID we also need to get the level of client
	// and the level data from the db
	// 
	// Get admin data
	ConstUserIterator userAdminData;
	if(adminDB.GetUser(hashedGuid, userAdminData)) 
	{
		// We now have the admin data and the GUID so we can save them all
		// to the temporary clientDB
		// 
		sessionDB.Set(ent, hashedGuid, userAdminData->second->level,
			userAdminData->second->name, userAdminData->second->personalCommands,
			userAdminData->second->personalGreeting, userAdminData->second->personalTitle);
	}
	else
	{
		// First time we saw the user, give the default level settings
		User user;
		user.guid = hashedGuid;
		user.level = 0;
		CharPtrToString(ent->client->pers.netname, user.name);
		// Add the user to DB and write the data to file
		adminDB.NewUser(user);
	
		// Add it to clientDB as well.
		sessionDB.Set(ent, hashedGuid, user.level, user.name, user.personalCommands,
			user.personalGreeting, user.personalTitle);
	}

	// Print greeting if it's needed 
	if(ent->client->sess.needGreeting)
	{
		PrintGreeting(ent);
	}

	// We've now added client to permanent & session database 
	// and printed greeting. 

}

// TODO: make one function for both cgame & game instead of one for each.
bool ValidGuid(const std::string& guid)
{
	// Make sure length is correct
	if(guid.length() != 40)
	{
		// Length was not correct
		return false;

	}

	// Check each character for illegal chars
	for(int i = 0; i < guid.length(); i++)
	{
		// It's a sha hashed guid converted to hexadecimal. If values
		// greater than F are found in it, it's a fake guid.
		if(guid[i] < '0' || guid[i] > 'F')
		{
			return false;

		}

	}

	return true;

}

void PrintUserDB()
{
    adminDB.PrintUsers();
}