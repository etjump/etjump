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

#include <string>
#include <vector>
using std::string;
using std::vector;

#include "etj_save_system.h"
#include "utilities.hpp"
#include "etj_session.h"

ETJump::SaveSystem::Client::Client()
{
	alliesBackupPositions = boost::circular_buffer<SavePosition>(MAX_BACKUP_POSITIONS);
	axisBackupPositions   = boost::circular_buffer<SavePosition>(MAX_BACKUP_POSITIONS);

	for (int i = 0; i < MAX_SAVED_POSITIONS; i++)
	{
		alliesSavedPositions[i].isValid = false;
		axisSavedPositions[i].isValid   = false;
	}

	for (int i = 0; i < MAX_BACKUP_POSITIONS; i++)
	{
		alliesBackupPositions.push_back(SavePosition());
		alliesBackupPositions[i].isValid = false;

		axisBackupPositions.push_back(SavePosition());
		axisBackupPositions[i].isValid = false;
	}

	alliesLastLoadPosition.isValid = false;
	axisLastLoadPosition.isValid = false;
}

ETJump::SaveSystem::DisconnectedClient::DisconnectedClient()
{
	for (int i = 0; i < MAX_SAVED_POSITIONS; i++)
	{
		alliesSavedPositions[i].isValid = false;
		axisSavedPositions[i].isValid   = false;
		progression                     = 0;
	}

	alliesLastLoadPosition.isValid = false;
	axisLastLoadPosition.isValid = false;
}

// Zero: required for saving saves to db
std::string UserDatabase_Guid(gentity_t *ent);

// Saves current position
void ETJump::SaveSystem::save(gentity_t *ent)
{
	auto *client = ent->client;

	if (!client)
	{
		return;
	}

	if (!g_save.integer)
	{
		CPTo(ent, "^3Save ^7is not enabled.");
		return;
	}

	if ((client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::Active)) && (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::NoSave)))
	{
		CPTo(ent, "^3Save ^7is disabled for this death run.");
		return;
	}

	if (level.saveLoadRestrictions & static_cast<int>(SaveLoadRestrictions::Move))
	{
		// comparing to zero vector
		if (!VectorCompare(client->ps.velocity, vec3_origin))
		{
			CPTo(ent, "^3Save ^7is disabled while moving on this map.");
			return;
		}
	}

	// No saving while dead if it's disabled by map
	if (client->ps.pm_type == PM_DEAD && level.saveLoadRestrictions & static_cast<int>(SaveLoadRestrictions::Dead))
	{
		CPTo(ent, "^3Save ^7is disabled while dead on this map.");
		return;
	}

	auto argv = GetArgs();
	auto position = 0;
	if (argv->size() > 1)
	{
		ToInt((*argv)[1], position);

		if (position < 0 || position >= MAX_SAVED_POSITIONS)
		{
			CPTo(ent, "Invalid position.");
			return;
		}

		if (position > 0 &&
			client->sess.timerunActive &&
			client->sess.runSpawnflags & TIMERUN_DISABLE_BACKUPS)
		{
			CPTo(ent, "Save slots are disabled for this timerun.");
			return;
		}
	}

	if (!client->sess.saveAllowed)
	{
		CPTo(ent, "You are not allowed to save a position.");
		return;
	}

	if (client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		CPTo(ent, "^7You can not ^3save^7 as a spectator.");
		return;
	}

	if (client->sess.timerunActive && client->sess.runSpawnflags & TIMERUN_DISABLE_SAVE)
	{
		CPTo(ent, "^3Save ^7is disabled for this timerun.");
		return;
	}

	trace_t trace;
	trap_TraceCapsule(&trace, client->ps.origin, ent->r.mins,
	                  ent->r.maxs, client->ps.origin, ent->s.number, CONTENTS_NOSAVE);

	if (level.noSave)
	{
		if ((!trace.fraction) != 1.0f)
		{
			CPTo(ent, "^7You can not ^3save ^7inside this area.");
			return;
		}
	}
	else
	{
		if (trace.fraction != 1.0f)
		{
			CPTo(ent, "^7You can not ^3save ^7inside this area.");
			return;
		}
	}

	if (client->pers.race.isRacing)
	{
		if (client->pers.race.saveLimit == 0)
		{
			CPTo(ent, "^5You've used all your saves.");
			return;
		}

		if (client->pers.race.saveLimit > 0)
		{
			client->pers.race.saveLimit--;
		}
	}
	else
	{
		fireteamData_t *ft;
		if (G_IsOnFireteam(ent - g_entities, &ft))
		{
			if (ft->saveLimit < 0)
			{
				client->sess.saveLimit = 0;
			}
			if (ft->saveLimit)
			{
				if (client->sess.saveLimit)
				{
					client->sess.saveLimit--;
				}
				else
				{
					CPTo(ent, "^5You've used all your fireteam saves.");
					return;
				}
			}
		}
	}


	SavePosition *pos = nullptr;
	if (client->sess.sessionTeam == TEAM_ALLIES)
	{
		pos = _clients[ClientNum(ent)].alliesSavedPositions + position;
	}
	else
	{
		pos = _clients[ClientNum(ent)].axisSavedPositions + position;
	}

	saveBackupPosition(ent, pos);

	storePosition(client, pos);

	G_AddEvent(ent, EV_SAVE, 0);

	if (position == 0)
	{
		CP(va("cp \"%s\n\"", g_savemsg.string));
	}
	else
	{
		CP(va("cp \"%s ^7%d\n\"", g_savemsg.string, position));
	}
}

// Loads position
void ETJump::SaveSystem::load(gentity_t *ent)
{
	auto *client = ent->client;

	if (!client)
	{
		return;
	}

	if (!g_save.integer)
	{
		CPTo(ent, "^3Load ^7is not enabled.");
		return;
	}

	if (!client->sess.saveAllowed)
	{
		CPTo(ent, "You are not allowed to load a position.");
		return;
	}

	if ((client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::Active)) && (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::NoSave)))
		{
		CPTo(ent, "^3Load ^7is disabled for this death run.");
		return;
	}

	if (client->ps.pm_type == PM_DEAD && level.saveLoadRestrictions & static_cast<int>(SaveLoadRestrictions::Dead))
	{
		CPTo(ent, "^3Load ^7is disabled while dead on this map.");
		return;
	}

	auto argv = GetArgs();
	auto slot = 0;
	if (argv->size() > 1)
	{
		ToInt((*argv)[1], slot);

		if (slot < 0 || slot >= MAX_SAVED_POSITIONS)
		{
			CPTo(ent, "^7Invalid save slot.");
			return;
		}

		if (slot > 0 &&
			client->sess.timerunActive &&
			client->sess.runSpawnflags & TIMERUN_DISABLE_BACKUPS)
		{
			CPTo(ent, "Save slots are disabled for this timerun.");
			return;
		}
	}

	if (client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		CPTo(ent, "^7You can not ^3load ^7as a spectator.");
		return;
	}

	auto validSave = getValidTeamSaveForSlot(ent, client->sess.sessionTeam, slot);
	if (validSave)
	{
		saveLastLoadPos(ent); // store position for unload command
		restoreStanceFromSave(ent, validSave);
		if (client->sess.timerunActive && client->sess.runSpawnflags & TIMERUN_DISABLE_SAVE)
		{
			InterruptRun(ent);
		}
		teleportPlayer(ent, validSave);
	}
	else
	{
		CPTo(ent, "^7Use ^3save ^7first.");
	}
}

// Saves position, does not check for anything. Used for target_save
void ETJump::SaveSystem::forceSave(gentity_t *location, gentity_t *ent)
{
	SavePosition *pos = nullptr;
	auto *client = ent->client;

	if (!client || !location)
	{
		return;
	}

	if (client->sess.sessionTeam == TEAM_ALLIES)
	{
		pos = &_clients[ClientNum(ent)].alliesSavedPositions[0];
	}
	else if (client->sess.sessionTeam == TEAM_AXIS)
	{
		pos = &_clients[ClientNum(ent)].axisSavedPositions[0];
	}
	else
	{
		return;
	}

	saveBackupPosition(ent, pos);

	VectorCopy(location->s.origin, pos->origin);
	VectorCopy(location->s.angles, pos->vangles);
	pos->isValid = true;
	pos->stance = client->ps.eFlags & EF_CROUCHING
		? Crouch
		: client->ps.eFlags & EF_PRONE ? Prone : Stand;

	trap_SendServerCommand(ent - g_entities, g_savemsg.string);
}

// Loads backup position
void ETJump::SaveSystem::loadBackupPosition(gentity_t *ent)
{
	auto *client = ent->client;

	if (!client)
	{
		return;
	}

	if (!g_save.integer)
	{
		CPTo(ent, "^3Load ^7is not enabled.");
		return;
	}

	if (!client->sess.saveAllowed)
	{
		CPTo(ent, "You are not allowed to load a position.");
		return;
	}

	if (client->sess.timerunActive &&
		client->sess.runSpawnflags & TIMERUN_DISABLE_BACKUPS)
	{
		CPTo(ent, "Backup is disabled for this timerun.");
		return;
	}

	if ((client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::Active)) && (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::NoSave)))
	{
		CPTo(ent, "^3Backup ^7is disabled for this death run.");
		return;
	}

	if (client->ps.pm_type == PM_DEAD && level.saveLoadRestrictions & static_cast<int>(SaveLoadRestrictions::Dead))
	{
		CPTo(ent, "^3Backup ^7is disabled while dead on this map.");
		return;
	}

	auto argv = GetArgs();
	auto slot = 0;
	if (argv->size() > 1)
	{
		ToInt(argv->at(1), slot);

		if (slot < 1 || slot > MAX_SAVED_POSITIONS)
		{
			CPTo(ent, "^7Invalid backup slot.");
			return;
		}

		if (slot > 0)
		{
			slot--;
		}
	}

	if (client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		CPTo(ent, "^7You can not ^3load ^7as a spectator.");
		return;
	}

	SavePosition *pos = nullptr;
	if (client->sess.sessionTeam == TEAM_ALLIES)
	{
		pos = &_clients[ClientNum(ent)].alliesBackupPositions[slot];
	}
	else
	{
		pos = &_clients[ClientNum(ent)].axisBackupPositions[slot];
	}

	if (pos->isValid)
	{
		restoreStanceFromSave(ent, pos);
		if (client->sess.timerunActive && client->sess.runSpawnflags & TIMERUN_DISABLE_SAVE)
		{
			InterruptRun(ent);
		}
		teleportPlayer(ent, pos);
	}
	else
	{
		CPTo(ent, "^7Use ^3save ^7first.");
	}
}

// Undo last load command and teleport to last position client loaded from
// Position validation is done here
void ETJump::SaveSystem::unload(gentity_t* ent)
{
	auto* client = ent->client;

	if (!client)
	{
		return;
	}

	if (!g_save.integer)
	{
		CPTo(ent, "^3Unload ^7is not enabled.");
		return;
	}

	if (!client->sess.saveAllowed)
	{
		CPTo(ent, "^7You are not allowed to ^3unload ^7a position.");
		return;
	}

	if ((client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::Active)) && (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::NoSave)))
	{
		CPTo(ent, "^3unload ^7is disabled for this death run.");
		return;
	}

	if (client->ps.pm_type == PM_DEAD && level.saveLoadRestrictions & static_cast<int>(SaveLoadRestrictions::Dead))
	{
		CPTo(ent, "^3unload ^7is disabled while dead on this map.");
		return;
	}

	if (client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		CPTo(ent, "^7You can not ^3unload ^7as a spectator.");
		return;
	}

	if (client->sess.timerunActive)
	{
		CPTo(ent, "^3unload ^7is not available during timeruns.");
		return;
	}

	auto validPos = getValidTeamUnloadPos(ent, client->sess.sessionTeam);
	
	if (validPos)
	{
		// check for nosave areas only if we have valid pos
		trace_t trace;
		trap_TraceCapsule(&trace, validPos->origin, ent->r.mins,
			ent->r.maxs, validPos->origin, ent->s.number, CONTENTS_NOSAVE);

		if (level.noSave)
		{
			if ((!trace.fraction) != 1.0f)
			{
				CPTo(ent, "^7You can not ^3unload ^7to this area.");
				return;
			}
		}
		else
		{
			if (trace.fraction != 1.0f)
			{
				CPTo(ent, "^7You can not ^3unload ^7to this area.");
				return;
			}
		}

		restoreStanceFromSave(ent, validPos);
		teleportPlayer(ent, validPos);
	}
	else
	{
		CPTo(ent, "^7Use ^3load ^7first.");
	}

}

// Saves position client loaded from. Executed on every successful load command,
// position validation is done later. This is to prevent unexpected behavior
// where the last load position is not a valid position, and client is
// teleported to a position that was valid before that.
void ETJump::SaveSystem::saveLastLoadPos(gentity_t* ent)
{
	SavePosition* pos = nullptr;
	auto *client = ent->client;

	if (client->sess.sessionTeam == TEAM_ALLIES)
	{
		pos = &_clients[ClientNum(ent)].alliesLastLoadPosition;
	}
	else if (client->sess.sessionTeam == TEAM_AXIS)
	{
		pos = &_clients[ClientNum(ent)].axisLastLoadPosition;
	}
	else
	{
		return;
	}

	storePosition(client, pos);
}

void ETJump::SaveSystem::reset()
{
	for (int clientIndex = 0; clientIndex < level.numConnectedClients; clientIndex++)
	{
		int clientNum = level.sortedClients[clientIndex];
		// TODO: reset saved positions here
		resetSavedPositions(g_entities + clientNum);
	}

	_savedPositions.clear();
}

// Used to reset positions on map change/restart
void ETJump::SaveSystem::resetSavedPositions(gentity_t *ent)
{
	int clientNum = ClientNum(ent);
	for (int saveIndex = 0; saveIndex < MAX_SAVED_POSITIONS; saveIndex++)
	{
		_clients[clientNum].alliesSavedPositions[saveIndex].isValid = false;
		_clients[clientNum].axisSavedPositions[saveIndex].isValid   = false;
	}

	for (int backupIndex = 0; backupIndex < MAX_BACKUP_POSITIONS; backupIndex++)
	{
		_clients[clientNum].alliesBackupPositions[backupIndex].isValid = false;
		_clients[clientNum].axisBackupPositions[backupIndex].isValid   = false;
	}

	_clients[clientNum].quickDeployPositions[TEAM_ALLIES].isValid = false;
	_clients[clientNum].quickDeployPositions[TEAM_AXIS].isValid = false;

	_clients[clientNum].alliesLastLoadPosition.isValid = false;
	_clients[clientNum].axisLastLoadPosition.isValid = false;
}

// Called on client disconnect. Saves saves for future sessions
void ETJump::SaveSystem::savePositionsToDatabase(gentity_t *ent)
{

	if (!ent->client)
	{
		return;
	}

	string guid = _session->Guid(ent);

	DisconnectedClient client;

	for (int i = 0; i < MAX_SAVED_POSITIONS; i++)
	{
		// Allied
		VectorCopy(_clients[ClientNum(ent)].alliesSavedPositions[i].origin,
		           client.alliesSavedPositions[i].origin);
		VectorCopy(_clients[ClientNum(ent)].alliesSavedPositions[i].vangles,
		           client.alliesSavedPositions[i].vangles);
		client.alliesSavedPositions[i].isValid =
		    _clients[ClientNum(ent)].alliesSavedPositions[i].isValid;
		// Axis
		VectorCopy(_clients[ClientNum(ent)].axisSavedPositions[i].origin,
		           client.axisSavedPositions[i].origin);
		VectorCopy(_clients[ClientNum(ent)].axisSavedPositions[i].vangles,
		           client.axisSavedPositions[i].vangles);
		client.axisSavedPositions[i].isValid
		    = _clients[ClientNum(ent)].axisSavedPositions[i].isValid;
	}

	client.progression                           = ent->client->sess.clientMapProgression;
	ent->client->sess.loadPreviousSavedPositions = qfalse;

	std::map<string, DisconnectedClient>::iterator it = _savedPositions.find(guid);

	if (it != _savedPositions.end())
	{
		it->second = client;
	}
	else
	{
		_savedPositions.insert(std::make_pair(guid, client));
	}

}

// Called on client connect. Loads saves from previous session
void ETJump::SaveSystem::loadPositionsFromDatabase(gentity_t *ent)
{

	if (!ent->client)
	{
		return;
	}

	if (!ent->client->sess.loadPreviousSavedPositions)
	{
		return;
	}

	string guid = _session->Guid(ent);

	std::map<string, DisconnectedClient>::iterator it = _savedPositions.find(guid);

	if (it != _savedPositions.end())
	{

		unsigned validPositionsCount = 0;

		for (int i = 0; i < MAX_SAVED_POSITIONS; i++)
		{
			// Allied
			VectorCopy(it->second.alliesSavedPositions[i].origin,
			           _clients[ClientNum(ent)].alliesSavedPositions[i].origin);
			VectorCopy(it->second.alliesSavedPositions[i].vangles,
			           _clients[ClientNum(ent)].alliesSavedPositions[i].vangles);
			_clients[ClientNum(ent)].alliesSavedPositions[i].isValid =
			    it->second.alliesSavedPositions[i].isValid;

			if (it->second.alliesSavedPositions[i].isValid)
			{
				++validPositionsCount;
			}

			// Axis
			VectorCopy(it->second.axisSavedPositions[i].origin,
			           _clients[ClientNum(ent)].axisSavedPositions[i].origin);
			VectorCopy(it->second.axisSavedPositions[i].vangles,
			           _clients[ClientNum(ent)].axisSavedPositions[i].vangles);
			_clients[ClientNum(ent)].axisSavedPositions[i].isValid =
			    it->second.axisSavedPositions[i].isValid;

			if (it->second.axisSavedPositions[i].isValid)
			{
				++validPositionsCount;
			}
		}

		ent->client->sess.loadPreviousSavedPositions = qfalse;
		ent->client->sess.clientMapProgression       = it->second.progression;
		if (validPositionsCount)
		{
			ChatPrintTo(ent, "^<ETJump: ^7loaded saved positions from previous session.");
		}
	}
}

void ETJump::SaveSystem::storeTeamQuickDeployPosition(gentity_t *ent, team_t team)
{
	auto lastValidSave = getValidTeamSaveForSlot(ent, team, 0);
	if (lastValidSave)
	{
		auto client = &_clients[ClientNum(ent)];
		auto autoSave = &(client->quickDeployPositions[team]);
		*autoSave = *lastValidSave;
	}
} 

void ETJump::SaveSystem::loadTeamQuickDeployPosition(gentity_t *ent, team_t team)
{
	auto validSave = getValidTeamQuickDeploySave(ent, team);
	if (validSave)
	{
		restoreStanceFromSave(ent, validSave);
		teleportPlayer(ent, validSave);
	}
}

void ETJump::SaveSystem::loadOnceTeamQuickDeployPosition(gentity_t *ent, team_t team)
{
	auto validSave = getValidTeamQuickDeploySave(ent, team);
	if (validSave)
	{
		restoreStanceFromSave(ent, validSave);
		teleportPlayer(ent, validSave);
		validSave->isValid = false;
	}
}

ETJump::SaveSystem::SavePosition* ETJump::SaveSystem::getValidTeamSaveForSlot(gentity_t *ent, team_t team, int slot)
{
	if (!ent || !ent->client)
	{
		return nullptr;
	}

	if (team != TEAM_ALLIES && team != TEAM_AXIS)
	{
		return nullptr;
	}

	auto client = &_clients[ClientNum(ent)];
	SavePosition *pos = nullptr;
	if (team == TEAM_ALLIES)
	{
		pos = &client->alliesSavedPositions[slot];
	}
	else
	{
		pos = &client->axisSavedPositions[slot];
	}

	if (!pos->isValid)
	{
		return nullptr;
	}

	return pos;
}

ETJump::SaveSystem::SavePosition* ETJump::SaveSystem::getValidTeamUnloadPos(gentity_t* ent, team_t team)
{
	if (!ent || !ent->client)
	{
		return nullptr;
	}

	if (team != TEAM_ALLIES && team != TEAM_AXIS)
	{
		return nullptr;
	}

	auto client = &_clients[ClientNum(ent)];
	SavePosition* pos = nullptr;
	if (team == TEAM_ALLIES)
	{
		pos = &client->alliesLastLoadPosition;
	}
	else
	{
		pos = &client->axisLastLoadPosition;
	}

	if (!pos->isValid)
	{
		return nullptr;
	}

	return pos;
}

ETJump::SaveSystem::SavePosition* ETJump::SaveSystem::getValidTeamQuickDeploySave(gentity_t *ent, team_t team)
{
	if (!ent || !ent->client)
	{
		return nullptr;
	}

	if (team != TEAM_ALLIES && team != TEAM_AXIS)
	{
		return nullptr;
	}

	auto client = &_clients[ClientNum(ent)];
	auto pos = &client->quickDeployPositions[team];

	if (!pos->isValid)
	{
		return nullptr;
	}

	return pos;
}

void ETJump::SaveSystem::restoreStanceFromSave(gentity_t *ent, SavePosition *pos)
{
	if (!ent || !ent->client)
	{
		return;
	}

	auto client = ent->client;
	if (pos->stance == Crouch)
	{
		client->ps.eFlags &= ~EF_PRONE;
		client->ps.eFlags &= ~EF_PRONE_MOVING;
		client->ps.pm_flags |= PMF_DUCKED;
	}
	else if (pos->stance == Prone)
	{
		client->ps.eFlags |= EF_PRONE;
		SetClientViewAngle(ent, pos->vangles);
	}
	else
	{
		client->ps.eFlags &= ~EF_PRONE;
		client->ps.eFlags &= ~EF_PRONE_MOVING;
		client->ps.pm_flags &= ~PMF_DUCKED;
	}
}

// Saves backup position
void ETJump::SaveSystem::saveBackupPosition(gentity_t *ent, SavePosition *pos)
{

	if (!ent->client)
	{
		return;
	}

	if (ent->client->sess.timerunActive &&
		ent->client->sess.runSpawnflags & TIMERUN_DISABLE_BACKUPS) {
		return;
	}

	SavePosition backup;
	VectorCopy(pos->origin, backup.origin);
	VectorCopy(pos->vangles, backup.vangles);
	backup.isValid = pos->isValid;
	backup.stance = pos->stance;
	// Can never be spectator as this would not be called
	if (ent->client->sess.sessionTeam == TEAM_ALLIES)
	{
		_clients[ClientNum(ent)].alliesBackupPositions.push_front(backup);
	}
	else
	{
		_clients[ClientNum(ent)].axisBackupPositions.push_front(backup);
	}

}

void ETJump::SaveSystem::storePosition(gclient_s* client, SavePosition *pos)
{
	VectorCopy(client->ps.origin, pos->origin);
	VectorCopy(client->ps.viewangles, pos->vangles);
	pos->isValid = true;

	if (client->ps.eFlags & EF_CROUCHING)
	{
		pos->stance = Crouch;
	}
	else if (client->ps.eFlags & EF_PRONE)
	{
		pos->stance = Prone;
	}
	else
	{
		pos->stance = Stand;
	}
}


void ETJump::SaveSystem::teleportPlayer(gentity_t* ent, SavePosition* pos)
{
	auto *client = ent->client;
	client->ps.eFlags ^= EF_TELEPORT_BIT;
	G_AddEvent(ent, EV_LOAD_TELEPORT, 0);

	G_SetOrigin(ent, pos->origin);
	VectorClear(client->ps.velocity);

	if (client->pers.loadViewAngles)
	{
		SetClientViewAngle(ent, pos->vangles);
	}

	client->ps.pm_time = 1; // Crashland + instant load bug fix.
}

ETJump::SaveSystem::SaveSystem(const std::shared_ptr<Session> session) :
	_session(session)
	//:guidInterface_(guidInterface)
{

}

ETJump::SaveSystem::~SaveSystem()
{

}
