#include "userdata.h"
#include "user.h"
#include "../g_local.hpp"
#include "../g_utilities.hpp"

#include <sqlite3.h>

UserData::UserData()
{
    highestId_ = -1;
    db_ = NULL;
    createUserTable_ = NULL;
    insertIntoUsers_ = NULL;
    selectAllUsers_ = NULL;
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

bool UserData::AddUserToDatabase( const std::string& guid, const User& user )
{
    // Make sure the prepared statement is ready to be (re-)executed.
    sqlite3_reset(insertIntoUsers_);

    // Bind the id to statement
    int rc = sqlite3_bind_int(insertIntoUsers_, 1, user.id);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind id to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    // Bind the guid & name to statement
    rc = sqlite3_bind_text(insertIntoUsers_, 2, guid.c_str(), 
        guid.length(), SQLITE_STATIC);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind guid to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_bind_text(insertIntoUsers_, 3, user.name.c_str(), 
        user.name.length(), SQLITE_STATIC);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind name to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_bind_text(insertIntoUsers_, 4, user.hwid.c_str(), 
        user.name.length(), SQLITE_STATIC);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind hwid to statement: (%d) %s\n",
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

bool UserData::Initialize()
{
    char dbFilename[MAX_QPATH] = "users.db";

    if(strlen(g_userConfig.string) > 0)
    {
        Q_strncpyz(dbFilename, g_userConfig.string, sizeof(dbFilename));
    }

    int rc = sqlite3_open(GetPath(dbFilename).c_str(), &db_);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't open database %s: (%d) %s\n",
            dbFilename, rc, sqlite3_errmsg(db_));
    }

    // Try to prepare the statements
    if(!PrepareStatements())
    {
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

            const char *text = 0;
            std::string guid;

            user->id = sqlite3_column_int(selectAllUsers_, 0);
            user->level = sqlite3_column_int(selectAllUsers_, 3);

            text = (const char*)(sqlite3_column_text(selectAllUsers_, 1));
            CharPtrToString(text, guid);

            text = (const char*)(sqlite3_column_text(selectAllUsers_, 2));
            CharPtrToString(text, user->hwid);

            text = (const char*)(sqlite3_column_text(selectAllUsers_, 4));
            CharPtrToString(text, user->name);

            text = (const char*)(sqlite3_column_text(selectAllUsers_, 5));
            CharPtrToString(text, user->commands);

            text = (const char*)(sqlite3_column_text(selectAllUsers_, 6));
            CharPtrToString(text, user->greeting);

            text = (const char*)(sqlite3_column_text(selectAllUsers_, 7));
            CharPtrToString(text, user->title);
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
    sqlite3_finalize(selectAllUsers_);

    sqlite3_close(db_);
    db_ = NULL;
    return true;

    return true;
}

bool UserData::PrepareStatements()
{
    // Result code
    int rc = 0;

    // Prepare create table statement
    rc = sqlite3_prepare_v2(db_, 
        "CREATE TABLE IF NOT EXISTS users"
        "("
        "id INTEGER PRIMARY KEY,"
        "guid VARCHAR(40) UNIQUE NOT NULL,"
        "hwid VARCHAR(40) NOT NULL,"
        "level INTEGER NOT NULL,"
        "name VARCHAR(36) NOT NULL,"
        "pcommands VARCHAR(255),"
        "pgreeting VARCHAR(255),"
        "ptitle VARCHAR(255)"
        ");", -1, &createUserTable_, 0);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing createUserTable statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    // Table needs to be created before anything else can be prepared
    CreateUserTable();

    // We can now finalize the statement as it is no longer needed
    sqlite3_finalize(createUserTable_);

    rc = sqlite3_prepare_v2(db_,
        "SELECT id, guid, level, name FROM users;", -1, &selectAllUsers_, 0);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing selectAllFromUsers statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    // Prepare insert into statement

    rc = sqlite3_prepare_v2(db_, 
        "INSERT INTO users "
        "(id, guid, level, name, pcommands, pgreeting, ptitle, hwid) "
        "VALUES (?, ?, 0, ?, '', '', '', ?);", -1, &insertIntoUsers_, 0);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing insertIntoUsers statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    return true;
}

bool UserData::CreateUserTable()
{
    // result code
    int rc = sqlite3_step(createUserTable_);
    if(rc != SQLITE_DONE)
    {
        G_LogPrintf("Creating table \"users\" failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    return true;
}

