#ifndef G_USERDATABASE_H
#define G_USERDATABASE_H

#include "../sqlite/sqlite3pp.h"

#include <map>
#include <string>
#include <vector>
using std::vector;
using std::map;
using std::string;

extern "C" {
#include "g_local.h"
}

const string INFO_LINE = "| ID |  GUID  |  LEVEL  |      IP       | NAME\n";
const string HORIZONTAL_LINE = "--------------------------------------------------------------------\n";
const int GUID_LEN = 40;
const int MAX_USERNAME = 40;
const int MAX_COMMANDS = 128;
const int MAX_GREETING_LEN = 256;

class UserDatabase {
public:

    struct Level {
        Level();
        int value;
        bool inuse;
    };

    UserDatabase();
    ~UserDatabase();
    // For adding a new user to database + to users_ -map
    bool newUser(const string& guid, int level,
        const string& name, const string& commands,
        const string& greeting, const string& username,
        const string& password, const string& ip);
    bool readConfig();
    int userCount() const;
    bool userExists(const string& guid) const;
    bool getUser(const string& guid, int& level, string& name, string& commands,
                           string& greeting, string& username, string& password, int& userid);
    bool updateUser(const string& guid, int level, const string& name);
    bool updateUser(int id, Level level, string cmds, string greeting);
    void listUsers(const string& guid, int level, const string& name, const string& ip, int id, gentity_t *ent) const;
    bool setLevel(int id, int level);
    bool printUserinfo(int id, gentity_t *ent);
private:
    void clearDatabase();
    // Used to add a user to the users_ datastructure
    bool addUser(const string& guid, int level,
        const string& name, const string& commands,
        const string& greeting, const string& username,
        const string& password, const string& ip, int id = -1);
    struct admin_user_t { 
        // Great names here.. 
        admin_user_t(int lev, 
            string nam, string cmds,        
            string greet, string user, 
            string pw, string ipaddr, int id = -1);

        int userid;
        int level;
        string name;
        string commands;
        string greeting;
        string username;
        string password;
        string ip;
    };

    static bool foundUsersComparator(const std::pair<string, admin_user_t*>& lhs,
        const std::pair<string, admin_user_t*>& rhs);

    std::vector<std::pair<string, admin_user_t*> > findUsers(int id, const string& guid,
        int level, const string& name, const string& ip) const;

    map<string, admin_user_t*> users_;
    sqlite3pp::database db_;

};

#endif // G_USERDATABASE_H