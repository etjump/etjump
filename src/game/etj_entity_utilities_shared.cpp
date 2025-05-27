/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "etj_entity_utilities_shared.h"

// Entity helper functions used by both games (client side predicted entities)

namespace ETJump {
void EntityUtilsShared::touchPusher(playerState_t *ps, const int time,
                                    const entityState_t *pusher) {
  if (ps->pm_type != PM_NORMAL) {
    return;
  }

  // 'pusher->frame' hold the wait key value
  if (ps->powerups[PW_PUSHERPREDICT] + pusher->frame > time) {
    return;
  }

  ps->powerups[PW_PUSHERPREDICT] = time;

  vec3_t dir;
  float s;

  // first 8 bits holds noise_index
  if (pusher->constantLight & 0xff) {
    VectorNormalize2(pusher->origin2, dir);
    s = DotProduct(ps->velocity, dir);

    if (s < 500) {
      BG_AddPredictableEventToPlayerstate(EV_GENERAL_SOUND,
                                          pusher->constantLight & 0xff, ps);
    }
  }

  if (pusher->eType == ET_VELOCITY_PUSH_TRIGGER) {
    vec3_t pushVel{};

    // bits 8-16 holds spawnflags
    const int spawnflags = (pusher->constantLight >> 8) & 0xff;
    setPushVelocity(ps, pusher->origin2, spawnflags, pushVel);
    VectorCopy(pushVel, ps->velocity);
  } else {
    VectorCopy(pusher->origin2, ps->velocity);
  }
}

void EntityUtilsShared::setPushVelocity(const playerState_t *ps,
                                        const vec_t *origin2,
                                        const int &spawnflags, vec_t *pushVel) {
  VectorCopy(ps->velocity, pushVel);

  if (spawnflags & static_cast<int>(PusherSpawnFlags::AddXY) &&
      spawnflags & static_cast<int>(PusherSpawnFlags::AddZ)) {
    VectorAdd(origin2, ps->velocity, pushVel);
  } else {
    if (spawnflags & static_cast<int>(PusherSpawnFlags::AddXY)) {
      pushVel[0] += origin2[0];
      pushVel[1] += origin2[1];
      pushVel[2] = origin2[2];
    }

    if (spawnflags & static_cast<int>(PusherSpawnFlags::AddZ)) {
      pushVel[0] = origin2[0];
      pushVel[1] = origin2[1];
      pushVel[2] += origin2[2];
    }
  }
}

void EntityUtilsShared::teleportPlayer(playerState_t *ps, entityState_t *player,
                                       entityState_t *teleporter,
                                       usercmd_t *cmd, const vec3_t origin,
                                       vec3_t angles) {
  // bits 8-31 contain the spawnflags
  const int spawnflags = (teleporter->constantLight >> 8) & 0xffffff;

  // first 8 bits hold noise key
  if (teleporter->constantLight & 0xff) {
    BG_AddPredictableEventToPlayerstate(EV_GENERAL_SOUND,
                                        teleporter->constantLight & 0xff, ps);
  }

  // frame holds 'outspeed' key
  if (teleporter->frame) {
    // if we don't have any velocity when teleporting,
    // there's nothing to scale from, so let's add some
    if (VectorCompare(ps->velocity, vec3_origin)) {
      VectorSet(ps->velocity, 0.01f, 0.01f, 0.0f);
    }

    VectorNormalize(ps->velocity);
    VectorScale(ps->velocity, teleporter->frame, ps->velocity);
  }

  if (spawnflags & static_cast<int>(TeleporterSpawnflags::Knockback)) {
    ps->pm_time = 160;
    ps->pm_flags |= PMF_TIME_KNOCKBACK;
  }

  if (spawnflags & static_cast<int>(TeleporterSpawnflags::ResetSpeed)) {
    VectorClear(ps->velocity);
  } else if (spawnflags &
             static_cast<int>(TeleporterSpawnflags::ConvertSpeed)) {
    float speed = VectorLength(ps->velocity);
    AngleVectors(angles, ps->velocity, nullptr, nullptr);
    VectorScale(ps->velocity, speed, ps->velocity);
  }

  VectorCopy(origin, ps->origin);

  if (!(spawnflags & static_cast<int>(TeleporterSpawnflags::NoZOffset))) {
    ps->origin[2] += 1;
  }

  // toggle the teleport bit so the client knows to not lerp
  ps->eFlags ^= EF_TELEPORT_BIT;

  // set angles
  setViewAngles(ps, player, cmd, angles);

  // save results of pmove
  BG_PlayerStateToEntityState(ps, player, qtrue);

  // linking is server side only
#ifndef CGAMEDLL
  gentity_t *ent = &g_entities[ps->clientNum];
  // use the precise origin for linking
  VectorCopy(ps->origin, ent->r.currentOrigin);

  if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
    trap_LinkEntity(ent);
  }
#endif
}

void EntityUtilsShared::setViewAngles(playerState_t *ps, entityState_t *es,
                                      usercmd_t *cmd, const vec3_t angle) {
  // set the delta angles
  for (int i = 0; i < 3; i++) {
    int cmdAngle;

    cmdAngle = ANGLE2SHORT(angle[i]);
    ps->delta_angles[i] = cmdAngle - cmd->angles[i];
  }

  VectorCopy(angle, es->angles);
  VectorCopy(es->angles, ps->viewangles);
}

void EntityUtilsShared::portalTeleport(playerState_t *ps, entityState_t *player,
                                       const entityState_t *portal,
                                       usercmd_t *cmd, const int time) {
  if (ps->powerups[PW_PORTALPREDICT] + PORTAL_TOUCH_COOLDOWN > time) {
    return;
  }

  ps->powerups[PW_PORTALPREDICT] = time;

  // Feen: Extra little boost coming out of a portal...
  static constexpr float PORTAL_NUDGE = 50.0f;
  const vec_t velocityLength = VectorLength(ps->velocity);

  vec3_t dstOrigin;
  vec3_t dstAngles;

  VectorCopy(portal->origin2, dstOrigin);
  VectorCopy(portal->angles2, dstAngles);

  // spit the player out
  AngleVectors(dstAngles, ps->velocity, nullptr, nullptr);

  // new origin method
  VectorMA(dstOrigin, 32.0f, ps->velocity, ps->origin);

  // Scale new vector back to old velocity
  VectorScale(ps->velocity, velocityLength + PORTAL_NUDGE, ps->velocity);

  // toggle the teleport bit so the client knows to not lerp
  ps->eFlags ^= EF_TELEPORT_BIT;

  // Adjust view angles so portals don't have you looking straight up or
  // down...
  if ((dstAngles[PITCH] >= -105 && dstAngles[PITCH] <= -75) ||
      (dstAngles[PITCH] >= -285 && dstAngles[PITCH] <= -255)) {
    // if the portal is in the ceiling, we need to offset the player more
    // so that their head doesn't get stuck inside the ceiling
    if (dstAngles[PITCH] >= -285 && dstAngles[PITCH] <= -255) {
      ps->origin[2] -= 32.0f;
    }
  } else {
    vec3_t viewAngles;

    viewAngles[PITCH] = ps->viewangles[PITCH];
    viewAngles[YAW] = dstAngles[YAW];
    viewAngles[ROLL] = dstAngles[ROLL];

    setViewAngles(ps, player, cmd, viewAngles);
  }

  // prevent crashland when landing on portals
  BG_AddPredictableEventToPlayerstate(EV_PORTAL_TELEPORT, 0, ps);

  // save results of pmove
  BG_PlayerStateToEntityState(ps, player, qtrue);

// linking is server side only
#ifdef GAMEDLL
  // use the precise origin for linking
  gentity_t *ent = &g_entities[ps->clientNum];
  VectorCopy(ps->origin, ent->r.currentOrigin);

  if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
    trap_LinkEntity(ent);
  }
#endif
}

void EntityUtilsShared::setPortalBBox(vec3_t mins, vec3_t maxs,
                                      const vec3_t angles, const float scale) {
  float width;
  float height;
  float depth;

  // vertical portal (on walls)
  if (angles[PITCH] == -0) {
    depth = PORTAL_BBOX_VERT_DEPTH;
    height = width = PORTAL_BBOX_WIDTH * scale;

    if ((angles[YAW] >= 45 && angles[YAW] <= 135) ||
        (angles[YAW] >= 225 && angles[YAW] <= 315)) {
      VectorSet(mins, -height, -depth, -width);
      VectorSet(maxs, height, depth, width);
    } else {
      VectorSet(mins, -depth, -height, -width);
      VectorSet(maxs, depth, height, width);
    }
  } else { // horizontal (including sloped surfaces)
    depth = width = PORTAL_BBOX_WIDTH * scale;
    height = PORTAL_BBOX_HEIGHT;

    VectorSet(mins, -depth, -width, -height);
    VectorSet(maxs, depth, width, height);
  }
}
} // namespace ETJump
