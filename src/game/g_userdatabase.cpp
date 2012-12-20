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
            string pw, string ipaddr, int id) 
{
    if(id != -1) {
        userid = id;
    }
    level = lev;
    name = nam;
    commands = cmds;
    greeting = greet;
    username = user;
    password = pw;
    ip = ipaddr;
}

UserDatabase::Level::Level() {
    inuse = false;
    value = 0;
}

bool UserDatabase::newUser(const string& guid, int level,
                           const string& name, const string& commands,
                           const string& greeting, const string& username,
                           const string& password, const string& ip) 
{
    if(!addUser(guid, level, name, commands, greeting, username, password, ip, -1)) {
        return false;
    }

    try {

        sqlite3pp::command cmd(db_, "INSERT INTO users (guid, level, \
                                    name, commands, greeting, username, \
                                    password, ip) VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8);");
        cmd.bind(1, guid.c_str());
        cmd.bind(2, level);
        cmd.bind(3, name.c_str());
        cmd.bind(4, commands.c_str());
        cmd.bind(5, greeting.c_str());
        cmd.bind(6, username.c_str());
        cmd.bind(7, password.c_str());
        cmd.bind(8, ip.c_str());
        cmd.execute();

    } catch( sqlite3pp::database_error& e ) {
        LogPrintln("ERROR: " + string(e.what()));
        return false;
    }

    return true;
}

bool UserDatabase::addUser(const string& guid, int level,
        const string& name, const string& commands,
        const string& greeting, const string& username,
        const string& password, const string& ip, int id) 
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
        new_user = new admin_user_t(level, name, commands, greeting, username, password, ip, id);
        if(id == -1) {
            new_user->userid = -1;
        }
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
                           string& greeting, string& username, string& password, int& userid) 
{
    map<string, admin_user_t*>::iterator it;
    it = users_.find(guid);

    // Should never happen
    if(it == users_.end()) {
        return false;
    }

    userid = it->second->userid;
    level = it->second->level;
    name = it->second->name;
    commands = it->second->commands;
    greeting = it->second->greeting;
    username = it->second->username;
    password = it->second->password;
    return true;
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

    const char *guidptr = 0,
        *nameptr = 0, *commandsptr = 0,
        *greetingptr = 0, *usernameptr = 0, 
        *passwordptr = 0, *ipptr = 0;
    string guid, name, commands, greeting, username, password, ip;
    int level, id;

    try {
        db_.connect(filename.c_str());
    
        sqlite3pp::command cmd(db_, "CREATE TABLE IF NOT EXISTS users(userid INTEGER PRIMARY KEY AUTOINCREMENT, guid varchar(40), level INTEGER, name varchar(36), commands varchar(128), greeting varchar(256), username varchar(40), password varchar(40), ip varchar(15));");
        cmd.execute();
    
        sqlite3pp::query query(db_, "SELECT * FROM users");
    
        for(sqlite3pp::query::iterator it = query.begin();
            it != query.end(); it++) 
        {
            boost::tie(id) = (*it).get_columns<int>(0);
            boost::tie(guidptr, level, nameptr, commandsptr, greetingptr, usernameptr, passwordptr, ipptr) =
                (*it).get_columns<const char*, int, 
                const char*, const char*, 
                const char*, const char*, 
                const char*, const char*>(1, 2, 3, 4, 5, 6, 7, 8);
            if(!guidptr) {
                guid = "";
            } else {
                guid = guidptr;
            }

            if(!nameptr) {
                name = "";
            } else {
                name = nameptr;
            }

            if(!commandsptr) {
                commands = "";
            } else {
                commands = commandsptr;
            }

            if(!greetingptr) {
                greeting = "";
            } else {
                greeting = greetingptr;
            }

            if(!usernameptr) {
                username = "";
            } else {
                username = usernameptr;
            }

            if(!passwordptr) {
                password = "";
            } else {
                password = passwordptr;
            }

            if(!ipptr) {
                ip = "";
            } else {
                ip = ipptr;
            }


            if(!addUser(guid, level, name, commands, greeting, username, password, ip, id)) {
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

    try {

        if(it != users_.end()) {
            it->second->name = name;
            it->second->level = level;

            sqlite3pp::command cmd(db_, string("UPDATE users SET name='"+name+"', level='"+IntToString(level)+"' WHERE guid='"+guid+"';").c_str());
            
            cmd.execute();

            return true;
        }

    } catch( sqlite3pp::database_error& e ) {
        LogPrintln("DATABASE ERROR: " + string(e.what()));
        return false;
    }

    return false;

}

vector<std::pair<string, UserDatabase::admin_user_t*> > UserDatabase::findUsers
    (int id, const string& guid, int level, const string& name, const string& ip) const
{
    bool match_guid = false;
    bool match_level = false;
    bool match_ip = false;
    bool match_name = false;
    bool match_id = false;

    vector< std::pair<string, UserDatabase::admin_user_t*> > found_users;

    // ID is always unique

    if( id >= 0 ) {
        match_id = true;
    }

    else {

        if( !guid.empty() ) {
            match_guid = true;
        } 

        if( level ) {
            match_level = true;
        }

        if( !ip.empty() ) {
            match_ip = true;
        }

        if( !name.empty() ) {
            match_name = true;
        }

    }

    if( match_id ) {

        map<string, admin_user_t*>::const_iterator it = users_.begin();

        while( it != users_.end() ) {

            if( it->second->userid == id ) {
                found_users.push_back(make_pair(it->first, it->second));
                return found_users;
            }
            it++;
        }

    } 
    
    else {

        // If we're matching by guid, search for it first as searches can be performed in 
        // logaritmic time

        if( match_guid ) {

            map<string, admin_user_t*>::const_iterator first_matching = users_.lower_bound( guid );
            map<string, admin_user_t*>::const_iterator it = first_matching;

            if(first_matching == users_.end()) {
                return found_users;
            }

            while( it != users_.end() && it->first.compare(0, guid.length(), guid) == 0 ) {
            
                if( match_ip ) {
                    string::size_type asterix_pos = ip.find("*");
                    if( asterix_pos != string::npos ) {
                        // Match the string before *
                        string to_match = ip.substr(0, asterix_pos);
                        if( it->second->ip.compare(0, to_match.length(), to_match) != 0 ) {
                            it++;
                            continue;
                        }
                    }

                    else {
                        if( it->second->ip.compare(0, ip.length(), ip) != 0 ) {
                            it++;
                            continue;
                        }
                    }
                
                }

                if( match_level ) {
                    if( it->second->level != level ) {
                        it++;
                        continue;
                    }
                }

                if( match_name ) {
                    if( it->second->name.find( name ) == string::npos ) {
                        it++;
                        continue;
                    }
                }

                // Push the found user to vector
                found_users.push_back( make_pair(it->first, it->second) );
                it++;
            }

        } 

        else if( match_ip ) {

            map<string, admin_user_t*>::const_iterator it = users_.begin();

            while(it != users_.end()) {

                string::size_type asterix_pos = ip.find("*");
                if( asterix_pos != string::npos ) {
                    // Match the string before *
                    string to_match = ip.substr(0, asterix_pos);
                    if( it->second->ip.compare(0, to_match.length(), to_match) != 0 ) {
                        it++;
                        continue;
                    }
                }

                else {
                    if( it->second->ip != ip ) {
                        it++;
                        continue;
                    }
                }

                if( match_level ) {
                    if( it->second->level != level ) {
                        it++;
                        continue;
                    }
                }

                if( match_name ) {
                    if(it->second->name.find(name) == string::npos) {
                        it++;
                        continue;
                    }
                }

                found_users.push_back( make_pair(it->first, it->second) );

                it++;
            }

        }

        else if( match_level ) {

            map<string, admin_user_t*>::const_iterator it = users_.begin();

            while( it != users_.end() ) {
                if( it->second->level == level ) {

                    if(match_name) {
                        if(it->second->name.find(name) == string::npos) {
                            it++;
                            continue;
                        }
                    }

                    found_users.push_back( make_pair(it->first, it->second) );
                }

                it++;
            }
        }

        else if( match_name ) {

            map<string, admin_user_t*>::const_iterator it = users_.begin();

            while( it != users_.end() ) {
                if( it->second->name.find( name ) != string::npos ) {
                    found_users.push_back( make_pair(it->first, it->second) );
                }

                it++;
            }

        }
    }

    return found_users;
}

bool UserDatabase::foundUsersComparator(const std::pair<string, admin_user_t*>& lhs,
        const std::pair<string, admin_user_t*>& rhs) 
{
    return lhs.second->userid < rhs.second->userid;
}

void UserDatabase::listUsers
    (const string& guid, int level, 
    const string& name, const string& ip, 
    int id, gentity_t *ent) const
{
    vector<std::pair<string, admin_user_t*> > found_users = findUsers(id, guid, level, name, ip);

    // Multiple matches

    // | Multiple matches found:
    // ----------------------------------------------------------------
    // | ID |  GUID  |  LEVEL  |      IP       | NAME
    // ----------------------------------------------------------------
    // |    | 8chars |9chars   | 15chars       |36 chars 
    // |---------------------------------------------------------------

    if(found_users.empty()) {
        ChatPrintTo(ent, "^3!findplayer:^7 0 matches found.");
        return;
    }

    if(found_users.size() > 50) {
        ChatPrintTo(ent, "^3!findplayer: ^7over 50 entries found. Please narrow your search.");
        return;
    }

    std::sort(found_users.begin(), found_users.end(), &UserDatabase::foundUsersComparator);

    ChatPrintTo(ent, "^3!findplayer: ^7check console for more information.");

    BeginBufferPrint();
    BufferPrint(ent, HORIZONTAL_LINE + INFO_LINE + HORIZONTAL_LINE);

    char to_print[MAX_TOKEN_CHARS];
    for(vector<std::pair<string, admin_user_t*> >::const_iterator it = found_users.begin();
        it != found_users.end(); it++) 
    {
        int id = -1;
        if(it->second->userid >= 0) {
            id = it->second->userid;
        }
        Com_sprintf(to_print, sizeof(to_print), "|%-4d|%-s| %-8d|%-15s|%-36s^7\n", id, it->first.substr(0, 8).c_str(),
            it->second->level, it->second->ip.c_str(), it->second->name.c_str());
        BufferPrint(ent, to_print);
    }

    FinishBufferPrint(ent);

}

bool UserDatabase::setLevel(int id, int level) 
{
    if(id == -1) {
        return false;
    }

    bool found = false;
    map<string, admin_user_t*>::iterator it = users_.begin();
    while(it != users_.end()) {

        if(it->second->userid == id) {
            found = true;
            it->second->level = level;
            
            try {
                sqlite3pp::command cmd(db_, string("UPDATE users SET level='"+IntToString(level)+"' WHERE userid='"+IntToString(id)+"';").c_str());
                cmd.execute();
            } catch( sqlite3pp::database_error& e ) {
                LogPrintln("ERROR: " + string(e.what()));
                return false;
            }

            break;
        }

        it++;
    }
    return found;
}

bool UserDatabase::printUserinfo(int id, gentity_t *ent) {

    map<string, admin_user_t*>::iterator it = users_.begin();
    while(it != users_.end()) {

        if(it->second->userid == id) {
            ChatPrintTo(ent, "^3userinfo: ^7check console for more information.");
            string to_print = "^5User\n";
            to_print += "^5id: ^7" + IntToString(it->second->userid) + "\n";
            to_print += "^5level: ^7" + IntToString(it->second->level) + "\n";
            to_print += "^5name: ^7" + it->second->name + "\n";
            to_print += "^5guid: ^7" + it->first.substr(0, 8) + "\n";
            to_print += "^5commands: ^7" + it->second->commands + "\n";
            to_print += "^5greeting: ^7" + it->second->greeting + "\n";
            to_print += "^5ip: ^7" + it->second->ip;
            PrintTo(ent, to_print);
            return true;
        }

        it++;
    }
    return false;
}

bool UserDatabase::updateUser(int id, Level level, string cmds, string greeting) {
    map<string, admin_user_t*>::iterator it = users_.begin();
    while(it != users_.end()) {

        if(it->second->userid == id) {
            if(level.inuse) {
                it->second->level = level.value;
            }

            if(cmds.length() > 0) {
                it->second->commands = cmds;
            }

            if(greeting.length() > 0) {
                it->second->greeting = greeting;
            }

            try {
                // UPDATE users SET level='level', commands='cmds', greeting='greeting' WHERE userid='id'
                string update = "UPDATE users SET level='"
                    +IntToString(it->second->level)
                    +"', commands='"
                    +it->second->commands
                    +"', greeting='"
                    +it->second->greeting
                    +"' WHERE userid='"
                    +IntToString(id)
                    +"'"; 
                sqlite3pp::command cmd(db_, update.c_str());
                cmd.execute();
            }

            catch( sqlite3pp::database_error& e ) {
                LogPrintln("ERROR: " + string(e.what()));
                return false;
            }

            return true;
        }

        it++;
    }
    return false;
}