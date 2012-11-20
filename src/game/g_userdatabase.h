#ifndef G_USERDATABASE_H
#define G_USERDATABASE_H

#include "../sqlite/sqlite3pp.h"

#include <map>
#include <string>
using std::map;
using std::string;

class UserDatabase {
public:

    UserDatabase();
    ~UserDatabase();
    // For adding a new user to db+memory
    bool newUser(string guid, int level,
        string name, string commands,
        string greeting, string username,
        string password);
    bool readConfig();
    int userCount() const;
    bool userExists(const string& guid) const;
    void getUser(const string& guid, int& level, string& name, string& commands,
                           string& greeting, string& username, string& password);
private:
    void clearDatabase();
    // For readconfig
    bool addUser(string guid, int level,
        string name, string commands,
        string greeting, string username,
        string password);
    bool updateUser(string guid, int level, string name, string commands,
        string greeting, string username, string password);
    struct admin_user_t { 
        // Great names here.. 
        admin_user_t(int lev, 
            string nam, string cmds,        
            string greet, string user, 
            string pw);

        int level;
        string name;
        string commands;
        string greeting;
        string username;
        string password;
    };

    map<string, admin_user_t*> users_;
    sqlite3pp::database db_;

};

#endif // G_USERDATABASE_H