#include "g_clientdatabase.h"
#include "g_leveldatabase.h"
#include "g_utilities.h"

static ClientDatabase clientDatabase;
static LevelDatabase levelDatabase;

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

    LogPrintln("Readconfig: loaded " + int2string(levelDatabase.levelCount()) + " levels.");
}