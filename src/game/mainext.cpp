#include "g_local.hpp"
#include "admin/game.hpp"
#include "admin/session.hpp"
#include "admin/commands.hpp"
#include "admin/levels.hpp"
#include "admin/database.hpp"
#include "races.hpp"
#include "mapdata.h"
#include "custommapvotes.hpp"
#include "g_utilities.hpp"
#include <boost/algorithm/string.hpp>
#include "operationqueue.hpp"

Game game;

void OnClientConnect(int clientNum, qboolean firstTime, qboolean isBot)
{
    // Do not do g_entities + clientNum here, entity is not initialized yet
    G_DPrintf("OnClientConnect called by %d\n", clientNum);

    if (firstTime)
    {
        game.session->Init(clientNum);

        G_DPrintf("Requesting guid from %d\n", clientNum);

        trap_SendServerCommand(clientNum,
            GUID_REQUEST);
    }
    else
    {
        game.session->ReadSessionData(clientNum);
    }

    if (game.session->IsIpBanned(clientNum))
    {
        G_LogPrintf("Kicked banned client: %d\n", clientNum);
        trap_DropClient(clientNum, "You are banned.", 0);
    }
}

void OnClientBegin(gentity_t *ent)
{
    G_DPrintf("OnClientBegin called by %d\n", ClientNum(ent));
}

void OnClientDisconnect(gentity_t *ent)
{
    G_DPrintf("OnClientDisconnect called by %d\n", ClientNum(ent));

    game.session->OnClientDisconnect(ClientNum(ent));
}

void WriteSessionData()
{
    for (int i = 0; i < level.numConnectedClients; i++)
    {
        int clientNum = level.sortedClients[i];
        game.session->WriteSessionData(clientNum);
    }
}

void OnGameInit()
{
    game.operationQueue->Init();

    if (strlen(g_levelConfig.string))
    {
        if (!game.levels->ReadFromConfig())
        {
            G_LogPrintf("Error while reading admin config: %s\n", game.levels->ErrorMessage().c_str());
        }
        else
        {
            G_Printf("Successfully loaded levels from config: %s\n", g_levelConfig.string);
        }
    }

    if (strlen(g_userConfig.string) > 0)
    {
        if (!game.database->InitDatabase(g_userConfig.string))
        {
            G_LogPrintf("DATABASE ERROR: %s\n", game.database->GetMessage().c_str());
        }
        else
        {
            G_LogPrintf("Users loaded successfully from database: %s\n", g_userConfig.string);
        }
    }

    if (!game.mapData->Initialize())
    {
        G_Error("Map database load failed: %s.\n", game.mapData->GetMessage().c_str());
    }
    else
    {
        G_LogPrintf("Map database loaded successfully\n");
    }

    game.customMapVotes->Load();
    game.races->Init();
}

void OnGameShutdown()
{
    WriteSessionData();
//    game.database->ExecuteQueuedOperations();
    game.database->CloseDatabase();
    game.mapData->Shutdown();
    game.operationQueue->Shutdown();
}

qboolean OnConnectedClientCommand(gentity_t *ent) 
{
    G_DPrintf("OnClientCommand called for %d (%s): %s\n", ClientNum(ent), ConcatArgs(0), ent->client->pers.netname);

    Arguments argv = GetArgs();
    std::string command = (*argv)[0];
    boost::to_lower(command);

    if (ent->client->pers.connected != CON_CONNECTED) {
        return qfalse;
    }

    if (game.commands->ClientCommand(ent, command))
    {
        return qtrue;
    }

    if (game.commands->AdminCommand(ent))
    {
        return qtrue;
    }

    return qfalse;
}

// Returning qtrue means no other commands will be checked
qboolean OnClientCommand(gentity_t *ent)
{
    
    G_DPrintf("OnClientCommand called for %d (%s): %s\n", ClientNum(ent), ConcatArgs(0), ent->client->pers.netname);

    Arguments argv = GetArgs();
    std::string command = (*argv)[0];
    boost::to_lower(command);

    if ((*argv)[0] == "etguid")
    {
        game.session->GuidReceived(ent);
        return qtrue;
    }

    return qfalse;
}

/*
=======================
Server console commands
=======================
*/
qboolean OnConsoleCommand()
{
    G_DPrintf("OnConsoleCommand called: %s.\n", ConcatArgs(0));

    Arguments argv = GetArgs();
    std::string command = (*argv)[0];
    boost::to_lower(command);

    if (command == "printsession")
    {
        game.session->PrintSessionData();
        return qtrue;
    }

    if (command == "printlevels")
    {
        game.levels->PrintLevels();
        return qtrue;
    }

    if (command == "executeoperations")
    {
//        game.database->ExecuteQueuedOperations();
        return qtrue;
    }

    if (game.commands->AdminCommand(NULL))
    {
        return qtrue;
    }

    return qfalse;
}

const char *GetRandomMap()
{
    // For some reason returning game.mapData->RandomMap().c_str()
    // messes up the map name
    static char buf[MAX_TOKEN_CHARS] = "\0";
    Q_strncpyz(buf, game.mapData->RandomMap().c_str(), sizeof(buf));
    return buf;
}

const char *GetRandomMapByType(const char *customType)
{
    static char buf[MAX_TOKEN_CHARS] = "\0";

    if (!customType)
    {
        G_Error("customType is NULL.");
    }

    Q_strncpyz(buf, game.customMapVotes->RandomMap(customType).c_str(), sizeof(buf));
    return buf;
}

// returns null if map type doesnt exists.
const char *CustomMapTypeExists(const char *mapType)
{
    static char buf[MAX_TOKEN_CHARS] = "\0";
    if (!mapType)
    {
        G_Error("mapType is NULL.");
    }

    CustomMapVotes::TypeInfo info = game.customMapVotes->GetTypeInfo(mapType);

    if (info.typeExists)
    {
        Q_strncpyz(buf, info.callvoteText.c_str(), sizeof(buf));
        return buf;
    }

    return NULL;
}
void ExecuteQueuedDatabaseOperations()
{
//    game.database->ExecuteQueuedOperations();
}

void StartRace(gentity_t *ent)
{
    game.races->StartRace(ent);
}

void CheckIfOperationsNeedToBeExecuted()
{
    game.operationQueue->ExecuteQueuedOperations();
}

void ClientNameChanged(gentity_t *ent)
{
    game.session->NewName(ent);
}