#include "g_ctf.h"
#include "g_utilities.h"

// Checks for level.ctfEnabled & g_gametype
qboolean CTFEnabled() {
    if(!level.ctfEnabled || g_gametype.integer != GT_CTF) {
        return qfalse;
    }
    return qtrue;
}

CTFSystem::CTFSystem() {
    axisScore = 0;
    alliedScore = 0;
    axisFlagStatus = FLAG_ATBASE;
    alliedFlagStatus = FLAG_ATBASE;
    gameIsRunning = false;
    int endTime = 0;
}

CTFSystem::~CTFSystem() {

}

void CTFSystem::Init() {
    if(!CTFEnabled) {
        return;
    }

    alliedScore = 0;
    axisScore = 0;
    alliedFlagStatus = FLAG_ATBASE;
    axisFlagStatus = FLAG_ATBASE;

    Team_ResetFlags();
}

void CTFSystem::AlliesScored() {
    alliedScore++;
}

void CTFSystem::AxisScored() {
    axisScore++;
}

void CTFSystem::StartGame() {
    if(!CTFEnabled()) {
        return;
    }

    // Initialize the CTF variables
    Init();

    // Kill all players when the game starts
    for(int i = 0; i < level.numConnectedClients; i++) {
        unsigned clientNum = level.sortedClients[i];

        gentity_t *ent = g_entities + clientNum;
        if(ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
            continue;
        }

        ent->flags &= ~FL_GODMODE;
        ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
        ent->client->ps.persistant[PERS_HWEAPON_USE] = 0;
        player_die(ent, ent, ent, 
            (g_gamestate.integer == GS_PLAYING) ? 100000 : 135, MOD_SUICIDE);       
    }

    endTime = level.time + 30000;
    gameIsRunning = true;
}

void CTFSystem::StopGame() {
    if(!CTFEnabled() || !gameIsRunning) {
        return;
    }

    CPPrintAll("^1Axis: ^7" + IntToString(axisScore) + "\n"
        "^4Allies: ^7" + IntToString(alliedScore) + "\n");
}

