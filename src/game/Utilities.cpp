//
// Created by Jussi on 7.4.2015.
//

#include "Utilities.h"
#include <boost/algorithm/string.hpp>

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

        if (!player->client) {
            continue;
        }

        if (player->client->sess.sessionTeam != TEAM_SPECTATOR) {
            continue;
        }

        if (player->client->ps.clientNum == clientNum) {
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
    ResetSavedPositions(player);

    // Disable any weapons except kife
    player->client->ps.weapons[0] = 0;
    player->client->ps.weapons[1] = 0;

    AddWeaponToPlayer(player->client, WP_KNIFE, 1, 0, qtrue);
    ClearPortals(player);
}

void Utilities::stopRun(int clientNum)
{
    gentity_t *player = g_entities + clientNum;

    player->client->sess.timerunActive = qfalse;
}

namespace UtilityHelperFunctions
{
	static void FS_ReplaceSeparators(char *path) {
		char    *s;

		for (s = path; *s; s++) {
			if (*s == '/' || *s == '\\') {
				*s = PATH_SEP;
			}
		}
	}

	static char* BuildOSPath(const char* file)
	{
		char base[MAX_CVAR_VALUE_STRING] = "\0";
		char temp[MAX_OSPATH] = "\0";
		char game[MAX_CVAR_VALUE_STRING] = "\0";
		static char ospath[2][MAX_OSPATH] = { "\0", "\0" };
		static int toggle;

		toggle ^= 1;        // flip-flop to allow two returns without clash

		trap_Cvar_VariableStringBuffer("fs_game", game, sizeof(game));
		trap_Cvar_VariableStringBuffer("fs_homepath", base, sizeof(base));

		Com_sprintf(temp, sizeof(temp), "/%s/%s", game, file);
		FS_ReplaceSeparators(temp);
		Com_sprintf(ospath[toggle], sizeof(ospath[0]), "%s%s", base, temp);

		return ospath[toggle];
	}
}

void Utilities::toConsole(gentity_t* ent, std::string message)
{
	const auto BYTES_PER_PACKET = 998;
	std::vector<std::string> packets;
	while (message.length() > BYTES_PER_PACKET)
	{
		packets.push_back(message.substr(0, BYTES_PER_PACKET));
		message = message.substr(BYTES_PER_PACKET);
	}
	packets.push_back(message);

	for (auto& packet : packets)
	{
		if (!ent)
		{
			G_Printf(packet.c_str());
		}
		else
		{
			trap_SendServerCommand(ClientNum(ent), ("print \"" + packet + "\"").c_str());
		}
	}
}

std::string Utilities::timestampToString(int timestamp, const char* format, const char *start)
{
	char buf[1024];
	struct tm *lt = NULL;
	time_t toConvert = timestamp;
	lt = localtime(&toConvert);
	if (timestamp > 0)
	{
		strftime(buf, sizeof(buf), format, lt);
	}
	else
	{
		return start;
	}

	return std::string(buf);
}

std::string Utilities::getPath(const std::string& name)
{
	auto osPath = UtilityHelperFunctions::BuildOSPath(name.c_str());
	return osPath ? osPath : std::string();
}

bool Utilities::anyonePlaying()
{
	for (auto i = 0; i < level.numConnectedClients; i++)
	{
		auto clientNum = level.sortedClients[i];
		auto target = g_entities + clientNum;

		if (target->client->sess.sessionTeam != TEAM_SPECTATOR)
		{
			return true;
		}
	}
	return false;
}

void Utilities::Log(const std::string& message)
{
	G_LogPrintf(message.c_str());
}

void Utilities::Console(const std::string& message)
{
	G_Printf(message.c_str());
}

void Utilities::Error(const std::string& error)
{
	G_Error(error.c_str());
}

std::vector<std::string> Utilities::getMaps()
{
	std::vector<std::string> maps;

	int i = 0;
	int numDirs = 0;
	int dirLen = 0;
	char dirList[8196];
	char *dirPtr = nullptr;
	numDirs = trap_FS_GetFileList("maps", ".bsp", dirList, sizeof(dirList));
	dirPtr = dirList;

	for (i = 0; i < numDirs; i++, dirPtr += dirLen + 1)
	{
		dirLen = strlen(dirPtr);
		if (strlen(dirPtr) > 4)
		{
			dirPtr[strlen(dirPtr) - 4] = '\0';
		}

		char buf[64] = "\0";
		Q_strncpyz(buf, dirPtr, sizeof(buf));
		boost::to_lower(buf);

		maps.push_back(buf);
	}

	return std::move(maps);
}