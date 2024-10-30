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

#include <stdexcept>

#include "etj_utilities.h"
#include "etj_save_system.h"
#include "etj_map_statistics.h"
#include "etj_timerun_v2.h"

std::vector<int> Utilities::getSpectators(int clientNum) {
  std::vector<int> spectators;

  for (auto i = 0; i < level.numConnectedClients; i++) {
    gentity_t *player = g_entities + level.sortedClients[i];

    if (level.sortedClients[i] == clientNum) {
      continue;
    }

    if (!player->client) {
      continue;
    }

    if (player->client->sess.sessionTeam != TEAM_SPECTATOR) {
      continue;
    }

    if (player->client->ps.clientNum == clientNum) {
      spectators.push_back(g_entities - player);
    }
  }

  return spectators;
}

void Utilities::startRun(int clientNum) {
  gentity_t *player = g_entities + clientNum;

  player->client->sess.timerunActive = qtrue;
  ETJump::UpdateClientConfigString(*player);

  if (!(player->client->sess.runSpawnflags &
        static_cast<int>(ETJump::TimerunSpawnflags::NoSave))) {
    ETJump::saveSystem->clearTimerunSaves(player);
  }

  // if cheats are enabled, just exit here
  if (g_cheats.integer) {
    return;
  }

  ETJump::TimerunV2::removeDisallowedWeapons(player);
  ETJump::TimerunV2::removePlayerProjectiles(player);

  // force swap if the current weapon isn't allowed in timeruns
  if (BG_WeaponDisallowedInTimeruns(player->client->ps.weapon)) {
    selectValidWeapon(player);
  }

  ClearPortals(player);
}

void Utilities::stopRun(int clientNum) {
  gentity_t *player = g_entities + clientNum;

  player->client->sess.timerunActive = qfalse;
  ETJump::UpdateClientConfigString(*player);
}

void Utilities::selectValidWeapon(const gentity_t *ent) {
  // primary > secondary > knife
  if (BG_WeaponHasAmmo(&ent->client->ps, ent->client->sess.playerWeapon)) {
    ent->client->ps.weapon = ent->client->sess.playerWeapon;
  } else if (BG_WeaponHasAmmo(&ent->client->ps,
                              ent->client->sess.playerWeapon2)) {
    ent->client->ps.weapon = ent->client->sess.playerWeapon2;
  } else {
    ent->client->ps.weapon = WP_KNIFE;
  }
}

bool Utilities::inNoNoclipArea(gentity_t *ent) {
  trace_t trace;
  trap_TraceCapsule(&trace, ent->client->ps.origin, ent->r.mins, ent->r.maxs,
                    ent->client->ps.origin, ent->client->ps.clientNum,
                    CONTENTS_NONOCLIP);

  // if we're touching a no-noclip area, do another trace for solids
  // so that we don't get instantly stuck in a wall, unable to noclip
  // if we fly to a no-noclip area through a wall
  if (level.noNoclip) {
    if (trace.fraction == 1.0f) {
      trap_TraceCapsule(&trace, ent->client->ps.origin, ent->r.mins,
                        ent->r.maxs, ent->client->ps.origin,
                        ent->client->ps.clientNum, CONTENTS_SOLID);

      if (!trace.allsolid) {
        return true;
      }
    }
  } else {
    if (trace.fraction != 1.0f) {
      trap_TraceCapsule(&trace, ent->client->ps.origin, ent->r.mins,
                        ent->r.maxs, ent->client->ps.origin,
                        ent->client->ps.clientNum, CONTENTS_SOLID);

      if (!trace.allsolid) {
        return true;
      }
    }
  }

  return false;
}

std::string Utilities::timestampToString(int timestamp, const char *format,
                                         const char *start) {
  char buf[1024];
  struct tm *lt = NULL;
  time_t toConvert = timestamp;
  lt = localtime(&toConvert);
  if (timestamp > 0) {
    strftime(buf, sizeof(buf), format, lt);
  } else {
    return start;
  }

  return std::string(buf);
}

bool Utilities::anyonePlaying() {
  for (auto i = 0; i < level.numConnectedClients; i++) {
    auto clientNum = level.sortedClients[i];
    auto target = g_entities + clientNum;

    if (target->client->sess.sessionTeam != TEAM_SPECTATOR) {
      return true;
    }
  }
  return false;
}

void Utilities::Log(const std::string &message) {
  G_LogPrintf(message.c_str());
}

void Utilities::Console(const std::string &message) {
  G_Printf(message.c_str());
}

void Utilities::Error(const std::string &error) { G_Error(error.c_str()); }

std::vector<std::string> Utilities::getMaps() {
  std::vector<std::string> maps;
  ETJump::MapStatistics mapStats;

  int i = 0;
  int numDirs = 0;
  int dirLen = 0;
  char dirList[32768];
  char *dirPtr = nullptr;
  numDirs = trap_FS_GetFileList("maps", ".bsp", dirList, sizeof(dirList));
  dirPtr = dirList;

  for (i = 0; i < numDirs; i++, dirPtr += dirLen + 1) {
    dirLen = static_cast<int>(strlen(dirPtr));
    if (strlen(dirPtr) > 4) {
      dirPtr[strlen(dirPtr) - 4] = '\0';
    }

    char buf[MAX_QPATH] = "\0";
    Q_strncpyz(buf, dirPtr, sizeof(buf));
    Q_strlwr(buf);

    if (ETJump::MapStatistics::isBlockedMap(buf)) {
      continue;
    }

    maps.emplace_back(buf);
  }

  return maps;
}

void Utilities::getOriginOrBmodelCenter(const gentity_t *ent, float dst[3]) {
  // ^ vec3_t is not used to avoid including g_local.h
  if (!VectorCompare(ent->r.currentOrigin, vec3_origin)) {
    VectorCopy(ent->r.currentOrigin, dst);
  } else {
    dst[0] = (ent->r.absmax[0] + ent->r.absmin[0]) / 2;
    dst[1] = (ent->r.absmax[1] + ent->r.absmin[1]) / 2;
    dst[2] = (ent->r.absmax[2] + ent->r.absmin[2]) / 2;
  }
}

void Utilities::copyStanceFromClient(gentity_t *self, const gentity_t *target) {
  // safeguards
  if (!target || !target->client || !self || !self->client) {
    return;
  }

  if (target->client->ps.eFlags & EF_CROUCHING) {
    self->client->ps.eFlags &= ~EF_PRONE;
    self->client->ps.eFlags &= ~EF_PRONE_MOVING;
    self->client->ps.pm_flags |= PMF_DUCKED;
  } else if (target->client->ps.eFlags & EF_PRONE ||
             target->client->ps.eFlags & EF_PRONE_MOVING) {
    self->client->ps.eFlags |= EF_PRONE;
    SetClientViewAngle(self, target->client->ps.viewangles);
  } else {
    self->client->ps.eFlags &= ~EF_PRONE;
    self->client->ps.eFlags &= ~EF_PRONE_MOVING;
    self->client->ps.pm_flags &= ~PMF_DUCKED;
  }
}
