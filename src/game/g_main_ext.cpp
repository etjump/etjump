#include <vector>
using std::vector;
#include "g_mapdata.h"

extern "C" {
#include "g_local.h"
}

MapData mapData;

// Called on ClientBegin() on g_main.c
void G_ClientBegin(gentity_t *ent) {
    
}

// Called on ClientConnect() on g_main.c
void G_ClientConnect(gentity_t *ent, qboolean firstTime) {
    if(firstTime) {
        ResetData(ent->client->ps.clientNum);
    }
}

// Called on ClientDisconnect() on g_main.c
void G_ClientDisconnect(gentity_t *ent) {
    ResetData(ent->client->ps.clientNum);
}

void G_InitGame_ext(int levelTime, int randomSeed, int restart ) {
    mapData.init();
}

// Very expensive, don't call if not necessary
void Svcmd_UpdateMapDatabase_f() {
    mapData.updateMapDatabase();
}

/*
int time_int = static_cast<int>(t);
G_LogPrintf("time_int: %d\n", time_int);
int msec_played = level.time - level.startTime;
char str[MAX_TOKEN_CHARS];
strftime(str, sizeof(str), "%d/%m/%y %H:%M:%S", localtime(&t));
G_LogPrintf("time: %s\n", str);
*/

void G_ShutdownGame_ext( int restart ) {
    time_t t;

    if(!time(&t)) {
        return;
    }
    
    int timeInt = static_cast<int>(t);
    int msecPlayed = level.time - level.startTime;
    int minutesPlayed = msecPlayed/1000/60;

    mapData.update(level.rawmapname, timeInt, 1, minutesPlayed);
}