#include <string>
#include <boost/algorithm/string/replace.hpp>
#include <boost/shared_ptr.hpp>

#include "g_save.hpp"
#include "g_local.hpp"
#include "g_utilities.hpp"
#include "admin/sessiondata.h"
#include "admin/userdata.h"

// Hack...
struct Game
{
    Game()
    {
        userData = boost::shared_ptr< UserData >( new UserData() );
        session = boost::shared_ptr< SessionData >( new SessionData( userData.get() ) );
        saveData = boost::shared_ptr< SaveSystem >( new SaveSystem( session.get() ) );
    }
    boost::shared_ptr< SessionData > session;
    boost::shared_ptr< UserData > userData;
    boost::shared_ptr< SaveSystem > saveData;
};

namespace
{
    static Game game = Game();
}

void OnGameInit() {
    // Init save db
    game.saveData->Reset();
    game.userData->Initialize();
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
}

void GuidReceived( gentity_t *ent )
{
    if(!game.session->GuidReceived( ent ))
    {
        ;
    }
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