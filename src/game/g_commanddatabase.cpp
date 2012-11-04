#include "g_commanddatabase.h"

admin_cmd_t::admin_cmd_t(qboolean (*handler) (gentity_t *ent, unsigned skipargs), char flag, const string& function, const string& syntax) {
    this->handler = handler;
    this->flag = flag;
    this->function = function;
    this->syntax = syntax;
}

CommandDatabase::CommandDatabase() {
    add("readconfig", 0, 'G', "Reads admin config.", "!readconfig");
}

CommandDatabase::~CommandDatabase() {

}

bool CommandDatabase::add(const string& keyword,
                          qboolean (*handler)(gentity_t *ent, unsigned skipargs), 
                          char flag,
                          const string& function, 
                          const string& syntax) 
{
    commands_.insert(std::make_pair(keyword, admin_cmd_t(handler, flag, function, syntax)));
    return true;
}

admin_cmd_t *CommandDatabase::find(const string& keyword) {
    std::map<string, admin_cmd_t>::iterator commands_it = commands_.find(keyword);

    if(commands_it == commands_.end()) {
        return 0;
    } else {
        return &commands_it->second;
    }
}

admin_cmd_t *CommandDatabase::findMatching(const string& partial, string& keyword) {
    std::map<string, admin_cmd_t>::iterator commands_it = commands_.begin();
    int count = 0;
    admin_cmd_t *cmd = 0;

    while(commands_it != commands_.end()) {
        if(commands_it->first.substr(0, partial.length()) == partial) {
            keyword = commands_it->first;
            cmd = &commands_it->second;
            count++;
        }
        commands_it++;
    }

    if(count == 1) {
        return cmd;
    }
    return 0;
}

string CommandDatabase::matchingFlags(vector<string>& commands) {
    string flags;
    std::map<string, admin_cmd_t>::iterator it = commands_.begin();

    while( it != commands_.end() ) {

        for(vector<string>::size_type i = 0; i < commands.size(); i++) {
            if(it->first == commands.at(i)) {
                flags += it->second.flag;
                break;
            }
        }

        it++;
    }

    return flags;
}