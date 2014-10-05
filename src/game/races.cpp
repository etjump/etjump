#include "races.hpp"
#include "g_local.hpp"
#include "g_utilities.hpp"

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
