#include "g_leveldatabase.h"
#include "g_utilities.h"

extern "C" {
#include "g_local.h"
}

#include <fstream>
#include <sstream>

LevelDatabase::admin_level_t::admin_level_t(string name, 
                                            string commands, 
                                            string greeting)
{

}

LevelDatabase::LevelDatabase() {
    admin_level_t *level_zero = 0;
    try {
        level_zero = new admin_level_t("", "", "");
    } catch (...) {
        // ???
    }

    levels_.insert(std::make_pair(0, level_zero));
}

LevelDatabase::~LevelDatabase() {
    map<int, admin_level_t*>::iterator it = levels_.begin();

    while(it != levels_.end()) {
        delete it->second;
        it++;
    }

    levels_.clear();
}

bool LevelDatabase::addLevel(int level, string name,
                             string commands, string greeting) 
{
    if(level == 0) {
        levels_[0]->name = name;
        levels_[0]->commands = commands;
        levels_[0]->greeting = greeting;
        return true;
    }

    admin_level_t *new_level = 0;

    try {
        new_level = new admin_level_t(name, commands, greeting);
    } catch (...) {
        return false;
    }

    std::pair<map<int, admin_level_t*>::iterator, bool> retval = 
        levels_.insert(std::make_pair(level, new_level));

    // Already is on the datastructure
    if(retval.second == false) {
        return false;
    }

    return true;
}

bool LevelDatabase::updateLevel(int level, string str, int type) {

    map<int, admin_level_t*>::iterator it = levels_.find(level);

    // Couldn't find level
    if(it == levels_.end()) {
        return false;
    }

    if(type == NAME) {
        it->second->name = str;
    } 
    
    else if (type == COMMANDS) {
        it->second->commands = str;
    } 
    
    else if (type == GREETING) {
        it->second->greeting = str;
    }

    return true;
}

bool LevelDatabase::deleteLevel(int level) {



    return true;
}

void LevelDatabase::clearDatabase() {
    map<int, admin_level_t*>::iterator it = levels_.begin();

    while(it != levels_.end()) {
        delete it->second;
        it++;
    }

    levels_.clear();
}

const string RCFG_ERROR = "Readconfig error: ";

bool LevelDatabase::readInteger(std::stringstream& current_line, int& data) {
    int count = 0;
    vector<string> args;
    while(current_line.peek() != EOF) {
        string arg;
        if(count < 2) {
            current_line >> arg;
            args.push_back(arg);
        } else {
            std::getline(current_line, arg);
        }
        count++;
    }

    if(args.size() != 3) {
        return false;
    }

    if(args.at(1) != "=") {
        return false;
    }

    if(!string2int(args.at(2), data)) {
        return false;
    }
    return true;
}

bool LevelDatabase::readString(std::stringstream& current_line, string& data) {
    int count = 0;
    vector<string> args;
    while(current_line.peek() != EOF) {
        string arg;
        if(count < 2) {
            current_line >> arg;
            args.push_back(arg);
        } else {
            std::getline(current_line, arg);
        }
        count++;
    }

    if(args.size() != 3) {
        return false;
    }

    if(args.at(1) != "=") {
        return false;
    }

    data = args.at(2).substr(1);

    return true;
}

bool LevelDatabase::readConfig() {

    bool level_open = false;
    bool read_failed = false;

    int level;
    string name;
    string commands;
    string greeting;

    string filename;
    char mod_folder[MAX_TOKEN_CHARS];

    trap_Cvar_VariableStringBuffer("fs_game", mod_folder, sizeof(mod_folder));

    if(strlen(mod_folder)) {
        filename = string(mod_folder) + "/" + string(g_admin.string);
    } else {
        filename = "etjump/" + string(g_admin.string);
    }

    std::ifstream admin_config(filename.c_str());

    if(!admin_config) {
        LogPrintln(RCFG_ERROR + "failed to open " 
            + string(g_admin.string) + ".");
        writeDefaultConfig();
        return false;
    }

    clearDatabase();

    while(admin_config.peek() != EOF) 
    {
        string arg;
        string current_line;
        std::stringstream ss;
        int count = 0;

        if(read_failed) {
            admin_config.close();
            clearDatabase();
            writeDefaultConfig();
            LogPrintln(RCFG_ERROR + "parsing level data failed on line: " + current_line);
            return false;
        }

        LogPrintln(int2string(count) + ":" + current_line);
        count++;

        std::getline(admin_config, current_line);

        if(current_line.length() == 0) {
            continue;
        }

        // Store the line to stringstream
        ss << current_line;
        // Read first arg from stringstream
        ss >> arg;

        // New level open
        if(arg == "[level]") {

            if(level_open == true) {

                if(!addLevel(level, name, commands, greeting)) {
                    return false;
                }

            }

            level = 0;
            name.clear();
            commands.clear();
            greeting.clear();
            level_open = true;
            continue;
        }

        if(level_open) {

            if(arg == "level") {
                if(!readInteger(ss, level)) {
                    read_failed = true;
                }
            }

            else if(arg == "name") {
                if(!readString(ss, name)) {
                    read_failed = true;
                }
            }

            else if(arg == "commands") {
                if(!readString(ss, commands)) {
                    read_failed = true;
                }
            }

            else if(arg == "greeting") {
                if(!readString(ss, greeting)) {
                    read_failed = true;
                }
            }

            else {
                read_failed = true;
            }

        }
    }
    
    return true;
}

void LevelDatabase::writeDefaultConfig() {
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
        LogPrintln("failed to open admin config " + file_name);
        return;
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
    return;
}

int LevelDatabase::levelCount() const {
    return levels_.size();
}