#ifndef g_sqlite_h__
#define g_sqlite_h__

#include <string>
#include "sqlite3.h"

// This has to be defined here because else we'd run into a circular
// include hell. Needed for the callback
// Users are stored in a map where GUID is the key
// which is a regular string
// The actual user data (dbid, hwid, level, name, pcmds, ptitle, pgreet)
// is stored in a struct

struct UserData_s
{
    UserData_s();
    int id;
    int level;
    std::string hwid;
    std::string name;
    std::string personalCmds;
    std::string personalTitle;
    std::string personalGreeting;
};

typedef boost::shared_ptr<UserData_s> UserData;

class SQLite 
{
public:
    SQLite();
    ~SQLite();

    // Opens a database connection, prepares statements and handles the 
    // creation of tables. Takes a callback function pointer, which 
    // is used to save users to memory as an argument
    bool Init(void (*callback)(const std::string&, UserData));
    // Finalizes the statements and closes the database connection.
    bool Shutdown();
    // Adds a new user to database
    bool AddNewUser(int id, const std::string& guid, const std::string& name);
    // Sets the level of an existing user
    bool SetLevel(gentity_t *ent, int level);
private:
    // This is called on Init() and will prepare all the statements for
    // later use. It will also create the tables.
    bool PrepareStatements();
    // This is called on PrepareStatements and will create the users
    // table
    bool CreateUsersTable();
    // This is called on Shutdown() and will finalize all the statements
    // in order to free all the allocated memory. Closes the connection
    // after finalizing statements
    void FinalizeStatements();
    // SQLite database access that is opened on initialization of the
    // database, which is done on G_InitGame(). It is only closed on 
    // G_ShutdownGame(). It is set to NULL by default.
    sqlite3 *db_;
    // All the prepared sqlite statements. They're set to NULL by default
    // users-database related statements
    sqlite3_stmt *createUsersTable_;
    sqlite3_stmt *selectAllFromUsers_;
    sqlite3_stmt *insertIntoUsers_;
    sqlite3_stmt *setlevelUpdate_;
};

#endif // g_database_h__