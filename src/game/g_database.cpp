#include "g_database.hpp"
#include "g_utilities.hpp"
#include <boost/format.hpp>
#include <vector>
#include "g_sqlite.hpp"
#include "g_levels.hpp"
#include "g_sessiondb.hpp"

SQLite Database::udb_;
LevelDatabase Database::ldb_;
int Database::highestId_;
std::map<std::string, UserData> Database::users_;

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
    udb_.Init();
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
        Session::ClientConnect(ent, it->second->id, guid, 
            it->second->level, 
            it->second->name,
            ldb_.Permissions(it->second->level), 
            it->second->personalCmds, 
            greeting,
            it->second->personalTitle);
    } else
    {
        UserData newUser;
        try
        {
        	newUser = UserData(new UserData_s);
        }
        catch( std::bad_alloc& e )
        {
            G_LogPrintf("Failed to allocate memory for a new user.\n");
            return;
        }
        // User is not in the DB
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
        Session::ClientConnect(ent, newUser->id, guid, newUser->level, newUser->name,
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
    
    int level = Session::Level(ent);
    ChatPrintAll(va("^3admintest: ^7%s^7 is a level %d user (%s^7)",
        ent->client->pers.netname, level, ldb_.Name(level).c_str()));
}

    void Database::PrintFinger( gentity_t *ent, gentity_t *target )
{
    int level = Session::Level( target );

    ChatPrintAll((boost::format("^3finger:^7 %s ^7(%s^7) is a level %d user (%s^7)") % target->client->pers.netname %
        Session::Name(target) % level % ldb_.Name(level)).str());
}

bool Database::SetLevel( gentity_t *ent, gentity_t *target, int level )
{
    if(!ldb_.LevelExists(level))
    {
        ChatPrintTo(ent, "^3setlevel: ^7level does not exist.");
        return false;
    }

    udb_.SetLevel(target, level);
    UserIter it = users_.find(Session::Guid(target));
    if(it == users_.end())
    {
        // Should never happen
        ChatPrintTo(ent, "^3setlevel: ^7couldn't find target's guid. Are you sure target is not connecting?");
        return false;
    }
    it->second->level = level;
    Session::SetLevel(target, level, ldb_.Permissions(level), it->second->personalCmds);
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
            if(it->second->level > Session::Level(ent))
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
    return true;
}

void Database::UpdateUserByID( gentity_t *ent, int id, int updated, 
                              int level, const std::string& commands, 
                              const std::string& greeting, 
                              const std::string& title )
{
    UserIter user = users_.begin();
    for(; user != users_.end(); user++)
    {
        if(user->second->id == id)
        {
            ChatPrintTo(ent, "^3system: ^7updated user with id=" + IntToString(user->second->id));
            if(ent)
            {
                ChatPrintTo(ent, "^3system: ^7check console for more information.");
            }
            BeginBufferPrint();

            if(updated & UPDATED_LEVEL)
            {
                user->second->level = level;
                BufferPrint(ent, "Updated level: " + IntToString(level) + "\n");
            }

            if(updated & UPDATED_COMMANDS)
            {
                user->second->personalCmds = commands;
                BufferPrint(ent, "Updated personal commands: " + commands + "\n");
            }

            if(updated & UPDATED_GREETING)
            {
                user->second->personalGreeting = greeting;
                BufferPrint(ent, "Updated personal greeting: " + greeting + "\n");
            }

            if(updated & UPDATED_TITLE)
            {
                user->second->personalTitle = title;
                BufferPrint(ent, "Updated personal title: " + title + "\n");
            }
            udb_.UpdateUser(user->second->id, 
                user->second->level, 
                user->second->personalCmds,
                user->second->personalGreeting, 
                user->second->personalTitle);
            FinishBufferPrint(ent, false);

            // personal and level commands must be given always
            Session::UpdateUser(user->second->id, updated, level, 
                ldb_.Permissions(level), user->second->personalCmds, greeting, title);
            return;
        }
    }
    ChatPrintTo(ent, "^3system: ^7couldn't find user with ID=" + IntToString(id) + "." );
}

void Database::UpdateUserByGUID( gentity_t *ent, const std::string& guid, 
                                int updated, int level, 
                                const std::string& commands, 
                                const std::string& greeting, 
                                const std::string& title )
{
    UserIter user = users_.lower_bound(guid);
    if(user == users_.end())
    {
        ChatPrintTo(ent, "^3system: ^7couldn't find user with GUID=" + guid + ".");
        return;
    }

    std::vector<UserIter> matchingUsers;

    while(user != users_.end())
    {
        // Just in case
        if(user->first.length() < guid.length())
        {
            break;
        }

        if(user->first.compare(0, guid.length(), guid) == 0)
        {
            matchingUsers.push_back(user);
        }
        user++;
    }

    // should never happen
    if(matchingUsers.size() == 0)
    {
        ChatPrintTo(ent, "^3system: ^7couldn't find user with GUID=" + guid + ".");
        return;
    } else if(matchingUsers.size() == 1)
    {
        ChatPrintTo(ent, "^3system: ^7updated user with GUID=" + guid);
        if(ent)
        {
            ChatPrintTo(ent, "^3system: ^7check console for more information.");
        }
        BeginBufferPrint();
        
        if(updated & UPDATED_LEVEL)
        {
            matchingUsers[0]->second->level = level;
            BufferPrint(ent, "Updated level: " + IntToString(level) + "\n");
        }

        if(updated & UPDATED_COMMANDS)
        {
            matchingUsers[0]->second->personalCmds = commands;
            BufferPrint(ent, "Updated personal commands: " + commands + "\n");
        }

        if(updated & UPDATED_GREETING)
        {
            matchingUsers[0]->second->personalGreeting = greeting;
            BufferPrint(ent, "Updated personal greeting: " + greeting + "\n");
        }

        if(updated & UPDATED_TITLE)
        {
            matchingUsers[0]->second->personalTitle = title;
            BufferPrint(ent, "Updated personal title: " + title + "\n");
        }
        udb_.UpdateUser(matchingUsers[0]->second->id, 
            matchingUsers[0]->second->level, 
            matchingUsers[0]->second->personalCmds,
            matchingUsers[0]->second->personalGreeting, 
            matchingUsers[0]->second->personalTitle);
        FinishBufferPrint(ent, false);
        // personal and level commands must be given always
        Session::UpdateUser(matchingUsers[0]->second->id, updated, level, 
            ldb_.Permissions(level), matchingUsers[0]->second->personalCmds, greeting, title);
    } else {
        ChatPrintTo(ent, "^3system: ^7multiple matching users. Check console for more information.");
        BeginBufferPrint();
        BufferPrint(ent, "Please use \"-id\" switch to specify target user.\n");
        BufferPrint(ent, "Matching users:\n");
        BufferPrint(ent, "ID   |GUID    |Name                                \n");
        for(std::vector<UserIter>::const_iterator it = matchingUsers.begin();
            it != matchingUsers.end(); it++)
        {
            BufferPrint(ent, (boost::format("%-5.d|%-8.8s|%-.36s\n") % (*it)->second->id % (*it)->first % (*it)->second->name).str());
        }
        FinishBufferPrint(ent, false);
    }
}

bool Database::LevelExists( int level )
{
    return ldb_.LevelExists(level);
}

void Database::AddLevel( gentity_t *ent, int level )
{
    if(ldb_.LevelExists(level))
    {
        ChatPrintTo(ent, "^3system: ^7level already exists.");
        return;
    }

    ldb_.AddLevel(level);
    ChatPrintTo(ent, "^3addlevel: ^7added level " + IntToString(level) + ".");
}

void Database::AddLevel( gentity_t *ent, int level, const std::string& commands, const std::string& greeting, const std::string& title )
{
    if(ldb_.LevelExists(level))
    {
        ChatPrintTo(ent, "^3system: ^7level already exists.");
        return;
    }

    ldb_.AddLevel(level, commands, greeting, title);
    ChatPrintTo(ent, "^3addlevel: ^7added level " + IntToString(level) + ".");
}


