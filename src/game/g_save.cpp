#include <string>
#include <vector>
using std::string;
using std::vector;

#include "g_save.hpp"
#include "g_utilities.hpp"
#include "g_local.hpp"
#include "admin/session.hpp"

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
        ToInt( (*argv)[1] , position );

        if(position < 0 || position >= MAX_SAVED_POSITIONS) {
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
        pos = clients_[ClientNum(ent)].alliesSavedPositions + position;
    } else {
        pos = clients_[ClientNum(ent)].axisSavedPositions + position;
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
        ToInt((*argv)[1], position);

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
        pos = clients_[ClientNum(ent)].alliesSavedPositions + position;
    } else {
        pos = clients_[ClientNum(ent)].axisSavedPositions + position;
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
            &clients_[ClientNum(ent)].alliesSavedPositions[0]);
		VectorCopy( location->s.origin, 
            clients_[ClientNum(ent)].alliesSavedPositions[0].origin);
		VectorCopy( location->s.angles, 
            clients_[ClientNum(ent)].alliesSavedPositions[0].vangles);
        clients_[ClientNum(ent)].alliesSavedPositions[0].isValid = true;
	} 
	else if ( ent->client->sess.sessionTeam == TEAM_AXIS )
	{
        SaveBackupPosition(ent, &clients_[ClientNum(ent)].axisSavedPositions[0]);
		VectorCopy( location->s.origin, 
            clients_[ClientNum(ent)].axisSavedPositions[0].origin);
		VectorCopy( location->s.angles, 
            clients_[ClientNum(ent)].axisSavedPositions[0].vangles);
        clients_[ClientNum(ent)].axisSavedPositions[0].isValid = true;
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
        ToInt(argv->at(1), position);

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
        pos = &clients_[ClientNum(ent)].alliesBackupPositions[position];
    } else {
        pos = &clients_[ClientNum(ent)].axisBackupPositions[position];
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
        // TODO: reset saved positions here
        ResetSavedPositions(g_entities + clientNum);
    }

    savedPositions.clear();
}

// Used to reset positions on map change/restart
void SaveSystem::ResetSavedPositions(gentity_t *ent) {
    for(unsigned saveIndex = 0; saveIndex < MAX_SAVED_POSITIONS; saveIndex++) {
        clients_[ClientNum(ent)].alliesSavedPositions[saveIndex].isValid = false;
        clients_[ClientNum(ent)].axisSavedPositions[saveIndex].isValid = false;
    }

    for(unsigned backupIndex = 0; backupIndex < MAX_BACKUP_POSITIONS; backupIndex++) {
        clients_[ClientNum(ent)].alliesBackupPositions[backupIndex].isValid = false;
        clients_[ClientNum(ent)].axisBackupPositions[backupIndex].isValid = false;
    }
}

// Called on client disconnect. Saves saves for future sessions
void SaveSystem::SavePositionsToDatabase(gentity_t *ent) {
	
    if(!ent->client) {
        return;
    }

    string guid = session_->Guid(ent);

    DisconnectedClient client;

    for(unsigned i = 0; i < MAX_SAVED_POSITIONS; i++) {
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

    client.progression = ent->client->sess.clientMapProgression;
    ent->client->sess.loadPreviousSavedPositions = qfalse;

    std::map<string, DisconnectedClient>::iterator it = savedPositions.find(guid);

    if(it != savedPositions.end()) {
        it->second = client;
    } else {
        savedPositions.insert(std::make_pair(guid, client));
    }
    
}

// Called on client connect. Loads saves from previous session
void SaveSystem::LoadPositionsFromDatabase(gentity_t *ent) {
	
    if(!ent->client) {
        return;
    }

    if(!ent->client->sess.loadPreviousSavedPositions)
    {
        return;
    }

    string guid = session_->Guid(ent);

    std::map<string, DisconnectedClient>::iterator it = savedPositions.find(guid);

    if(it != savedPositions.end()) {

        for(unsigned i = 0; i < MAX_SAVED_POSITIONS; i++) {
            // Allied
            VectorCopy(it->second.alliesSavedPositions[i].origin,
                clients_[ClientNum(ent)].alliesSavedPositions[i].origin);
            VectorCopy(it->second.alliesSavedPositions[i].vangles,
                clients_[ClientNum(ent)].alliesSavedPositions[i].vangles);
            clients_[ClientNum(ent)].alliesSavedPositions[i].isValid = 
                it->second.alliesSavedPositions[i].isValid;
            // Axis
            VectorCopy(it->second.axisSavedPositions[i].origin,
                clients_[ClientNum(ent)].axisSavedPositions[i].origin);
            VectorCopy(it->second.axisSavedPositions[i].vangles,
                clients_[ClientNum(ent)].axisSavedPositions[i].vangles);
            clients_[ClientNum(ent)].axisSavedPositions[i].isValid = 
                it->second.axisSavedPositions[i].isValid;
        }

        ent->client->sess.loadPreviousSavedPositions = qfalse;
        ent->client->sess.clientMapProgression = it->second.progression;
        ChatPrintTo(ent, "^<ETJump: ^7loaded saved positions from previous session.");
    }
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
        clients_[ClientNum(ent)].alliesBackupPositions.push_front(backup);
    } else {
        clients_[ClientNum(ent)].axisBackupPositions.push_front(backup);
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
                ToString(it->second.alliesSavedPositions[i].isValid) +
                ToString(it->second.axisSavedPositions[i].isValid) +
                "B: " + ToString(b0, b1, b2) + 
                "R: " + ToString(r0, r1, r2) + NEWLINE;
        }
        it++;
    }
    ConsolePrintTo(NULL, toPrint);
}

SaveSystem::SaveSystem(const Session *session):
session_(session)
    //:guidInterface_(guidInterface)
{

}

SaveSystem::~SaveSystem()
{

}

void ForceSave(gentity_t *location, gentity_t *ent)
{
    game.saves->ForceSave(location, ent);
}

void ResetSavedPositions(gentity_t *ent)
{
    game.saves->ResetSavedPositions(ent);
}

void InitSaveSystem()
{
    game.saves->Reset();
}

void SavePositionsToDatabase(gentity_t *ent) {
    game.saves->SavePositionsToDatabase(ent);
}

void LoadPositionsFromDatabase(gentity_t *ent) {
    game.saves->LoadPositionsFromDatabase(ent);
}