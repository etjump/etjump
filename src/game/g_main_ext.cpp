#include "g_save.h"
#include "g_local.hpp"
#include "g_utilities.hpp"
#include "g_sessiondb.hpp"
#include <string>
#include "g_database.hpp"
#include <boost/algorithm/string/replace.hpp>

SaveSystem       positions;
Database         adminDB;
SessionDB        sessionDB;

void OnGameInit() {
    // Init save db
    positions.Reset();
    adminDB.Init();
}

void OnGameShutdown() {
    adminDB.Shutdown();
}

void OnClientConnect( int clientNum, qboolean firstTime, qboolean isBot )
{
    if(firstTime) {
        sessionDB.ResetClient(g_entities + clientNum);
    }
}

void OnClientBegin(gentity_t *ent) 
{
    positions.LoadPositionsFromDatabase(ent);
}

void OnClientDisconnect(gentity_t *ent) 
{
    positions.SavePositionsToDatabase(ent);
    positions.ResetSavedPositions(ent);
    sessionDB.ResetClient(ent);
}

bool ValidGuid(const std::string& guid)
{
    if(guid.length() != 40)
    {
        return false;
    }

    for(int i = 0; i < guid.length(); i++)
    {
        // It's a sha hashed guid converted to hexadecimal. If values
        // greater than F are found in it, it's a fake guid.
        if(guid[i] < '0' || guid[i] > 'F')
        {
            return false;

        }

    }

    return true;
}

void PrintGreeting( gentity_t * ent ) 
{
    std::string greeting = sessionDB.Greeting(ent);

    // Print level greeting instead
    if(greeting.length() == 0)
    {
        // Greeting is checked on sessionDB.Set() and will always have 
        // a) personal greeting b) level greeting c) no greeting so if
        // there's none don't do anything.
        return;
    }
    else
    {
        boost::replace_all(greeting, "[n]", ent->client->pers.netname);    

        ChatPrintAll(greeting);
    }


}

void GuidReceived(gentity_t *ent)
{
    Arguments argv = GetArgs();
    char userinfo[MAX_INFO_STRING] = "\0";
    char *value = NULL;

    trap_GetUserinfo(ent->client->ps.clientNum, userinfo, sizeof(userinfo));
    value = Info_ValueForKey(userinfo, "ip");
    
    if(argv->size() != 2)
    {
        G_LogPrintf("Possible attempt of spoofing GUID (no GUID) by: %s|IP: %s\n",
            ent->client->pers.netname, value);
        return;
    }

    if(!ValidGuid(argv->at(1)))
    {
        G_LogPrintf("Possible attempt of spoofing GUID (invalid GUID) by: %s|IP: %s\n",
            ent->client->pers.netname, value);
        return;
    }

    char *hashedGuid = G_SHA1(argv->at(1).c_str());
    if(!hashedGuid)
    {
        G_LogPrintf("Couldn't hash %s's GUID. Unknown error.\n",
            ent->client->pers.netname);
        return;
    }

    // This is a valid guid
    std::string guid = hashedGuid;
    adminDB.ClientGuidReceived(ent, guid);

    if(ent->client->sess.needGreeting)
    {
        PrintGreeting(ent);
    }
}