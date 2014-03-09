#ifndef userdata_h__
#define userdata_h__

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include <sqlite3.h>

#include "iuserdata.h"

class UserData : public IUserData
{
public:
    UserData();
    ~UserData();

    virtual const User *GetUserData( const std::string& guid );
    virtual int CreateNewUser( const std::string& guid, const std::string& name, const std::string& hwid );
    virtual void UpdateLastSeen( int id, int seen );
    virtual void UpdateLevel( const std::string& guid, int level );
    virtual bool UserListData( int page, std::vector<std::string>& linesToPrint );
    virtual void UserIsOnline( const std::string& guid );
    virtual bool UpdateUser( const std::string& guid, const std::string& commands, const std::string& greeting, const std::string& title, int updated );

    // SQLite3 related functions
    bool Initialize();
    bool Shutdown();
    // End of SQLite3 functions
private:
    // SQLite3 related functions
    bool AddUserToDatabase( const std::string& guid, const User& user );
    bool PrepareStatements();
    bool PrepareQueries();
    bool PrepareUpdates();
    bool PrepareInserts();
    bool CreateUsersTable();

    // SQLite3 related variables
    sqlite3 *db_;
    // SQLite3 statements
    sqlite3_stmt *insertIntoUsers_;
    sqlite3_stmt *selectAllUsers_;
    sqlite3_stmt *updateUser_;
    sqlite3_stmt *updateLastSeen_;
    sqlite3_stmt *updateLevel_;
    // End of SQLite3

    int highestId_;
    std::map< std::string, boost::shared_ptr<User> > users_;

   
};

#endif // userdata_h__
