#include <string>
#include <boost/algorithm/string/replace.hpp>

#include "g_local.hpp"
#include "g_utilities.hpp"
#include "g_save.hpp"
#include "admin/game.h"
#include "admin/userdata.h"
#include "admin/sessiondata.h"
#include "admin/commandinterpreter.h"
#include "admin/leveldata.h"

Game game;

void OnGameInit() {
    // Init save db
    game.saveData->Reset();
    game.userData->Initialize();
    game.levelData->ReadLevels();
}

void OnGameShutdown() {
    game.userData->Shutdown();
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
    game.saveData->LoadPositionsFromDatabase(ent);
}

void OnClientDisconnect(gentity_t *ent) 
{
    game.saveData->SavePositionsToDatabase(ent);
    game.saveData->ResetSavedPositions(ent);
    game.session->UpdateLastSeen(ent);
    game.session->Reset(ent);
}

void GuidReceived( gentity_t *ent )
{
    if(!game.session->GuidReceived( ent ))
    {
        ;
    }
}

qboolean AdminCommandCheck( gentity_t *ent )
{
    if(game.command->ClientCommand( ent ))
    {
        return qtrue;
    }
    return qfalse;
}

// C API for save&load db
void Cmd_Load_f(gentity_t *ent) {
    game.saveData->Load(ent);
}

void Cmd_Save_f(gentity_t *ent) {
    game.saveData->Save(ent);
}

void Cmd_BackupLoad_f(gentity_t *ent) {
    game.saveData->LoadBackupPosition(ent);
}

void ResetSavedPositions(gentity_t *ent) {
    game.saveData->ResetSavedPositions(ent);
}

void ForceSave(gentity_t *location, gentity_t *ent) {
    game.saveData->ForceSave(location, ent);
}

void SavePositionsToDatabase(gentity_t *ent) {
    game.saveData->SavePositionsToDatabase(ent);
}

void LoadPositionsFromDatabase(gentity_t *ent) {
    game.saveData->LoadPositionsFromDatabase(ent);
}

void InitSaveDatabase() {
    game.saveData->Reset();
}

void SaveSystem_Print( gentity_t *ent ) {
    game.saveData->Print( ent );
}

void Cmd_SaveReset_f(gentity_t *ent)
{
    game.saveData->ResetSavedPositions(ent);
    CPTo(ent, "Your saved positions have been reseted.");
}