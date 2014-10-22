#include "races.hpp"
#include "g_local.hpp"
#include "g_utilities.hpp"
#include "../json/json.h"
#include "asyncoperations.hh"
#include "g_utilities.hpp"
#include "operationqueue.hpp"
#include <sqlite3.h>

Races::Races(OperationQueue* operationQueue) : operationQueue_(operationQueue)
{

}

void Races::Init()
{
    sqlite3 *db = NULL;
    int rc = sqlite3_open(GetPath(g_raceDatabase.string).c_str(), &db);
    if (rc != SQLITE_OK)
    {
        G_LogPrintf("ERROR: Failed to initialize race database. %s\n", sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS races (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, creator TEXT, map TEXT, date INTEGER, route TEXT, UNIQUE(name, map));", NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        G_LogPrintf("ERROR: Failed to initialize race database. %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_close(db);
    db = NULL;
}

void Races::Shutdown()
{
}

Races::Races() : start_(NULL), end_(NULL), numCheckpoints_(0), designMode_(false)
{
    for (unsigned i = 0; i < MAX_CHECKPOINTS; i++)
    {
        checkpoints_[i] = NULL;
    }
}

Races::~Races()
{
}

std::string Races::GetMessage() const
{
    return message_;
}

bool Races::CreateStart(vec3_t origin, vec3_t angles, bool onlyReplaceStart)
{
    if (start_ != NULL)
    {
        G_FreeEntity(start_);
        start_ = NULL;
    }

    if (!onlyReplaceStart)
    {
        ClearRoute();
    }

    start_ = G_Spawn();
    start_->classname = "race_start";

    G_SetOrigin(start_, origin);
    G_SetAngle(start_, angles);

    trap_LinkEntity(start_);

    return true;
}

bool Races::CreateEnd(vec3_t origin, vec3_t angles, vec3_t dimensions)
{
    if (end_ != NULL)
    {
        G_FreeEntity(end_);
        end_ = NULL;
    }

    end_ = G_Spawn();
    end_->classname = "race_end";
    end_->think = EndThink;
    end_->nextthink = level.time + FRAMETIME;

    G_SetOrigin(end_, origin);
    G_SetAngle(end_, angles);
    VectorCopy(dimensions, end_->dimensions);

    trap_LinkEntity(end_);

    return true;
}

bool Races::CreateCheckpoint(vec3_t origin, vec3_t angles, vec3_t dimensions)
{
    if (numCheckpoints_ == MAX_CHECKPOINTS)
    {
        message_ = "Maximum number of checkpoints (20) created.";
        return false;
    }

    gentity_t *checkpoint = G_Spawn();
    checkpoint->classname = "race_checkpoint";
    checkpoint->think = CheckpointThink;
    checkpoint->nextthink = level.time + FRAMETIME;
    
    G_SetOrigin(checkpoint, origin);
    G_SetAngle(checkpoint, angles);
    VectorCopy(dimensions, checkpoint->dimensions);

    trap_LinkEntity(checkpoint);
    checkpoints_[numCheckpoints_++] = checkpoint;
    checkpoint->seqNum = numCheckpoints_;

    return true;
}

void Races::ClearRoute()
{
    for (unsigned i = 0; i < MAX_CHECKPOINTS; i++)
    {
        if (checkpoints_[i] != NULL)
        {
            G_FreeEntity(checkpoints_[i]);
            checkpoints_[i] = NULL;
        }
    }

    numCheckpoints_ = 0;

    if (end_ != NULL)
    {
        G_FreeEntity(end_);
        end_ = NULL;
    }
}

bool Races::StartRace(gentity_t *caller)
{
    fireteamData_t *entFt = NULL;
    G_IsOnFireteam(caller - g_entities, &entFt);

    if (designMode_)
    {
        message_ = "you cannot race while design mode is active.";
        return false;
    }

    if (!start_)
    {
        message_ = "no start point defined.";
        return false;
    }

    if (!end_)
    {
        message_ = "no end point defined";
        return false;
    }

    for (int i = 0; i < level.numConnectedClients; i++)
    {
        int num = level.sortedClients[i];
        fireteamData_t *otherFt = NULL;
        gentity_t *ent = g_entities + num;
        
        if (!G_IsOnFireteam(num, &otherFt))
        {
            continue;
        }

        if (entFt != otherFt)
        {
            continue;
        }

        for (int j = 0; j < MAX_CHECKPOINTS; j++)
        {
            ent->client->pers.race.visitedCheckpoints[j] = qfalse;
        }

        ent->client->pers.race.isRacing = qtrue;
        ent->client->pers.race.startTime = level.time;
        TeleportPlayerToStart(ent);
        ent->client->pers.race.saveLimit = raceSettings_.saveLimit;
        CPTo(ent, "^2Race started!");
    }

    return true;
}

void Races::StopRace()
{
    for (int i = 0; i < level.numConnectedClients; i++)
    {
        int cnum = level.sortedClients[i];
        gentity_t *ent = g_entities + cnum;
        
        ent->client->pers.race.isRacing = qfalse;
    }
    CPMAll("^<ETJump: ^7Route design mode activated. Racing stopped");
}

std::string GetEntityOriginAndAngles(gentity_t *ent)
{
    return std::string(va("%f %f %f %f %f %f",
        ent->r.currentOrigin[0],
        ent->r.currentOrigin[1],
        ent->r.currentOrigin[2],
        ent->r.currentAngles[0],
        ent->r.currentAngles[1],
        ent->r.currentAngles[2]));
}



bool Races::Save(std::string const& routeName, gentity_t *ent)
{
    if (!start_ || !end_)
    {
        message_ = "No route start or end defined.";
        return false;
    }

    Race race;
    race.name = routeName;
    race.map = level.rawmapname;
    race.creator = ent->client->pers.netname;
    time_t t;
    time(&t);
    race.date = static_cast<unsigned long>(t);
    race.start = GetEntityOriginAndAngles(start_);
    race.end = GetEntityOriginAndAngles(end_);
    for (unsigned i = 0; i < numCheckpoints_; i++)
    {
        race.checkpoints.push_back(GetEntityOriginAndAngles(checkpoints_[i]));
    }
    
    AsyncSaveRace *save = new AsyncSaveRace(race, ent);
    // After thread finishes execution, the object is deleted
    save->RunAndDeleteObject();

    return true;
}

bool Races::CreateLoadedRaceEntities(Race race)
{
    G_LogPrintf("Creating race entities\n");
    return true;

}

bool Races::Load(std::string const& name, gentity_t *ent)
{
    AsyncLoadRace *load = new AsyncLoadRace(this, name, ent, operationQueue_);
    load->RunAndDeleteObject();
    return true;
}



bool Races::SetSettings(std::string const& name, std::string const& map, std::string const& creator, int date, int saveLimit)
{
    if (name.length() > 0 && map.length() > 0 && creator.length() > 0)
    {
        raceSettings_.name = name;
        raceSettings_.map = map;
        raceSettings_.creator = creator;
        raceSettings_.date = date;
        raceSettings_.saveLimit = saveLimit;
    }

    return false;
}

void Races::DesignMode(bool state)
{
    designMode_ = state;
}

bool Races::UndoLastCheckpoint()
{
    if (numCheckpoints_ == 0)
    {
        message_ = "no checkpoints defined.";
        return false;
    }

    numCheckpoints_--;
    G_FreeEntity(checkpoints_[numCheckpoints_]);
    checkpoints_[numCheckpoints_] = NULL;

    return true;
}

bool Races::TeleportPlayerToStart(gentity_t* player)
{
    if (!start_)
    {
        return false;
    }

    player->client->ps.eFlags ^= EF_TELEPORT_BIT;
    VectorCopy(start_->r.currentOrigin, player->client->ps.origin);
    VectorClear(player->client->ps.velocity);
    SetClientViewAngle(player, start_->r.currentAngles);

    return true;
}

void Races::EndThink(gentity_t* self)
{
    vec3_t mins = { 0, 0, 0 };
    vec3_t maxs = { 0, 0, 0 };

    VectorSubtract(self->r.currentOrigin, self->dimensions, mins);
    VectorAdd(self->r.currentOrigin, self->dimensions, maxs);

    int entityList[MAX_GENTITIES];
    int entitiesInBox = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

    for (int i = 0; i < entitiesInBox; i++)
    {
        gentity_t *ent = NULL;
        ent = &g_entities[entityList[i]];

        if (!ent->client)
        {
            continue;
        }

        if (ClientIsRacing(ent))
        {
            fireteamData_t *entFt = NULL;
            int msec = 0;
            int sec = 0;
            int min = 0;
            
            if (!G_IsOnFireteam(ent - g_entities, &entFt))
            {
                continue;
            }

            msec = level.time - ent->client->pers.race.startTime;
            min = msec / 60000;
            msec = msec - min * 60000;
            sec = msec / 1000;
            msec = msec - sec * 1000;
            ent->client->pers.race.isRacing = qfalse;
            
            const char *msg = va("%s ^7reached the ^1end^7 in %02d:%02d:%03d.",
                ent->client->pers.netname, min, sec, msec);

            for (int j = 0; j < level.numConnectedClients; j++)
            {
                int cnum = level.sortedClients[j];
                gentity_t *ftMember = g_entities + cnum;
                fireteamData_t *memberFt = NULL;
                if (!G_IsOnFireteam(cnum, &memberFt))
                {
                    continue;
                }

                if (memberFt != entFt)
                {
                    continue;
                }

                CPMTo(ftMember, msg);
            }
        }
    }

    self->nextthink = FRAMETIME;
}

void Races::CheckpointThink(gentity_t* self)
{
    vec3_t mins = { 0, 0, 0 };
    vec3_t maxs = { 0, 0, 0 };

    VectorSubtract(self->r.currentOrigin, self->dimensions, mins);
    VectorAdd(self->r.currentOrigin, self->dimensions, maxs);

    int entityList[MAX_GENTITIES];
    int entitiesInBox = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

    for (int i = 0; i < entitiesInBox; i++)
    {
        gentity_t *ent = NULL;
        ent = &g_entities[entityList[i]];

        if (!ent->client)
        {
            continue;
        }

        if (ClientIsRacing(ent) && !CheckpointVisited(ent, self->seqNum))
        {
            int msec, sec, min;
            fireteamData_t *entFt = NULL;
            if (!G_IsOnFireteam(ent - g_entities, &entFt))
            {
                continue;
            }

            msec = level.time - ent->client->pers.race.startTime;
            min = msec / 60000;
            msec = msec - min * 60000;
            sec = msec / 1000;
            msec = msec - sec * 1000;

            const char *msg = va("^7%s ^7reached ^3checkpoint ^7%d in %02d:%02d:%03d.",
                ent->client->pers.netname, self->seqNum, min, sec, msec);

            for (int j = 0; j < level.numConnectedClients; j++)
            {
                int cnum = level.sortedClients[j];
                gentity_t *ftMember = g_entities + cnum;
                fireteamData_t *memberFt = NULL;
                if (!G_IsOnFireteam(cnum, &memberFt))
                {
                    continue;
                }

                if (memberFt != entFt)
                {
                    continue;
                }

                CPMTo(ftMember, msg);
            }

            ent->client->pers.race.visitedCheckpoints[self->seqNum] = qtrue;
        }
    }

    self->nextthink = FRAMETIME;
}

bool Races::ClientIsRacing(gentity_t* player)
{
    if (player->client->pers.race.isRacing)
    {
        return true;
    }
    return false;
}

bool Races::CheckpointVisited(gentity_t* player, unsigned num)
{
    return player->client->pers.race.visitedCheckpoints[num];
}
