#include "g_userdatabase.h"
#include "g_utilities.h"
extern "C" {
#include "g_local.h"
}

UserDatabase::UserDatabase() {
}

UserDatabase::~UserDatabase() {
    map<string, admin_user_t*>::iterator it = users_.begin();
    while(it != users_.end()) {
        delete it->second;
        it++;
    }

    users_.clear();
    db_.disconnect();
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

bool UserDatabase::newUser(string& const guid, int level,
                           string& const name, string& const commands,
                           string& const greeting, string& const username,
                           string& const password) 
{
    if(!addUser(guid, level, name, commands, greeting, username, password)) {
        return false;
    }

    try {

        sqlite3pp::command cmd(db_, "INSERT INTO users (guid, level, \
                                    name, commands, greeting, username, \
                                    password) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7);");
        cmd.bind(1, guid.c_str());
        cmd.bind(2, level);
        cmd.bind(3, name.c_str());
        cmd.bind(4, commands.c_str());
        cmd.bind(5, greeting.c_str());
        cmd.bind(6, username.c_str());
        cmd.bind(7, password.c_str());
        cmd.execute();

    } catch( sqlite3pp::database_error& e ) {
        LogPrintln("ERROR: " + string(e.what()));
        return false;
    }


    return true;
}

bool UserDatabase::addUser(string& const guid, int level,
        string& const name, string& const commands,
        string& const greeting, string& const username,
        string& const password) 
{
    // No checks related to the validity of guid etc. are done here
    // except that it must be 40 chars
    if(guid.length() != 40) {
        return false;
    }

    // Make sure user doesn't exist on datastructure
    if(users_.find(guid) != users_.end()) {
        return false;
    }
    
    admin_user_t *new_user = 0;

    try {
        new_user = new admin_user_t(level, name, commands, 
            greeting, username, password);
    } catch(...) {
        LogPrintln("USER DATABASE ERROR: Failed to allocate memory for a user");
        return false;
    }

    users_.insert(std::make_pair(guid, new_user));        

    return true;

}

bool UserDatabase::userExists(const string& guid) const {
    if(users_.find(guid) != users_.end()) {
        return true;
    }
    return false;
}

bool UserDatabase::getUser(const string& guid, int& level, string& name, string& commands,
                           string& greeting, string& username, string& password) 
{
    map<string, admin_user_t*>::iterator it;
    it = users_.find(guid);

    // Should never happen
    if(it == users_.end()) {
        return false;
    }

    level = it->second->level;
    name = it->second->name;
    commands = it->second->commands;
    greeting = it->second->greeting;
    username = it->second->username;
    password = it->second->password;
}

void UserDatabase::clearDatabase() {
    map<string, admin_user_t*>::iterator it = users_.begin();
    while(it != users_.end()) {
        delete it->second;
        it++;
    }

    users_.clear();
    db_.disconnect();
}

const int GUID_LEN = 40;
const int MAX_USERNAME = 40;
const int MAX_COMMANDS = 128;
const int MAX_GREETING_LEN = 256;

bool UserDatabase::readConfig() {

    clearDatabase();

    string filename;
    char mod_folder[MAX_TOKEN_CHARS];

    trap_Cvar_VariableStringBuffer("fs_game", mod_folder, sizeof(mod_folder));

    if(strlen(mod_folder)) {
        filename = string(mod_folder) + "/" + string("etjump.db");
    } else {
        filename = "etjump/" + string("etjump.db");
    }

    string guid, name, commands, greeting, username, password;
    int level;

    try {
        db_.connect(filename.c_str());
    
        sqlite3pp::command cmd(db_, "CREATE TABLE IF NOT EXISTS users(guid varchar(40), level INTEGER, name varchar(36), commands varchar(128), greeting varchar(256), username varchar(40), password varchar(40));");
        cmd.execute();
    
        sqlite3pp::query query(db_, "SELECT * FROM users");
    
        for(sqlite3pp::query::iterator it = query.begin();
            it != query.end(); it++) 
        {

            (*it).getter() >> guid >> level >> name >> commands >> greeting >> username >> password;

            if(!addUser(guid, level, name, commands, greeting, username, password)) {
                LogPrintln("WARNING: failed to add a user.");
                continue;
            }

        }
    } catch( sqlite3pp::database_error& e ) {
        clearDatabase();
        LogPrintln("DATABASE ERROR: " + string(e.what()));
        return false;
    }

    return true;
}

int UserDatabase::userCount() const {
    return users_.size();
}

// Used for setlevel
bool UserDatabase::updateUser(const string& guid, int level, const string& name) {

    // Let's not add a user to DB if guid != 40 chars long
    if(guid.length() != 40) {
        return false;
    }

    map<string, admin_user_t*>::iterator it = users_.find(guid);

    if(it != users_.end()) {
        it->second->name = name;
        it->second->level = level;

        sqlite3pp::command cmd(db_, "UPDATE users WHERE guid='?1' SET name='?2', level='?3';");
        cmd.bind(1, guid.c_str());
        cmd.bind(2, name.c_str());
        cmd.bind(3, level);
        cmd.execute();

        return true;
    }

    return false;

}