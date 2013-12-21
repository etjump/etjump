#include <string>
#include <vector>
using std::string;
using std::vector;

#include "g_save.hpp"
#include "g_utilities.hpp"
#include "g_local.hpp"

// All clients' save related data
SaveSystem::Client SaveSystem::clients_[MAX_CLIENTS];

// Disconnected clients saved position data
std::map<std::string, SaveSystem::DisconnectedClient> SaveSystem::savedPositions;

SaveSystem::Client::Client() {
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

SaveSystem::DisconnectedClient::DisconnectedClient() {
    for(unsigned i = 0; i < MAX_SAVED_POSITIONS; i++) {
        alliesSavedPositions[i].isValid = false;
        axisSavedPositions[i].isValid = false;
        progression = 0;
    }
}

// Zero: required for saving saves to db
std::string UserDatabase_Guid(gentity_t *ent);

// Saves current position
void SaveSystem::Save(gentity_t *ent) {

    if(!ent->client) {
        return;
    }

    if(!g_save.integer) {
        CPTo(ent, "^3Save ^7is not enabled.");
        return;
    }

    Arguments argv = GetArgs();

    int position = 0;
    if(argv->size() > 1) {
        StringToInt((*argv)[1], position);

        if(position < 0 || position > MAX_SAVED_POSITIONS) {
            CPTo(ent, "Invalid position.");
            return;
        }
    }

    if(!ent->client->sess.saveAllowed) {
        CPTo(ent, "You are not allowed to save a position.");
        return;
    }

    if(ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
        CPTo(ent, "^7You can not ^3save^7 as a spectator.");
        return;
    }

    trace_t trace;
    trap_TraceCapsule(&trace, ent->client->ps.origin, ent->r.mins, 
        ent->r.maxs, ent->client->ps.origin, ent->s.number, CONTENTS_NOSAVE);

    if(level.noSave) {
		if(!trace.fraction != 1.0f) {
            CPTo(ent, "^7You can not ^3save ^7inside this area.");
			return;
		}
	} else {
		if (trace.fraction != 1.0f)
		{
			CPTo(ent, "^7You can not ^3save ^7inside this area.");
			return;
		}
	}

    SavePosition *pos = 0;
    if(ent->client->sess.sessionTeam == TEAM_ALLIES) {
        pos = clients_[ent->client->ps.clientNum].alliesSavedPositions + position;
    } else {
        pos = clients_[ent->client->ps.clientNum].axisSavedPositions + position;
    }

    SaveBackupPosition(ent, pos);

    VectorCopy(ent->client->ps.origin, pos->origin);
	VectorCopy(ent->client->ps.viewangles, pos->vangles);
	pos->isValid = true;

    if (position == 0) 
		CP(va("cp \"%s\n\"", g_savemsg.string));
	else
		CP(va("cp \"%s ^7%d\n\"", g_savemsg.string, position));
}

// Loads position
void SaveSystem::Load(gentity_t *ent) {

    if(!ent->client) {
        return;
    }

    if(!g_save.integer) {
        CPTo(ent, "^3Load ^7is not enabled.");
        return;
    }

    if(!ent->client->sess.saveAllowed) {
        CPTo(ent, "You are not allowed to load a position.");
        return;
    }

    Arguments argv = GetArgs();

    int position = 0;
    if(argv->size() > 1) {
        StringToInt(argv->at(1), position);

        if(position < 0 || position >= MAX_SAVED_POSITIONS) {
            CPTo(ent, "^7Invalid position.");
            return;
        }
    }

    if(ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
        CPTo(ent, "^7You can not ^3load ^7as a spectator.");
        return;
    }

    SavePosition *pos = 0;
    if(ent->client->sess.sessionTeam == TEAM_ALLIES) {
        pos = clients_[ent->client->ps.clientNum].alliesSavedPositions + position;
    } else {
        pos = clients_[ent->client->ps.clientNum].axisSavedPositions + position;
    }

    if(pos->isValid) {
        ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
        VectorCopy(pos->origin, ent->client->ps.origin);
		VectorClear(ent->client->ps.velocity);
		if(ent->client->pers.loadViewAngles) {
			SetClientViewAngle(ent, pos->vangles);
		}
		// Crashland + instant load bug fix.
		ent->client->ps.pm_time = 1;
    }

    else {
        CPTo(ent, "^7Use ^3save ^7first.");
    }
}

// Saves position, does not check for anything. Used for target_save
void SaveSystem::ForceSave(gentity_t *location, gentity_t *ent)  {

    if(!ent->client || !location) {
        return;
    }

    if( ent->client->sess.sessionTeam == TEAM_SPECTATOR )
	{
		return;
	} 
	else if ( ent->client->sess.sessionTeam == TEAM_ALLIES )
	{
        SaveBackupPosition(ent, 
            &clients_[ent->client->ps.clientNum].alliesSavedPositions[0]);
		VectorCopy( location->s.origin, 
            clients_[ent->client->ps.clientNum].alliesSavedPositions[0].origin );
		VectorCopy( location->s.angles, 
            clients_[ent->client->ps.clientNum].alliesSavedPositions[0].vangles );
		clients_[ent->client->ps.clientNum].alliesSavedPositions[0].isValid = true;
	} 
	else if ( ent->client->sess.sessionTeam == TEAM_AXIS )
	{
        SaveBackupPosition(ent, &clients_[ent->client->ps.clientNum].axisSavedPositions[0]);
		VectorCopy( location->s.origin, 
            clients_[ent->client->ps.clientNum].axisSavedPositions[0].origin );
		VectorCopy( location->s.angles, 
            clients_[ent->client->ps.clientNum].axisSavedPositions[0].vangles );
		clients_[ent->client->ps.clientNum].axisSavedPositions[0].isValid = true;
	}

    trap_SendServerCommand(ent-g_entities, g_savemsg.string);
}
    
// Loads backup position
void SaveSystem::LoadBackupPosition(gentity_t *ent) {

    if(!ent->client) {
        return;
    }

    if(!g_save.integer) {
        CPTo(ent, "^3Load ^7is not enabled.");
        return;
    }

    if(!ent->client->sess.saveAllowed) {
        CPTo(ent, "You are not allowed to load a position.");
        return;
    }

    Arguments argv = GetArgs();

    int position = 0;
    if(argv->size() > 1) {
        StringToInt(argv->at(1), position);

        if(position < 1 || position > MAX_SAVED_POSITIONS) {
            CPTo(ent, "^7Invalid position.");
            return;
        }

       if(position > 0) {
           position--;
       }
    }

    if(ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
        CPTo(ent, "^7You can not ^3load ^7as a spectator.");
        return;
    }

    SavePosition *pos = 0;
    if(ent->client->sess.sessionTeam == TEAM_ALLIES) {
        pos = &clients_[ent->client->ps.clientNum].alliesBackupPositions[position];
    } else {
        pos = &clients_[ent->client->ps.clientNum].axisBackupPositions[position];
    }

    if(pos->isValid) {
        ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
        VectorCopy(pos->origin, ent->client->ps.origin);
		VectorClear(ent->client->ps.velocity);
		if(ent->client->pers.loadViewAngles) {
			SetClientViewAngle(ent, pos->vangles);
		}
		// Crashland + instant load bug fix.
		ent->client->ps.pm_time = 1;
    }

    else {
        CPTo(ent, "^7Use ^3save ^7first.");
    }
}

void SaveSystem::Reset() {
    for(int clientIndex = 0; clientIndex < level.numConnectedClients; clientIndex++) {
        int clientNum = level.sortedClients[clientIndex];
        ResetSavedPositions(g_entities + clientNum);
    }

    savedPositions.clear();
}

// Used to reset positions on map change/restart
void SaveSystem::ResetSavedPositions(gentity_t *ent) {
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
void SaveSystem::SavePositionsToDatabase(gentity_t *ent) {
	
    if(!ent->client) {
        return;
    }

//     string guid = Session::Guid(ent);
// 
//     DisconnectedClient client;
// 
//     for(unsigned i = 0; i < MAX_SAVED_POSITIONS; i++) {
//         // Allied
//         VectorCopy(clients_[ent->client->ps.clientNum].alliesSavedPositions[i].origin,
//             client.alliesSavedPositions[i].origin);
//         VectorCopy(clients_[ent->client->ps.clientNum].alliesSavedPositions[i].vangles,
//             client.alliesSavedPositions[i].vangles);
//             client.alliesSavedPositions[i].isValid = 
//                 clients_[ent->client->ps.clientNum].alliesSavedPositions[i].isValid;
//         // Axis
//         VectorCopy(clients_[ent->client->ps.clientNum].axisSavedPositions[i].origin,
//             client.axisSavedPositions[i].origin);
//         VectorCopy(clients_[ent->client->ps.clientNum].axisSavedPositions[i].vangles,
//             client.axisSavedPositions[i].vangles);
//         client.axisSavedPositions[i].isValid 
//             = clients_[ent->client->ps.clientNum].axisSavedPositions[i].isValid;
//     }
// 
//     client.progression = ent->client->sess.clientMapProgression;
//     ent->client->sess.loadedSavedPositions = qfalse;
// 
//     std::map<string, DisconnectedClient>::iterator it = savedPositions.find(guid);
// 
//     if(it != savedPositions.end()) {
//         it->second = client;
//     } else {
//         savedPositions.insert(std::make_pair(guid, client));
//     }
    
}

// Called on client connect. Loads saves from previous session
void SaveSystem::LoadPositionsFromDatabase(gentity_t *ent) {
	
    if(!ent->client) {
        return;
    }

//     if(ent->client->sess.loadedSavedPositions) {
//         return;
//     }
// 
//     string guid = Session::Guid(ent);
// 
//     std::map<string, DisconnectedClient>::iterator it = savedPositions.find(guid);
// 
//     if(it != savedPositions.end()) {
// 
//         for(unsigned i = 0; i < MAX_SAVED_POSITIONS; i++) {
//             // Allied
//             VectorCopy(it->second.alliesSavedPositions[i].origin,
//                 clients_[ent->client->ps.clientNum].alliesSavedPositions[i].origin);
//             VectorCopy(it->second.alliesSavedPositions[i].vangles,
//                 clients_[ent->client->ps.clientNum].alliesSavedPositions[i].vangles);
//             clients_[ent->client->ps.clientNum].alliesSavedPositions[i].isValid = 
//                 it->second.alliesSavedPositions[i].isValid;
//             // Axis
//             VectorCopy(it->second.axisSavedPositions[i].origin,
//                 clients_[ent->client->ps.clientNum].axisSavedPositions[i].origin);
//             VectorCopy(it->second.axisSavedPositions[i].vangles,
//                 clients_[ent->client->ps.clientNum].axisSavedPositions[i].vangles);
//             clients_[ent->client->ps.clientNum].axisSavedPositions[i].isValid = 
//                 it->second.axisSavedPositions[i].isValid;
//         }
// 
//         ent->client->sess.clientMapProgression = it->second.progression;
//         ent->client->sess.loadedSavedPositions = qtrue;
//         ChatPrintTo(ent, "^5ETJump: ^7loaded positions from previous session.");
//     }
}

// Saves backup position
void SaveSystem::SaveBackupPosition(gentity_t *ent, SavePosition *pos) {
	
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

void SaveSystem::Print( gentity_t *ent )
{
    std::map<string, DisconnectedClient>::const_iterator it = savedPositions.begin();

    string toPrint = "Save database:\n";
    while(it != savedPositions.end()) {

        toPrint += it->first + NEWLINE;

        int b0 = 0;
        int b1 = 0;
        int b2 = 0;

        int r0 = 0;
        int r1 = 0;
        int r2 = 0;

        for(int i = 0; i < 3; i++) {

            b0 = it->second.alliesSavedPositions[i].origin[0];
            b1 = it->second.alliesSavedPositions[i].origin[1];
            b2 = it->second.alliesSavedPositions[i].origin[2];

            r0 = it->second.axisSavedPositions[i].origin[0];
            r1 = it->second.axisSavedPositions[i].origin[1];
            r2 = it->second.axisSavedPositions[i].origin[2];

            toPrint += 
                IntToString(it->second.alliesSavedPositions[i].isValid) +
                IntToString(it->second.axisSavedPositions[i].isValid) +
                "B: " + Vec3ToString(b0, b1, b2) + 
                "R: " + Vec3ToString(r0, r1, r2) + NEWLINE;
        }
        it++;
    }
    ConsolePrintTo(NULL, toPrint);
}

// C API for save&load db
void Cmd_Load_f(gentity_t *ent) {
    SaveSystem::Load(ent);
}

void Cmd_Save_f(gentity_t *ent) {
    SaveSystem::Save(ent);
}

void Cmd_BackupLoad_f(gentity_t *ent) {
    SaveSystem::LoadBackupPosition(ent);
}

void ResetSavedPositions(gentity_t *ent) {
    SaveSystem::ResetSavedPositions(ent);
}

void ForceSave(gentity_t *location, gentity_t *ent) {
    SaveSystem::ForceSave(location, ent);
}

void SavePositionsToDatabase(gentity_t *ent) {
    SaveSystem::SavePositionsToDatabase(ent);
}

void LoadPositionsFromDatabase(gentity_t *ent) {
    SaveSystem::LoadPositionsFromDatabase(ent);
}

void InitSaveDatabase() {
    SaveSystem::Reset();
}

void SaveSystem_Print( gentity_t *ent ) {
    SaveSystem::Print( ent );
}

void Cmd_SaveReset_f(gentity_t *ent)
{
    SaveSystem::ResetSavedPositions(ent);
    CPTo(ent, "Your saved positions have been reseted.");
}