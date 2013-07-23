#include "g_database.hpp"
#include "g_utilities.hpp"

UserData_s::UserData_s()
{
    this->id = -1;
    this->level = 0;
}

Database::Database()
{
    highestId_ = 0;
}

Database::~Database()
{

}

void Database::Init()
{
    users_.clear();
    // Hack, couldn't really figure this out @ 3am
    udb_.Init( *this );
    ldb_.ReadLevels();
}

void Database::Shutdown()
{
    udb_.Shutdown();
    users_.clear();
}

void Database::ClientGuidReceived( gentity_t *ent, const std::string& guid )
{
    ConstUserIter it = users_.find(guid);

    // Check if client is already in the database
    if(it != users_.end())
    {
        std::string greeting;
        if(it->second->personalGreeting.length() == 0)
        {
            greeting = ldb_.Greeting(it->second->level);
        } else
        {
            greeting = it->second->personalGreeting;   
        }

        // User is already in the DB
        sessionDB.Set(ent, it->second->id, guid, 
            it->second->level, 
            it->second->name,
            ldb_.Permissions(it->second->level), 
            it->second->personalCmds, 
            greeting,
            it->second->personalTitle);
    } else
    {
        // User is not in the DB
        UserData newUser = UserData(new UserData_s);
        newUser->hwid = "";
        newUser->id = highestId_ + 1;
        highestId_ = newUser->id;
        newUser->level = 0;
        newUser->name = ent->client->pers.netname;
        newUser->personalCmds = "";
        newUser->personalGreeting = "";
        newUser->personalTitle = "";

        users_.insert(std::make_pair(guid, newUser));
        // MUST use newUser->id because users_.size() is already +1
        udb_.AddNewUser(newUser->id, guid, newUser->name);
        // Also add it to the sessionDB
        sessionDB.Set(ent, newUser->id, guid, newUser->level, newUser->name,
            ldb_.Permissions(newUser->level), newUser->personalCmds, 
            ldb_.Greeting(newUser->level), 
            newUser->personalTitle);
    }
}

void Database::SaveUser( const std::string& guid, UserData u )
{
    users_.insert(std::make_pair(guid, u));

    if(highestId_ < u->id)
    {
        highestId_ = u->id;
    }
}

void Database::PrintAdminTest( gentity_t *ent )
{
    int level = sessionDB.Level(ent);
    ChatPrintAll(va("^3admintest: ^7%s^7 is a level %d user (%s^7)",
        ent->client->pers.netname, level, ldb_.Name(level).c_str()));
}

bool Database::SetLevel( gentity_t *ent, gentity_t *target, int level )
{
    if(!ldb_.LevelExists(level))
    {
        ChatPrintTo(ent, "^3setlevel: ^7level does not exist.");
        return false;
    }

    udb_.SetLevel(target, level);
    UserIter it = users_.find(sessionDB.Guid(target));
    if(it == users_.end())
    {
        // Should never happen
        ChatPrintTo(ent, "^3setlevel: ^7couldn't find target's guid. Are you sure target is not connecting?");
        return false;
    }
    it->second->level = level;
    sessionDB.SetLevel(target, level, ldb_.Permissions(level), it->second->personalCmds);
    ChatPrintAll(va("^3setlevel: ^7%s is now a level %d user (%s)", 
        target->client->pers.netname,
        level, 
        ldb_.Name(level).c_str()));
    return true;
}

bool Database::IDSetLevel( gentity_t *ent, int id, int level )
{
    ConstUserIter it = users_.begin();

    while(it != users_.end())
    {
        if(it->second->id == id)
        {
            if(it->second->level > sessionDB.Level(ent))
            {
                ChatPrintTo(ent, "^3setlevel: ^7you can't set the level of a fellow admin.");
                return false;
            }
        }
        it++;
    }

    if(it == users_.end())
    {
        ChatPrintTo(ent, "^3setlevel: ^7couldn't find user with id " + IntToString(id));
        return false;
    }

    if(!ldb_.LevelExists(level))
    {
        ChatPrintTo(ent, "^3setlevel: ^7level does not exist.");
        return false;
    }

    it->second->level = level;
    ChatPrintTo(ent, va("^3setlevel: ^7%s is now a level %d user (%s)", 
        it->second->name.c_str(),
        level, 
        ldb_.Name(level).c_str()));
}


