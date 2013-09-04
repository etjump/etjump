#ifndef g_sqlite_h__
#define g_sqlite_h__

#include <string>
#include <boost/shared_ptr.hpp>
#include "sqlite3.h"

class Database;
struct UserData_s;

class SQLite 
{
public:
    SQLite();
    ~SQLite();

    // Opens a database connection, prepares statements and handles the 
    // creation of tables.
    bool Init();
    // Finalizes the statements and closes the database connection.
    bool Shutdown();
    // Adds a new user to database
    bool AddNewUser(int id, const std::string& guid, const std::string& name);
    // Sets the level of an existing user
    bool SetLevel(gentity_t *ent, int level);
    // Updates user attributes
    bool UpdateUser(int id, int level, const std::string& cmds, const std::string& greeting,
        const std::string& title);
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
    sqlite3_stmt *edituserUpdate_;
};

#endif // g_database_h__