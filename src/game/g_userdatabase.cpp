#include "g_userdatabase.h"

UserDatabase::UserDatabase() {

}

UserDatabase::~UserDatabase() {
    map<string, admin_user_t*>::iterator it = users_.begin();
    while(it != users_.end()) {
        delete it->second;
    }

    users_.clear();
}

UserDatabase::admin_user_t::admin_user_t(int lev, 
            string nam, string cmds,        
            string greet, string user, 
            string pw) 
{
    level = lev;
    name = nam;
    commands = cmds;
    greeting = greet;
    username = user;
    password = pw;
}

bool UserDatabase::addUser(string guid, int level,
        string name, string commands,
        string greeting, string username,
        string password) 
{
    // No checks related to the validity of guid etc. are done here
    // except that it must be 40 chars
    if(guid.length() != 40) {
        return false;
    }
    
    admin_user_t *new_user = 0;

    try {
        new_user = new admin_user_t(level, name, commands, 
            greeting, username, password);
    } catch(...) {
        return false;
    }

    if(users_.insert(std::make_pair(guid, new_user)).second == false) {
        return false;
    }

    // Add it to sqlite database.

    return true;

}