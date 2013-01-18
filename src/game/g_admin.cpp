#include <algorithm>
#include <bitset>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "g_leveldatabase.h"
#include "g_userdatabase.h"
#include "g_utilities.h"
#include "g_admin.h"
#include "g_mapdata.h"

using std::vector;
using std::string;
using admin::Client;
using admin::AdminCommand;
using admin::UNKNOWN_ID;

// level data
static LevelDatabase levelDatabase;
// All players' admin related data
static UserDatabase userDatabase;
// All clients' admin related data
static Client clients[MAX_CLIENTS];

// Stores all the admin commands
static AdminCommand Commands[] = {
    {"admintest",		G_AdminTest,		'a',	"Displays your current admin level.", "!admintest"},
    {"edituser",		G_EditUser,		    'A',	"Edits user admin information.", "!edituser <id> <-level|-cmds|-greeting> <level|cmds|greeting>"},
    {"findplayer",      G_FindPlayer,       'F',    "Looks for target player in the database.", "!findplayer <-id|-name|-level|-guid|-ip> <id|name|level|guid|ip>"},
    {"finger",			G_Finger,			'f',	"Displays target's admin level.", "!finger <target>"},
    {"help",		    G_Help,			    'h',	"Prints useful information about commands.", "!help <command>"},
    {"kick",			G_Kick,			    'k',	"Kicks target.", "!kick <player> <time> <reason>"},
    {"listcmds",		G_Help,			    'h',	"Prints useful information about commands.", "!help <command>"},
    {"readconfig",		G_ReadConfig,		'G',	"Reads admin config.", "!readconfig"},
    {"setlevel",		G_SetLevel,		    's',	"Sets target level.", "!setlevel <target> <level>"},
    {"userinfo",        G_Userinfo,         'A',    "Prints user admin information.", "!userinfo <id>"},
    {"addlevel",		G_AddLevel,			'A',	"Adds admin level to admin level database.", "!levadd <level>"},
	{"editlevel",		G_EditLevel,		'A',	"Edits admin level.", "!levedit <level> <name|gtext|cmds> <third parameter>"},
	{"levinfo",			G_LevInfo,	    	'A',	"Prints information about admin levels.", "!levinfo or !levinfo <level>"},
    {"listmaps",		G_ListMaps,		    'a',	"Prints a list of all maps on the server.", "!listmaps"},
    {"mapinfo",         G_MapInfo,          'H',    "Prints statistics about map.", "!mapinfo <map>"},
    {"mostplayed",      G_MostPlayed,       'H',    "Prints most played maps.", "!mostplayed"},
    {"leastplayed",     G_LeastPlayedMaps,  'H',    "Prints least played maps.", "!leastplayed"},
    {"", 0, 0, "", ""}
};

///////////////////////////////////////////////////
// Active client DB
///////////////////////////////////////////////////

Client::Client() {
    // Set every new clients level to 0
    level = 0;
    // Set every new clients id to unknown
    database_id = UNKNOWN_ID;
    // Tells the server to print greeting on connect
    need_greeting = true;
}

void ResetData(int clientNum) {
    if(clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return;
    }

    clients[clientNum].database_id = -1;
    clients[clientNum].guid.clear();
    clients[clientNum].commands.reset();
    clients[clientNum].level = 0;
    clients[clientNum].password.clear();
    clients[clientNum].username.clear();
    clients[clientNum].need_greeting = true;
    clients[clientNum].ip.clear();
}

// Ask client to send username & password 
void RequestLogin(int clientNum) {
    trap_SendServerCommand(clientNum, "login_request");
}

// Ask client to send guid
void RequestGuid(int clientNum) {
    trap_SendServerCommand(clientNum, "guid_request");
}

// Parses clients guid and loads user from database
// or adds a new user to database.
void GuidReceived(gentity_t *ent) {

    if(!*g_admin.string) {
        return;
    }

    vector<string> argv = GetArgs();

    // etguid <hash>
    if(argv.size() != 2) {
        RequestGuid(ent->client->ps.clientNum);
        return;
    }

    if(argv.at(1).length() != 40) {
        RequestGuid(ent->client->ps.clientNum);
        return;
    }

    string guid = SHA1(argv.at(1));
    
    clients[ent->client->ps.clientNum].guid = guid;

    char userinfo[MAX_INFO_STRING];
    trap_GetUserinfo( ent->client->ps.clientNum, userinfo, sizeof(userinfo) );

    clients[ent->client->ps.clientNum].ip = Info_ValueForKey(userinfo, "ip");
    string::size_type port_pos = clients[ent->client->ps.clientNum].ip.find(":");
    if(port_pos != string::npos) {
        clients[ent->client->ps.clientNum].ip = clients[ent->client->ps.clientNum].ip.substr(0, port_pos);
    }

    int level = 0;
    int userid = -1;
    string name, commands, greeting, username, password;
    // Get user info from database
    if(!userDatabase.getUser(guid, level, name, commands, greeting, username, password, userid)) {
        // If fails, add the user
        string s;
        userDatabase.newUser(guid, 0, string(ent->client->pers.netname), s, s, s, s, clients[ent->client->ps.clientNum].ip);
        return;
    } 
    
    clients[ent->client->ps.clientNum].level = level;
    clients[ent->client->ps.clientNum].database_id = userid;
    clients[ent->client->ps.clientNum].personal_commands = commands;
    // Parses commands string & sets bits to bitset
    UpdatePermissions(ent->client->ps.clientNum);
    clients[ent->client->ps.clientNum].greeting = greeting;
    clients[ent->client->ps.clientNum].name = name;

    G_PrintGreeting(ent);
}

// Stores clients username + password to temporary database
void AdminLogin(gentity_t *ent) {

    if(!*g_admin.string) {
        return;
    }

    vector<string> argv = GetArgs();

    // adminlogin <user> <pass>
    if(argv.size() != 3) {
        return;
    }

    string username = argv.at(1);
    string password = SHA1(argv.at(2));

    clients[ent->client->ps.clientNum].username = username;
    clients[ent->client->ps.clientNum].password = password;
}

/////////////////////////////////////////////////////
// Admin commands
/////////////////////////////////////////////////////

// Returns a pointer to a command, looks for regular expression aswell
AdminCommand *MatchCommand(string keyword) {
    int matchcount = 0;
    AdminCommand *command = 0;
    // string to lower
    std::transform(keyword.begin(), keyword.end(), keyword.begin(), ::tolower);
    // Find command(s)
    for(int i = 0; Commands[i].handler != 0; i++) {
        if(Commands[i].keyword == keyword) {
            return &Commands[i];
        } else if(Commands[i].keyword.compare(0, keyword.length(), keyword) == 0) {
            matchcount++;
            command = &Commands[i];
        }  
    }
    // only return a command if there's just one
    if(matchcount == 1) {
        return command;
    } 
    return 0;
}

// Prints greeting. if entity has own greeting set,
// prints it instead of the level-based greeting
void G_PrintGreeting(gentity_t *ent) {
    // Check that we actually need to print the greeting
    if(clients[ent->client->ps.clientNum].need_greeting) {

        string to_print;

        if(clients[ent->client->ps.clientNum].greeting.length() > 0) {
            to_print = clients[ent->client->ps.clientNum].greeting;
        } else if(levelDatabase.greeting(clients[ent->client->ps.clientNum].level).length() > 0) {
            to_print = levelDatabase.greeting(clients[ent->client->ps.clientNum].level);
        } 

        if(to_print.length() > 0) {
            boost::replace_all(to_print, "[n]", string("^7") + ent->client->pers.netname + string("^7"));
            ChatPrintAll(to_print);
        }
    }
    clients[ent->client->ps.clientNum].need_greeting = false;
}

void UpdatePermissions(int clientNum) {
    std::bitset<admin::MAX_COMMANDS> levelset;
    std::bitset<admin::MAX_COMMANDS> personalset;
    
    string personal_commands = clients[clientNum].personal_commands;
    string level_commands = levelDatabase.commands(clients[clientNum].level);
    
    if(!level_commands.empty()) {
        string::iterator it = level_commands.begin();
        while(it != level_commands.end()) {

            if(*it != '*' && *it != '-') {
                levelset.set(*it, true);
            }

            else if(*it == '*') {
                it++;
                levelset.reset();
                levelset.flip();
                while(it != level_commands.end()) {
                    levelset.set(*it, false);
                    it++;
                }
                continue;
            }

            else if(*it == '-') {
                it++;
                while(it != level_commands.end()) {
                    if(*it == '+') {
                        break;
                    }

                    levelset.set(*it, false);
                    it++;
                }
                continue;
            }

            levelset.set(*it, true);
            it++;
        }
    }

    if(!personal_commands.empty()) {
        string::iterator it = personal_commands.begin();
        while(it != personal_commands.end()) {

            if(*it != '*' && *it != '-') {
                personalset.set(*it, true);
            }

            else if(*it == '*') {
                it++;
                personalset.reset();
                personalset.flip();
                while(it != personal_commands.end()) {
                    personalset.set(*it, false);
                    it++;
                }
                continue;
            }

            else if(*it == '-') {
                it++;
                while(it != personal_commands.end()) {
                    if(*it == '+') {
                        break;
                    }

                    personalset.set(*it, false);
                    it++;
                }
                continue;
            }

            personalset.set(*it, true);
            it++;
        }
    }
    clients[clientNum].commands = (levelset | personalset);
}

// Prints everything about a level
void PrintLevelInfo(int level) {
    LogPrintln(levelDatabase.getAll(level));
}

// Used to check if target is higher level than calling entity
bool isTargetHigher(gentity_t *ent, gentity_t *target, bool equalIsHigher) {
    if(!ent) {
        return false;
    }

    if(equalIsHigher) {
        if(clients[target->client->ps.clientNum].level >= clients[ent->client->ps.clientNum].level) {
            return true;
        }
    }

    else {
        if(clients[target->client->ps.clientNum].level > clients[ent->client->ps.clientNum].level) {
            return true;
        }
    }
    return false;
}

// Calls both user&level&ban database readconfig methods to read
// users, levels and bans from databases
qboolean G_ReadConfig(gentity_t *ent, unsigned skipargs) {

    if(!*g_admin.string) {
        return qfalse;
    }

    levelDatabase.readConfig();
    userDatabase.readConfig();
    ChatPrintTo(ent, "^3!readconfig:^7 loaded " + IntToString(levelDatabase.levelCount()) + " levels, " + IntToString(userDatabase.userCount()) + " users.");
    return qtrue;
}

// Checks if client actually has permission to use a certain
// commands
bool G_HasPermission(gentity_t *ent, const char flag) {

    if(flag < 0 || flag > 255) {
        return false;        
    }

    if(!*g_admin.string) {
        return false;
    }

    if(!ent) {
        return true;
    }

    // Both personal & level commands are stored on bitset
    if(clients[ent->client->ps.clientNum].commands.test(flag)) {
        return true;
    }
    return false;
}

// C-interface for calling G_HasPermission from C-code
qboolean G_HasPermissionC(gentity_t *ent, const char flag) {
    if(G_HasPermission(ent, flag)) {
        return qtrue;
    }
    return qfalse;
}

// Check if commands exists, if calling entity can actually execute
// it and calls the exec handler.
qboolean G_CommandCheck(gentity_t *ent) {

    // Let's not do anything if admin system is off
    if(!*g_admin.string) {
        return qfalse;
    }

    vector<string> argv = GetSayArgs();
    unsigned skipargs = 0;

    if(argv.size() < 1) {
        return qfalse;
    }

    // If calling entity was a player that used chat
    // to call a command, skip the "say" argument
    if(argv[0] == "say" || argv[0] == "enc_say") {
        skipargs = 1;
    }

    if(argv.size() < 1 + skipargs) {
        return qfalse;
    }

    // Log all admin commands to admin log file
    if(g_logCommands.integer) {
        string to_log = ConcatArgs(0+skipargs);
        if(ent) {
            // name (guid): command
            G_ALog("^7%s^7 (%s^7): %s", 
                clients[ent->client->ps.clientNum].name.c_str(), 
                clients[ent->client->ps.clientNum].guid.c_str(), 
                to_log.c_str());
        }

        else {
            G_ALog("Console: %s", to_log.c_str());
        }

    }

    string keyword;
    // Remove the ! from the command
    if(argv[0+skipargs][0] == '!') {
        keyword = argv[0+skipargs].substr(1);
    } 
    // If there was no !, check if caller was console
    // if yes, just use the arg itself, no need to remove !
    else if(!ent) {
        keyword = argv[0+skipargs];
    }
    // Should never get here
    else {
        return qfalse;
    }

    // Find the command
    AdminCommand *command = MatchCommand(keyword);

    // couldn't find command
    if(!command) {
        return qfalse;
    }
    
    // if calling entity isn't console, check if he has the permission
    // to exec the command
    if(ent) {
        if(!G_HasPermission(ent, command->flag)) {
            ChatPrintTo(ent, "^3" + command->keyword + ":^7 permission denied.");
            return qfalse;
        }
    }
    // Exec the command
    command->handler(ent, skipargs);

    return qtrue;
}

const string MORE_INFO = "^7check console for more information.";

qboolean G_SetIdLevel(gentity_t *ent, unsigned skipargs) {

    // !setlevel -id id level

    int id = UNKNOWN_ID;
    int alevel = -1;
    vector<string> argv = GetSayArgs();

    // This shouldn't ever happen
    if(argv.size() != 4 + skipargs) {
        ChatPrintTo(ent, "^3usage:^7 !setlevel -id <id> <level>");
        return qfalse;
    }
    
    if(argv.at(1 + skipargs) != "-id") {
        ChatPrintTo(ent, "^3usage:^7 !setlevel -id <id> <level>");
        return qfalse;
    }

    if(!StringToInt(argv.at(2 + skipargs), id)) {
        ChatPrintTo(ent, "^3usage:^7 !setlevel -id <id> <level>");
        return qfalse;
    }

    if(!StringToInt(argv.at(3 + skipargs), alevel)) {
        ChatPrintTo(ent, "^3usage:^7 !setlevel -id <id> <level>");
        return qfalse;
    }

    if(!levelDatabase.levelExists(alevel)) {
        ChatPrintTo(ent, string("^3setlevel:^7 level " + IntToString(alevel) + " does not exist.").c_str());
        return qfalse;
    }

    if(ent) {
        if(alevel > clients[ent->client->ps.clientNum].level) {
            ChatPrintTo(ent, "^3setlevel:^7 you may not setlevel higher than your current level");
            return qfalse;
        }
    }

    if(id < 0) {
        ChatPrintTo(ent, "^3setlevel: ^7user cannot have negative id. if id is negative it hasn't been updated to database yet. Do !readconfig");
        return qfalse;
    }

    if(!userDatabase.setLevel(id, alevel)) {
        return qfalse;
    }

    for(int i = 0; i < level.numConnectedClients; i++) {
        int pid = level.sortedClients[i];

        if(clients[pid].database_id == id) {
            clients[pid].level = alevel;
            UpdatePermissions(pid);
            break;
        }
    }
    
    ChatPrintTo(ent, "^3setlevel:^7 id: " + argv.at(2+skipargs)
        + "^7 is now a level " + argv.at(3+skipargs) + " user.");
    return qtrue;
}
 
// Used to give client an admin level
qboolean G_SetLevel(gentity_t *ent, unsigned skipargs) {

    if(!*g_admin.string) {
        return qfalse;
    }

    vector<string> argv = GetSayArgs();
    string error_msg;
    gentity_t *target = 0;

    if(argv.size() != 3 + skipargs) {
        if(argv.size() == 4 + skipargs) {
            return G_SetIdLevel(ent, skipargs);            
        }
        ChatPrintTo(ent, "^3usage: ^7!setlevel <target> <level>");
        return qfalse;
    }

    int level = -1;

    if(!StringToInt(argv.at(2 + skipargs), level)) {
        ChatPrintTo(ent, string("^3setlevel:^7 invalid number " + argv.at(2)).c_str());
        return qfalse;
    }

    if(!levelDatabase.levelExists(level)) {
        ChatPrintTo(ent, string("^3setlevel:^7 level " + IntToString(level) + " does not exist.").c_str());
        return qfalse;
    }

    if(ent) {
        if(level > clients[ent->client->ps.clientNum].level) {
            ChatPrintTo(ent, "^3setlevel:^7 you may not setlevel higher than your current level");
            return qfalse;
        }
    }

    target = PlayerForName(argv.at(1 + skipargs), error_msg);

    if(!target) {
        ChatPrintTo(ent, "^3setlevel:^7 " + error_msg);
        return qfalse;
    }

    if(isTargetHigher(ent, target, false)) {
        return qfalse;
    }

    string guid = clients[target->client->ps.clientNum].guid;

    // if user does not have a guid let's request it.
    // by the time of re-setlevel server should already
    // have it

    if(guid.length() != 40) {
        ChatPrintTo(ent, "^3setlevel: ^7" + string(target->client->pers.netname) + " ^7does not have a guid. Try again.");
        RequestGuid(target->client->ps.clientNum);
        return qfalse;
    }

    clients[target->client->ps.clientNum].level = level;
    clients[target->client->ps.clientNum].name = target->client->pers.netname;
    ChatPrintTo(ent, "^3setlevel:^7 " + string(target->client->pers.netname)
        + "^7 is now a level " + IntToString(level) + " user.");

    // User should already be stored on sqlite database, so we just 
    // need to update it.

    userDatabase.updateUser(guid, level, string(target->client->pers.netname));

    UpdatePermissions(target->client->ps.clientNum);

    return qtrue;
}

// Prints calling entity's admin level
qboolean G_AdminTest(gentity_t *ent, unsigned skipargs) {
    if(!ent) {
        ChatPrintAll("^3admintest: ^7Hello, I'm an admin.");
        return qtrue;
    }
    ChatPrintAll("^3admintest: ^7" + 
        string(ent->client->pers.netname) + 
        " ^7is a level " + 
        IntToString(clients[ent->client->ps.clientNum].level) + 
        " user ("
        + levelDatabase.name(clients[ent->client->ps.clientNum].level) + "^7)");
    return qtrue;
}

// Prints the level of target player
qboolean G_Finger(gentity_t *ent, unsigned skipargs) {
    vector<string> argv = GetSayArgs();

    // !Finger name
    if(argv.size() != 2 + skipargs) {
        ChatPrintTo(ent, "^3usage: ^7!finger <name>");
        return qfalse;
    }

    string error;
    gentity_t *target = PlayerForName(argv.at(1 + skipargs), error);

    if(!target) {
        ChatPrintTo(ent, "^3finger: ^7" + error);
        return qfalse;
    }

    string level_name = levelDatabase.name(clients[target->client->ps.clientNum].level);
    ChatPrintAll("^3finger:^7 "+string(target->client->pers.netname)+" ^7("+clients[target->client->ps.clientNum].name+"^7) is a level "+IntToString(clients[target->client->ps.clientNum].level)+" user ("+level_name+"^7)");
    return qtrue;
}

// Prints a list of all commands OR detailed information about one command
qboolean G_Help(gentity_t *ent, unsigned skipargs) {
    vector<string> argv = GetSayArgs();

    BeginBufferPrint();

    if(argv.size() == 1 + skipargs) {
        int count = 0;

        ChatPrintTo(ent, "^3help:^7 check console for more information.");
    
        for(unsigned i = 0; Commands[i].handler; i++) {
            if(G_HasPermission(ent, Commands[i].flag)) {
                if(count != 0 && count % 3 == 0) {
                    BufferPrint(ent, "\n");
                }
                BufferPrint(ent, va("%-21s ", Commands[i].keyword.c_str()));
                count++;
            }
        }
        BufferPrint(ent, "\n^3help: ^7" + IntToString(count) + " available commands.");
        FinishBufferPrint(ent);
    }

    else {

        for(unsigned i = 0; Commands[i].handler; i++) {
            if(argv.at(1 + skipargs) == Commands[i].keyword) {
                if(!G_HasPermission(ent, Commands[i].flag)) {
                    return qfalse;
                }
                ChatPrintTo(ent, va("^3%s:^7 %s", Commands[i].keyword.c_str(), Commands[i].function.c_str()));
				ChatPrintTo(ent, va("^3Syntax: ^7%s", Commands[i].syntax.c_str()));
                return qtrue;
            }
        }
        ChatPrintTo(ent, "^3help: ^7unknown command.");
        return qfalse;
    }
    return qtrue;
}

// Kicks a client from the server

qboolean G_Kick(gentity_t *ent, unsigned skipargs) {
    vector<string> argv = GetSayArgs();

    if(argv.size() < 2 + skipargs) {
        ChatPrintTo(ent, "^3usage: ^7!kick <player> <timeout> <reason>");
        return qfalse;
    }

    string error_msg;
    gentity_t *target = PlayerForName(argv.at(1 + skipargs), error_msg);

    if(!target) {
        ChatPrintTo(ent, "^3kick: ^7" + error_msg);
        return qfalse;
    }

    if(target == ent) {
        ChatPrintTo(ent, "^3kick:^7 you cannot kick yourself.");
        return qfalse;
    }

    if(isTargetHigher(ent, target, true)) {
        ChatPrintTo(ent, "^3kick:^7 you cannot kick a fellow admin.");
        return qfalse;
    }

    int timeout = 60;
    string reason = "You've been kicked.";

    if(argv.size() >= 3 + skipargs) {
        if(!StringToInt(argv.at(2+skipargs), timeout)) {
            ChatPrintTo(ent, "^3kick: ^7invalid timeout \""+argv.at(2+skipargs)+"\" specified. Using default (60).");
        }
    }

    if(argv.size() >= 4 + skipargs) {
        reason = Q_SayConcatArgs(3 + skipargs);
    }

    trap_DropClient(target->client->ps.clientNum, reason.c_str(), timeout);
    return qtrue;
}

// !findplayer <-name|-level|-level> <name|level|guid>
// We need to be able to find players from the database based on
// name, level and guid. Lists all matching players.
// Lists first 8-12 characters of a guid so users won't get straight
// access to targets guid.

// ----------------------------------------------------------------
// |  GUID  |  LEVEL  |      IP       | NAME
// ----------------------------------------------------------------

qboolean G_FindPlayer(gentity_t *ent, unsigned skipargs) {

    vector<string> argv = GetSayArgs();
    const string USAGE = "^3usage:^7 !findplayer <-name|-level|-level> <name|level|guid>";

    if(argv.size() < 3 + skipargs) {
        ChatPrintTo(ent, USAGE);
        return qfalse;
    }

    vector<string>::iterator it = argv.begin();
    vector<string>::iterator name_it = argv.end();
    vector<string>::iterator ip_it = argv.end();
    vector<string>::iterator guid_it = argv.end();
    vector<string>::iterator level_it = argv.end();
    vector<string>::iterator id_it = argv.end();

    // Find all switches with a following argument
    // and save them to iterators
    while(it != argv.end()) {

        if(*it == "-name" && (it + 1) != argv.end()) {
            name_it = it;
        }

        else if(*it == "-level" && (it + 1) != argv.end()) {
            level_it = it;
        }

        else if(*it == "-guid" && (it + 1) != argv.end()) {
            guid_it = it;
        }

        else if(*it == "-ip" && (it + 1) != argv.end()) {
            ip_it = it;
        }

        else if(*it == "-id" && (it + 1) != argv.end()) {
            id_it = it;
        }

        it++;
    }

    string name;
    string guid;
    string ip;
    int level = 0;
    int id = -1;

    // the next argument in vector exists if iterator is not end()
    // because it's checked on the previous loop

    if(name_it != argv.end()) {
        name = *(name_it + 1);
    }

    if(level_it != argv.end()) {
        if(!StringToInt(*(level_it + 1), level)) {
            level = 0;
        }
    }

    if(guid_it != argv.end()) {
        guid = *(guid_it + 1);
        std::transform(guid.begin(), guid.end(), guid.begin(), ::toupper);
    }

    if(ip_it != argv.end()) {
        ip = *(ip_it + 1);
    }

    if(id_it != argv.end()) {
        if(!StringToInt(*(id_it + 1), id)) {
            id = -1;
            ChatPrintTo(ent, "^3!listplayer: ^7invalid id");
            return qfalse;
        }
    }

    userDatabase.listUsers(guid, level, name, ip, id, ent);
    
    return qtrue;
}

qboolean G_Userinfo(gentity_t *ent, unsigned skipargs) {

    // !userinfo id 
    vector<string> argv = GetSayArgs();
    int id = UNKNOWN_ID;

    if(argv.size() != 2 + skipargs) {
        ChatPrintTo(ent, "^3usage:^7 !userinfo id");
        return qfalse;
    }

    if(!StringToInt(argv.at(1 + skipargs), id)) {
        ChatPrintTo(ent, "^3usage:^7 invalid id specified");
        return qfalse;
    }

    if(!userDatabase.printUserinfo(id, ent)) {
        ChatPrintTo(ent, "^3userinfo: ^7couldn't find id");
        return qfalse;
    }

    return qtrue;

}

qboolean G_EditUser(gentity_t *ent, unsigned skipargs) {

    // Editable:
    // level, commands, greeting
    vector<string> argv = GetSayArgs();
    int id = UNKNOWN_ID;
    const string USAGE = "^3usage:^7 !edituser <id> <-level|-cmds|-greeting> <level|cmds|greeting>";
    const string INVALID_ID = "^3edituser:^7 invalid id specified";
    const string INVALID_LEVEL = "^3edituser:^7 invalid level specified";
    const string LEVEL_NOT_FOUND = "^3edituser:^7 level could not be found.";
    const string ID_NOT_FOUND = "^3edituser:^7 user with that id could not be found.";

    if(argv.size() < 4 + skipargs) {
        ChatPrintTo(ent, USAGE);
        return qfalse;
    }

    if(!StringToInt(argv.at(1 + skipargs), id)) {
        ChatPrintTo(ent, INVALID_ID);
        return qfalse;
    }

    if(id < 0) {
        ChatPrintTo(ent, INVALID_ID);
        return qfalse;
    }

    // skip (say) !edituser <id>
    vector<string>::const_iterator it = argv.begin() + (2 + skipargs);
    vector<string>::const_iterator level_it = argv.end();
    vector<string>::const_iterator cmds_it = argv.end();
    vector<string>::const_iterator greeting_it = argv.end();

    UserDatabase::Level new_level;
    string cmds;
    string greeting;

    bool greeting_open = false;

    while(it != argv.end()) {
        
        if(*it == "-level" && (it + 1) != argv.end()) {
            greeting_open = false;
            level_it = (it + 1);
        }

        else if(*it == "-cmds" && (it + 1) != argv.end()) {
            greeting_open = false;
            cmds_it = (it + 1);
        }

        else if(*it == "-greeting" && (it + 1) != argv.end()) {
            greeting_open = true;
            greeting.clear();
        }

        else if(greeting_open) {
            greeting += *it;
            greeting += " ";
        }

        it++;
    }

    if(level_it != argv.end()) {
        if(!StringToInt(*level_it, new_level.value)) {
            ChatPrintTo(ent, INVALID_LEVEL);
            return qfalse;
        }

        if(!levelDatabase.levelExists(new_level.value)) {
            ChatPrintTo(ent, LEVEL_NOT_FOUND);
            return qfalse;
        }
        new_level.inuse = true;
    }

    if(cmds_it != argv.end()) {
        cmds = *cmds_it;
    }

    if(!userDatabase.updateUser(id, new_level, cmds, greeting)) {
        ChatPrintTo(ent, ID_NOT_FOUND);
        return qfalse;
    }

    for(int i = 0; i < level.numConnectedClients; i++) {
        int clientNum = level.sortedClients[i];

        if(clients[clientNum].database_id == id) {

            if(new_level.inuse) {
                clients[clientNum].level = new_level.value;
            }

            if(cmds.length() > 0) {
                clients[clientNum].personal_commands = cmds;
                UpdatePermissions(clientNum);
            }

            if(greeting.length() > 0) {
                clients[clientNum].greeting = greeting;
            }

            break;
        }
    }

    return qtrue;
}

// !addlevel level -name <name> -cmds <cmds> -greeting <greeting>

qboolean G_AddLevel(gentity_t *ent, unsigned skipargs) {

    vector<string> argv = GetSayArgs();

    int level = 0;

    // !addlevel <level>
    if(argv.size() < 2 + skipargs) {
        ChatPrintTo(ent, "^3usage: ^7!addlevel <level> <-cmds|-greeting> <cmds|greeting>");
        return qfalse;
    }

    if(!StringToInt(argv.at(1+skipargs), level)) {
        ChatPrintTo(ent, "^3addlevel: ^7invalid level specified.");
        return qfalse;
    }

    if(levelDatabase.levelExists(level)) {
        ChatPrintTo(ent, "^3addlevel: ^7level exists.");
        return qfalse;
    }

    // Skip !addlevel <level>
    vector<string>::const_iterator it = argv.begin() + (2 + skipargs);

    bool name_open = false;
    bool commands_open = false;
    bool greeting_open = false;

    string name;
    string commands;
    string greeting;

    while(it != argv.end()) {

        if(*it == "-greeting" && (it + 1) != argv.end()) {
            commands_open = false;
            greeting_open = true;
            name_open = false;
        }

        else if(*it == "-cmds" && (it + 1) != argv.end()) {
            commands_open = true;
            greeting_open = false;
            name_open = false;
        }

        else if(*it == "-name" && (it + 1) != argv.end()) {
            commands_open = false;
            greeting_open = false;
            name_open = true;
        }

        else if(commands_open) {
            commands += *it;
        }

        else if(greeting_open) {
            greeting += *it;
            greeting += " ";
        }

        else if(name_open) {
            name += *it;
            name += " ";
        }
        it++;
    }

    boost::trim(name);
    boost::trim(commands);
    boost::trim(greeting);
    if(!levelDatabase.addNewLevel(level, name, commands, greeting)) {
        return qfalse;
    }
    return qtrue;
}

qboolean G_EditLevel(gentity_t *ent, unsigned skipargs) {

    return qtrue;
}
qboolean G_LevInfo(gentity_t *ent, unsigned skipargs) {

    return qtrue;
}

qboolean G_ListMaps(gentity_t *ent, unsigned skipargs) {    
    if(ent) {
		if(ent->client->sess.last_listmaps_time != 0 && level.time - ent->client->sess.last_listmaps_time < 60000) {
			ChatPrintTo(ent, va("^3!listmaps:^7 you must wait atleast %d seconds before using !listmaps again.", 
				((ent->client->sess.last_listmaps_time + 60000 - level.time)/1000)));
			return qfalse;
		}
	}

    vector<string> argv = GetSayArgs();

    ChatPrintTo(ent, "^3!listmaps:^7 check console for more information.");

    vector<string>::const_iterator it = argv.begin();
    int columns = 3;


    if(argv.size() > 1 + skipargs) {
        
        while(it != argv.end()) {
            if(*it == "-col" && (it + 1) != argv.end()) {
                // Doesn't matter if this succeeds or not.
                // if it doesn't, just use the default value
                StringToInt(*(it+1), columns);
                break;
            }
            it++;
        }

    }

    mapData.printMapList(ent, columns);
    if(ent)
		ent->client->sess.last_listmaps_time = level.time;
    return qtrue;
}

qboolean G_MapInfo(gentity_t *ent, unsigned skipargs) {
    vector<string> argv = GetSayArgs();
    MapInfo minfo;
    time_t t;
    tm *lt;
    char date[MAX_TOKEN_CHARS];
    
    if(argv.size() == 1 + skipargs) {
        minfo = mapData.mapInfo(level.rawmapname);
    }

    else if(argv.size() >= 1 + skipargs) {
        minfo = mapData.mapInfo(argv[1+skipargs]);
    } else {
        return qfalse;
    }

    t = minfo.lastPlayed;
    lt = localtime(&t);
    strftime(date, sizeof(date), "%d/%m/%y %H:%M:%S", lt);

    int hours = minfo.minutesPlayed / 60;
    int minutes = minfo.minutesPlayed - hours*60;

    boost::format output = 
        boost::format("^g%1% ^7was last played on %2%. It has been played %3% times. (%4% hours %5% minutes)");
    output % minfo.mapName % date % minfo.timesPlayed % hours % minutes;

    ChatPrintTo(ent, "^3!mapinfo: ^7" + output.str());

    return qtrue;
}

qboolean G_MostPlayed(gentity_t *ent, unsigned skipargs) {
    //FIXME: CRASHING
    if(ent->client->sess.lastMostPlayedListTime > level.time + 15000) {
        ChatPrintTo(ent, va("^3!mostplayed:^7 you must wait atleast %d seconds before using !mostplayed again.", 
				((ent->client->sess.lastMostPlayedListTime + 15000 - level.time)/1000)));
        return qfalse;
    }

    int mapCount = 10;
    mapData.printMostPlayedMaps(ent, mapCount);
    return qtrue;
}

qboolean G_LeastPlayedMaps(gentity_t *ent, unsigned skipargs) {

    if(ent->client->sess.lastMostPlayedListTime > level.time + 15000) {
        ChatPrintTo(ent, va("^3!mostplayed:^7 you must wait atleast %d seconds before using !leastplayed again.", 
				((ent->client->sess.lastMostPlayedListTime + 15000 - level.time)/1000)));
        return qfalse;
    }

    int mapCount = 10;
    mapData.printLeastPlayedMaps(ent, mapCount);
    return qtrue;
}