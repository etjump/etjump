#include "g_commanddatabase.h"
#include "g_clientdatabase.h"
#include "g_leveldatabase.h"
#include "g_userdatabase.h"
#include "g_utilities.h"

static CommandDatabase commandDatabase;
static ClientDatabase clientDatabase;
static LevelDatabase levelDatabase;
static UserDatabase userDatabase;

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

    string guid = G_SHA1(argv.at(1));
    
    if(guid.length() != 40) {
        RequestGuid(ent->client->ps.clientNum);
        return;
    }
    
    clientDatabase.setGuid(ent->client->ps.clientNum, guid);

    int level;
    string name, commands, greeting, username, password;
    // Get user info from database
    if(!userDatabase.getUser(guid, level, name, commands, greeting, username, password)) {
        // If fails, add the user
        userDatabase.newUser(guid, 0, string(ent->client->pers.netname), string(""), string(""), string(""), string(""));
        return;
    } 
    
    clientDatabase.setLevel(ent->client->ps.clientNum, level);
    clientDatabase.setCommands(ent->client->ps.clientNum, commands);
    clientDatabase.setGreeting(ent->client->ps.clientNum, greeting);
    

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

    clientDatabase.setUsername(ent->client->ps.clientNum, username);
    clientDatabase.setPassword(ent->client->ps.clientNum, password);
}

void PrintClientInfo(gentity_t *ent, int clientNum) {
    LogPrintln(clientDatabase.getAll(clientNum));
}

void PrintLevelInfo(int level) {
    LogPrintln(levelDatabase.getAll(level));
}

void ResetData(int clientNum) {
    clientDatabase.ResetData(clientNum);
}

bool G_HasPermission(gentity_t *ent, char flag) {
    int clientNum = ent->client->ps.clientNum;
    int level = clientDatabase.level(clientNum);

    string personal_commands = clientDatabase.commands(clientNum);
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

qboolean G_CommandCheck(gentity_t *ent) {

    // Let's not do anything if admin system is off
    if(!*g_admin.string) {
        return qfalse;
    }

    vector<string> argv = GetSayArgs();
    int skipargs;

    if(argv.size() <= 0) {
        return qfalse;
    }

    // Skip the /say & /enc_say
    if(argv[0] == "say" || argv[0] == "enc_say") {
        skipargs = 1;
    }

    if(argv.size() < 2) {
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
    if(argv[1][0] == '!') {
        keyword = argv[1].substr(1);
    } 

    else if(!ent) {
        keyword = argv[1];
    }

    else {
        return qfalse;
    }
    // Find the command
    AdminCommand *command = commandDatabase.Command(keyword);

    if(!command) {
        return qfalse;
    }

    if(!G_HasPermission(ent, command->flag)) {
        return qfalse;
    }

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

    if(argv.size() != 3) {
        ChatPrintTo(ent, "^3usage: ^7!setlevel <target> <level>");
        return qfalse;
    }

    int level = -1;

    if(!string2int(argv.at(2), level)) {
        ChatPrintTo(ent, string("^3!setlevel:^7 invalid number " + argv.at(2)).c_str());
        return qfalse;
    }

    if(!levelDatabase.levelExists(level)) {
        ChatPrintTo(ent, string("^3!setlevel:^7 level " + int2string(level) + " does not exist.").c_str());
        return qfalse;
    }

    target = playerFromName(argv.at(1), error_msg);

    if(!target) {
        ChatPrintTo(ent, "^3!setlevel:^7 " + error_msg);
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

    string guid = clientDatabase.guid(target->client->ps.clientNum);

    // if user does not have a guid let's request it.
    // by the time of re-setlevel server should already
    // have it

    if(guid.length() != 40) {
        ChatPrintTo(ent, "^3!setlevel: ^7" + string(target->client->pers.netname) + " ^7does not have a guid. Try again.");
        RequestGuid(target->client->ps.clientNum);
        return qfalse;
    }

    
}