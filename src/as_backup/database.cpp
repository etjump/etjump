#include "database.hpp"
#include "g_sessiondb.hpp"

Database::Database()
{

}

Database::~Database()
{

}

void Database::ClientConnect( gentity_t *ent, const std::string& guid )
{
    ConstUserIterator it = users_.find(guid);

    // Check if client is already in the database
    if(it != users_.end())
    {
        // User is already in the DB
        sessionDB.Set(ent, guid, it->second->level, it->second->name,
            it->second->personalCmds, it->second->personalGreeting,
            it->second->personalTitle);
    } else
    {
        // User is not in the DB
        UserData newUser = UserData(new UserData_s);
        newUser->hwid = "";
        newUser->id = users_.size();
        newUser->level = 0;
        newUser->name = ent->client->pers.netname;
        newUser->personalCmds = "";
        newUser->personalGreeting = "";
        newUser->personalTitle = "";

        users_.insert(std::make_pair(guid, newUser));
        // MUST use newUser->id because users_.size() is already +1
        db_.AddNewUser(newUser->id, guid, newUser->name);
        // Also add it to the sessionDB
        sessionDB.Set(ent, guid, newUser->level, newUser->name,
            newUser->personalCmds, newUser->personalGreeting, 
            newUser->personalTitle);
    }
}

void Database::SetLevel( gentity_t *ent, int level )
{
    UserIterator it = users_.find(sessionDB.Guid(ent));

    if(it != users_.end())
    {

    } else
    {
        // This should never happen
    }
}

void Database::Init()
{
    db_.Init();
}

void Database::Shutdown()
{
    db_.Shutdown();
}
