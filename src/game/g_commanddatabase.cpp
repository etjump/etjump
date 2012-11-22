#include "g_commanddatabase.h"

#include <algorithm>

static AdminCommand Commands[] = {
    {"admintest",		G_AdminTest,		'a',	"Displays your current admin level.", "!admintest"},
    {"finger",			G_Finger,			'f',	"Displays target's admin level.", "!finger <target>"},
    {"readconfig",		G_ReadConfig,		'G',	"Reads admin config.", "!readconfig"},
    {"setlevel",		G_SetLevel,		's',	"Sets target level.", "!setlevel <target> <level>"},
    {"", 0, 0, "", ""}
};

CommandDatabase::CommandDatabase() {

}

CommandDatabase::~CommandDatabase() {

}

AdminCommand *CommandDatabase::Command(string keyword) {

    int matchcount = 0;
    AdminCommand *command = 0;

    std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);

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