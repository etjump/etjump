#include "g_save.h"
#include "g_local.hpp"

SaveSystem       positions;

void OnGameInit() {
    // Init save db
	adminDB.ReadUserConfig(NULL);
    positions.Reset();
}

void OnGameShutdown() {
    // users.Shutdown();
}

void OnClientConnect( int clientNum, qboolean firstTime, qboolean isBot )
{
    if(firstTime) {
        // users.ResetData((g_entities + clientNum));
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
    // users.ResetData(ent);
}