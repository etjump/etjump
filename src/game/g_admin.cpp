#include "g_clientdatabase.h"
#include "g_leveldatabase.h"
#include "g_userdatabase.h"
#include "g_utilities.h"

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
}

void AdminLogin(gentity_t *ent) {

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

void G_ReadConfig(gentity_t *ent, int skipargs) {
    levelDatabase.readConfig();
    userDatabase.readConfig();
    LogPrintln("Readconfig: loaded " + int2string(levelDatabase.levelCount()) + " levels, " + int2string(userDatabase.userCount()) + " users.");
}

const int LOGIN_TYPE_GUID = 1;
const int LOGIN_TYPE_USERPW = 2;
 
// !setlevel <target> <level>
void G_SetLevel(gentity_t *ent, int skipargs) {
    vector<string> argv = GetArgs();

    if(argv.size() != 3) {
        ChatPrintTo(ent, "^3usage: ^7!setlevel <target> <level>");
        return;
    }

    int level = -1;

    if(!string2int(argv.at(2), level)) {
        ChatPrintTo(ent, string("^3!setlevel:^7 invalid number " + argv.at(2)).c_str());
        return;
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

    if(g_adminLoginType.integer == LOGIN_TYPE_GUID) {
        
    } 
    
    else if(g_adminLoginType.integer == LOGIN_TYPE_USERPW) {

    } 
    
    else if(g_adminLoginType.integer == LOGIN_TYPE_GUID + LOGIN_TYPE_USERPW) {

    }
}

void G_WriteClientSessionAdminData( gclient_t *client ) 
{
    const char *s;

    // user+password is max 80 chars so cvars limit of 256 chars
    // is enough
    s = va("%s %s", clientDatabase.username(client->ps.clientNum).c_str(), 
        clientDatabase.password(client->ps.clientNum).c_str());

    trap_Cvar_Set( va("adminsession%i", client - level.clients), s);
    LogPrintln("Writing session data: " + string(s));
}

void G_ReadClientSessionAdminData( gclient_t *client ) 
{
    char s[MAX_STRING_CHARS];
    char *username = 0;
    char *password = 0;

    trap_Cvar_LatchedVariableStringBuffer( va("adminsession%i", client - level.clients), s, sizeof(s));

    sscanf(s, "%s %s", username, password);

    clientDatabase.setUsername(client->ps.clientNum, username);
    clientDatabase.setPassword(client->ps.clientNum, password);
    LogPrintln("Reading session data: " + string(username) + " " + string(password));
}

void G_InitClientSessionAdminData( gclient_t *client ) 
{
    G_WriteClientSessionAdminData( client );
}