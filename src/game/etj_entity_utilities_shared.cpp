/*
 * MIT License
 *
 * Copyright (c) 2024 ETJump team <zero@etjump.com>
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

    // bits 8-23 holds spawnflags
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
                                       usercmd_t *cmd, vec3_t origin,
                                       vec3_t angles, vec3_t mins,
                                       vec3_t maxs) {
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
    ps->pm_type |= PMF_TIME_KNOCKBACK;
  }

  if (spawnflags & static_cast<int>(TeleporterSpawnflags::ResetSpeed)) {
    // pitch/yaw preservation requires a tiny amount of speed,
    // else we can't determine which side we're entering from
    VectorSet(ps->velocity, 0.01f, 0.01f, 0.0f);
  }

  if (spawnflags & static_cast<int>(TeleporterSpawnflags::ConvertSpeed)) {
    float speed = VectorLength(ps->velocity);
    AngleVectors(angles, ps->velocity, nullptr, nullptr);
    VectorScale(ps->velocity, speed, ps->velocity);
  } else if (spawnflags &
                 static_cast<int>(TeleporterSpawnflags::RelativePitch) ||
             spawnflags &
                 static_cast<int>(TeleporterSpawnflags::RelativePitchYaw)) {
    calcRelativeAngles(ps, mins, maxs, spawnflags, origin, angles);
  }

  VectorCopy(origin, ps->origin);
  ps->origin[2] += 1;

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

void EntityUtilsShared::calcRelativeAngles(playerState_t *ps, const vec3_t mins,
                                           const vec3_t maxs,
                                           const int &spawnflags, vec3_t origin,
                                           vec3_t angles) {
  vec3_t newOrigin, newViewAngles{};
  vec3_t newVelocity = {0, 0, 0}, offset = {0, 0, 0};
  vec3_t triggerOrigin, tempActivator, tempOrigin;
  vec3_t normalizedVelocity, veloAngles;
  vec3_t sPlane[6];
  vec_t minDistYZ, minDistXZ, minDistXY;
  vec_t speed, length;
  bool negDir = false;
  const bool relativePitch =
      spawnflags & static_cast<int>(TeleporterSpawnflags::RelativePitchYaw);

  // normalized velocity is the direction vector
  // for calculating the intersection points
  VectorNormalize2(ps->velocity, normalizedVelocity);

  // calculate the origin of the trigger brush
  VectorSubtract(maxs, mins, tempOrigin);
  VectorMA(mins, 0.5f, tempOrigin, triggerOrigin);

  // calculate the intersection-points of player movement
  // with the 6 planes of the trigger brush
  // the shortest distance between player and intersection points
  // selects the plane (where did we get into the trigger, from what side?)
  if (normalizedVelocity[0] != 0.0f) {
    VectorMA(ps->origin, (mins[0] - ps->origin[0]) / normalizedVelocity[0],
             normalizedVelocity, sPlane[0]);
    VectorMA(ps->origin, (maxs[0] - ps->origin[0]) / normalizedVelocity[0],
             normalizedVelocity, sPlane[3]);
    minDistYZ = std::min(VectorDistance(ps->origin, sPlane[0]),
                         VectorDistance(ps->origin, sPlane[3]));
  } else {
    minDistYZ = 10000.0f;
  }

  if (normalizedVelocity[1] != 0.0f) {
    VectorMA(ps->origin, (mins[1] - ps->origin[1]) / normalizedVelocity[1],
             normalizedVelocity, sPlane[1]);
    VectorMA(ps->origin, (maxs[1] - ps->origin[1]) / normalizedVelocity[1],
             normalizedVelocity, sPlane[4]);
    minDistXZ = std::min(VectorDistance(ps->origin, sPlane[1]),
                         VectorDistance(ps->origin, sPlane[4]));
  } else {
    minDistXZ = 10000.0f;
  }

  if (normalizedVelocity[2] != 0.0f) {
    VectorMA(ps->origin, (mins[2] - ps->origin[2]) / normalizedVelocity[2],
             normalizedVelocity, sPlane[2]);
    VectorMA(ps->origin, (maxs[2] - ps->origin[2]) / normalizedVelocity[2],
             normalizedVelocity, sPlane[5]);
    minDistXY = std::min(VectorDistance(ps->origin, sPlane[2]),
                         VectorDistance(ps->origin, sPlane[5]));
  } else {
    minDistXY = 10000.0f;
  }

  // copy origins to tempvars
  VectorCopy(triggerOrigin, tempOrigin);
  VectorCopy(ps->origin, tempActivator);

  // PLANE YZ (NORMAL X)
  if (minDistYZ < minDistXZ && minDistYZ < minDistXY) {
    // check +/- direction of trigger
    if (triggerOrigin[0] < ps->origin[0]) {
      negDir = true;
    }

    // calc viewangles
    VectorCopy(angles, newViewAngles);
    newViewAngles[YAW] += ps->viewangles[YAW];

    if (relativePitch) {
      newViewAngles[PITCH] += ps->viewangles[PITCH];
    }

    // calc velocity
    speed = VectorLength(ps->velocity);
    vectoangles(ps->velocity, veloAngles);
    veloAngles[YAW] += angles[YAW];

    if (negDir) {
      veloAngles[YAW] += 180.0f;
    }

    AngleVectors(veloAngles, newVelocity, nullptr, nullptr);
    VectorScale(newVelocity, speed, newVelocity);

    // calc origin
    tempOrigin[0] = tempActivator[0] = 0.0f;
    VectorSubtract(tempActivator, tempOrigin, offset);

    if (negDir) {
      offset[1] *= -1.0f;
    }

    length = offset[1];
    offset[0] = static_cast<float>(std::sin(-DEG2RAD(angles[1])) * length);
    offset[1] = static_cast<float>(std::cos(-DEG2RAD(angles[1])) * length);
  }
  // PLANE XZ (NORMAL Y)
  else if (minDistXZ < minDistYZ && minDistXZ < minDistXY) {
    // check +/- direction of trigger
    if (triggerOrigin[1] < ps->origin[1]) {
      negDir = qtrue;
    }

    // calc viewangles
    VectorCopy(angles, newViewAngles);
    newViewAngles[YAW] += (ps->viewangles[YAW] - 90.0f);

    if (relativePitch) {
      newViewAngles[PITCH] += ps->viewangles[PITCH];
    }

    // calc velocity
    speed = VectorLength(ps->velocity);
    vectoangles(ps->velocity, veloAngles);
    veloAngles[YAW] += angles[YAW] - 90.0f;

    if (negDir) {
      veloAngles[YAW] += 180.0f;
    }

    AngleVectors(veloAngles, newVelocity, nullptr, nullptr);
    VectorScale(newVelocity, speed, newVelocity);

    // calc origin offset
    tempOrigin[1] = tempActivator[1] = 0.0f;
    VectorSubtract(tempActivator, tempOrigin, offset);
    if (triggerOrigin[1] < ps->origin[1]) {
      offset[0] *= -1.0f;
    }

    length = offset[0];
    offset[1] =
        static_cast<float>(std::sin(DEG2RAD(angles[1] - 90.0f)) * length);
    offset[0] =
        static_cast<float>(std::cos(DEG2RAD(angles[1] - 90.0f)) * length);
  }
  // PLANE XY (NORMAL Z) // makes no sense, set dest values
  else if (minDistXY < minDistXZ && minDistXY < minDistYZ) {
    // viewangles
    VectorCopy(angles, newViewAngles);
    // velocity
    VectorCopy(vec3_origin, newVelocity);
    // origin
    VectorCopy(vec3_origin, offset);
  }

  // add origin offset
  VectorAdd(origin, offset, newOrigin);
  // set velocity
  VectorCopy(newVelocity, ps->velocity);
  // correct Viewangles
  if (negDir) {
    newViewAngles[YAW] += 180.0f;
  }

  // setup new origin and angles
  VectorCopy(newOrigin, origin);
  VectorCopy(newViewAngles, angles);
}

bool EntityUtilsShared::canPredictTeleport(const entityState_t *teleporter,
                                           const int &spawnflags) {
  // FIXME: I'd like to make these work but they just completely break
  //  since they run the angle calculations on same values multiple times
  if (spawnflags & static_cast<int>(TeleporterSpawnflags::RelativePitch) ||
      spawnflags & static_cast<int>(TeleporterSpawnflags::RelativePitch)) {
    return false;
  }

  // multi-target teleporter
  if (VectorCompare(teleporter->origin2, vec3_origin) &&
      VectorCompare(teleporter->angles2, vec3_origin)) {
    return false;
  }

  return true;
}

} // namespace ETJump
