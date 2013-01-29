#include <string>
#include <vector>
using std::string;
using std::vector;

#include "g_save.h"
#include "g_utilities.h"
#include "g_admin.h"

Save positionData;

Save::Client::Client() {
    alliesBackupPositions = boost::circular_buffer<SavePosition>(MAX_BACKUP_POSITIONS);
    axisBackupPositions = boost::circular_buffer<SavePosition>(MAX_BACKUP_POSITIONS);

    for(unsigned i = 0; i < MAX_SAVED_POSITIONS; i++) {
        alliesSavedPositions[i].isValid = false;
        axisSavedPositions[i].isValid = false;
    }

    for(unsigned i = 0; i < MAX_BACKUP_POSITIONS; i++) {
        alliesBackupPositions.push_back(SavePosition());
        alliesBackupPositions[i].isValid = false;

        axisBackupPositions.push_back(SavePosition());
        axisBackupPositions[i].isValid = false;
    }
}

Save::DisconnectedClient::DisconnectedClient() {
    for(unsigned i = 0; i < MAX_SAVED_POSITIONS; i++) {
        alliesSavedPositions[i].isValid = false;
        axisSavedPositions[i].isValid = false;
        progression = 0;
    }
}

// Saves current position
void Save::save(gentity_t *ent) {

    if(!ent->client) {
        return;
    }

    if(!g_save.integer) {
        CPPrintTo(ent, "^3Save ^7is not enabled.");
        return;
    }

    Arguments argv = GetArgs();

    int position = 0;
    if(argv->size() > 1) {
        StringToInt((*argv)[1], position);

        if(position < 0 || position > MAX_SAVED_POSITIONS) {
            CPPrintTo(ent, "Invalid position.");
            return;
        }
    }

    if(!ent->client->sess.save_allowed) {
        CPPrintTo(ent, "You are not allowed to save a position.");
        return;
    }

    if(ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
        CPPrintTo(ent, "^7You can not ^3save^7 as a spectator.");
        return;
    }

    trace_t trace;
    trap_TraceCapsule(&trace, ent->client->ps.origin, ent->r.mins, 
        ent->r.maxs, ent->client->ps.origin, ent->s.number, CONTENTS_NOSAVE);

    if(level.noSave) {
		if(!trace.fraction != 1.0f) {
            CPPrintTo(ent, "^7You can not ^3save ^7inside this area.");
			return;
		}
	} else {
		if (trace.fraction != 1.0f)
		{
			CPPrintTo(ent, "^7You can not ^3save ^7inside this area.");
			return;
		}
	}

    SavePosition *pos = 0;
    if(ent->client->sess.sessionTeam == TEAM_ALLIES) {
        pos = clients_[ent->client->ps.clientNum].alliesSavedPositions + position;
    } else {
        pos = clients_[ent->client->ps.clientNum].axisSavedPositions + position;
    }

    saveBackupPosition(ent, pos);

    VectorCopy(ent->client->ps.origin, pos->origin);
	VectorCopy(ent->client->ps.viewangles, pos->vangles);
	pos->isValid = true;

    if (position == 0) 
		CP(va("cp \"%s\n\"", g_savemsg.string));
	else
		CP(va("cp \"%s ^7%d\n\"", g_savemsg.string, position));
}

// Loads position
void Save::load(gentity_t *ent) {

    if(!ent->client) {
        return;
    }

    if(!g_save.integer) {
        CPPrintTo(ent, "^3Load ^7is not enabled.");
        return;
    }

    if(!ent->client->sess.save_allowed) {
        CPPrintTo(ent, "You are not allowed to load a position.");
        return;
    }

    Arguments argv = GetArgs();

    int position = 0;
    if(argv->size() > 1) {
        StringToInt(argv->at(1), position);

        if(position < 0 || position >= MAX_SAVED_POSITIONS) {
            CPPrintTo(ent, "^7Invalid position.");
            return;
        }
    }

    if(ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
        CPPrintTo(ent, "^7You can not ^3load ^7as a spectator.");
        return;
    }

    SavePosition *pos = 0;
    if(ent->client->sess.sessionTeam == TEAM_ALLIES) {
        pos = clients_[ent->client->ps.clientNum].alliesSavedPositions + position;
    } else {
        pos = clients_[ent->client->ps.clientNum].axisSavedPositions + position;
    }

    if(pos->isValid) {
        VectorCopy(pos->origin, ent->client->ps.origin);
		VectorClear(ent->client->ps.velocity);
		if(ent->client->pers.loadViewAngles) {
			SetClientViewAngle(ent, pos->vangles);
		}
		// Crashland + instant load bug fix.
		ent->client->ps.pm_time = 1;
    }

    else {
        CPPrintTo(ent, "^7Use ^3save ^7first.");
    }
}

// Saves position, does not check for anything
void Save::forceSave(gentity_t *location, gentity_t *ent)  {

    if(!ent->client || !location) {
        return;
    }

    if( ent->client->sess.sessionTeam == TEAM_SPECTATOR )
	{
		return;
	} 
	else if ( ent->client->sess.sessionTeam == TEAM_ALLIES )
	{
        saveBackupPosition(ent, &clients_[ent->client->ps.clientNum].alliesSavedPositions[0]);
		VectorCopy( location->s.origin, 
            clients_[ent->client->ps.clientNum].alliesSavedPositions[0].origin );
		VectorCopy( location->s.angles, 
            clients_[ent->client->ps.clientNum].alliesSavedPositions[0].vangles );
		clients_[ent->client->ps.clientNum].alliesSavedPositions[0].isValid = true;
	} 
	else if ( ent->client->sess.sessionTeam == TEAM_AXIS )
	{
        saveBackupPosition(ent, &clients_[ent->client->ps.clientNum].axisSavedPositions[0]);
		VectorCopy( location->s.origin, 
            clients_[ent->client->ps.clientNum].axisSavedPositions[0].origin );
		VectorCopy( location->s.angles, 
            clients_[ent->client->ps.clientNum].axisSavedPositions[0].vangles );
		clients_[ent->client->ps.clientNum].axisSavedPositions[0].isValid = true;
	}

    trap_SendServerCommand(ent-g_entities, g_savemsg.string);
}
    
// Loads backup position
void Save::loadBackupPosition(gentity_t *ent) {

    if(!ent->client) {
        return;
    }

    if(!g_save.integer) {
        CPPrintTo(ent, "^3Load ^7is not enabled.");
        return;
    }

    if(!ent->client->sess.save_allowed) {
        CPPrintTo(ent, "You are not allowed to load a position.");
        return;
    }

    Arguments argv = GetArgs();

    int position = 0;
    if(argv->size() > 1) {
        StringToInt(argv->at(1), position);

        if(position < 1 || position > MAX_SAVED_POSITIONS) {
            CPPrintTo(ent, "^7Invalid position.");
            return;
        }

       if(position > 0) {
           position--;
       }
    }

    if(ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
        CPPrintTo(ent, "^7You can not ^3load ^7as a spectator.");
        return;
    }

    SavePosition *pos = 0;
    if(ent->client->sess.sessionTeam == TEAM_ALLIES) {
        pos = &clients_[ent->client->ps.clientNum].alliesBackupPositions[position];
    } else {
        pos = &clients_[ent->client->ps.clientNum].axisBackupPositions[position];
    }

    if(pos->isValid) {
        VectorCopy(pos->origin, ent->client->ps.origin);
		VectorClear(ent->client->ps.velocity);
		if(ent->client->pers.loadViewAngles) {
			SetClientViewAngle(ent, pos->vangles);
		}
		// Crashland + instant load bug fix.
		ent->client->ps.pm_time = 1;
    }

    else {
        CPPrintTo(ent, "^7Use ^3save ^7first.");
    }
}

void Save::reset() {
    for(unsigned clientIndex = 0; clientIndex < level.numConnectedClients; clientIndex++) {
        unsigned clientNum = level.sortedClients[clientIndex];
        resetSavedPositions(g_entities + clientNum);
    }

    savedPositions.clear();
}

// Used to reset positions on map change/restart
void Save::resetSavedPositions(gentity_t *ent) {
    for(unsigned saveIndex = 0; saveIndex < MAX_SAVED_POSITIONS; saveIndex++) {
        clients_[ent->client->ps.clientNum].alliesSavedPositions[saveIndex].isValid = false;
        clients_[ent->client->ps.clientNum].axisSavedPositions[saveIndex].isValid = false;
    }

    for(unsigned backupIndex = 0; backupIndex < MAX_BACKUP_POSITIONS; backupIndex++) {
        clients_[ent->client->ps.clientNum].alliesBackupPositions[backupIndex].isValid = false;
        clients_[ent->client->ps.clientNum].axisBackupPositions[backupIndex].isValid = false;
    }
}

// Called on client disconnect. Saves saves for future sessions
void Save::savePositionsToDatabase(gentity_t *ent) {

    if(!ent->client) {
        return;
    }

    string guid = admin::Guid(ent);

    if(guid.length() != 40) {
        return;
    }

    DisconnectedClient client;

    for(unsigned i = 0; i < MAX_SAVED_POSITIONS; i++) {
        // Allied
        VectorCopy(clients_[ent->client->ps.clientNum].alliesSavedPositions[i].origin,
            client.alliesSavedPositions[i].origin);
        VectorCopy(clients_[ent->client->ps.clientNum].alliesSavedPositions[i].vangles,
            client.alliesSavedPositions[i].vangles);
        clients_[ent->client->ps.clientNum].alliesSavedPositions[i].isValid =
            client.alliesSavedPositions[i].isValid;
        // Axis
        VectorCopy(clients_[ent->client->ps.clientNum].axisSavedPositions[i].origin,
            client.axisSavedPositions[i].origin);
        VectorCopy(clients_[ent->client->ps.clientNum].axisSavedPositions[i].vangles,
            client.axisSavedPositions[i].vangles);
        clients_[ent->client->ps.clientNum].axisSavedPositions[i].isValid =
            client.axisSavedPositions[i].isValid;
    }

    client.progression = ent->client->sess.client_map_id;
    ent->client->sess.loadedSavedPositions = qfalse;

    map<string, DisconnectedClient>::iterator it = savedPositions.find(guid);

    if(it != savedPositions.end()) {
        it->second = client;
    } else {
        savedPositions.insert(std::make_pair(guid, client));
    }
}

// Called on client connect. Loads saves from previous session
void Save::loadPositionsFromDatabase(gentity_t *ent) {

    if(!ent->client) {
        return;
    }

    if(ent->client->sess.loadedSavedPositions) {
        return;
    }

    string guid = admin::Guid(ent);

    if(guid.length() != 40) {
        return;
    }

    map<string, DisconnectedClient>::iterator it = savedPositions.find(guid);

    if(it != savedPositions.end()) {
        for(unsigned i = 0; i < MAX_SAVED_POSITIONS; i++) {
            // Allied
            VectorCopy(it->second.alliesSavedPositions[i].origin,
                clients_[ent->client->ps.clientNum].alliesSavedPositions[i].origin);
            VectorCopy(it->second.alliesSavedPositions[i].vangles,
                clients_[ent->client->ps.clientNum].alliesBackupPositions[i].vangles);
            it->second.alliesSavedPositions[i].isValid =
                clients_[ent->client->ps.clientNum].alliesBackupPositions[i].isValid;
            // Axis
            VectorCopy(it->second.axisSavedPositions[i].origin,
                clients_[ent->client->ps.clientNum].axisSavedPositions[i].origin);
            VectorCopy(it->second.axisSavedPositions[i].vangles,
                clients_[ent->client->ps.clientNum].axisBackupPositions[i].vangles);
            it->second.axisSavedPositions[i].isValid =
                clients_[ent->client->ps.clientNum].axisBackupPositions[i].isValid;
        }
        ent->client->sess.client_map_id = it->second.progression;
        ent->client->sess.loadedSavedPositions = qtrue;
        ChatPrintTo(ent, "^5ETJump: ^7loaded positions from previous session.");
    }
}

// Saves backup position
void Save::saveBackupPosition(gentity_t *ent, SavePosition *pos) {

    if(!ent->client) {
        return;
    }

    SavePosition backup;
    VectorCopy(pos->origin, backup.origin);
	VectorCopy(pos->vangles, backup.vangles);
    backup.isValid = pos->isValid;
    // Can never be spectator as this would not be called
    if(ent->client->sess.sessionTeam == TEAM_ALLIES) {
        clients_[ent->client->ps.clientNum].alliesBackupPositions.push_front(backup);
    } else {
        clients_[ent->client->ps.clientNum].axisBackupPositions.push_front(backup);
    }
}

// C API for save&load db
void Cmd_Load_f(gentity_t *ent) {
    positionData.load(ent);
}

void Cmd_Save_f(gentity_t *ent) {
    positionData.save(ent);
}

void Cmd_BackupLoad_f(gentity_t *ent) {
    positionData.loadBackupPosition(ent);
}

void ResetSavedPositions(gentity_t *ent) {
    positionData.resetSavedPositions(ent);
}

void forceSave(gentity_t *location, gentity_t *ent) {
    positionData.forceSave(location, ent);
}

void savePositionsToDatabase(gentity_t *ent) {
    positionData.savePositionsToDatabase(ent);
}

void loadPositionsFromDatabase(gentity_t *ent) {
    positionData.loadPositionsFromDatabase(ent);
}

void initSaveDatabase() {
    positionData.reset();
}