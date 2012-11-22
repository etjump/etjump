#include "g_leveldatabase.h"
#include "g_utilities.h"

#include <fstream>
#include <sstream>

extern "C" {
#include "g_local.h"
};

LevelDatabase::admin_level_t::admin_level_t(string n, 
                                            string c, 
                                            string g)
{
    name = n;
    commands = c;
    greeting = g;
}

LevelDatabase::LevelDatabase() {

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
        levels_.find(level)->second->name = name;
        levels_.find(level)->second->commands = commands;
        levels_.find(level)->second->greeting = greeting;
        return true;
    }

    admin_level_t *new_level = 0;

    try {
        new_level = new admin_level_t(name, commands, greeting);
    } catch (...) {
        return false;
    }

    if(levels_.insert(std::make_pair(level, new_level)).second == false) {
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

    writeConfig();

    return true;
}

bool LevelDatabase::deleteLevel(int level) {

    map<int, admin_level_t*>::iterator it = levels_.find(level);

    // Couldn't find level
    if(it == levels_.end()) {
        return false;
    }

    levels_.erase(it);

    writeConfig();
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

// We must always have level 0 
void LevelDatabase::createLevelZero() {
    admin_level_t *level_zero = 0;
    try {
        level_zero = new admin_level_t("", "", "");
    } catch (...) {
        // ???
    }

    levels_.insert(std::make_pair(0, level_zero));
}

const string RCFG_ERROR = "Readconfig error: ";

bool LevelDatabase::readConfig() {

    int linecount = -1;
    bool level_open = false;

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
    // Have to do this here because for some reason constructor won't
    // do anything if I add it there.
    createLevelZero();

    while(admin_config.peek() != EOF) 
    {
        linecount++;
        string arg;
        string current_line;
        vector<string> argv;
        std::stringstream ss;
        std::getline(admin_config, current_line);

        if(current_line.length() == 0) {
            continue;
        }

        ss << current_line;

        for(int i = 0; ss.peek() != EOF; i++) {
            if(i < 2) {
                ss >> arg;
            } else {
                // Skip the space
                ss.ignore(1);
                std::getline(ss, arg);
            }
            argv.push_back(arg);
        }
        
        if(argv.size() == 0) {
            // shouldn't happen
            continue;
        }

        if(argv.at(0) == "[level]") {
            if(level_open) {
                if(!addLevel(level, name, commands, greeting)) {
                    admin_config.close();
                    return false;
                }
            }

            level_open = false;
        }

        if(level_open) {

            if(argv.size() != 3) {
                LogPrintln("WARNING: Invalid line(" + int2string(linecount) + ") \"" + current_line + "\" on admin config.");
                continue;
            }

            if(argv.at(1) != "=") {
                LogPrintln("WARNING: Missing \"=\" on line(" + int2string(linecount) + ") \"" + current_line + "\" on admin config.");
            }

            else if(argv.at(0) == "level") {

                if(!string2int(argv.at(2), level)) {
                    LogPrintln(RCFG_ERROR + "invalid level(" + int2string(linecount) + ") \"" + argv.at(2) + "\" on admin config.");
                    return false;
                }   

            } 

            else if(argv.at(0) == "name") {

                name = argv.at(2);

            }

            else if(argv.at(0) == "greeting") {

                greeting = argv.at(2);

            }

            else if(argv.at(0) == "commands") {

                commands = argv.at(2);

            }

        }

        if(argv.at(0) == "[level]") {
            level = 0;
            name.clear();
            commands.clear();
            greeting.clear();
            level_open = true;
        }

    }

    if(level_open) {
        if(!addLevel(level, name, commands, greeting)) {
            admin_config.close();
            return false;
        }
    }
    admin_config.close();
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

void LevelDatabase::writeConfig() {
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

    for(map<int, admin_level_t*>::iterator it = levels_.begin();
        it != levels_.end(); it++) {
            admin_config << "level = " << it->first << "\n";
            admin_config << "name = " << it->second->name << "\n";
            admin_config << "commands = " << it->second->commands << "\n";
            admin_config << "greeting = " << it->second->greeting << "\n\n";
    }

    admin_config.close();
}

int LevelDatabase::levelCount() const {
    return levels_.size();
}

string LevelDatabase::getAll(int level) {
    map<int, admin_level_t*>::iterator it = levels_.find(level);

    // Couldn't find level
    if(it == levels_.end()) {
        return "";
    }

    return string("---------------------------------------------------\n") + 
           string("- Level " + int2string(level)) + string("\n") +
           string("---------------------------------------------------\n") +
           string("- NAME: ") + it->second->name + 
           string("\n- CMDS: ") + it->second->commands + 
           string("\n- GRTN: ") + it->second->greeting +
           string("\n---------------------------------------------------\n");
}

bool LevelDatabase::levelExists(int level) const {
    if(levels_.find(level) != levels_.end()) {
        return true;
    }
    return false;
}

string LevelDatabase::commands(int level) const {
    map<int, admin_level_t*>::const_iterator it = levels_.find(level);
    if(it != levels_.end()) {
        return it->second->commands;
    }
    return "";
}

string LevelDatabase::name(int level) const {
    map<int, admin_level_t*>::const_iterator it = levels_.find(level);
    if(it != levels_.end()) {
        return it->second->name;
    }
    return "";
}