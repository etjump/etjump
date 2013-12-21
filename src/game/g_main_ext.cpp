#include "g_save.hpp"
#include "g_local.hpp"
#include "g_utilities.hpp"
#include <string>
#include <boost/algorithm/string/replace.hpp>

void OnGameInit() {
    // Init save db
    SaveSystem::Reset();
}

void OnGameShutdown() {

}

void OnClientConnect( int clientNum, qboolean firstTime, qboolean isBot )
{
    if(firstTime) {
        // Zero: gentity isn't _always_ updated yet, so if ResetClient is called with
        // gentity (g_entities + clientNum) instead of just clientNum, it will
        // always reset client with cnum 0
    }
}

void OnClientBegin(gentity_t *ent) 
{
    SaveSystem::LoadPositionsFromDatabase(ent);
}

void OnClientDisconnect(gentity_t *ent) 
{
    SaveSystem::SavePositionsToDatabase(ent);
    SaveSystem::ResetSavedPositions(ent);
}