#include "sqlite.hpp"
#include "g_utilities.hpp"

sqlite3 SQLite::*db_;
sqlite3_stmt SQLite::*createUsersTable_;
sqlite3_stmt SQLite::*selectAllFromUsers_;
sqlite3_stmt SQLite::*insertIntoUsers_;
sqlite3_stmt SQLite::*setlevelUpdate_;

SQLite::SQLite()
{

}

SQLite::~SQLite()
{

}

bool SQLite::Init()
{
    if(g_userConfig.string[0] == 0)
    {
        return false;
    }

    int rc = sqlite3_open(g_userConfig.string, &db_);
    if(rc != SQLITE_OK)
    {
        G_LogPrintf("Couldn't open database %s: (%d) %s\n",
            g_userConfig.string, rc, sqlite3_errmsg(db_));
    }

    // Try to prepare the statements
    if(!PrepareStatements())
    {
        return false;
    }

    Admin::UserData temp;
    while(true)
    {
        temp = Admin::UserData(new Admin::UserData_s);

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

            // TODO: FIXME
            // SaveUser(guid, temp);

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

bool SQLite::AddNewUser(int id, const std::string& guid, const std::string& name)
{

}

bool SQLite::SetLevel(gentity_t *ent, int level)
{

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

bool SQLite::CreateUsersTable()
{

}

void SQLite::FinalizeStatements()
{
    sqlite3_finalize(insertIntoUsers_);
    sqlite3_finalize(selectAllFromUsers_);
    sqlite3_finalize(setlevelUpdate_);
}
