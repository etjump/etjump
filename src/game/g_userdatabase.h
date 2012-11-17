#ifndef G_USERDATABASE_H
#define G_USERDATABASE_H

#include <map>
#include <string>
using std::map;
using std::string;

class UserDatabase {
public:

    UserDatabase();
    ~UserDatabase();

    bool addUser(string guid, int level,
        string name, string commands,
        string greeting, string username,
        string password);

private:
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

};

#endif // G_USERDATABASE_H