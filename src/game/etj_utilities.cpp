/*
 * MIT License
 * 
 * Copyright (c) 2020 ETJump team <zero@etjump.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "etj_utilities.h"
#include "etj_save_system.h"
#include <boost/algorithm/string.hpp>

#include "g_local.h"

std::vector<int> Utilities::getSpectators(int clientNum)
{
	std::vector<int> spectators;

	for (auto i = 0; i < level.numConnectedClients; i++)
	{
		gentity_t *player = g_entities + level.sortedClients[i];

		if (level.sortedClients[i] == clientNum)
		{
			continue;
		}

		if (!player->client)
		{
			continue;
		}

		if (player->client->sess.sessionTeam != TEAM_SPECTATOR)
		{
			continue;
		}

		if (player->client->ps.clientNum == clientNum)
		{
			spectators.push_back(g_entities - player);
		}
	}

	return spectators;
}

static void SelectCorrectWeapon(gclient_t *cl)
{
	auto primary = cl->sess.playerWeapon;
	auto secondary = cl->sess.playerWeapon2;

	// we have already removed disallowed weapons at this point,
	// so we only need to check if we have the weapon & ammo for it
	if (COM_BitCheck(cl->ps.weapons, primary) && BG_WeaponHasAmmo(&cl->ps, primary))
	{
		cl->ps.weapon = primary;
	}
	else if (COM_BitCheck(cl->ps.weapons, secondary) && BG_WeaponHasAmmo(&cl->ps, secondary))
	{
		cl->ps.weapon = secondary;
	}
	else
	{
		cl->ps.weapon = WP_KNIFE;
	}
}

static void RemovePlayerProjectiles(int clientNum)
{
	// Iterate entitylist and remove all projectiles
	// that belong to the activator
	for (int i = MAX_CLIENTS; i < level.num_entities; i++)
	{
		auto ent = g_entities + i;
		if (ent->s.eType == ET_MISSILE)
		{
			if (ent->parent && ent->parent->s.number == clientNum)
			{
				G_FreeEntity(ent);
			}
		}
	}
}

void Utilities::startRun(int clientNum)
{
	gentity_t *player = g_entities + clientNum;

	player->client->sess.timerunActive = qtrue;
	ETJump::UpdateClientConfigString(*player);

	if (!(player->client->sess.runSpawnflags & TIMERUN_DISABLE_SAVE))
	{
		ETJump::saveSystem->resetSavedPositions(player);
	}

	// If we are debugging, just exit here
	if (g_debugTimeruns.integer > 0)
	{
		return;
	}

	RemovePlayerWeapons(clientNum);
	RemovePlayerProjectiles(clientNum);
	SelectCorrectWeapon(player->client);

//	// Disable any weapons except kife
//	player->client->ps.weapons[0] = 0;
//	player->client->ps.weapons[1] = 0;
//
//	AddWeaponToPlayer(player->client, WP_KNIFE, 1, 0, qtrue);
	ClearPortals(player);
}

void Utilities::stopRun(int clientNum)
{
	gentity_t *player = g_entities + clientNum;

	player->client->sess.timerunActive = qfalse;
	ETJump::UpdateClientConfigString(*player);
}

namespace UtilityHelperFunctions
{
static void FS_ReplaceSeparators(char *path)
{
	char *s;

	for (s = path; *s; s++)
	{
		if (*s == '/' || *s == '\\')
		{
			*s = PATH_SEP;
		}
	}
}

static char *BuildOSPath(const char *file)
{
	char        base[MAX_CVAR_VALUE_STRING] = "\0";
	char        temp[MAX_OSPATH]            = "\0";
	char        game[MAX_CVAR_VALUE_STRING] = "\0";
	static char ospath[2][MAX_OSPATH]       = { "\0", "\0" };
	static int  toggle;

	toggle ^= 1;            // flip-flop to allow two returns without clash

	trap_Cvar_VariableStringBuffer("fs_game", game, sizeof(game));
	trap_Cvar_VariableStringBuffer("fs_homepath", base, sizeof(base));

	Com_sprintf(temp, sizeof(temp), "/%s/%s", game, file);
	FS_ReplaceSeparators(temp);
	Com_sprintf(ospath[toggle], sizeof(ospath[0]), "%s%s", base, temp);

	return ospath[toggle];
}
}

void Utilities::toConsole(gentity_t *ent, std::string message)
{
	const auto               BYTES_PER_PACKET = 998;
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

void Utilities::RemovePlayerWeapons(int clientNum)
{
	auto *cl = (g_entities + clientNum)->client;
	for (auto i = 0; i < WP_NUM_WEAPONS; i++) 
	{
		if (BG_WeaponDisallowedInTimeruns(i))
		{
			COM_BitClear(cl->ps.weapons, i);
		}
	}
}

std::string Utilities::timestampToString(int timestamp, const char *format, const char *start)
{
	char      buf[1024];
	struct tm *lt       = NULL;
	time_t    toConvert = timestamp;
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
		auto target    = g_entities + clientNum;

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

std::string Utilities::ReadFile(const std::string& filepath)
{
	fileHandle_t f;
	auto         len = trap_FS_FOpenFile(filepath.c_str(), &f, FS_READ);
	if (len == -1)
	{
		throw std::runtime_error("Could not open file for reading: " + filepath);
	}

	std::unique_ptr<char[]> buf(new char[len + 1]);
	trap_FS_Read(buf.get(), len, f);
	trap_FS_FCloseFile(f);
	buf[len] = 0;
	return std::move(std::string(buf.get()));
}

void Utilities::WriteFile(const std::string& filepath, const std::string& content)
{
	fileHandle_t f;
	auto         len = trap_FS_FOpenFile(filepath.c_str(), &f, FS_WRITE);
	if (len == -1)
	{
		throw std::runtime_error("Could not open file for writing: " + filepath);
	}

	auto bytesWritten = trap_FS_Write(content.c_str(), content.length(), f);
	if (bytesWritten == 0)
	{
		trap_FS_FCloseFile(f);
		throw std::runtime_error("Wrote 0 bytes to " + filepath);
	}
	trap_FS_FCloseFile(f);
}

void Utilities::Logln(const std::string& message)
{
	G_LogPrintf("%s\n", message.c_str());
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

	int  i       = 0;
	int  numDirs = 0;
	int  dirLen  = 0;
	char dirList[8196];
	char *dirPtr = nullptr;
	numDirs = trap_FS_GetFileList("maps", ".bsp", dirList, sizeof(dirList));
	dirPtr  = dirList;

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

	return maps;
}
