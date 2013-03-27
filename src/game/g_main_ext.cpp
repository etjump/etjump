
#include "g_users.h"
#include "g_save.h"
#include "g_levels.h"
#include "g_local.hpp"

UserDatabase     users;
SaveSystem       positions;
LevelDatabase    levels;

void OnGameInit() {
    // Init save db
    levels.ReadConfig();
    positions.Reset();
    users.Init();
}

void OnGameShutdown() {
    users.Shutdown();
}

void OnClientConnect( int clientNum, qboolean firstTime, qboolean isBot )
{
    if(firstTime) {
        users.ResetData((g_entities + clientNum));
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
    users.ResetData(ent);
}