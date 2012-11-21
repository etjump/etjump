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
    // For adding a new user to database + to users_ -map
    bool newUser(string& const guid, int level,
        string& const name, string& const commands,
        string& const greeting, string& const username,
        string& const password);
    bool readConfig();
    int userCount() const;
    bool userExists(const string& guid) const;
    bool getUser(const string& guid, int& level, string& name, string& commands,
                           string& greeting, string& username, string& password);
    bool updateUser(const string& guid, int level, const string& name);
private:
    void clearDatabase();
    // Used to add a user to the users_ datastructure
    bool addUser(string& const guid, int level,
        string& const name, string& const commands,
        string& const greeting, string& const username,
        string& const password);
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