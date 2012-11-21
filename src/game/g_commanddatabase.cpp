#include "g_commanddatabase.h"

static AdminCommand Commands[] = {
    {"readconfig",		G_ReadConfig,		'G',	"Reads admin config.", "!readconfig"},
    {"setlevel",		G_SetLevel,		's',	"Sets target level.", "!setlevel <target> <level>"},
    {"", 0, 0, "", ""}
};

CommandDatabase::CommandDatabase() {

}

CommandDatabase::~CommandDatabase() {

}

AdminCommand *CommandDatabase::Command(const string& keyword) {

    int matchcount = 0;
    AdminCommand *command = 0;

    for(int i = 0; Commands[i].handler != 0; i++) {
        
        if(Commands[i].keyword.find(keyword) != string::npos) {
            matchcount++;
            command = &Commands[i];
        }

    }

    if(matchcount == 1) {
        return command;
    } 
    return 0;
}