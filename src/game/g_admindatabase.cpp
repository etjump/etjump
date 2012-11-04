#include "g_admindatabase.h"
#include "g_commanddatabase.h"

#include <sstream>
#include <fstream>

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

void ClientDatabase::resetClientData(int clientNum) {
    clients_[clientNum].active = false;
    clients_[clientNum].guid = "";
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
        error_msg_ = "failed to open admin config file " + file_name + ".";
        writeDefaultConfig();
        return false;
    }

    clearDatabase();
    
    while(admin_config.peek() != EOF) {
        std::stringstream ss;
        string line;
        std::getline(admin_config, line);

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

        }

        else if(user_open) {

        }

        else if(ban_open) {

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
    return true;
}

bool AdminDatabase::writeConfig() {

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
bool AdminDatabase::addUser(int level, const string& guid,
        const string& commands, const string& username,
        const string& password, const string& ingame_name) 
{
    return true;
}
bool AdminDatabase::addUser(admin_user_t user) {

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
    return 0;
}

// Checks if user exists in database
bool AdminDatabase::userExists(const string& key, int keytype) {
    return true;
}

/////////////////////
// Levels
/////////////////////

bool AdminDatabase::addLevel(int level, const string& name, const string& commands,
                const string& greeting, bool protected_level) 
{
    return true;
}

bool AdminDatabase::addLevel(admin_level_t level) {
    return true;
}

bool AdminDatabase::deleteLevel(int level) {
    return true;
}

bool AdminDatabase::updateLevel(int level, const string& name, const string& commands,
                    const string& greeting, bool protected_level) 
{
    return true;
}

admin_level_t *AdminDatabase::findLevel(int level) {
    return 0;
}

bool AdminDatabase::levelExists(int level) {
    return true;
}

/////////////////////
// Bans
/////////////////////

bool AdminDatabase::addBan(const string& name, const string& ip, const string& hardware_id,
            const string& etguid, const string& reason, const string& date,
            int expires, const string& banned_by)
{
    return true;
}

bool AdminDatabase::addBan(admin_ban_t ban) {
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