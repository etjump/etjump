#include "g_admindatabase.h"
#include "g_commanddatabase.h"

#include <sstream>
#include <fstream>

void DPrint(std::string s) {
    AP(string("print \"" + s + "\"").c_str());
}

void DPrintln(std::string s) {
    AP(string("print \"" + s + "\n\"").c_str());
}

CommandDatabase commandDatabase;

ClientDatabase::ClientDatabase() {

}

ClientDatabase::~ClientDatabase() {

}

ClientDatabase::Client::Client(): active(false), guid("") {

}

void ClientDatabase::setActive(int clientNum) {
    clients_[clientNum].active = true;
}

void ClientDatabase::setInactive(int clientNum) {
    clients_[clientNum].active = false;
}

string ClientDatabase::guid(int clientNum) {
    if(clients_[clientNum].active) {
        return clients_[clientNum].guid;
    } 
    return "";
}

void ClientDatabase::setGuid(int clientNum, string guid) {
    if(!clients_[clientNum].active) {
        return;
    }
    clients_[clientNum].guid = guid;
}

void ClientDatabase::setCommands(int clientNum, string commands) {
    if(!clients_[clientNum].active) {
        return;
    }

    clients_[clientNum].commands = commands;
}

void ClientDatabase::setUsername(int clientNum, string username) {
    if(!clients_[clientNum].active) {
        return;
    }

    clients_[clientNum].username = username;
}

void ClientDatabase::setPassword(int clientNum, string password) {
    if(!clients_[clientNum].active) {
        return;
    }

    clients_[clientNum].password = password;
}

void ClientDatabase::resetClientData(int clientNum) {
    clients_[clientNum].active = false;
    clients_[clientNum].guid.clear();
    clients_[clientNum].level = 0;
    clients_[clientNum].username.clear();
    clients_[clientNum].password.clear();
    clients_[clientNum].commands.clear();
}

AdminDatabase::AdminDatabase() {

}

AdminDatabase::~AdminDatabase() {

}

void trim(string& s) {
	while(s.length() > 0 && s.at(0) == ' ') {
		s.erase(0);
	}

	while(s.length() > 0 && s.at(s.size()-1) == ' ') {
		s.erase(s.size()-1);
	}
}

void AdminDatabase::clearDatabase() {
    for(vector<admin_ban_t*>::size_type i = 0;
		i < admin_levels_.size(); i++) {
			delete admin_levels_.at(i);
	}

	for(vector<admin_user_t*>::size_type i = 0;
		i < admin_users_.size(); i++) {
			delete admin_users_.at(i);
	}

	for(vector<admin_ban_t*>::size_type i = 0;
		i < admin_bans_.size(); i++) {
			delete admin_bans_.at(i);
	}
	admin_levels_.clear();
	admin_users_.clear();
	admin_bans_.clear();
}

std::string int2string(int i) {
	std::stringstream ss;
	ss << i;
    return ss.str();
}

bool string2int(const string& s, int& i) {
	std::istringstream iss(s);
	if((iss >> i).fail()) {
		return false;
	}	

	return true;
}

bool AdminDatabase::readConfig_string(std::stringstream& current_line,
                       string& data) 
{
    string arg;
    
    if(current_line.peek() == EOF) {
        error_msg_ = "missing \"=\" on line ";
        return false;
    }

    current_line >> arg;

    if(arg != "=") {
        error_msg_ = "missing \"=\" on line ";
        return false;
    }

    current_line.ignore(1);

    if(current_line.peek() == EOF) {
        error_msg_ = "missing additional parameters on line ";
        return false;
    }
    
    std::getline(current_line, data);
    return true;
}

bool AdminDatabase::readConfig_int(std::stringstream& current_line,
                    int& data) 
{
    string arg;

    if(current_line.peek() == EOF) {
        error_msg_ = "missing \"=\" on line ";
        return false;
    }

    current_line >> arg;

    if(arg != "=") {
        error_msg_ = "missing \"=\" on line ";
        return false;
    }

    current_line.ignore(1);

    if(current_line.peek() == EOF) {
        error_msg_ = "missing additional parameters on line ";
        return false;
    }

    std::getline(current_line, arg);
    
    if(!string2int(arg, data)) {
        error_msg_ = "invalid number \"" + arg + "\" on line ";
        return false;
    }

    return true;
}

bool AdminDatabase::readConfig_parseCommands(std::stringstream& current_line,
                              string& commands)
{
    string arg;
    vector<string> commandlist;

    if(current_line.peek() == EOF) {
        error_msg_ = "missing \"=\" on line ";
        return false;
    }

    current_line >> arg;

    if(arg != "=") {
        error_msg_ = "missing \"=\" on line ";
        return false;
    }   
    while(current_line.peek() != EOF) {
        current_line >> arg;
        commandlist.push_back(arg);
    }

    commands = commandDatabase.matchingFlags(commandlist);
    return true;
}

bool AdminDatabase::readConfig() {
    admin_level_t   temp_level;
	admin_user_t    temp_user;
	admin_ban_t	    temp_ban;
	bool		    level_open = false;
	bool		    user_open = false;
	bool		    ban_open = false;
	bool		    read_failed = false;
    char            mod_folder[MAX_TOKEN_CHARS];
    string          file_name;
    string          error;
    trap_Cvar_VariableStringBuffer("fs_game", mod_folder, sizeof(mod_folder));

    if(strlen(mod_folder)) {
        file_name = string(mod_folder) + "/" + string(g_admin.string);
    } else {
        file_name = "etjump/" + string(g_admin.string);
    }

    std::ifstream admin_config(file_name.c_str());

    if(!admin_config) {
        error_msg_ = "failed to open admin config file \"" + file_name + "\".";
        writeDefaultConfig();
        return false;
    }

    clearDatabase();
    
    while(admin_config.peek() != EOF) {
        std::stringstream ss;
        string line;
        std::getline(admin_config, line);

        ss.clear();

        ss << line;

        string arg;
        ss >> arg;
        
        if( arg == "[level]" || arg == "[user]"  || arg == "[ban]" ) {
            
            if(level_open) {
                if(!addLevel(temp_level)) {
                    read_failed = true;
                }
            } else if(user_open) {
                if(!addUser(temp_user)) {
                    read_failed = true;
                }
            } else if(ban_open) {
                if(!addBan(temp_ban)) {
                    read_failed = true;
                }
            }

            level_open = false;
            user_open = false;
            ban_open = false;

        }

        if(level_open) {

            if(arg == "level") {
                if(!readConfig_int(ss, temp_level.level)) {
                    read_failed = true;
                }
            }

            else if(arg == "name") {
                if(!readConfig_string(ss, temp_level.name)) {
                    read_failed = true;
                }
            }

            else if(arg == "commands") {
                if(!readConfig_parseCommands(ss, temp_level.commands)) {
                    read_failed = true;
                }
            }

            else if(arg == "greeting") {
                if(!readConfig_string(ss, temp_level.commands)) {
                    read_failed = true;
                }
            }

            else {
                error_msg_ = "parse error on line " + line;
                read_failed = true;
            }
        }

        else if(user_open) {

            if(arg == "level") {
                if(!readConfig_int(ss, temp_user.level)) {
                    read_failed = true;
                }
            }

            else if(arg == "username") {
                if(!readConfig_string(ss, temp_user.username)) {
                    read_failed = true;
                }
            }

            else if(arg == "password") {
                if(!readConfig_string(ss, temp_user.password)) {
                    read_failed = true;
                }
            }

            else if(arg == "guid") {
                if(!readConfig_string(ss, temp_user.guid)) {
                    read_failed = true;
                }
            }

            else if(arg == "commands") {
                if(!readConfig_parseCommands(ss, temp_user.commands)) {
                    read_failed = true;
                }
            }

            else if(arg == "name") {
                if(!readConfig_string(ss, temp_user.ingame_name)) {
                    read_failed = true;
                }
            }

            else {
                error_msg_ = "parse error on line " + line;
                read_failed = true;
            }
        }

        else if(ban_open) {

            if(arg == "name") {
                if(!readConfig_string(ss, temp_ban.name)) {
                    read_failed = true;
                }
            }

            else if(arg == "ip") {
                if(!readConfig_string(ss, temp_ban.ip)) {
                    read_failed = true;
                }
            }

            else if(arg == "hardware") {
                if(!readConfig_string(ss, temp_ban.hardware_id)) {
                    read_failed = true;
                }
            }

            else if(arg == "reason") {
                if(!readConfig_string(ss, temp_ban.reason)) {
                    read_failed = true;
                }
            }

            else if(arg == "made") {
                if(!readConfig_string(ss, temp_ban.date)) {
                    read_failed = true;
                }
            }

            else if(arg == "date") {
                if(!readConfig_string(ss, temp_ban.date)) {
                    read_failed = true;
                }
            }

            else if(arg == "expires") {
                if(!readConfig_int(ss, temp_ban.expires)) {
                    read_failed = true;
                }
            }

            else if(arg == "banner") {
                if(!readConfig_string(ss, temp_ban.banned_by)) {
                    read_failed = true;
                }
            }
            
            else {
                error_msg_ = "parse error on line " + line;
                read_failed = true;
            }
        }

        if( arg == "[level]" ) {
            temp_level.level = 0;
			temp_level.name.clear();
			temp_level.commands.clear();
			temp_level.greeting.clear();
			level_open = true;
        } 

        else if( arg == "[user]" ) {
            temp_user.level = 0;
			temp_user.ingame_name.clear();
			temp_user.password.clear();
			temp_user.commands.clear();
			temp_user.username.clear();
			user_open = true;
        }

        else if( arg == "[ban]" ) {
            temp_ban.banned_by.clear();
			temp_ban.date.clear();
			temp_ban.expires = 0;
			temp_ban.hardware_id.clear();
			temp_ban.ip.clear();
			temp_ban.name.clear();
			temp_ban.reason.clear();
        }
    }

    if(level_open) {
        if(!addLevel(temp_level)) {
            return false;
        }
    }

    else if(user_open) {
        if(!addUser(temp_user)) {
            return false;
        }
    }

    else if(ban_open) {
        if(!addBan(temp_ban)) {
            return false;
        }
    }

    return true;
}

bool AdminDatabase::writeConfig() {
    vector<admin_level_t*>::size_type li = 0;
    vector<admin_user_t*>::size_type ui = 0;
    vector<admin_ban_t*>::size_type bi = 0;

    char mod_folder[MAX_TOKEN_CHARS];
    string file_name;

    trap_Cvar_VariableStringBuffer("fs_game", mod_folder, sizeof(mod_folder));

    if(strlen(mod_folder)) {
        file_name = string(mod_folder) + "/" + string(g_admin.string);
    } else {
        file_name = "etjump/" + string(g_admin.string);
    }

    std::ofstream admin_config(file_name.c_str());

    if(!admin_config) {
        error_msg_ = "failed to open admin config \"" + file_name + "\".";
        return false;
    }

    for(;li < admin_levels_.size(); li++) {

        admin_config << "[level]\n";
		admin_config << "name = " << admin_levels_.at(li)->name << "\n";
		admin_config << "commands = " << admin_levels_.at(li)->commands << "\n";
		admin_config << "greeting = " << admin_levels_.at(li)->greeting << "\n\n";

    }

    for(;ui < admin_users_.size(); ui++) {

        admin_config << "[user]\n";
		admin_config << "level = " << admin_users_.at(ui)->level << "\n";
		admin_config << "name = " << admin_users_.at(ui)->ingame_name << "\n";
        admin_config << "guid = " << admin_users_.at(ui)->guid << "\n";
		admin_config << "username = " << admin_users_.at(ui)->username << "\n";
		admin_config << "password = " << admin_users_.at(ui)->password << "\n";
		admin_config << "commands = " << admin_users_.at(ui)->commands << "\n\n";

    }

    for(;bi < admin_bans_.size(); bi++) {

        admin_config << "[ban]\n";
		admin_config << "name = " << admin_bans_.at(bi)->name << "\n";
		admin_config << "ip = " << admin_bans_.at(bi)->ip << "\n";
		admin_config << "hardware = " << admin_bans_.at(bi)->hardware_id << "\n";
		admin_config << "reason = " << admin_bans_.at(bi)->reason << "\n";
		admin_config << "made = " << admin_bans_.at(bi)->date << "\n";
		admin_config << "expires = " << admin_bans_.at(bi)->expires << "\n";
		admin_config << "banner = " << admin_bans_.at(bi)->banned_by << "\n\n";

    }

    admin_config.close();

    return true;
}

bool AdminDatabase::writeDefaultConfig() {
    char mod_folder[MAX_TOKEN_CHARS];
    string file_name;
    trap_Cvar_VariableStringBuffer("fs_game", mod_folder, sizeof(mod_folder));

    if(!strlen(mod_folder)) {
        file_name = "etjump/" + string(g_admin.string);
    } else {
        file_name = string(mod_folder) + "/" + string(g_admin.string);
    }

    std::ofstream admin_config( file_name.c_str() );

    if(!admin_config) {
        error_msg_ = "failed to open admin config " + file_name;
        return false;
    }

    admin_config << "[level]\n";
	admin_config << "level = 0\n";
	admin_config << "name = ET Jumper\n";
	admin_config << "commands = admintest help spec listmaps\n";
	admin_config << "greeting = Welcome ET Jumper [n]\n\n";

	admin_config << "[level]\n";
	admin_config << "level = 1\n";
	admin_config << "name = Moderator\n";
	admin_config << "commands = kick mute unmute putteam rename rmsaves nogoto nocall noclip\n";
	admin_config << "greeting = Welcome ET Admin I [n]\n\n";

	admin_config << "[level]\n";
	admin_config << "level = 2\n";
	admin_config << "name = Admin\n";
	admin_config << "commands = *\n";
	admin_config << "greeting = Welcome ET Admin II [n]\n\n";

    admin_config.close();
    return true;
}

/////////////////////
// Users
/////////////////////
// Adds user to database
bool AdminDatabase::addUser(admin_user_t user) {

    admin_user_t *new_user = 0;

    if(user.level == 0) {
        return true;
    }

    if(!levelExists(user.level)) {
        error_msg_ = "level \"" + int2string(user.level) + "\" does not exist.";
        return false;
    }

    new_user = findUser(user.username, USERNAME);

    if(new_user) {
        *new_user = user;
        admin_users_.push_back(new_user);
        return true;
    }

    new_user = findUser(user.guid, GUID);

    if(new_user) {
        *new_user = user;
        admin_users_.push_back(new_user);
        return true;
    }

    try {
        new_user = new admin_user_t;
    } catch( ... ) {
        error_msg_ = "failed to allocate memory.";
        return false;
    }

    *new_user = user;

    admin_users_.push_back(new_user);

    return true;
}

// Deletes user from database
bool AdminDatabase::deleteUser(const string& key, int keytype) {

    return true;
}

// Updates user in database
bool AdminDatabase::updateUser(const string& username, const string& guid,
            int level, const string& commands, const string& password,
            const string& ingame_name)
{
    return true;
}

// Finds user in database
admin_user_t *AdminDatabase::findUser(const string& key, int keytype) {

    vector<admin_user_t*>::size_type i = 0;

    while(i < admin_users_.size()) {

        if(keytype == GUID) {
            if(admin_users_.at(i)->guid == key) {
                return admin_users_.at(i);
            }
        } else if(keytype == USERNAME) {
            if(admin_users_.at(i)->username == key) {
                return admin_users_.at(i);
            }
        } else {
            return 0;
        }

        i++;
    }


    return 0;
}

// Checks if user exists in database
bool AdminDatabase::userExists(const string& key, int keytype) {

    vector<admin_user_t*>::size_type i = 0;

    while(i < admin_users_.size()) {

        if(keytype == GUID) {
            if(admin_users_.at(i)->guid == key) {
                return true;
            }
        } else if(keytype == USERNAME) {
            if(admin_users_.at(i)->username == key) {
                return true;
            }
        } else {
            return false;
        }

        i++;
    }

    return false;
}

/////////////////////
// Levels
/////////////////////

bool AdminDatabase::addLevel(admin_level_t level) {

    if(levelExists(level.level)) {
	    error_msg_ = "Readconfig: level \"" + int2string(level.level) + " exists.";
		return false;
	}

	admin_level_t *new_level = new admin_level_t;
	new_level->name = level.name;
	new_level->commands = level.commands;
	new_level->greeting = level.greeting;
	new_level->level = level.level;
	admin_levels_.push_back(new_level);
	return true;
}

bool AdminDatabase::deleteLevel(int level) {
    return true;
}

bool AdminDatabase::updateLevel(int level, const string& name, const string& commands,
                    const string& greeting) 
{
    return true;
}

admin_level_t *AdminDatabase::findLevel(int level) {
    return 0;
}

bool AdminDatabase::levelExists(int level) {
    for(vector<admin_level_t*>::size_type i = 0; i < admin_levels_.size(); i++) {
        if(admin_levels_.at(i)->level == level) {
            return true;
        }
    }
    return false;
}

/////////////////////
// Bans
/////////////////////

bool AdminDatabase::addBan(const string& name, const string& ip, const string& hardware_id,
            const string& etguid, const string& reason, const string& date,
            int expires, const string& banned_by)
{
    if(name.length() == 0) { 
		return false;
	}

	if(ip.length() == 0) {
		return false;
	}

	if(hardware_id.length() == 0) {
		return false;
	}

	if(date.length() == 0) {
		return false;
	}

	if(banned_by.length() == 0) {
		return false;
	}

	admin_ban_t* new_ban = 0;

    try {
        new_ban = new admin_ban_t;
    } catch(...) {
        error_msg_ = "failed to allocate memory.";
        return false;
    }

	new_ban->name = name;
	new_ban->ip = ip;
	new_ban->hardware_id = hardware_id;
	new_ban->reason = reason;
	new_ban->date = date;
	new_ban->expires = expires;
	new_ban->banned_by = banned_by;
	admin_bans_.push_back(new_ban);
	return true;
}

bool AdminDatabase::addBan(admin_ban_t ban) {

    if(ban.name.length() == 0) { 
		return false;
	}

	if(ban.ip.length() == 0) {
		return false;
	}

	if(ban.hardware_id.length() == 0) {
		return false;
	}

	if(ban.date.length() == 0) {
		return false;
	}

	if(ban.banned_by.length() == 0) {
		return false;
	}

    admin_ban_t* new_ban = 0;

    try {
        new_ban = new admin_ban_t;
    } catch(...) {
        error_msg_ = "failed to allocate memory.";
        return false;
    }

    *new_ban = ban;

    return true;
}

bool AdminDatabase::deleteBan(int index) {
    return true;
}

bool AdminDatabase::deleteBan(const string& key, int keytype) {
    return true;
}


// If any function returns false, this function returns
// error message.
string AdminDatabase::error() const {
    return error_msg_;
}

void AdminDatabase::sort() {

}

int AdminDatabase::levelCount() const {
    return admin_levels_.size();
}

int AdminDatabase::userCount() const {
    return admin_users_.size();
}

int AdminDatabase::banCount() const {
    return admin_bans_.size();
}
