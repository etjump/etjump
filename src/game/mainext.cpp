#include "g_local.hpp"
#include "admin\session.hpp"
#include "g_utilities.hpp"

static Session session;

void OnClientConnect(int clientNum, qboolean firstTime, qboolean isBot)
{
    // Do not do g_entities + clientNum here, entity is not initialized yet
    G_DPrintf("OnClientConnect called by %d\n", clientNum);

    if (firstTime)
    {
        session.Init(clientNum);

        G_DPrintf("Requesting guid from %d\n", clientNum);

        trap_SendServerCommand(clientNum,
            GUID_REQUEST);
    }
    else
    {
        session.ReadSessionData(clientNum);
    }
}

void OnClientBegin(gentity_t *ent)
{
    G_DPrintf("OnClientBegin called by %d\n", ent->client->ps.clientNum);
}

void OnClientDisconnect(gentity_t *ent)
{
    G_DPrintf("OnClientDisconnect called by %d\n", ent->client->ps.clientNum);

    session.WriteSessionData(ent->client->ps.clientNum);
}

void WriteSessionData()
{
    for (int i = 0; i < level.numConnectedClients; i++)
    {
        int clientNum = level.sortedClients[i];
        session.WriteSessionData(clientNum);
    }
}

void OnGameShutdown()
{
    WriteSessionData();
}

// Returning qtrue means no other commands will be checked
qboolean OnClientCommand(gentity_t *ent)
{
    
    G_DPrintf("OnClientCommand called for %d: %s\n", ent->client->ps.clientNum, ConcatArgs(0));

    Arguments argv = GetArgs();

    if ((*argv)[0] == "etguid")
    {
        session.GuidReceived(ent);
        return qtrue;
    }
    else if ((*argv)[0] == "guid")
    {
        session.PrintGuid(ent);
        return qtrue;
    }

    return qfalse;
}

qboolean OnConsoleCommand()
{
    G_DPrintf("OnConsoleCommand called: %s.\n", ConcatArgs(0));

    Arguments argv = GetArgs();

    if ((*argv)[0] == "printsession")
    {
        session.PrintSessionData();
        return qtrue;
    }
    return qfalse;
}