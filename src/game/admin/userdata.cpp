#include "userdata.h"
#include "user.h"
#include "../g_local.hpp"
#include "../g_utilities.hpp"

#include <sqlite3.h>

UserData::UserData()
{
    highestId_ = -1;
    db_ = NULL;
    insertIntoUsers_ = NULL;
    selectAllUsers_ = NULL;
    updateUser_ = NULL;
    updateLastSeen_ = NULL;
    updateLevel_ = NULL;
}

UserData::~UserData()
{

}

const User * UserData::GetUserData( const std::string& guid )
{
    std::map< std::string, boost::shared_ptr<User> >::const_iterator it =
        users_.find( guid );

    if(it != users_.end())
    {
        return it->second.get();
    }
    return NULL;
}

int UserData::CreateNewUser( const std::string& guid, const std::string& name, const std::string& hwid )
{
    boost::shared_ptr<User> newUser( new User() );

    newUser->name = name;
    newUser->hwid = hwid;
    newUser->id = highestId_ + 1;
    newUser->lastSeen = 0;

    users_.insert( std::make_pair(guid, newUser) );

    if(AddUserToDatabase( guid, *newUser ))
    {
        highestId_++;
    } else
    {
        G_LogPrintf("Error while trying to add user to database.\n");
        return -1;
    }

    return newUser->id;
}

/*
"INSERT INTO users "
        "(id, guid, level, name, pcommands, pgreeting, ptitle, hwid) "
        "VALUES (?, ?, 0, ?, '', '', '', ?);"
*/

bool UserData::AddUserToDatabase( const std::string& guid, const User& user )
{
    int index = 1;
    // Make sure the prepared statement is ready to be (re-)executed.
    sqlite3_reset(insertIntoUsers_);

    // Bind the id to statement
    int rc = sqlite3_bind_int(insertIntoUsers_, index++, user.id);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind id to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    // Bind the guid & name to statement
    rc = sqlite3_bind_text(insertIntoUsers_, index++, guid.c_str(), 
        guid.length(), SQLITE_STATIC);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind guid to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_bind_text(insertIntoUsers_, index++, user.name.c_str(), 
        user.name.length(), SQLITE_STATIC);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind name to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_bind_text(insertIntoUsers_, index++, user.hwid.c_str(), 
        user.hwid.length(), SQLITE_STATIC);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind hwid to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_bind_int(insertIntoUsers_, index++, user.lastSeen);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind seen to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_step(insertIntoUsers_);
    if(rc != SQLITE_DONE)
    {
        G_LogPrintf("Failed to insert a new user to database: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    // No need to clear bindings as we will replace them later
    return true;
}

void UserData::UpdateLastSeen(int id, int seen)
{
    int index = 1;
    int rc = 0;

    for(std::map<std::string, boost::shared_ptr<User> >::iterator it = users_.begin(); it != users_.end(); it++)
    {
        if(it->second->id == id)
        {
            it->second->lastSeen = seen;
        }
    }

    sqlite3_reset(updateLastSeen_);

    rc = sqlite3_bind_int(updateLastSeen_, index++, seen);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind seen to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return;
    }

    rc = sqlite3_bind_int(updateLastSeen_, index++, id);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind id to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return;
    }

    rc = sqlite3_step(updateLastSeen_);
    if(rc != SQLITE_DONE)
    {
        G_LogPrintf("Failed to update last seen of %d: (%d) %s\n",
            id, rc, sqlite3_errmsg(db_));
        return;
    }
}

void UserData::UpdateLevel(std::string const& guid, int level)
{
    std::map<std::string, boost::shared_ptr<User> >::iterator it = 
        users_.find(guid);

    if(it == users_.end())
    {
        G_LogPrintf("ERROR: couldn't find user with guid: %s\n", guid.c_str());
        return;
    }

    it->second->level = level;

    sqlite3_reset(updateLevel_);

    int index = 1;
    int rc = sqlite3_bind_int(updateLevel_, index++, level);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind level to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return;
    }

    rc = sqlite3_bind_int(updateLevel_, index++, it->second->id);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind id to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return;
    }

    rc = sqlite3_step(updateLevel_);
    if(rc != SQLITE_DONE)
    {
        G_LogPrintf("Failed to update level of %d: (%d) %s\n",
            it->second->id, rc, sqlite3_errmsg(db_));
        return;
    }
}

bool UserData::Initialize()
{
    char databaseDefaultName[MAX_QPATH] = "users.db";

    if(strlen(g_userConfig.string) > 0)
    {
        Q_strncpyz(databaseDefaultName, g_userConfig.string, sizeof(databaseDefaultName));
    }

    int rc = sqlite3_open(GetPath(databaseDefaultName).c_str(), &db_);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't open database %s: (%d) %s\n",
            databaseDefaultName, rc, sqlite3_errmsg(db_));
        return false;
    }

    // Try to prepare the statements
    if(!PrepareStatements())
    {
        sqlite3_close(db_);
        return false;
    }

    boost::shared_ptr< User > user;
    while(true)
    {
        try
        {
        	user = boost::shared_ptr< User >( new User() );
        }
        catch( std::bad_alloc& e )
        {
            Shutdown();
            G_Error("Failed to allocate memory for a user.\n");
        }

        rc = sqlite3_step(selectAllUsers_);
        if(rc == SQLITE_ROW)
        {
            /*
             * id
             * guid
             * hwid
             * level
             * name
             * pcommands
             * pgreeting
             * ptitle
             */

            int index = 0;
            const char *text = 0;
            std::string guid;
            user->id = sqlite3_column_int(selectAllUsers_, index++);

            text = (const char*)(sqlite3_column_text(selectAllUsers_, index++));
            CharPtrToString(text, guid);

            text = (const char*)(sqlite3_column_text(selectAllUsers_, index++));
            CharPtrToString(text, user->hwid);

            user->level = sqlite3_column_int(selectAllUsers_, index++);

            text = (const char*)(sqlite3_column_text(selectAllUsers_, index++));
            CharPtrToString(text, user->name);

            text = (const char*)(sqlite3_column_text(selectAllUsers_, index++));
            CharPtrToString(text, user->commands);

            text = (const char*)(sqlite3_column_text(selectAllUsers_, index++));
            CharPtrToString(text, user->greeting);

            text = (const char*)(sqlite3_column_text(selectAllUsers_, index++));
            CharPtrToString(text, user->title);

            user->lastSeen = sqlite3_column_int(selectAllUsers_, index++);
            
            if(highestId_ < user->id)
            {
                highestId_ = user->id;
            }

            users_.insert( std::make_pair(guid, user) );
        } else if(rc == SQLITE_DONE)
        {
            break;
        } else
        {
            G_LogPrintf("Couldn't read all users from database.\n");
            return false;
        }

    }

    return true;
}

bool UserData::Shutdown()
{
    sqlite3_finalize(insertIntoUsers_);
    insertIntoUsers_ = NULL;
    sqlite3_finalize(selectAllUsers_);
    selectAllUsers_ = NULL;
    sqlite3_finalize(updateUser_);
    updateUser_ = NULL;
    sqlite3_finalize(updateLastSeen_);
    updateLastSeen_ = NULL;
    sqlite3_finalize(updateLevel_);
    updateLevel_ = NULL;

    sqlite3_close(db_);
    db_ = NULL;
    return true;
}

bool UserData::PrepareStatements()
{
    // Table needs to be created before anything else can be prepared
    if(!CreateUsersTable())
    {
        return false;
    }

    if(!PrepareQueries())
    {
        return false;
    }

    if(!PrepareUpdates())
    {
        return false;
    }

    if(!PrepareInserts())
    {
        return false;
    }

    return true;
}

bool UserData::PrepareQueries()
{
    int rc = sqlite3_prepare_v2(db_,
        "SELECT id, guid, hwid, level, name, pcommands, pgreeting, ptitle, seen FROM users;", -1, &selectAllUsers_, 0);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing selectAllFromUsers statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    return true;
}

bool UserData::PrepareUpdates()
{
    int rc = sqlite3_prepare_v2(db_, 
        "UPDATE users SET pcommands=?, pgreeting=?, ptitle=?, seen=? WHERE id=?;",
        -1, &updateUser_, 0);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing updateUser statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_prepare_v2(db_,
        "UPDATE users SET seen=? WHERE id=?;",
        -1, &updateLastSeen_, 0);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing updateLastSeen statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_prepare_v2(db_,
        "UPDATE users SET level=? WHERE id=?;",
        -1, &updateLevel_, 0);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing updateLevel statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    return true;
}

bool UserData::PrepareInserts()
{
    int rc = sqlite3_prepare_v2(db_, 
        "INSERT INTO users "
        "(id, guid, level, name, pcommands, pgreeting, ptitle, hwid, seen) "
        "VALUES (?, ?, 0, ?, '', '', '', ?, ?);", -1, &insertIntoUsers_, 0);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing insertIntoUsers statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    return true;
}

bool UserData::CreateUsersTable()
{
    sqlite3_stmt *createUserTable = NULL;
    // result code
    int rc = sqlite3_prepare_v2(db_, 
        "CREATE TABLE IF NOT EXISTS users"
        "("
        "id INTEGER PRIMARY KEY,"
        "guid VARCHAR(40) UNIQUE NOT NULL,"
        "hwid VARCHAR(40) NOT NULL,"
        "level INTEGER NOT NULL,"
        "name VARCHAR(36) NOT NULL,"
        "pcommands VARCHAR(255),"
        "pgreeting VARCHAR(255),"
        "ptitle VARCHAR(255),"
        "seen INTEGER"
        ");", -1, &createUserTable, 0);
    
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing createUserTable statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_step(createUserTable);
    if(rc != SQLITE_DONE)
    {
        G_LogPrintf("Creating table \"users\" failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    sqlite3_finalize(createUserTable);

    return true;
}

