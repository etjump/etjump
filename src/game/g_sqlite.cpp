#include <string>
#include "g_local.hpp"
#include "g_sqlite.hpp"
#include "g_utilities.hpp"

SQLite::SQLite()
{
    db_ = NULL;
    // users-table related statements
    createUsersTable_ = NULL;
    selectAllFromUsers_ = NULL;
    insertIntoUsers_ = NULL;
    setlevelUpdate_ = NULL;

}

SQLite::~SQLite()
{

}

bool SQLite::Init()
{
    char dbName[] = "users.SQLite";

    // TODO: make it open the correct file based on g_dbFile
    int rc = sqlite3_open(GetPath(dbName).c_str(), &db_);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't open database %s: (%d) %s\n",
            dbName, rc, sqlite3_errmsg(db_));
    }

    // Try to prepare the statements
    if(!PrepareStatements())
    {
        return false;
    }


    UserData temp;
    while(true)
    {
        try
        {
        	temp = UserData(new UserData_s);
        }
        catch( std::bad_alloc& e )
        {
            G_LogPrintf("Failed to allocate memory for a user.\n");
            return false;
        }
        

        rc = sqlite3_step(selectAllFromUsers_);
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

            temp->id = sqlite3_column_int(selectAllFromUsers_, 0);
            temp->level = sqlite3_column_int(selectAllFromUsers_, 3);

            text = (const char*)(sqlite3_column_text(selectAllFromUsers_, 1));
            CharPtrToString(text, guid);

            text = (const char*)(sqlite3_column_text(selectAllFromUsers_, 2));
            CharPtrToString(text, temp->hwid);

            text = (const char*)(sqlite3_column_text(selectAllFromUsers_, 4));
            CharPtrToString(text, temp->name);

            text = (const char*)(sqlite3_column_text(selectAllFromUsers_, 5));
            CharPtrToString(text, temp->personalCmds);

            text = (const char*)(sqlite3_column_text(selectAllFromUsers_, 6));
            CharPtrToString(text, temp->personalGreeting);

            text = (const char*)(sqlite3_column_text(selectAllFromUsers_, 7));
            CharPtrToString(text, temp->personalTitle);

            Database::SaveUser(guid, temp);

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

bool SQLite::Shutdown()
{
    FinalizeStatements();
    sqlite3_close(db_);
    db_ = NULL;
    return true;
}

bool SQLite::PrepareStatements()
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
        ");", -1, &createUsersTable_, 0);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing createTableUsers statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
    }

    // Table needs to be created before anything else can be prepared
    CreateUsersTable();

    // We can now finalize the statement as it is no longer needed
    sqlite3_finalize(createUsersTable_);

    // Prepare select all statement

    rc = sqlite3_prepare_v2(db_,
        "SELECT id, guid, hwid, level, name, pcommands, pgreeting, ptitle FROM users;", -1, &selectAllFromUsers_, 0);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing selectAllFromUsers statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
    }

    // Prepare insert into statement

    rc = sqlite3_prepare_v2(db_, 
        "INSERT INTO users "
        "(id, guid, level, name, pcommands, pgreeting, ptitle, hwid) "
        "VALUES (?, ?, 0, ?, '', '', '', 'a');", -1, &insertIntoUsers_, 0);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing insertIntoUsers statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
    }

    // Prepare setlevel update statement

    rc = sqlite3_prepare_v2(db_, 
        "UPDATE users SET level=?, name=? WHERE id=?;",
        -1, &setlevelUpdate_, 0);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Preparing setLevelUpdate statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
    }
    return true;
}

void SQLite::FinalizeStatements()
{
    sqlite3_finalize(insertIntoUsers_);
    sqlite3_finalize(selectAllFromUsers_);
    sqlite3_finalize(setlevelUpdate_);
}

bool SQLite::AddNewUser( int id, const std::string& guid, const std::string& name )
{
    // Make sure the prepared statement is ready to be (re-)executed.
    sqlite3_reset(insertIntoUsers_);

    // Bind the id to statement
    int rc = sqlite3_bind_int(insertIntoUsers_, 1, id);
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


    rc = sqlite3_bind_text(insertIntoUsers_, 3, name.c_str(), 
        name.length(), SQLITE_STATIC);

    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind name to statement: (%d) %s\n",
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

bool SQLite::CreateUsersTable()
{
    // result code
    int rc = sqlite3_step(createUsersTable_);
    if(rc != SQLITE_DONE)
    {
        G_LogPrintf("Creating table \"users\" failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    return true;
}

bool SQLite::SetLevel( gentity_t *ent, int level )
{
    sqlite3_reset(setlevelUpdate_);

    int rc = sqlite3_bind_int(setlevelUpdate_, 3, Session::ID(ent));
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind id to setlevel query: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_bind_text(setlevelUpdate_, 2, ent->client->pers.netname,
        strlen(ent->client->pers.netname), 0);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind name to setlevel query: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_bind_int(setlevelUpdate_, 1, level);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't bind level to setlevel query: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_step(setlevelUpdate_);
    if(rc != SQLITE_DONE)
    {
        G_LogPrintf("Couldn't execute setlevel query: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    return true;
}
