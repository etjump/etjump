#ifndef sqlite_h__
#define sqlite_h__

#include <string>
#include <boost/shared_ptr.hpp>
#include "g_admin.hpp"
#include "g_local.h"
#include "sqlite3.h"

class SQLite
{
public:
    SQLite();
    ~SQLite();

    static bool Init();
    // Finalizes the statements and closes the database connection.
    static bool Shutdown();
    // Adds a new user to database
    static bool AddNewUser(int id, const std::string& guid, const std::string& name);
    // Sets the level of an existing user
    static bool SetLevel(gentity_t *ent, int level);
private:
    // This is called on Init() and will prepare all the statements for
    // later use. It will also create the tables.
    static bool PrepareStatements();
    // This is called on PrepareStatements and will create the users
    // table
    static bool CreateUsersTable();
    // This is called on Shutdown() and will finalize all the statements
    // in order to free all the allocated memory. Closes the connection
    // after finalizing statements
    static void FinalizeStatements();
    // SQLite database access that is opened on initialization of the
    // database, which is done on G_InitGame(). It is only closed on 
    // G_ShutdownGame(). It is set to NULL by default.
    static sqlite3 *db_;
    // All the prepared sqlite statements. They're set to NULL by default
    // users-database related statements
    static sqlite3_stmt *createUsersTable_;
    static sqlite3_stmt *selectAllFromUsers_;
    static sqlite3_stmt *insertIntoUsers_;
    static sqlite3_stmt *setlevelUpdate_;
};

#endif // sqlite_h__
