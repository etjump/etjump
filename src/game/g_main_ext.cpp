
#include "g_users.h"
#include "g_save.h"
#include "g_local.hpp"

UserDatabase     users;
SaveSystem       positions;

void OnGameInit() {
    // Init save db
    positions.Reset();
    users.Init();
}

void OnGameShutdown() {
    users.Shutdown();
}