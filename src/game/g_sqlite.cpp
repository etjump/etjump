#include <cstdio>
#include <string>
#include "g_sqlite.hpp"


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
    int rc = sqlite3_open(dbName, &db_);
    if(rc != SQLITE_OK)
    {
        printf("Couldn't open database %s: (%d) %s\n",
            dbName, rc, sqlite3_errmsg(db_));
    }

    // Try to prepare the statements
    if(!PrepareStatements())
    {
        return false;
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
        printf("Preparing createTableUsers statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
    }

    // Table needs to be created before anything else can be prepared
    CreateUsersTable();

    // We can now finalize the statement as it is no longer needed
    sqlite3_finalize(createUsersTable_);

    // Prepare select all statement

    rc = sqlite3_prepare_v2(db_,
        "SELECT * FROM users;", -1, &selectAllFromUsers_, 0);

    if(rc != SQLITE_OK)
    {
        printf("Preparing selectAllFromUsers statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
    }

    // Prepare insert into statement

    rc = sqlite3_prepare_v2(db_, 
        "INSERT INTO users "
        "(id, guid, level, name, pcommands, pgreeting, ptitle) "
        "VALUES (?, ?, 0, ?, '', '', '');", -1, &insertIntoUsers_, 0);

    if(rc != SQLITE_OK)
    {
        printf("Preparing insertIntoUsers statement failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
    }

    // Prepare setlevel update statement

    rc = sqlite3_prepare_v2(db_, 
        "UPDATE users SET level=?, name=? WHERE id=?;",
        -1, &setlevelUpdate_, 0);

    if(rc != SQLITE_OK)
    {
        printf("Preparing setLevelUpdate statement failed: (%d) %s\n",
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
        printf("Couldn't bind id to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    // Bind the guid & name to statement
    rc = sqlite3_bind_text(insertIntoUsers_, 2, guid.c_str(), 
        guid.length(), SQLITE_STATIC);

    if(rc != SQLITE_OK)
    {
        printf("Couldn't bind guid to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }


    rc = sqlite3_bind_text(insertIntoUsers_, 3, name.c_str(), 
        name.length(), SQLITE_STATIC);

    if(rc != SQLITE_OK)
    {
        printf("Couldn't bind name to statement: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }

    rc = sqlite3_step(insertIntoUsers_);
    if(rc != SQLITE_DONE)
    {
        printf("Failed to insert a new user to database: (%d) %s\n",
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
        printf("Creating table \"users\" failed: (%d) %s\n",
            rc, sqlite3_errmsg(db_));
        return false;
    }
    return true;
}

