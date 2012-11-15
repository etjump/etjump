#include "g_clientdatabase.h"
#include "g_leveldatabase.h"
#include "g_utilities.h"

static ClientDatabase clientDatabase;
static LevelDatabase levelDatabase;

// For debugging
void DPrint(string msg) {
    AP(string("print \"" + msg + "\"").c_str());
}

void DPrintln(string msg) {
    AP(string("print \"" + msg + "\n\"").c_str());
}

void RequestGuid(gentity_t *ent) {

}

void GuidReceived(gentity_t *ent) {

    vector<string> argv = GetArgs();


    // etguid <hash>
    if(argv.size() != 2) {
        RequestGuid(ent);
        return;
    }

    string guid = G_SHA1(argv.at(1));
    
    if(guid.length() != 40) {
        RequestGuid(ent);
        return;
    }

    clientDatabase.setGuid(ent->client->ps.clientNum, guid); 
    DPrintln("Setting " + int2string(ent->client->ps.clientNum) 
        + "'s guid to " + guid);
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

    DPrintln("Setting " + int2string(ent->client->ps.clientNum) + "'s user " +
        "to " + username + " and password to " + password);
}

void PrintClientInfo(gentity_t *ent, int clientNum) {
    DPrintln(clientDatabase.getAll(clientNum));
}

void ResetData(int clientNum) {
    clientDatabase.ResetData(clientNum);
}

void RequestLogin(int clientNum) {
    trap_SendServerCommand(clientNum, "login_request");
}

void G_ReadConfig(gentity_t *ent, int skipargs) {
    levelDatabase.readConfig();

    LogPrintln("Readconfig: loaded " + int2string(levelDatabase.levelCount()) + " levels.");
}