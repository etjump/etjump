#include "g_local.hpp"
#include "admin/game.hpp"
#include "admin/session.hpp"
#include "admin/commands.hpp"
#include "admin/levels.hpp"
#include "g_utilities.hpp"
#include <boost/algorithm/string.hpp>

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
}

void OnClientBegin(gentity_t *ent)
{
    G_DPrintf("OnClientBegin called by %d\n", ClientNum(ent));
}

void OnClientDisconnect(gentity_t *ent)
{
    G_DPrintf("OnClientDisconnect called by %d\n", ClientNum(ent));

    game.session->WriteSessionData(ClientNum(ent));
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
    if (g_admin.integer)
    {
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
    }
}

void OnGameShutdown()
{
    WriteSessionData();
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
    
    if (ent->client->pers.connected != CON_CONNECTED) {
        return qfalse;
    }

    if ((*argv)[0] == "guid")
    {
        game.session->PrintGuid(ent);
        return qtrue;
    }

    if (game.commands->ClientCommand(ent, command))
    {
        return qtrue;
    }

    return qfalse;
}

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

    return qfalse;
}