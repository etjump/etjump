

// cg_predict.c -- this file generates cg.predictedPlayerState by either
// interpolating between snapshots from the server or locally predicting
// ahead the client's movement.
// It also handles local physics interaction, like fragments bouncing off walls

#include <array>
#include "cg_local.h"
#include "etj_utilities.h"
#include "../game/etj_entity_utilities_shared.h"

/*static*/ pmove_t cg_pmove;

static int cg_numSolidEntities;
static centity_t *cg_solidEntities[MAX_ENTITIES_IN_SNAPSHOT];
static int cg_numSolidFTEntities;
static centity_t *cg_solidFTEntities[MAX_ENTITIES_IN_SNAPSHOT];
static int cg_numTriggerEntities;
static centity_t *cg_triggerEntities[MAX_ENTITIES_IN_SNAPSHOT];

/*
====================
CG_BuildSolidList

When a new cg.snap has been set, this function builds a sublist
of the entities that are actually solid, to make for more
efficient collision detection
====================
*/
void CG_BuildSolidList(void) {
  int i;
  centity_t *cent;
  snapshot_t *snap;
  entityState_t *ent;

  cg_numSolidEntities = 0;
  cg_numSolidFTEntities = 0;
  cg_numTriggerEntities = 0;

  if (cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport) {
    snap = cg.nextSnap;
  } else {
    snap = cg.snap;
  }

  for (i = 0; i < snap->numEntities; i++) {
    cent = &cg_entities[snap->entities[i].number];
    ent = &cent->currentState;

    // rain - don't clip against temporarily non-solid
    // SOLID_BMODELS (e.g. constructibles); use current state so
    // prediction isn't fubar
    if (cent->currentState.solid == SOLID_BMODEL &&
        cent->currentState.eType !=
            ET_PLAYER && // etjump: flag repurposed for players
        (cent->currentState.eFlags & EF_NONSOLID_BMODEL)) {
      continue;
    }

    if (ent->eType == ET_ITEM || ent->eType == ET_PUSH_TRIGGER ||
        ent->eType == ET_VELOCITY_PUSH_TRIGGER ||
        ent->eType == ET_TELEPORT_TRIGGER ||
        ent->eType == ET_TELEPORT_TRIGGER_CLIENT ||
        ent->eType == ET_CONCUSSIVE_TRIGGER || ent->eType == ET_OID_TRIGGER
#ifdef VISIBLE_TRIGGERS
        || ent->eType == ET_TRIGGER_MULTIPLE ||
        ent->eType == ET_TRIGGER_FLAGONLY ||
        ent->eType == ET_TRIGGER_FLAGONLY_MULTIPLE
#endif // VISIBLE_TRIGGERS
    ) {

      cg_triggerEntities[cg_numTriggerEntities] = cent;
      cg_numTriggerEntities++;
      continue;
    }

    if (ent->eType == ET_CONSTRUCTIBLE) {
      cg_triggerEntities[cg_numTriggerEntities] = cent;
      cg_numTriggerEntities++;
    }

    if (cent->nextState.solid) {
      /*			if(cg_fastSolids.integer)
         { // Gordon: "optimization" (disabling until i
         fix it) vec3_t vec; float len;

                      cg_solidFTEntities[cg_numSolidFTEntities]
         = cent; cg_numSolidFTEntities++;

                      // FIXME: use range to bbox, not
         to origin if ( cent->nextState.solid ==
         SOLID_BMODEL ) { VectorAdd(
         cgs.inlineModelMidpoints[
         cent->currentState.modelindex ],
         cent->lerpOrigin, vec ); VectorSubtract( vec,
         cg.predictedPlayerEntity.lerpOrigin, vec ); }
         else { VectorSubtract( cent->lerpOrigin,
         cg.predictedPlayerEntity.lerpOrigin, vec );
                      }
                      if((len = DotProduct( vec, vec ))
         < (512 * 512)) {
                          cg_solidEntities[cg_numSolidEntities]
         = cent; cg_numSolidEntities++; continue;
                      }
                  } else*/
      {
        cg_solidEntities[cg_numSolidEntities] = cent;
        cg_numSolidEntities++;

        cg_solidFTEntities[cg_numSolidFTEntities] = cent;
        cg_numSolidFTEntities++;
        continue;
      }
    }
  }
}

/*
====================
CG_ClipMoveToEntities

====================
*/
static void CG_ClipMoveToEntities(const vec3_t start, const vec3_t mins,
                                  const vec3_t maxs, const vec3_t end,
                                  int skipNumber, int mask, int capsule,
                                  qboolean tracePlayers, trace_t *tr) {
  int i, x, zd, zu;
  trace_t trace;
  entityState_t *ent;
  clipHandle_t cmodel;
  vec3_t bmins, bmaxs;
  vec3_t origin, angles;
  centity_t *cent;

  for (i = 0; i < cg_numSolidEntities; i++) {
    cent = cg_solidEntities[i];
    ent = &cent->currentState;

    if (ent->number == skipNumber ||
        (!tracePlayers && ent->eType == ET_PLAYER)) {
      continue;
    }

    if (ent->number < MAX_CLIENTS &&
        ETJump::tempTraceIgnoredClients[ent->number]) {
      continue;
    }

    if (ent->solid == SOLID_BMODEL) {
      // special value for bmodel
      cmodel = trap_CM_InlineModel(ent->modelindex);
      //			VectorCopy(
      // cent->lerpAngles, angles ); 			VectorCopy(
      // cent->lerpOrigin, origin );
      BG_EvaluateTrajectory(&cent->currentState.apos, cg.physicsTime, angles,
                            qtrue, cent->currentState.effect2Time);
      BG_EvaluateTrajectory(&cent->currentState.pos, cg.physicsTime, origin,
                            qfalse, cent->currentState.effect2Time);
    } else {
      // see g_misc.c SP_func_fakebrush...
      if (ent->eFlags & EF_FAKEBMODEL && ent->eType != ET_PLAYER) {
        VectorCopy(ent->origin2, bmins);
        VectorCopy(ent->angles2, bmaxs);
      } else {
        // encoded bbox
        x = (ent->solid & 255);
        zd = ((ent->solid >> 8) & 255);
        zu = ((ent->solid >> 16) & 255) - 32;

        bmins[0] = bmins[1] = -x;
        bmaxs[0] = bmaxs[1] = x;
        bmins[2] = -zd;
        bmaxs[2] = zu;
      }

      // cmodel = trap_CM_TempCapsuleModel( bmins, bmaxs
      // );
      cmodel = trap_CM_TempBoxModel(bmins, bmaxs);

      VectorCopy(vec3_origin, angles);
      VectorCopy(cent->lerpOrigin, origin);
    }
    // MrE: use bbox of capsule
    if (capsule) {
      trap_CM_TransformedCapsuleTrace(&trace, start, end, mins, maxs, cmodel,
                                      mask, origin, angles);
    } else {
      trap_CM_TransformedBoxTrace(&trace, start, end, mins, maxs, cmodel, mask,
                                  origin, angles);
    }

    if (trace.allsolid || trace.fraction < tr->fraction) {
      trace.entityNum = ent->number;
      *tr = trace;
    } else if (trace.startsolid) {
      tr->startsolid = qtrue;
    }
    if (tr->allsolid) {
      return;
    }
  }
}

static void CG_ClipMoveToEntities_FT(const vec3_t start, const vec3_t mins,
                                     const vec3_t maxs, const vec3_t end,
                                     int skipNumber, int mask, int capsule,
                                     trace_t *tr) {
  int i, x, zd, zu;
  trace_t trace;
  entityState_t *ent;
  clipHandle_t cmodel;
  vec3_t bmins, bmaxs;
  vec3_t origin, angles;
  centity_t *cent;

  for (i = 0; i < cg_numSolidFTEntities; i++) {
    cent = cg_solidFTEntities[i];
    ent = &cent->currentState;

    if (ent->number == skipNumber) {
      continue;
    }

    if (ent->solid == SOLID_BMODEL) {
      // special value for bmodel
      cmodel = trap_CM_InlineModel(ent->modelindex);
      //			VectorCopy(
      // cent->lerpAngles, angles ); 			VectorCopy(
      // cent->lerpOrigin, origin );
      BG_EvaluateTrajectory(&cent->currentState.apos, cg.physicsTime, angles,
                            qtrue, cent->currentState.effect2Time);
      BG_EvaluateTrajectory(&cent->currentState.pos, cg.physicsTime, origin,
                            qfalse, cent->currentState.effect2Time);
    } else {
      // encoded bbox
      x = (ent->solid & 255);
      zd = ((ent->solid >> 8) & 255);
      zu = ((ent->solid >> 16) & 255) - 32;

      bmins[0] = bmins[1] = -x;
      bmaxs[0] = bmaxs[1] = x;
      bmins[2] = -zd;
      bmaxs[2] = zu;

      cmodel = trap_CM_TempCapsuleModel(bmins, bmaxs);

      VectorCopy(vec3_origin, angles);
      VectorCopy(cent->lerpOrigin, origin);
    }
    // MrE: use bbox of capsule
    if (capsule) {
      trap_CM_TransformedCapsuleTrace(&trace, start, end, mins, maxs, cmodel,
                                      mask, origin, angles);
    } else {
      trap_CM_TransformedBoxTrace(&trace, start, end, mins, maxs, cmodel, mask,
                                  origin, angles);
    }

    if (trace.allsolid || trace.fraction < tr->fraction) {
      trace.entityNum = ent->number;
      *tr = trace;
    } else if (trace.startsolid) {
      tr->startsolid = qtrue;
    }
    if (tr->allsolid) {
      return;
    }
  }
}

/*
================
CG_Trace
================
*/
void CG_Trace(trace_t *result, const vec3_t start, const vec3_t mins,
              const vec3_t maxs, const vec3_t end, int skipNumber, int mask) {
  trace_t t;

  trap_CM_BoxTrace(&t, start, end, mins, maxs, 0, mask);
  t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
  // check all other solid models
  CG_ClipMoveToEntities(start, mins, maxs, end, skipNumber, mask, qfalse, qtrue,
                        &t);

  *result = t;
}

void CG_Trace_World(trace_t *result, const vec3_t start, const vec3_t mins,
                    const vec3_t maxs, const vec3_t end, int skipNumber,
                    int mask) {
  trace_t t;

  trap_CM_BoxTrace(&t, start, end, mins, maxs, 0, mask);
  t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;

  *result = t;
}

void CG_FTTrace(trace_t *result, const vec3_t start, const vec3_t mins,
                const vec3_t maxs, const vec3_t end, int skipNumber, int mask) {
  trace_t t;

  trap_CM_BoxTrace(&t, start, end, mins, maxs, 0, mask);
  t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
  // check all other solid models

  CG_ClipMoveToEntities_FT(start, mins, maxs, end, skipNumber, mask, qfalse,
                           &t);

  *result = t;
}

/*
================
CG_TraceCapsule
================
*/
void CG_TraceCapsule(trace_t *result, const vec3_t start, const vec3_t mins,
                     const vec3_t maxs, const vec3_t end, int skipNumber,
                     int mask) {
  trace_t t;

  trap_CM_CapsuleTrace(&t, start, end, mins, maxs, 0, mask);
  t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
  // check all other solid models
  CG_ClipMoveToEntities(start, mins, maxs, end, skipNumber, mask, qtrue, qtrue,
                        &t);

  *result = t;
}

void CG_TraceCapsule_NoPlayers(trace_t *result, const vec3_t start,
                               const vec3_t mins, const vec3_t maxs,
                               const vec3_t end, int skipNumber, int mask) {
  trace_t t;

  trap_CM_CapsuleTrace(&t, start, end, mins, maxs, 0, mask);
  t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
  // check all other solid models
  CG_ClipMoveToEntities(start, mins, maxs, end, skipNumber, mask, qtrue, qfalse,
                        &t);

  *result = t;
}

void CG_TraceCapsule_World(trace_t *result, const vec3_t start,
                           const vec3_t mins, const vec3_t maxs,
                           const vec3_t end, int skipNumber, int mask) {
  trace_t t;
  trap_CM_CapsuleTrace(&t, start, end, mins, maxs, 0, mask);

  t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
  *result = t;
}
/*
================
CG_PointContents
================
*/
int CG_PointContents(const vec3_t point, int passEntityNum) {
  int i;
  entityState_t *ent;
  centity_t *cent;
  clipHandle_t cmodel;
  int contents;

  contents = trap_CM_PointContents(point, 0);

  for (i = 0; i < cg_numSolidEntities; i++) {
    cent = cg_solidEntities[i];

    ent = &cent->currentState;

    if (ent->number == passEntityNum) {
      continue;
    }

    if (ent->solid != SOLID_BMODEL) // special value for bmodel
    {
      continue;
    }

    cmodel = trap_CM_InlineModel(ent->modelindex);
    if (!cmodel) {
      continue;
    }

    contents |= trap_CM_TransformedPointContents(
        point, cmodel, cent->lerpOrigin, cent->lerpAngles);
    // Gordon: again, need to use the projected water position
    // to allow for moving entity based water.
    //		contents |= trap_CM_TransformedPointContents( point,
    // cmodel,
    // ent->origin, ent->angles );
  }

  return contents;
}

/*
========================
CG_InterpolatePlayerState

Generates cg.predictedPlayerState by interpolating between
cg.snap->player_state and cg.nextFrame->player_state
========================
*/
static void CG_InterpolatePlayerState(qboolean grabAngles) {
  float f;
  int i;
  playerState_t *out;
  snapshot_t *prev, *next;

  out = &cg.predictedPlayerState;
  prev = cg.snap;
  next = cg.nextSnap;

  *out = cg.snap->ps;

  if (cg.showGameView) {
    return;
  }

  // if we are still allowing local input, short circuit the view angles
  if (grabAngles) {
    usercmd_t cmd;
    int cmdNum;

    cmdNum = trap_GetCurrentCmdNumber();
    trap_GetUserCmd(cmdNum, &cmd);

    // rain - added tracemask
    PM_UpdateViewAngles(out, &cg.pmext, &cmd, CG_Trace, MASK_PLAYERSOLID);
  }

  // if the next frame is a teleport, we can't lerp to it
  if (cg.nextFrameTeleport) {
    return;
  }

  if (!next || next->serverTime <= prev->serverTime) {
    return;
  }

  f = (float)(cg.time - prev->serverTime) /
      (next->serverTime - prev->serverTime);

  i = next->ps.bobCycle;
  if (i < prev->ps.bobCycle) {
    i += 256; // handle wraparound
  }
  out->bobCycle = prev->ps.bobCycle + f * (i - prev->ps.bobCycle);

  for (i = 0; i < 3; i++) {
    out->origin[i] =
        prev->ps.origin[i] + f * (next->ps.origin[i] - prev->ps.origin[i]);
    if (!grabAngles) {
      out->viewangles[i] =
          LerpAngle(prev->ps.viewangles[i], next->ps.viewangles[i], f);
    }
    out->velocity[i] = prev->ps.velocity[i] +
                       f * (next->ps.velocity[i] - prev->ps.velocity[i]);
  }

  out->leanf = prev->ps.leanf + f * (next->ps.leanf - prev->ps.leanf);
}

void CG_AddDirtBulletParticles(vec3_t origin, vec3_t dir, int speed,
                               int duration, int count, float randScale,
                               float width, float height, float alpha,
                               qhandle_t shader);

/*
=========================
CG_TouchTriggerPrediction

Predict push triggers and items
=========================
*/
static void CG_TouchTriggerPrediction() {
  int i;
  entityState_t *ent;
  clipHandle_t cmodel;
  centity_t *cent;
  qboolean spectator;
  const char *cs;

  // dead clients don't activate triggers
  if (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0) {
    return;
  }

  spectator = ((cg.predictedPlayerState.pm_type == PM_SPECTATOR) ||
               (cg.predictedPlayerState.pm_flags & PMF_LIMBO))
                  ? qtrue
                  : qfalse; // JPW NERVE

  if (cg.predictedPlayerState.pm_type != PM_NORMAL && !spectator) {
    return;
  }

  for (i = 0; i < cg_numTriggerEntities; i++) {
    cent = cg_triggerEntities[i];
    ent = &cent->currentState;

    if (ent->eType == ET_ITEM && !spectator &&
        (cg.predictedPlayerState.groundEntityNum == ENTITYNUM_WORLD)) {
      continue;
    }

    if (ent->solid != SOLID_BMODEL) {
      continue;
    }

    // Gordon: er, this lookup was wrong...
    cmodel = cgs.inlineDrawModel[ent->modelindex];
    if (!cmodel) {
      continue;
    }

    if (ent->eType == ET_CONSTRUCTIBLE || ent->eType == ET_OID_TRIGGER ||
        ent->eType == ET_PUSH_TRIGGER ||
        ent->eType == ET_VELOCITY_PUSH_TRIGGER ||
        ent->eType == ET_TELEPORT_TRIGGER_CLIENT
#ifdef VISIBLE_TRIGGERS
        || ent->eType == ET_TRIGGER_MULTIPLE ||
        ent->eType == ET_TRIGGER_FLAGONLY ||
        ent->eType == ET_TRIGGER_FLAGONLY_MULTIPLE
#endif // VISIBLE_TRIGGERS
    ) {
      vec3_t mins, maxs, pmins, pmaxs;

      if (ent->eType == ET_CONSTRUCTIBLE && ent->aiState) {
        continue;
      }

      trap_R_ModelBounds(cmodel, mins, maxs);

      VectorAdd(cent->lerpOrigin, mins, mins);
      VectorAdd(cent->lerpOrigin, maxs, maxs);

#ifdef VISIBLE_TRIGGERS
      if (ent->eType == ET_TRIGGER_MULTIPLE ||
          ent->eType == ET_TRIGGER_FLAGONLY ||
          ent->eType == ET_TRIGGER_FLAGONLY_MULTIPLE) {
      } else
#endif // VISIBLE_TRIGGERS
      {
        if (ent->eType != ET_PUSH_TRIGGER &&
            ent->eType != ET_VELOCITY_PUSH_TRIGGER &&
            ent->eType != ET_TELEPORT_TRIGGER_CLIENT) {
          // expand the bbox a bit
          VectorSet(mins, mins[0] - 48, mins[1] - 48, mins[2] - 48);
          VectorSet(maxs, maxs[0] + 48, maxs[1] + 48, maxs[2] + 48);
        }
      }

      VectorAdd(cg.predictedPlayerState.origin, cg_pmove.mins, pmins);
      VectorAdd(cg.predictedPlayerState.origin, cg_pmove.maxs, pmaxs);

#ifdef VISIBLE_TRIGGERS
      CG_RailTrail(NULL, mins, maxs, 1);
#endif // VISIBLE_TRIGGERS

      if (!BG_BBoxCollision(pmins, pmaxs, mins, maxs)) {
        continue;
      }

      cs = nullptr;
      if (ent->eType == ET_OID_TRIGGER) {
        cs = CG_ConfigString(CS_OID_TRIGGERS + ent->teamNum);
      } else if (ent->eType == ET_CONSTRUCTIBLE) {
        cs = CG_ConfigString(CS_OID_TRIGGERS + ent->otherEntityNum2);
      }

      if (cs) {
        CG_ObjectivePrint(va("You are near %s\n", cs), SMALLCHAR_WIDTH);
      }

      // trace for non-axial triggers for teleporters/pushers
      // for more accurate collision
      if (ent->eType == ET_TELEPORT_TRIGGER_CLIENT ||
          ent->eType == ET_PUSH_TRIGGER ||
          ent->eType == ET_VELOCITY_PUSH_TRIGGER) {
        cmodel = trap_CM_InlineModel(ent->modelindex);

        if (!cmodel) {
          continue;
        }

        trace_t trace;
        trap_CM_CapsuleTrace(&trace, cg.predictedPlayerState.origin,
                             cg.predictedPlayerState.origin, cg_pmove.mins,
                             cg_pmove.maxs, cmodel, -1);

        if (trace.fraction == 1.0f) {
          continue;
        }

        if (ent->eType == ET_TELEPORT_TRIGGER_CLIENT) {
          entityState_t *playerEs =
              &cg_entities[cg.snap->ps.clientNum].currentState;
          ETJump::EntityUtilsShared::teleportPlayer(
              &cg.predictedPlayerState, playerEs, ent, &cg_pmove.cmd,
              ent->origin2, ent->angles2);
        } else {
          ETJump::EntityUtilsShared::touchPusher(&cg.predictedPlayerState,
                                                 cg.physicsTime, ent);
        }
      }

      continue;
    }
  }
}

const char *predictionStrings[] = {
    "OK",
    "PM TYPE FLAGS TIME",
    "origin",
    "velocity",
    "eFlags",
    "weapon time",
    "groundEntityNum",
    "--", // 7
    "speed || delta_angles",
    "anim || timer",
    "--", // 10
    "eventSequence",
    "events || eventParms",
    "externalEvent",
    "clientNum",
    "weapon || weaponState",
    "viewangles",
    "viewheight",
    "damage event ...",
    "stats",
    "persistant", // 20
    "powerups",
    "ammo",
    "viewlocked",
    "onFireStart",
    "--",
    "--",
    "grenadeTimeLeft", // 27
};

#define MAX_PREDICT_ORIGIN_DELTA 0.1f
#define MAX_PREDICT_VELOCITY_DELTA 0.1f
#define MAX_PREDICT_VIEWANGLES_DELTA 1.0f

// error codes returned by this function are not sequential
// this is just kept in sync with legacy to keep comparisons easier
int CG_PredictionOk(playerState_t *ps1, playerState_t *ps2) {
  vec3_t vec;
  int i;

  if (ps2->pm_type != ps1->pm_type || ps2->pm_flags != ps1->pm_flags ||
      ps2->pm_time != ps1->pm_time) {
    return 1;
  }

  VectorSubtract(ps2->origin, ps1->origin, vec);
  if (DotProduct(vec, vec) > Square(MAX_PREDICT_ORIGIN_DELTA)) {
    return 2;
  }

  VectorSubtract(ps2->velocity, ps1->velocity, vec);
  if (DotProduct(vec, vec) > Square(MAX_PREDICT_VELOCITY_DELTA)) {
    return 3;
  }

  if (ps2->eFlags != ps1->eFlags) {
    if (cg_showmiss.integer & 8) {
      CG_Printf("CG_PredictionOk info: return 4 - backup: '%x' server: '%x' "
                "Diff: '%x'\n",
                ps2->eFlags, ps1->eFlags, ps2->eFlags - ps1->eFlags);
    }
    return 4;
  }

  if (ps2->weaponTime != ps1->weaponTime) {
    if (cg_showmiss.integer & 8) {
      CG_Printf("CG_PredictionOk info: return 5 - backup time: '%d' server "
                "time: '%d'\n",
                ps2->weaponTime, ps1->weaponTime);
    }
    return 5;
  }

  if (ps2->groundEntityNum != ps1->groundEntityNum) {
    return 6;
  }

  if (ps2->speed != ps1->speed ||
      ps2->delta_angles[0] != ps1->delta_angles[0] ||
      ps2->delta_angles[1] != ps1->delta_angles[1] ||
      ps2->delta_angles[2] != ps1->delta_angles[2]) {
    return 8;
  }

  if (ps2->legsTimer != ps1->legsTimer || ps2->legsAnim != ps1->legsAnim ||
      ps2->torsoTimer != ps1->torsoTimer || ps2->torsoAnim != ps1->torsoAnim) {
    return 9;
  }

  // common with item pickups
  if (ps2->eventSequence != ps1->eventSequence) {
    return 11;
  }

  for (i = 0; i < MAX_EVENTS; i++) {
    if (ps2->events[i] != ps1->events[i] ||
        ps2->eventParms[i] != ps1->eventParms[i]) {
      return 12;
    }
  }

  if (ps2->externalEvent != ps1->externalEvent ||
      ps2->externalEventParm != ps1->externalEventParm ||
      ps2->externalEventTime != ps1->externalEventTime) {
    return 13;
  }

  if (ps2->clientNum != ps1->clientNum) {
    return 14;
  }

  if (ps2->weapon != ps1->weapon || ps2->weaponstate != ps1->weaponstate) {
    return 15;
  }

  for (i = 0; i < 3; i++) {
    if (abs(ps2->viewangles[i] - ps1->viewangles[i]) >
        MAX_PREDICT_VIEWANGLES_DELTA) {
      return 16;
    }
  }

  if (ps2->viewheight != ps1->viewheight) {
    return 17;
  }

  if (ps2->damageEvent != ps1->damageEvent ||
      ps2->damageYaw != ps1->damageYaw ||
      ps2->damagePitch != ps1->damagePitch ||
      ps2->damageCount != ps1->damageCount) {
    return 18;
  }

  for (i = 0; i < MAX_STATS; i++) {
    if (ps2->stats[i] != ps1->stats[i]) {
      if (cg_showmiss.integer & 8) {
        CG_Printf(
            "CG_PredictionOk info: return 19 - MAX_STATS[%i] ps1: %i ps2: %i\n",
            i, ps1->stats[i], ps2->stats[i]);
      }
      return 19;
    }
  }

  for (i = 0; i < MAX_PERSISTANT; i++) {
    if (ps2->persistant[i] != ps1->persistant[i]) {
      if (cg_showmiss.integer & 8) {
        CG_Printf("CG_PredictionOk info: return 20 - MAX_PERSISTANT[%i] ps1: "
                  "%i ps2: %i\n",
                  i, ps1->persistant[i], ps2->persistant[i]);
      }
      return 20;
    }
  }

  for (i = 0; i < MAX_POWERUPS; i++) {
    if (ps2->powerups[i] != ps1->powerups[i]) {
      if (cg_showmiss.integer & 8) {
        CG_Printf("CG_PredictionOk info: return 21 - MAX_POWERUPS[%i] ps1: %i "
                  "ps2: %i\n",
                  i, ps1->powerups[i], ps2->powerups[i]);
      }
      return 21;
    }
  }

  for (i = 0; i < MAX_WEAPONS; i++) {
    if (ps2->ammo[i] != ps1->ammo[i] || ps2->ammoclip[i] != ps1->ammoclip[i]) {
      return 22;
    }
  }

  if (ps1->viewlocked != ps2->viewlocked ||
      ps1->viewlocked_entNum != ps2->viewlocked_entNum) {
    return 23;
  }

  if (ps1->onFireStart != ps2->onFireStart) {
    return 24;
  }

  // grenadeTimeLeft was not fully predicted
  if (ps1->grenadeTimeLeft != ps2->grenadeTimeLeft) {
    if (cg_showmiss.integer & 8) {
      CG_Printf("CG_PredictionOk info: return 27 - backup time '%d' - server "
                "time: '%d'\n",
                ps2->grenadeTimeLeft, ps1->grenadeTimeLeft);
    }
    return 27;
  }

  return 0;
}

/*
=================
CG_PredictPlayerState

Generates cg.predictedPlayerState for the current cg.time
cg.predictedPlayerState is guaranteed to be valid after exiting.

For demo playback, this will be an interpolation between two valid
playerState_t.

For normal gameplay, it will be the result of predicted usercmd_t on
top of the most recent playerState_t received from the server.

Each new snapshot will usually have one or more new usercmd over the last,
but we simulate all unacknowledged commands each time, not just the new ones.
This means that on an internet connection, quite a few pmoves may be issued
each frame.

OPTIMIZE: don't re-simulate unless the newly arrived snapshot playerState_t
differs from the predicted one.  Would require saving all intermediate
playerState_t during prediction. (this is "dead reckoning" and would definately
be nice to have in there (SA))

We detect prediction errors and allow them to be decayed off over several frames
to ease the jerk.
=================
*/

// rain - we need to keep pmext around for old frames, because Pmove()
// fills in some values when it does prediction.  This in itself is fine,
// but the prediction loop starts in the past and predicts from the
// snapshot time up to the current time, and having things like jumpTime
// appear to be set for prediction runs where they previously weren't
// is a Bad Thing.  This is my bugfix for #166.

pmoveExt_t oldpmext[CMD_BACKUP];

void CG_PredictPlayerState() {
  int cmdNum, current;
  playerState_t oldPlayerState;
  bool moved, predictError;
  usercmd_t oldestCmd;
  usercmd_t latestCmd;
  vec3_t deltaAngles;
  pmoveExt_t pmext;

  // unlagged - optimized prediction
  int stateIndex = 0, predictCmd = 0;
  int numPredicted = 0, numPlayedBack = 0; // debug code
  // END unlagged - optimized prediction

  cg.hyperspace = qfalse; // will be set if touching a trigger_teleport

  // if this is the first frame we must guarantee
  // predictedPlayerState is valid even if there is some
  // other error condition
  if (!cg.validPPS) {
    cg.validPPS = qtrue;
    cg.predictedPlayerState = cg.snap->ps;
  }

  // demo playback just copies the moves
  if ((cg.demoPlayback) || (cg.snap->ps.pm_flags & PMF_FOLLOW)) {
    CG_InterpolatePlayerState(qfalse);
    return;
  }

  // non-predicting local movement will grab the latest angles
  if (cg_nopredict.integer
#ifdef ALLOW_GSYNC
      || cgs.synchronousClients
#endif // ALLOW_GSYNC
  ) {
    cg_pmove.ps = &cg.predictedPlayerState;
    cg_pmove.pmext = &cg.pmext;

    cg.pmext.airleft = (cg.waterundertime - cg.time);

    // Arnout: are we using an mg42?
    if (cg_pmove.ps->eFlags & EF_MG42_ACTIVE ||
        cg_pmove.ps->eFlags & EF_AAGUN_ACTIVE) {
      cg.pmext.harc =
          cg_entities[cg_pmove.ps->viewlocked_entNum].currentState.origin2[0];
      cg.pmext.varc =
          cg_entities[cg_pmove.ps->viewlocked_entNum].currentState.origin2[1];

      VectorCopy(
          cg_entities[cg_pmove.ps->viewlocked_entNum].currentState.angles2,
          cg.pmext.centerangles);

      cg.pmext.centerangles[PITCH] =
          AngleNormalize180(cg.pmext.centerangles[PITCH]);
      cg.pmext.centerangles[YAW] =
          AngleNormalize180(cg.pmext.centerangles[YAW]);
      cg.pmext.centerangles[ROLL] =
          AngleNormalize180(cg.pmext.centerangles[ROLL]);
    }

    CG_InterpolatePlayerState(qtrue);
    return;
  }

  if (cg_pmove.ps && cg_pmove.ps->eFlags & EF_MOUNTEDTANK) {
    centity_t *tank =
        &cg_entities[cg_entities[cg.snap->ps.clientNum].tagParent];

    cg.pmext.centerangles[YAW] = tank->lerpAngles[YAW];
    cg.pmext.centerangles[PITCH] = tank->lerpAngles[PITCH];
  }

  // prepare for pmove
  cg_pmove.ps = &cg.predictedPlayerState;
  cg_pmove.pmext = &pmext; //&cg.pmext;
  cg_pmove.character =
      CG_CharacterForClientinfo(&cgs.clientinfo[cg.snap->ps.clientNum],
                                &cg_entities[cg.snap->ps.clientNum]);
  cg.pmext.airleft = (cg.waterundertime - cg.time);

  // Arnout: are we using an mg42?
  if (cg_pmove.ps->eFlags & EF_MG42_ACTIVE ||
      cg_pmove.ps->eFlags & EF_AAGUN_ACTIVE) {
    cg.pmext.harc =
        cg_entities[cg_pmove.ps->viewlocked_entNum].currentState.origin2[0];
    cg.pmext.varc =
        cg_entities[cg_pmove.ps->viewlocked_entNum].currentState.origin2[1];

    VectorCopy(cg_entities[cg_pmove.ps->viewlocked_entNum].currentState.angles2,
               cg.pmext.centerangles);

    cg.pmext.centerangles[PITCH] =
        AngleNormalize180(cg.pmext.centerangles[PITCH]);
    cg.pmext.centerangles[YAW] = AngleNormalize180(cg.pmext.centerangles[YAW]);
    cg.pmext.centerangles[ROLL] =
        AngleNormalize180(cg.pmext.centerangles[ROLL]);
  } else if (cg_pmove.ps->eFlags & EF_MOUNTEDTANK) {
    centity_t *tank =
        &cg_entities[cg_entities[cg.snap->ps.clientNum].tagParent];

    cg.pmext.centerangles[PITCH] = tank->lerpAngles[PITCH];
  }

  cg_pmove.skill = cgs.clientinfo[cg.snap->ps.clientNum].skill;

  for (int i = 0; i < MAX_CLIENTS; i++) {
    const int other = cg_entities[i].currentState.number;

    if (cg.snap->ps.clientNum == other) {
      continue;
    }

    if (!ETJump::playerIsSolid(cg.snap->ps.clientNum, other) ||
        ETJump::playerIsNoclipping(other)) {
      ETJump::tempTraceIgnoreClient(other);
    }
  }

  cg_pmove.trace = CG_TraceCapsule;
  cg_pmove.pointcontents = CG_PointContents;

  switch (cg_pmove.ps->pm_type) {
    case PM_DEAD:
      cg_pmove.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
      cg_pmove.ps->eFlags |= EF_DEAD;
      break;
    case PM_SPECTATOR:
      cg_pmove.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
      cg_pmove.trace = CG_TraceCapsule_World;
      break;
    case PM_NOCLIP:
      cg_pmove.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
      cg_pmove.trace = CG_TraceCapsule_NoPlayers;
      break;
    default:
      cg_pmove.tracemask = MASK_PLAYERSOLID;
      break;
  }

  if ((cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) ||
      (cg.snap->ps.pm_flags & PMF_LIMBO)) // JPW NERVE limbo
  {
    cg_pmove.tracemask &= ~CONTENTS_BODY; // spectators can fly through bodies
  }

  cg_pmove.noFootsteps = qfalse;
  cg_pmove.noWeapClips = qfalse;

  // save the state before the pmove so we can detect transitions
  oldPlayerState = cg.predictedPlayerState;

  current = trap_GetCurrentCmdNumber();

  // rain - fill in the current cmd with the latest prediction from
  // cg.pmext (#166)

  if (etj_noclipScale.value < 0.1) {
    cg.pmext.noclipScale = 0.1;
  } else if (etj_noclipScale.value > 20) {
    cg.pmext.noclipScale = 20;
  } else {
    cg.pmext.noclipScale = etj_noclipScale.value;
  }

  // let server handle mounted MG42 cooldown since client doesn't know
  // the heat values of each individual mounted gun in the map
  // otherwise we fire excess overheat events when client exits and re-enters
  // the gun, as the correct heat value is never assigned client side
  cg.pmext.weapHeat[WP_DUMMY_MG42] = 0.0f;

  memcpy(&oldpmext[current & CMD_MASK], &cg.pmext, sizeof(pmoveExt_t));

  // if we don't have the commands right after the snapshot, we
  // can't accurately predict a current position, so just freeze at
  // the last good position we had
  cmdNum = current - CMD_BACKUP + 1;
  trap_GetUserCmd(cmdNum, &oldestCmd);
  if (oldestCmd.serverTime > cg.snap->ps.commandTime &&
      oldestCmd.serverTime < cg.time) // special check for map_restart
  {
    if (cg_showmiss.integer) {
      CG_Printf("exceeded PACKET_BACKUP on commands\n");
    }
  }

  // get the latest command, so we can know which commands are from
  // previous map_restarts
  trap_GetUserCmd(current, &latestCmd);

  // get the most recent information we have, even if
  // the server time is beyond our current cg.time,
  // because predicted player positions are going to
  // be ahead of everything else anyway
  // rain - NEIN - this'll cause us to execute events from the next
  // frame early, resulting in doubled events and the like.  it seems to
  // be worse as far as prediction, too, so BLAH at id. (#405)
#if 0
	if (cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport)
	{
		cg.predictedPlayerState = cg.nextSnap->ps;
		cg.physicsTime          = cg.nextSnap->serverTime;
	}
	else
	{
#endif
  cg.predictedPlayerState = cg.snap->ps;
  cg.physicsTime = cg.snap->serverTime;
  //	}

  cg_pmove.pmove_fixed = pmove_fixed.integer;
  cg_pmove.pmove_msec = cgs.pmove_msec;

  // Zero: shared values between server & client
  cg_pmove.shared = cgs.shared;

  // unlagged - optimized prediction
  //  Like the comments described above, a player's state is entirely
  //  re-predicted from the last valid snapshot every client frame, which
  //  can be really, really, really slow.  Every old command has to be
  //  run again.  For every client frame that is *not* directly after a
  //  snapshot, this is unnecessary, since we have no new information.
  //  For those, we'll play back the predictions from the last frame and
  //  predict only the newest commands.  Essentially, we'll be doing
  //  an incremental predict instead of a full predict.
  //
  //  If we have a new snapshot, we can compare its player state's command
  //  time to the command times in the queue to find a match.  If we find
  //  a matching state, and the predicted version has not deviated, we can
  //  use the predicted state as a base - and also do an incremental predict.
  //
  //  With this method, we get incremental predicts on every client frame
  //  except a frame following a new snapshot in which there was a prediction
  //  error. This yields anywhere from a 15% to 40% performance increase,
  //  depending on how much of a bottleneck the CPU is.

  if (etj_optimizePrediction.integer) {
    if (cg.nextFrameTeleport || cg.thisFrameTeleport) {
      // do a full predict
      cg.lastPredictedCommand = 0;
      cg.backupStateTail = cg.backupStateTop;
      predictCmd = current - CMD_BACKUP + 1;
    }
    // cg.physicsTime is the current snapshot's physicsTime
    // if it's the same as the last one
    else if (cg.physicsTime == cg.lastPhysicsTime) {
      // we have no new information, so do an incremental predict
      predictCmd = cg.lastPredictedCommand + 1;
    } else {
      // we have a new snapshot
      bool error = true;

      // loop through the saved states queue
      for (int i = cg.backupStateTop; i != cg.backupStateTail;
           i = (i + 1) % MAX_BACKUP_STATES) {
        // if we find a predicted state whose commandTime matches the snapshot
        // player state's commandTime
        if (cg.backupStates[i].commandTime ==
            cg.predictedPlayerState.commandTime) {
          // make sure the state differences are acceptable
          int errorcode =
              CG_PredictionOk(&cg.predictedPlayerState, &cg.backupStates[i]);

          // too much change?
          if (errorcode) {
            if (cg_showmiss.integer) {
              CG_Printf(
                  "CG_PredictPlayerState: errorcode %i '%s' at cg.time: %i\n",
                  errorcode, predictionStrings[errorcode], cg.time);
            }
            // yeah, so do a full predict
            break;
          }

          // this one is almost exact, so we'll copy it in as the starting point
          *cg_pmove.ps = cg.backupStates[i];
          // advance the head
          cg.backupStateTop = (i + 1) % MAX_BACKUP_STATES;

          // set the next command to predict
          predictCmd = cg.lastPredictedCommand + 1;

          // a saved state matched, so flag it
          error = false;
          break;
        }
      }

      // if no saved states matched
      if (error) {
        // do a full predict
        cg.lastPredictedCommand = 0;
        cg.backupStateTail = cg.backupStateTop;
        predictCmd = current - CMD_BACKUP + 1;
      }
    }

    // keep track of the server time of the last snapshot,
    // so we know when we're starting from a new one in future calls
    cg.lastPhysicsTime = cg.physicsTime;
    stateIndex = cg.backupStateTop;
  }
  // END unlagged - optimized prediction

  // run cmds
  moved = false;
  predictError = true;
  for (cmdNum = current - CMD_BACKUP + 1; cmdNum <= current; cmdNum++) {
    // get the command
    trap_GetUserCmd(cmdNum, &cg_pmove.cmd);
    // get the previous command
    trap_GetUserCmd(cmdNum - 1, &cg_pmove.oldcmd);

    // check for a prediction error from last frame on a lan,
    // this will often be the exact value from the snapshot,
    // but on a wan we will have to predict several commands
    // to get to the point we want to compare

    // this was moved here to fix pmove_fixed prediction error detection and
    // subsequent error smoothing:
    // when using pmove_fixed not all user commands will result in
    // an actual move because commands are run for the duration of pmove_msec
    // if commands are generated every 4ms (250fps) and pmove_msec is 8ms,
    // only 1 out of those 2 commands will be run
    // this will mean that in most situations, the check below would happen
    // on 2nd user command which is not run, and the prediction error
    // is not registered, causing more jittery game when miss prediction happens
    // since there might be many user commands for same playerstate commandTime
    // (as explained above) need to make sure it is only checked once per frame
    if (cg.predictedPlayerState.commandTime == oldPlayerState.commandTime &&
        predictError) {
      vec3_t delta;
      float len;

      predictError = false;
      if (BG_PlayerMounted(cg_pmove.ps->eFlags)) {
        // no prediction errors here, we're locked in place
        VectorClear(cg.predictedError);
      } else if (cg.thisFrameTeleport) {
        // a teleport will not cause an error decay
        VectorClear(cg.predictedError);
        if (cg_showmiss.integer) {
          CG_Printf("PredictionTeleport\n");
        }
        cg.thisFrameTeleport = qfalse;
      } else if (!cg.showGameView) {
        vec3_t adjusted;
        CG_AdjustPositionForMover(cg.predictedPlayerState.origin,
                                  cg.predictedPlayerState.groundEntityNum,
                                  cg.physicsTime, cg.oldTime, adjusted,
                                  nullptr);

        if (cg_showmiss.integer) {
          if (!VectorCompare(oldPlayerState.origin, adjusted)) {
            CG_Printf("prediction error\n");
          }
        }
        VectorSubtract(oldPlayerState.origin, adjusted, delta);
        len = VectorLength(delta);
        if (len > 0.1) {
          if (cg_showmiss.integer) {
            CG_Printf("Prediction miss: %f\n", len);
          }
          if (cg_errorDecay.integer) {
            auto t = static_cast<float>(cg.time - cg.predictedErrorTime);
            float f = (cg_errorDecay.value - t) / cg_errorDecay.value;
            if (f < 0) {
              f = 0;
            }
            if (f > 0 && cg_showmiss.integer) {
              CG_Printf("Double prediction decay: %f\n", f);
            }
            VectorScale(cg.predictedError, f, cg.predictedError);
          } else {
            VectorClear(cg.predictedError);
          }
          VectorAdd(delta, cg.predictedError, cg.predictedError);
          cg.predictedErrorTime = cg.oldTime;
        }
      }
    }

    // don't do anything if the time is before the snapshot player time
    if (cg_pmove.cmd.serverTime <= cg.predictedPlayerState.commandTime) {
      memcpy(&pmext, &oldpmext[cmdNum & CMD_MASK], sizeof(pmoveExt_t));
      continue;
    }

    // don't do anything if the command was from a previous
    // map_restart
    if (cg_pmove.cmd.serverTime > latestCmd.serverTime) {
      continue;
    }

    // don't predict gauntlet firing, which is only supposed to
    // happen when it actually inflicts damage
    cg_pmove.gauntletHit = qfalse;

    if (cg_pmove.pmove_fixed) {
      cg_pmove.cmd.serverTime =
          ((cg_pmove.cmd.serverTime + cgs.pmove_msec - 1) / cgs.pmove_msec) *
          cgs.pmove_msec;
    }

    // ydnar: if server respawning, freeze the player
    if (cg.serverRespawning) {
      cg_pmove.ps->pm_type = PM_FREEZE;
    }

    cg_pmove.gametype = cgs.gametype;

    // rain - only fill in the charge times if we're on a
    // playing team
    if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_AXIS ||
        cg.snap->ps.persistant[PERS_TEAM] == TEAM_ALLIES) {
      cg_pmove.ltChargeTime =
          cg.ltChargeTime[cg.snap->ps.persistant[PERS_TEAM] - 1];
      cg_pmove.soldierChargeTime =
          cg.soldierChargeTime[cg.snap->ps.persistant[PERS_TEAM] - 1];
      cg_pmove.engineerChargeTime =
          cg.engineerChargeTime[cg.snap->ps.persistant[PERS_TEAM] - 1];
      cg_pmove.medicChargeTime =
          cg.medicChargeTime[cg.snap->ps.persistant[PERS_TEAM] - 1];
      cg_pmove.covertopsChargeTime =
          cg.covertopsChargeTime[cg.snap->ps.persistant[PERS_TEAM] - 1];
    }

    // ETJump: client side no activate lean
    cg_pmove.noActivateLean = etj_noActivateLean.integer ? qtrue : qfalse;
    cg_pmove.noPanzerAutoswitch = etj_noPanzerAutoswitch.integer;

    // grab data, we only want the final result
    // rain - copy the pmext as it was just before we
    // previously ran this cmd (or, this will be the
    // current predicted data if this is the current cmd) (#166)
    memcpy(&pmext, &oldpmext[cmdNum & CMD_MASK], sizeof(pmoveExt_t));

    fflush(stdout);

    // unlagged - optimized prediction
    if (etj_optimizePrediction.integer) {
      // if we need to predict this command, or we've run out of space in the
      // saved states queue
      if (cmdNum >= predictCmd ||
          (stateIndex + 1) % MAX_BACKUP_STATES == cg.backupStateTop) {
        // run the Pmove
        Pmove(&cg_pmove);
        numPredicted++; // debug code

        // record the last predicted command
        cg.lastPredictedCommand = cmdNum;

        // if we haven't run out of space in the saved states queue
        if ((stateIndex + 1) % MAX_BACKUP_STATES != cg.backupStateTop) {
          // save the state for the false case (of cmdNum >= predictCmd)
          // in later calls to this function
          cg.backupStates[stateIndex] = *cg_pmove.ps;
          stateIndex = (stateIndex + 1) % MAX_BACKUP_STATES;
          cg.backupStateTail = stateIndex;
        }
      } else {
        numPlayedBack++; // debug code

        if (cg_showmiss.integer && cg.backupStates[stateIndex].commandTime !=
                                       cg_pmove.cmd.serverTime) {
          // this should ONLY happen just after changing the value of
          // pmove_fixed
          CG_Printf("CG_PredictPlayerState: saved state miss\n");
        }

        // play back the command from the saved states
        *cg_pmove.ps = cg.backupStates[stateIndex];

        // go to the next element in the saved states array
        stateIndex = (stateIndex + 1) % MAX_BACKUP_STATES;
      }
    } else {
      // run the Pmove
      Pmove(&cg_pmove);
      numPredicted++; // debug code
    }
    // END unlagged - optimized prediction

    moved = true;

    // add push trigger movement effects
    CG_TouchTriggerPrediction();
  }

  ETJump::resetTempTraceIgnoredClients();

  // unlagged - optimized prediction
  //  do a /condump after a few seconds of this
  //  if everything is working right, numPredicted should be 1 more than 98%
  //  of the time, meaning only ONE predicted move was done in the frame
  //  you should see other values for numPredicted after CG_PredictionOk
  //  returns nonzero, and that's it
  if (cg_showmiss.integer & 2) {
    CG_Printf("cg.time: %d, numPredicted: %d, numPlayedBack: %d\n", cg.time,
              numPredicted, numPlayedBack); // debug code
  }
  // END unlagged - optimized prediction

  if (cg_showmiss.integer & 4) {
    CG_Printf("[%i : %i] ", cg_pmove.cmd.serverTime, cg.time);
  }

  if (!moved) {
    if (cg_showmiss.integer & 16) {
      CG_Printf("CG_PredictPlayerState: not moved\n");
    }
    return;
  }

  // restore pmext
  memcpy(&cg.pmext, &pmext, sizeof(pmoveExt_t));

  if (!cg.showGameView) {
    // adjust for the movement of the groundentity
    CG_AdjustPositionForMover(
        cg.predictedPlayerState.origin, cg.predictedPlayerState.groundEntityNum,
        cg.physicsTime, cg.time, cg.predictedPlayerState.origin, deltaAngles);

    // add deltaAngles (fixes jittery view while riding on movers)
    // only do this if player is prone or using set mortar
    if (deltaAngles[YAW] != 0.0f &&
        ((cg.predictedPlayerState.eFlags & EF_PRONE) ||
         cg.weaponSelect == WP_MORTAR_SET)) {
      cg.predictedPlayerState.delta_angles[YAW] +=
          ANGLE2SHORT(deltaAngles[YAW]);
      PM_UpdateViewAngles(&cg.predictedPlayerState, &cg.pmext, &cg_pmove.cmd,
                          cg_pmove.trace, cg_pmove.tracemask);
    }
  }

  // fire events and other transition triggered things
  CG_TransitionPlayerState(&cg.predictedPlayerState, &oldPlayerState);

  // ydnar: shake player view here, rather than fiddle with view angles
  if (cg.time > cg.cameraShakeTime) {
    cg.cameraShakeScale = 0;
  } else {
    // starts at 1, approaches 0 over time
    const auto x = static_cast<double>(cg.cameraShakeTime - cg.time) /
                   cg.cameraShakeLength;
    const auto shakeX =
        static_cast<float>(std::cos(M_PI * 7 * x + cg.cameraShakePhase) * x *
                           6.0f * cg.cameraShakeScale);
    const auto shakeY =
        static_cast<float>(std::sin(M_PI * 17 * x + cg.cameraShakePhase) * x *
                           6.0f * cg.cameraShakeScale);
    const auto shakeZ =
        static_cast<float>(std::sin(M_PI * 8 * 13 + cg.cameraShakePhase) * x *
                           6.0f * cg.cameraShakeScale);
    const float frameTime = 8.0f; // 125fps
    const float scale = static_cast<float>(cg.frametime) / frameTime;

    const vec3_t shake = {shakeX * scale, shakeY * scale, shakeZ * scale};

    const int time = cg.time - cg.predictedErrorTime;
    float frac =
        (cg_errorDecay.value - static_cast<float>(time)) / cg_errorDecay.value;

    if (frac < 0) {
      frac = 0;
    }

    VectorScale(cg.predictedError, frac, cg.predictedError);
    VectorAdd(shake, cg.predictedError, cg.predictedError);
    cg.predictedErrorTime = cg.oldTime;
  }
}
