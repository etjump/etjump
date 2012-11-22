#include "g_commanddatabase.h"
#include "g_leveldatabase.h"
#include "g_userdatabase.h"
#include "g_utilities.h"

static CommandDatabase commandDatabase;
static LevelDatabase levelDatabase;
static UserDatabase userDatabase;

struct Client {
    Client();
    int level;
    string guid;
    string username;
    string password;
    string commands;
    string hardware_id;
    string greeting;
    string name;
};

Client::Client() {
    level = 0;
}

static Client clients[MAX_CLIENTS];

void ResetData(int clientNum) {
    if(clientNum < 0 || clientNum >= MAX_CLIENTS) {
        return;
    }

    clients[clientNum].guid.clear();
    clients[clientNum].commands.clear();
    clients[clientNum].hardware_id.clear();
    clients[clientNum].level = 0;
    clients[clientNum].password.clear();
    clients[clientNum].username.clear();
}

void G_ClientBegin(gentity_t *ent) {

}

void G_ClientConnect(gentity_t *ent, qboolean firstTime) {
    if(firstTime) {
        ResetData(ent->client->ps.clientNum);
    }
}

void G_ClientDisconnect(gentity_t *ent) {
    ResetData(ent->client->ps.clientNum);
}

void RequestLogin(int clientNum) {
    trap_SendServerCommand(clientNum, "login_request");
}

void RequestGuid(int clientNum) {
    trap_SendServerCommand(clientNum, "guid_request");
}

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

    string guid = G_SHA1(argv.at(1));
    
    clients[ent->client->ps.clientNum].guid = guid;

    int level = 0;
    string name, commands, greeting, username, password;
    // Get user info from database
    if(!userDatabase.getUser(guid, level, name, commands, greeting, username, password)) {
        // If fails, add the user
        string s;
        userDatabase.newUser(guid, 0, string(ent->client->pers.netname), s, s, s, s);
        return;
    } 
    
    clients[ent->client->ps.clientNum].level = level;
    clients[ent->client->ps.clientNum].commands = commands;
    clients[ent->client->ps.clientNum].greeting = greeting;
    clients[ent->client->ps.clientNum].name = name;
}

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
    string password = G_SHA1(argv.at(2));

    clients[ent->client->ps.clientNum].username = username;
    clients[ent->client->ps.clientNum].password = password;
}

void PrintClientInfo(gentity_t *ent, int clientNum) {
    LogPrintln(string("---------------------------------------------------\n") + 
           string("- Client \n") +
           string("---------------------------------------------------\n") +
           string("\n- NAME: ") + clients[clientNum].name +
           string("\n- GUID: ") + clients[clientNum].guid + 
           string("\n- USER: ") + clients[clientNum].username + 
           string("\n- PASS: ") + clients[clientNum].password +
           string("\n- CMDS: ") + clients[clientNum].commands +
           string("\n- HWID: ") + clients[clientNum].hardware_id +
           string("\n---------------------------------------------------\n"));
}

void PrintLevelInfo(int level) {
    LogPrintln(levelDatabase.getAll(level));
}

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

bool G_HasPermission(gentity_t *ent, char flag) {

    if(*g_admin.string) {
        return false;
    }

    if(!ent) {
        return true;
    }

    int level = clients[ent->client->ps.clientNum].level;

    string personal_commands = clients[ent->client->ps.clientNum].commands;
    string commands = levelDatabase.commands(level);

    string::size_type flag_pos = personal_commands.find(flag);
    string::size_type minus_pos = personal_commands.find('-');

    // If person has all commands check for minus sign
    // in commands string
    if(personal_commands.find('*') != string::npos) {
        if(flag_pos != string::npos) {

            if(minus_pos != string::npos) {
                if(minus_pos < flag_pos) {
                    return false;
                }
            }
            
        }
        return true;
    }

    
    if(flag_pos != string::npos) {

        if(minus_pos != string::npos) {
            if(minus_pos < flag_pos) {
                return false;
            }
        }

        return true;
    }

    // check level commands aswell

    flag_pos = commands.find(flag);
    minus_pos = commands.find('-');

    if(commands.find('*') != string::npos) {
        if(flag_pos != string::npos) {

            if(minus_pos != string::npos) {
                if(minus_pos < flag_pos) {
                    return false;
                }
            }

        }
        return true;
    }

    if(flag_pos != string::npos) {

        if(minus_pos != string::npos) {
            if(minus_pos < flag_pos) {
                return false;
            }
        }

        return true;
    }
    return false;
}

qboolean G_HasPermissionC(gentity_t *ent, char flag) {
    if(G_HasPermission(ent, flag)) {
        return qtrue;
    }
    return qfalse;
}

qboolean G_CommandCheck(gentity_t *ent) {

    // Let's not do anything if admin system is off
    if(!*g_admin.string) {
        return qfalse;
    }

    vector<string> argv = GetSayArgs();
    int skipargs = 0;

    if(argv.size() <= 0) {
        return qfalse;
    }

    // Skip the /say & /enc_say
    if(argv[0] == "say" || argv[0] == "enc_say") {
        skipargs = 1;
    }

    if(argv.size() < 1 + skipargs) {
        return qfalse;
    }

    if(g_logCommands.integer) {
        
        if(ent) {

        }

        else {

        }

    }

    string keyword;
    // Remove the ! from the command
    if(argv[0+skipargs][0] == '!') {
        keyword = argv[0+skipargs].substr(1);
    } 

    else if(!ent) {
        keyword = argv[0+skipargs];
    }

    else {
        return qfalse;
    }

    // Find the command
    AdminCommand *command = commandDatabase.Command(keyword);

    if(!command) {
        return qfalse;
    }
    
    if(ent) {
        if(!G_HasPermission(ent, command->flag)) {
            ChatPrintTo(ent, "^3!" + command->keyword + ":^7 permission denied.");
            return qfalse;
        }
    }

    command->handler(ent, skipargs);

    return qtrue;
}

qboolean G_ReadConfig(gentity_t *ent, int skipargs) {

    if(!*g_admin.string) {
        LogPrintln("SERVER: admin system is disabled.");
        return qfalse;
    }

    levelDatabase.readConfig();
    userDatabase.readConfig();
    ChatPrintTo(ent, "Readconfig: loaded " + int2string(levelDatabase.levelCount()) + " levels, " + int2string(userDatabase.userCount()) + " users.");
    return qtrue;
}
 
// !setlevel <target> <level>
qboolean G_SetLevel(gentity_t *ent, int skipargs) {

    if(!*g_admin.string) {
        return qfalse;
    }

    vector<string> argv = GetSayArgs();
    string error_msg;
    gentity_t *target = 0;

    if(argv.size() != 3 + skipargs) {
        ChatPrintTo(ent, "^3usage: ^7!setlevel <target> <level>");
        return qfalse;
    }

    int level = -1;

    if(!string2int(argv.at(2 + skipargs), level)) {
        ChatPrintTo(ent, string("^3!setlevel:^7 invalid number " + argv.at(2)).c_str());
        return qfalse;
    }

    if(!levelDatabase.levelExists(level)) {
        ChatPrintTo(ent, string("^3!setlevel:^7 level " + int2string(level) + " does not exist.").c_str());
        return qfalse;
    }

    if(ent) {
        if(level > clients[ent->client->ps.clientNum].level) {
            ChatPrintTo(ent, "^3!setlevel:^7 you may not setlevel higher than your current level");
            return qfalse;
        }
    }

    target = playerFromName(argv.at(1 + skipargs), error_msg);

    if(!target) {
        ChatPrintTo(ent, "^3!setlevel:^7 " + error_msg);
        return qfalse;
    }

    if(isTargetHigher(ent, target, false)) {
        return qfalse;
    }

    // User should already be stored on sqlite database, so we just 
    // need to update it.

    // if g_adminLoginType is set to 1
    // only guids are used for authentication. usernames and password
    // are stored if possible, but not necessary

    // if g_adminLoginType is set to 2
    // only usernames + passwords are used for authentication. guids are
    // stored but not used

    // if g_adminLoginType is set to 3
    // guids, username and passwords are used for authentication.

    string guid = clients[target->client->ps.clientNum].guid;

    // if user does not have a guid let's request it.
    // by the time of re-setlevel server should already
    // have it

    if(guid.length() != 40) {
        ChatPrintTo(ent, "^3!setlevel: ^7" + string(target->client->pers.netname) + " ^7does not have a guid. Try again.");
        RequestGuid(target->client->ps.clientNum);
        return qfalse;
    }

    clients[target->client->ps.clientNum].level = level;
    clients[target->client->ps.clientNum].name = target->client->pers.netname;
    ChatPrintTo(ent, "^3!setlevel:^7 " + string(target->client->pers.netname)
        + "^7 is now a level " + int2string(level) + " user.");

    userDatabase.updateUser(guid, level, string(target->client->pers.netname));

    return qtrue;
}

qboolean G_AdminTest(gentity_t *ent, int skipargs) {
    if(!ent) {
        return qtrue;
    }
    ChatPrintAll("^3admintest: ^7" + 
        string(ent->client->pers.netname) + 
        " ^7is a level " + 
        int2string(clients[ent->client->ps.clientNum].level) + 
        " user ("
        + levelDatabase.name(clients[ent->client->ps.clientNum].level) + ")");
    return qtrue;
}

qboolean G_Finger(gentity_t *ent, int skipargs) {
    vector<string> argv = GetSayArgs();

    // !Finger name
    if(argv.size() != 2 + skipargs) {
        ChatPrintTo(ent, "^3usage: ^7!finger <name>");
        return qfalse;
    }

    string error;
    gentity_t *target = playerFromName(argv.at(1 + skipargs), error);

    if(!target) {
        ChatPrintTo(ent, "^3!finger: ^7" + error);
        return qfalse;
    }

    string level_name = levelDatabase.name(clients[target->client->ps.clientNum].level);
    ChatPrintAll("^3finger:^7 "+string(target->client->pers.netname)+" ^7("+clients[target->client->ps.clientNum].name+"^7) is a level "+int2string(clients[target->client->ps.clientNum].level)+" user ("+level_name+"^7)");
}