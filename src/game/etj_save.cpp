#include <vector>

#include "etj_server_commands_handler.h"
#include "etj_save.h"
#include "etj_client.h"
#include "utilities.hpp"
#include "etj_isession.h"

ETJump::SaveSystem::Client::Client()
{
	alliesBackupPositions = boost::circular_buffer<SavePosition>(MAX_BACKUP_POSITIONS);
	axisBackupPositions   = boost::circular_buffer<SavePosition>(MAX_BACKUP_POSITIONS);

	for (auto i = 0; i < MAX_SAVED_POSITIONS; i++)
	{
		alliesSavedPositions[i].isValid = false;
		axisSavedPositions[i].isValid   = false;
	}

	for (auto i = 0; i < MAX_BACKUP_POSITIONS; i++)
	{
		alliesBackupPositions.push_back(SavePosition());
		alliesBackupPositions[i].isValid = false;

		axisBackupPositions.push_back(SavePosition());
		axisBackupPositions[i].isValid = false;
	}
}

ETJump::SaveSystem::DisconnectedClient::DisconnectedClient()
{
	for (auto i = 0; i < MAX_SAVED_POSITIONS; i++)
	{
		alliesSavedPositions[i].isValid = false;
		axisSavedPositions[i].isValid   = false;
		progression                     = 0;
	}
}

// Saves current position
void ETJump::SaveSystem::save(gentity_t *ent, const std::vector<std::string>& arguments)
{
	if (!ent->client)
	{
		return;
	}

	if (!g_save.integer)
	{
		CPTo(ent, "^3Save ^7is not enabled.");
		return;
	}

	auto position = 0;
	if (arguments.size() > 1)
	{
		ToInt(arguments[1], position);

		if (position < 0 || position >= MAX_SAVED_POSITIONS)
		{
			CPTo(ent, "Invalid position.");
			return;
		}
	}

	if (!ent->client->sess.saveAllowed)
	{
		CPTo(ent, "You are not allowed to save a position.");
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		CPTo(ent, "^7You can not ^3save^7 as a spectator.");
		return;
	}

	trace_t trace;
	trap_TraceCapsule(&trace, ent->client->ps.origin, ent->r.mins,
	                  ent->r.maxs, ent->client->ps.origin, ent->s.number, CONTENTS_NOSAVE);

	if (level.noSave)
	{
		if (!trace.fraction != 1.0f)
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

	if (ent->client->pers.race.isRacing)
	{
		if (ent->client->pers.race.saveLimit == 0)
		{
			CPTo(ent, "^5You've used all your saves.");
			return;
		}

		if (ent->client->pers.race.saveLimit > 0)
		{
			ent->client->pers.race.saveLimit--;
		}
	}
	else
	{
		fireteamData_t *ft;
		if (G_IsOnFireteam(ent - g_entities, &ft))
		{
			if (ft->saveLimit < 0)
			{
				ent->client->sess.saveLimit = 0;
			}
			if (ft->saveLimit)
			{
				if (ent->client->sess.saveLimit)
				{
					ent->client->sess.saveLimit--;
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
	if (ent->client->sess.sessionTeam == TEAM_ALLIES)
	{
		pos = clients_[ClientNum(ent)].alliesSavedPositions + position;
	}
	else
	{
		pos = clients_[ClientNum(ent)].axisSavedPositions + position;
	}

	saveBackupPosition(ent, pos);

	VectorCopy(ent->client->ps.origin, pos->origin);
	VectorCopy(ent->client->ps.viewangles, pos->vangles);
	pos->isValid = true;

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
void ETJump::SaveSystem::load(gentity_t *ent, const std::vector<std::string>& arguments)
{

	if (!ent->client)
	{
		return;
	}

	if (!g_save.integer)
	{
		CPTo(ent, "^3Load ^7is not enabled.");
		return;
	}

	if (!ent->client->sess.saveAllowed)
	{
		CPTo(ent, "You are not allowed to load a position.");
		return;
	}

	auto position = 0;
	if (arguments.size() > 1)
	{
		ToInt(arguments[1], position);

		if (position < 0 || position >= MAX_SAVED_POSITIONS)
		{
			CPTo(ent, "^7Invalid position.");
			return;
		}
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		CPTo(ent, "^7You can not ^3load ^7as a spectator.");
		return;
	}

	SavePosition *pos = nullptr;
	if (ent->client->sess.sessionTeam == TEAM_ALLIES)
	{
		pos = clients_[ClientNum(ent)].alliesSavedPositions + position;
	}
	else
	{
		pos = clients_[ClientNum(ent)].axisSavedPositions + position;
	}

	if (pos->isValid)
	{
		ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
		VectorCopy(pos->origin, ent->client->ps.origin);
		VectorClear(ent->client->ps.velocity);
		if (ent->client->pers.loadViewAngles)
		{
			SetClientViewAngle(ent, pos->vangles);
		}
		// Crashland + instant load bug fix.
		ent->client->ps.pm_time = 1;
	}
	else
	{
		CPTo(ent, "^7Use ^3save ^7first.");
	}
}

// Saves position, does not check for anything. Used for target_save
void ETJump::SaveSystem::forceSave(gentity_t *location, gentity_t *ent)
{

	if (!ent->client || !location)
	{
		return;
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		return;
	}
	if (ent->client->sess.sessionTeam == TEAM_ALLIES)
	{
		saveBackupPosition(ent,
		                   &clients_[ClientNum(ent)].alliesSavedPositions[0]);
		VectorCopy(location->s.origin,
		           clients_[ClientNum(ent)].alliesSavedPositions[0].origin);
		VectorCopy(location->s.angles,
		           clients_[ClientNum(ent)].alliesSavedPositions[0].vangles);
		clients_[ClientNum(ent)].alliesSavedPositions[0].isValid = true;
	}
	else if (ent->client->sess.sessionTeam == TEAM_AXIS)
	{
		saveBackupPosition(ent, &clients_[ClientNum(ent)].axisSavedPositions[0]);
		VectorCopy(location->s.origin,
		           clients_[ClientNum(ent)].axisSavedPositions[0].origin);
		VectorCopy(location->s.angles,
		           clients_[ClientNum(ent)].axisSavedPositions[0].vangles);
		clients_[ClientNum(ent)].axisSavedPositions[0].isValid = true;
	}

	trap_SendServerCommand(ent - g_entities, g_savemsg.string);
}

// Loads backup position
void ETJump::SaveSystem::loadBackupPosition(gentity_t *ent, const std::vector<std::string>& arguments)
{

	if (!ent->client)
	{
		return;
	}

	if (!g_save.integer)
	{
		CPTo(ent, "^3Load ^7is not enabled.");
		return;
	}

	if (!ent->client->sess.saveAllowed)
	{
		CPTo(ent, "You are not allowed to load a position.");
		return;
	}

	auto position = 0;
	if (arguments.size() > 1)
	{
		ToInt(arguments.at(1), position);

		if (position < 1 || position > MAX_SAVED_POSITIONS)
		{
			CPTo(ent, "^7Invalid position.");
			return;
		}

		if (position > 0)
		{
			position--;
		}
	}

	if (ent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		CPTo(ent, "^7You can not ^3load ^7as a spectator.");
		return;
	}

	SavePosition *pos = nullptr;
	if (ent->client->sess.sessionTeam == TEAM_ALLIES)
	{
		pos = &clients_[ClientNum(ent)].alliesBackupPositions[position];
	}
	else
	{
		pos = &clients_[ClientNum(ent)].axisBackupPositions[position];
	}

	if (pos->isValid)
	{
		ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
		VectorCopy(pos->origin, ent->client->ps.origin);
		VectorClear(ent->client->ps.velocity);
		if (ent->client->pers.loadViewAngles)
		{
			SetClientViewAngle(ent, pos->vangles);
		}
		// Crashland + instant load bug fix.
		ent->client->ps.pm_time = 1;
	}
	else
	{
		CPTo(ent, "^7Use ^3save ^7first.");
	}
}

void ETJump::SaveSystem::reset()
{
	for (auto clientIndex = 0; clientIndex < level.numConnectedClients; clientIndex++)
	{
		auto clientNum = level.sortedClients[clientIndex];
		// TODO: reset saved positions here
		resetSavedPositions(g_entities + clientNum);
	}

	savedPositions.clear();
}

// Used to reset positions on map change/restart
void ETJump::SaveSystem::resetSavedPositions(gentity_t *ent)
{
	for (auto saveIndex = 0; saveIndex < MAX_SAVED_POSITIONS; saveIndex++)
	{
		clients_[ClientNum(ent)].alliesSavedPositions[saveIndex].isValid = false;
		clients_[ClientNum(ent)].axisSavedPositions[saveIndex].isValid   = false;
	}

	for (auto backupIndex = 0; backupIndex < MAX_BACKUP_POSITIONS; backupIndex++)
	{
		clients_[ClientNum(ent)].alliesBackupPositions[backupIndex].isValid = false;
		clients_[ClientNum(ent)].axisBackupPositions[backupIndex].isValid   = false;
	}
}

// Called on client disconnect. Saves saves for future sessions
void ETJump::SaveSystem::savePositionsToDatabase(gentity_t *ent)
{

	if (!ent->client)
	{
		return;
	}

	auto clientSession = _session->client((g_entities - ent));
	if (!clientSession)
	{
		return;
	}

	auto guid = clientSession->guid();

	DisconnectedClient client;

	for (auto i = 0; i < MAX_SAVED_POSITIONS; i++)
	{
		// Allied
		VectorCopy(clients_[ClientNum(ent)].alliesSavedPositions[i].origin,
		           client.alliesSavedPositions[i].origin);
		VectorCopy(clients_[ClientNum(ent)].alliesSavedPositions[i].vangles,
		           client.alliesSavedPositions[i].vangles);
		client.alliesSavedPositions[i].isValid =
		    clients_[ClientNum(ent)].alliesSavedPositions[i].isValid;
		// Axis
		VectorCopy(clients_[ClientNum(ent)].axisSavedPositions[i].origin,
		           client.axisSavedPositions[i].origin);
		VectorCopy(clients_[ClientNum(ent)].axisSavedPositions[i].vangles,
		           client.axisSavedPositions[i].vangles);
		client.axisSavedPositions[i].isValid
		    = clients_[ClientNum(ent)].axisSavedPositions[i].isValid;
	}

	client.progression                           = ent->client->sess.clientMapProgression;
	ent->client->sess.loadPreviousSavedPositions = qfalse;

	auto it = savedPositions.find(guid);

	if (it != savedPositions.end())
	{
		it->second = client;
	}
	else
	{
		savedPositions.insert(std::make_pair(guid, client));
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

	auto clientSession = _session->client((g_entities - ent));
	if (!clientSession)
	{
		return;
	}

	auto it = savedPositions.find(clientSession->guid());

	if (it != savedPositions.end())
	{

		unsigned validPositionsCount = 0;

		for (auto i = 0; i < MAX_SAVED_POSITIONS; i++)
		{
			// Allied
			VectorCopy(it->second.alliesSavedPositions[i].origin,
			           clients_[ClientNum(ent)].alliesSavedPositions[i].origin);
			VectorCopy(it->second.alliesSavedPositions[i].vangles,
			           clients_[ClientNum(ent)].alliesSavedPositions[i].vangles);
			clients_[ClientNum(ent)].alliesSavedPositions[i].isValid =
			    it->second.alliesSavedPositions[i].isValid;

			if (it->second.alliesSavedPositions[i].isValid)
			{
				++validPositionsCount;
			}

			// Axis
			VectorCopy(it->second.axisSavedPositions[i].origin,
			           clients_[ClientNum(ent)].axisSavedPositions[i].origin);
			VectorCopy(it->second.axisSavedPositions[i].vangles,
			           clients_[ClientNum(ent)].axisSavedPositions[i].vangles);
			clients_[ClientNum(ent)].axisSavedPositions[i].isValid =
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

// Saves backup position
void ETJump::SaveSystem::saveBackupPosition(gentity_t *ent, SavePosition *pos)
{

	if (!ent->client)
	{
		return;
	}

	SavePosition backup;
	VectorCopy(pos->origin, backup.origin);
	VectorCopy(pos->vangles, backup.vangles);
	backup.isValid = pos->isValid;
	// Can never be spectator as this would not be called
	if (ent->client->sess.sessionTeam == TEAM_ALLIES)
	{
		clients_[ClientNum(ent)].alliesBackupPositions.push_front(backup);
	}
	else
	{
		clients_[ClientNum(ent)].axisBackupPositions.push_front(backup);
	}

}

ETJump::SaveSystem::SaveSystem(ISession* session, ServerCommandsHandler *commandsHandler): _session(session), _commandsHandler(commandsHandler)
{
	_commandsHandler->subscribe("save", [&](int clientNum, const std::vector<std::string>& arguments)
	{
		save(g_entities + clientNum, arguments);
	});
	_commandsHandler->subscribe("load", [&](int clientNum, const std::vector<std::string>& arguments)
	{
		load(g_entities + clientNum, arguments);
	});
	_commandsHandler->subscribe("backup", [&](int clientNum, const std::vector<std::string>& arguments)
	{
		loadBackupPosition(g_entities + clientNum, arguments);
	});
}


ETJump::SaveSystem::~SaveSystem()
{
	_commandsHandler->unsubcribe("save");
	_commandsHandler->unsubcribe("load");
	_commandsHandler->unsubcribe("backup");
}

