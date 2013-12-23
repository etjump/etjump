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
    virtual void CreateNewUser( const std::string& guid, const std::string& name, const std::string& hwid );

    // SQLite3 related functions
    bool Initialize();
    bool Shutdown();
    // End of SQLite3 functions
private:
    // SQLite3 related functions
    bool AddUserToDatabase( const std::string& guid, const User& name );
    void ReadUsersFromDatabase();
    bool PrepareStatements();
    bool CreateUserTable();

    // SQLite3 related variables
    sqlite3 *db_;
    // SQLite3 statements
    sqlite3_stmt *createUserTable_;
    sqlite3_stmt *insertIntoUsers_;
    sqlite3_stmt *selectAllUsers_;
    // End of SQLite3

    int highestId_;
    std::map< std::string, boost::shared_ptr<User> > users_;

   
};

#endif // userdata_h__
