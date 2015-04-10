//
// Created by Jussi on 7.4.2015.
//

#include "Utilities.h"

extern "C" {
#include "g_local.h"
}

std::vector<int> Utilities::getSpectators(int clientNum)
{
    std::vector<int> spectators;

    for (auto i = 0; i < level.numConnectedClients; i++) {
        gentity_t *player = g_entities + level.sortedClients[i];

        if (level.sortedClients[i] == clientNum) {
            continue;
        }

        if (player->client) {
            continue;
        }

        if (player->client->sess.sessionTeam != TEAM_SPECTATOR) {
            continue;
        }

        if (player->client->sess.spectatorClient == clientNum) {
            spectators.push_back(g_entities - player);
        }
    }

    return std::move(spectators);
}

void Utilities::startRun(int clientNum)
{
    gentity_t *player = g_entities + clientNum;

    player->client->sess.timerunActive = qtrue;

    // also disable the noclip if it's active
    if (player->client->noclip) {
        player->client->noclip = qfalse;
        VectorClear(player->client->ps.velocity);
    }
    // same thing for god mode
    player->flags &= ~FL_GODMODE;
}

void Utilities::stopRun(int clientNum)
{
    gentity_t *player = g_entities + clientNum;

    player->client->sess.timerunActive = qfalse;
}
