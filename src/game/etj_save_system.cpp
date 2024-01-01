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

#include "etj_save_system.h"
#include "utilities.hpp"
#include "etj_session.h"
#include <iostream>

namespace ETJump {
SaveSystem::Client::Client() {
  for (int i = 0; i < MAX_SAVED_POSITIONS; i++) {
    alliesSaves[i].isValid = false;
    alliesSaves[i].isLatest = false;
    axisSaves[i].isValid = false;
    axisSaves[i].isLatest = false;
  }

  alliesBackups.clear();
  axisBackups.clear();

  for (int i = 0; i < MAX_BACKUP_POSITIONS; i++) {
    alliesBackups.emplace_back();
    axisBackups.emplace_back();
  }

  alliesLastLoadPos.isValid = false;
  axisLastLoadPos.isValid = false;
}

SaveSystem::DisconnectedClient::DisconnectedClient() {
  for (int i = 0; i < MAX_SAVED_POSITIONS; i++) {
    alliesSaves[i].isValid = false;
    alliesSaves[i].isLatest = false;
    axisSaves[i].isValid = false;
    axisSaves[i].isLatest = false;
  }

  alliesLastLoadPos.isValid = false;
  axisLastLoadPos.isValid = false;
  progression = 0;
  saveLimit = 0;
}

// Saves current position
void SaveSystem::save(gentity_t *ent) {
  auto *client = ent->client;
  const int clientNum = ClientNum(ent);

  if (!client) {
    return;
  }

  if (!g_save.integer) {
    CPTo(ent, "^3Save ^7is not enabled.");
    return;
  }

  if (!g_cheats.integer &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::Active)) &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::NoSave))) {
    CPTo(ent, "^3Save ^7is disabled for this death run.");
    return;
  }

  if (!g_cheats.integer && level.saveLoadRestrictions &
                               static_cast<int>(SaveLoadRestrictions::Move)) {
    // comparing to zero vector
    if (!VectorCompare(client->ps.velocity, vec3_origin)) {
      CPTo(ent, "^3Save ^7is disabled while moving on this map.");
      return;
    }
  }

  // No saving while dead if it's disabled by map
  if (!g_cheats.integer && client->ps.pm_type == PM_DEAD &&
      level.saveLoadRestrictions &
          static_cast<int>(SaveLoadRestrictions::Dead)) {
    CPTo(ent, "^3Save ^7is disabled while dead on this map.");
    return;
  }

  const auto argv = GetArgs();
  int position = 0;

  if (argv->size() > 1) {
    ToInt((*argv)[1], position);

    if (position < 0 || position >= MAX_SAVED_POSITIONS) {
      CPTo(ent, "Invalid position.");
      return;
    }

    if (!g_cheats.integer && position > 0 && client->sess.timerunActive &&
        client->sess.runSpawnflags &
            static_cast<int>(ETJump::TimerunSpawnflags::NoBackups)) {
      CPTo(ent, "Save slots are disabled for this timerun.");
      return;
    }
  }

  if (!client->sess.saveAllowed) {
    CPTo(ent, "You are not allowed to save a position.");
    return;
  }

  if (client->sess.sessionTeam == TEAM_SPECTATOR) {
    CPTo(ent, "^7You can not ^3save^7 as a spectator.");
    return;
  }

  if (!g_cheats.integer && client->sess.timerunActive &&
      client->sess.runSpawnflags &
          static_cast<int>(ETJump::TimerunSpawnflags::NoSave)) {
    CPTo(ent, "^3Save ^7is disabled for this timerun.");
    return;
  }

  trace_t trace;
  trap_TraceCapsule(&trace, client->ps.origin, ent->r.mins, ent->r.maxs,
                    client->ps.origin, ent->s.number, CONTENTS_NOSAVE);

  if (!g_cheats.integer) {
    if (level.noSave) {
      if (trace.fraction == 1.0f) {
        CPTo(ent, "^7You can not ^3save ^7inside this area.");
        return;
      }
    } else {
      if (trace.fraction != 1.0f) {
        CPTo(ent, "^7You can not ^3save ^7inside this area.");
        return;
      }
    }

    if (level.limitedSaves > 0) {
      if (client->sess.saveLimit == 0) {
        CPTo(ent, "^7You've used all your saves.");
        return;
      }

      client->sess.saveLimit--;
    } else {
      if (client->pers.race.isRacing) {
        if (client->pers.race.saveLimit == 0) {
          CPTo(ent, "^7You've used all your saves.");
          return;
        }

        if (client->pers.race.saveLimit > 0) {
          client->pers.race.saveLimit--;
        }
      } else {
        fireteamData_t *ft;
        if (G_IsOnFireteam(clientNum, &ft)) {
          if (ft->saveLimit == FT_SAVELIMIT_NOT_SET) {
            client->sess.saveLimitFt = FT_SAVELIMIT_NOT_SET;
          } else {
            if (client->sess.saveLimitFt) {
              client->sess.saveLimitFt--;
            } else {
              CPTo(ent, "^7You've used all your fireteam saves.");
              return;
            }
          }
        }
      }
    }
  }

  const auto teamSaves = client->sess.sessionTeam == TEAM_ALLIES
                             ? _clients[clientNum].alliesSaves
                             : _clients[clientNum].axisSaves;
  SavePosition *pos = teamSaves + position;

  if (pos->isLatest) {
    saveBackupPosition(ent, pos);
  } else {
    const int latestSlot = getLatestSaveSlot(client);

    if (latestSlot != -1) {
      saveBackupPosition(ent, teamSaves + latestSlot);
    }
  }

  resetLatestSaveSlot(ent);
  storePosition(client, pos);
  sendClientCommands(ent, position);
}

// Loads position
void SaveSystem::load(gentity_t *ent) {
  auto *client = ent->client;

  if (!client) {
    return;
  }

  if (!g_save.integer) {
    CPTo(ent, "^3Load ^7is not enabled.");
    return;
  }

  if (!client->sess.saveAllowed) {
    CPTo(ent, "You are not allowed to load a position.");
    return;
  }

  if (!g_cheats.integer &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::Active)) &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::NoSave))) {
    CPTo(ent, "^3Load ^7is disabled for this death run.");
    return;
  }

  if (!g_cheats.integer && client->ps.pm_type == PM_DEAD &&
      level.saveLoadRestrictions &
          static_cast<int>(SaveLoadRestrictions::Dead)) {
    CPTo(ent, "^3Load ^7is disabled while dead on this map.");
    return;
  }

  const auto argv = GetArgs();
  int slot = 0;

  if (argv->size() > 1) {
    ToInt((*argv)[1], slot);

    if (slot < 0 || slot >= MAX_SAVED_POSITIONS) {
      CPTo(ent, "^7Invalid save slot.");
      return;
    }

    if (!g_cheats.integer && slot > 0 && client->sess.timerunActive &&
        client->sess.runSpawnflags &
            static_cast<int>(ETJump::TimerunSpawnflags::NoBackups)) {
      CPTo(ent, "Save slots are disabled for this timerun.");
      return;
    }
  }

  if (client->sess.sessionTeam == TEAM_SPECTATOR) {
    CPTo(ent, "^7You can not ^3load ^7as a spectator.");
    return;
  }

  const auto pos = getValidTeamSaveForSlot(ent, client->sess.sessionTeam, slot);

  if (pos) {
    saveLastLoadPos(ent); // store position for unload command
    restoreStanceFromSave(ent, pos);

    if (!g_cheats.integer && client->sess.timerunActive &&
        client->sess.runSpawnflags &
            static_cast<int>(ETJump::TimerunSpawnflags::NoSave)) {
      InterruptRun(ent);
    }

    // allow fast respawn + load if we got gibbed to skip death sequence
    if (ent->client->ps.stats[STAT_HEALTH] <= GIB_HEALTH) {
      ent->client->ps.pm_flags &= ~PMF_LIMBO;
      ClientSpawn(ent, qfalse);
    }

    teleportPlayer(ent, pos);
  } else {
    CPTo(ent, "^7Use ^3save ^7first.");
  }
}

// Saves position, does not check for anything. Used for target_save
void ETJump::SaveSystem::forceSave(gentity_t *location, gentity_t *ent) {
  SavePosition *pos;
  auto *client = ent->client;
  const int clientNum = ClientNum(ent);

  if (!client || !location) {
    return;
  }

  if (client->sess.sessionTeam == TEAM_ALLIES) {
    pos = &_clients[clientNum].alliesSaves[0];
  } else if (client->sess.sessionTeam == TEAM_AXIS) {
    pos = &_clients[clientNum].axisSaves[0];
  } else {
    return;
  }

  if (pos->isLatest) {
    saveBackupPosition(ent, pos);
  } else {
    const int latestSlot = getLatestSaveSlot(client);

    if (latestSlot != -1) {
      saveBackupPosition(ent, pos + latestSlot);
    }
  }

  resetLatestSaveSlot(ent);

  VectorCopy(location->s.origin, pos->origin);
  VectorCopy(location->s.angles, pos->vangles);
  pos->isValid = true;
  pos->isLatest = true;

  if (client->ps.eFlags & EF_PRONE) {
    pos->stance = Prone;
  } else {
    pos->stance = client->ps.eFlags & EF_CROUCHING ? Crouch : Stand;
  }

  trap_SendServerCommand(clientNum, "savePrint");
}

// Loads backup position
void SaveSystem::loadBackupPosition(gentity_t *ent) {
  auto *client = ent->client;
  const int clientNum = ClientNum(ent);

  if (!client) {
    return;
  }

  if (!g_save.integer) {
    CPTo(ent, "^3Load ^7is not enabled.");
    return;
  }

  if (!client->sess.saveAllowed) {
    CPTo(ent, "You are not allowed to load a position.");
    return;
  }

  if (!g_cheats.integer && client->sess.timerunActive &&
      client->sess.runSpawnflags &
          static_cast<int>(ETJump::TimerunSpawnflags::NoBackups)) {
    CPTo(ent, "Backup is disabled for this timerun.");
    return;
  }

  if (!g_cheats.integer &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::Active)) &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::NoSave))) {
    CPTo(ent, "^3Backup ^7is disabled for this death run.");
    return;
  }

  if (!g_cheats.integer && client->ps.pm_type == PM_DEAD &&
      level.saveLoadRestrictions &
          static_cast<int>(SaveLoadRestrictions::Dead)) {
    CPTo(ent, "^3Backup ^7is disabled while dead on this map.");
    return;
  }

  const auto argv = GetArgs();
  int slot = 0;

  if (argv->size() > 1) {
    ToInt(argv->at(1), slot);

    if (slot < 1 || slot > MAX_SAVED_POSITIONS) {
      CPTo(ent, "^7Invalid backup slot.");
      return;
    }

    if (slot > 0) {
      slot--;
    }
  }

  if (client->sess.sessionTeam == TEAM_SPECTATOR) {
    CPTo(ent, "^7You can not ^3load ^7as a spectator.");
    return;
  }

  SavePosition *pos;

  if (client->sess.sessionTeam == TEAM_ALLIES) {
    pos = &_clients[clientNum].alliesBackups[slot];
  } else {
    pos = &_clients[clientNum].axisBackups[slot];
  }

  if (pos->isValid) {
    restoreStanceFromSave(ent, pos);

    if (client->sess.timerunActive &&
        client->sess.runSpawnflags &
            static_cast<int>(ETJump::TimerunSpawnflags::NoSave)) {
      InterruptRun(ent);
    }

    teleportPlayer(ent, pos);
  } else {
    CPTo(ent, "^7Use ^3save ^7first.");
  }
}

// Undo last load command and teleport to last position client loaded from
// Position validation is done here
void SaveSystem::unload(gentity_t *ent) {
  const auto *client = ent->client;

  if (!client) {
    return;
  }

  if (!g_save.integer) {
    CPTo(ent, "^3Unload ^7is not enabled.");
    return;
  }

  if (!client->sess.saveAllowed) {
    CPTo(ent, "^7You are not allowed to ^3unload ^7a position.");
    return;
  }

  if (!g_cheats.integer &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::Active)) &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::NoSave))) {
    CPTo(ent, "^3unload ^7is disabled for this death run.");
    return;
  }

  if (!g_cheats.integer && client->ps.pm_type == PM_DEAD &&
      level.saveLoadRestrictions &
          static_cast<int>(SaveLoadRestrictions::Dead)) {
    CPTo(ent, "^3unload ^7is disabled while dead on this map.");
    return;
  }

  if (client->sess.sessionTeam == TEAM_SPECTATOR) {
    CPTo(ent, "^7You can not ^3unload ^7as a spectator.");
    return;
  }

  if (!g_cheats.integer && client->sess.timerunActive) {
    CPTo(ent, "^3unload ^7is not available during timeruns.");
    return;
  }

  const auto pos = getValidTeamUnloadPos(ent, client->sess.sessionTeam);

  if (pos) {
    if (!g_cheats.integer) {
      // check for nosave areas only if we have valid pos
      trace_t trace;
      trap_TraceCapsule(&trace, pos->origin, ent->r.mins, ent->r.maxs,
                        pos->origin, ent->s.number, CONTENTS_NOSAVE);

      if (level.noSave) {
        if (trace.fraction == 1.0f) {
          CPTo(ent, "^7You can not ^3unload ^7to this area.");
          return;
        }
      } else {
        if (trace.fraction != 1.0f) {
          CPTo(ent, "^7You can not ^3unload ^7to this area.");
          return;
        }
      }
    }

    restoreStanceFromSave(ent, pos);
    teleportPlayer(ent, pos);
  } else {
    CPTo(ent, "^7Use ^3load ^7first.");
  }
}

// Saves position client loaded from. Executed on every successful load command,
// position validation is done later. This is to prevent unexpected behavior
// where the last load position is not a valid position, and client is
// teleported to a position that was valid before that.
void SaveSystem::saveLastLoadPos(gentity_t *ent) {
  SavePosition *pos;
  auto *client = ent->client;
  const int clientNum = ClientNum(ent);

  if (client->sess.sessionTeam == TEAM_ALLIES) {
    pos = &_clients[clientNum].alliesLastLoadPos;
  } else if (client->sess.sessionTeam == TEAM_AXIS) {
    pos = &_clients[clientNum].axisLastLoadPos;
  } else {
    return;
  }

  storePosition(client, pos);
}

void SaveSystem::reset() {
  for (int i = 0; i < level.numConnectedClients; i++) {
    resetSavedPositions(g_entities + level.sortedClients[i]);
  }

  _savedPositions.clear();
}

// Used to reset positions on map change/restart
void SaveSystem::resetSavedPositions(gentity_t *ent) {
  const int clientNum = ClientNum(ent);
  for (int i = 0; i < MAX_SAVED_POSITIONS; i++) {
    _clients[clientNum].alliesSaves[i].isValid = false;
    _clients[clientNum].alliesSaves[i].isLatest = false;
    _clients[clientNum].axisSaves[i].isValid = false;
    _clients[clientNum].axisSaves[i].isLatest = false;
  }

  for (int i = 0; i < MAX_BACKUP_POSITIONS; i++) {
    _clients[clientNum].alliesBackups[i].isValid = false;
    _clients[clientNum].axisBackups[i].isValid = false;
  }

  _clients[clientNum].quickDeployPositions[TEAM_ALLIES].isValid = false;
  _clients[clientNum].quickDeployPositions[TEAM_AXIS].isValid = false;

  _clients[clientNum].alliesLastLoadPos.isValid = false;
  _clients[clientNum].axisLastLoadPos.isValid = false;
}

// Called on client disconnect. Saves saves for future sessions
void SaveSystem::savePositionsToDatabase(gentity_t *ent) {
  if (!ent->client) {
    return;
  }

  const std::string guid = _session->Guid(ent);
  const int clientNum = ClientNum(ent);
  DisconnectedClient client;

  for (int i = 0; i < MAX_SAVED_POSITIONS; i++) {
    // Allied
    VectorCopy(_clients[clientNum].alliesSaves[i].origin,
               client.alliesSaves[i].origin);
    VectorCopy(_clients[clientNum].alliesSaves[i].vangles,
               client.alliesSaves[i].vangles);

    client.alliesSaves[i].isValid = _clients[clientNum].alliesSaves[i].isValid;
    client.alliesSaves[i].isLatest =
        _clients[clientNum].alliesSaves[i].isLatest;
    // Axis
    VectorCopy(_clients[clientNum].axisSaves[i].origin,
               client.axisSaves[i].origin);
    VectorCopy(_clients[clientNum].axisSaves[i].vangles,
               client.axisSaves[i].vangles);

    client.axisSaves[i].isValid = _clients[clientNum].axisSaves[i].isValid;
    client.axisSaves[i].isLatest = _clients[clientNum].axisSaves[i].isLatest;
  }

  client.progression = ent->client->sess.clientMapProgression;
  client.saveLimit = ent->client->sess.saveLimit;
  ent->client->sess.loadPreviousSavedPositions = qfalse;

  const auto it = _savedPositions.find(guid);

  if (it != _savedPositions.end()) {
    it->second = client;
  } else {
    _savedPositions.insert(std::make_pair(guid, client));
  }
}

// Called on client connect. Loads saves from previous session
void SaveSystem::loadPositionsFromDatabase(gentity_t *ent) {
  if (!ent->client) {
    return;
  }

  if (!ent->client->sess.loadPreviousSavedPositions) {
    return;
  }

  const std::string guid = _session->Guid(ent);
  const int clientNum = ClientNum(ent);
  const auto it = _savedPositions.find(guid);

  if (it != _savedPositions.end()) {
    uint8_t validPositionsCount = 0;

    for (int i = 0; i < MAX_SAVED_POSITIONS; i++) {
      // Allied
      VectorCopy(it->second.alliesSaves[i].origin,
                 _clients[clientNum].alliesSaves[i].origin);
      VectorCopy(it->second.alliesSaves[i].vangles,
                 _clients[clientNum].alliesSaves[i].vangles);

      _clients[clientNum].alliesSaves[i].isValid =
          it->second.alliesSaves[i].isValid;
      _clients[clientNum].alliesSaves[i].isLatest =
          it->second.alliesSaves[i].isLatest;

      if (it->second.alliesSaves[i].isValid) {
        ++validPositionsCount;
      }

      // Axis
      VectorCopy(it->second.axisSaves[i].origin,
                 _clients[clientNum].axisSaves[i].origin);
      VectorCopy(it->second.axisSaves[i].vangles,
                 _clients[clientNum].axisSaves[i].vangles);

      _clients[clientNum].axisSaves[i].isValid =
          it->second.axisSaves[i].isValid;
      _clients[clientNum].axisSaves[i].isLatest =
          it->second.axisSaves[i].isLatest;

      if (it->second.axisSaves[i].isValid) {
        ++validPositionsCount;
      }
    }

    ent->client->sess.loadPreviousSavedPositions = qfalse;
    ent->client->sess.clientMapProgression = it->second.progression;
    ent->client->sess.saveLimit = it->second.saveLimit;

    if (validPositionsCount) {
      ChatPrintTo(ent,
                  "^gETJump: ^7loaded saved positions from previous session.");
    }
  }
}

void SaveSystem::storeTeamQuickDeployPosition(gentity_t *ent, team_t team) {
  const auto lastValidSave = getValidTeamSaveForSlot(ent, team, 0);

  if (lastValidSave) {
    const auto client = &_clients[ClientNum(ent)];
    auto autoSave = &(client->quickDeployPositions[team]);
    *autoSave = *lastValidSave;
  }
}

void SaveSystem::loadOnceTeamQuickDeployPosition(gentity_t *ent, team_t team) {
  const auto validSave = getValidTeamQuickDeploySave(ent, team);

  if (validSave) {
    restoreStanceFromSave(ent, validSave);
    teleportPlayer(ent, validSave);
    validSave->isValid = false;
  }
}

SaveSystem::SavePosition *
SaveSystem::getValidTeamSaveForSlot(gentity_t *ent, team_t team, int slot) {
  if (!ent || !ent->client) {
    return nullptr;
  }

  if (team != TEAM_ALLIES && team != TEAM_AXIS) {
    return nullptr;
  }

  const auto client = &_clients[ClientNum(ent)];
  SavePosition *pos;

  if (team == TEAM_ALLIES) {
    pos = &client->alliesSaves[slot];
  } else {
    pos = &client->axisSaves[slot];
  }

  if (!pos->isValid) {
    return nullptr;
  }

  return pos;
}

SaveSystem::SavePosition *SaveSystem::getValidTeamUnloadPos(gentity_t *ent,
                                                            team_t team) {
  if (!ent || !ent->client) {
    return nullptr;
  }

  if (team != TEAM_ALLIES && team != TEAM_AXIS) {
    return nullptr;
  }

  const auto client = &_clients[ClientNum(ent)];
  SavePosition *pos;

  if (team == TEAM_ALLIES) {
    pos = &client->alliesLastLoadPos;
  } else {
    pos = &client->axisLastLoadPos;
  }

  if (!pos->isValid) {
    return nullptr;
  }

  return pos;
}

SaveSystem::SavePosition *
SaveSystem::getValidTeamQuickDeploySave(gentity_t *ent, team_t team) {
  if (!ent || !ent->client) {
    return nullptr;
  }

  if (team != TEAM_ALLIES && team != TEAM_AXIS) {
    return nullptr;
  }

  const auto client = &_clients[ClientNum(ent)];
  const auto pos = &client->quickDeployPositions[team];

  if (!pos->isValid) {
    return nullptr;
  }

  return pos;
}

void SaveSystem::restoreStanceFromSave(gentity_t *ent, SavePosition *pos) {
  if (!ent || !ent->client) {
    return;
  }

  auto client = ent->client;

  if (pos->stance == Crouch) {
    client->ps.eFlags &= ~EF_PRONE;
    client->ps.eFlags &= ~EF_PRONE_MOVING;
    client->ps.pm_flags |= PMF_DUCKED;
  } else if (pos->stance == Prone) {
    client->ps.eFlags |= EF_PRONE;
    SetClientViewAngle(ent, pos->vangles);
  } else {
    client->ps.eFlags &= ~EF_PRONE;
    client->ps.eFlags &= ~EF_PRONE_MOVING;
    client->ps.pm_flags &= ~PMF_DUCKED;
  }
}

// Saves backup position
void SaveSystem::saveBackupPosition(gentity_t *ent, SavePosition *pos) {
  if (!ent->client) {
    return;
  }

  if (ent->client->sess.timerunActive &&
      ent->client->sess.runSpawnflags &
          static_cast<int>(TimerunSpawnflags::NoBackups)) {
    return;
  }

  const int clientNum = ClientNum(ent);
  SavePosition backup;

  VectorCopy(pos->origin, backup.origin);
  VectorCopy(pos->vangles, backup.vangles);

  backup.isValid = pos->isValid;
  backup.isLatest = pos->isLatest;
  backup.stance = pos->stance;

  // Can never be spectator as this would not be called
  if (ent->client->sess.sessionTeam == TEAM_ALLIES) {
    _clients[clientNum].alliesBackups.pop_back();
    _clients[clientNum].alliesBackups.push_front(backup);
  } else {
    _clients[clientNum].axisBackups.pop_back();
    _clients[clientNum].axisBackups.push_front(backup);
  }
}

void ETJump::SaveSystem::storePosition(gclient_s *client, SavePosition *pos) {
  VectorCopy(client->ps.origin, pos->origin);
  VectorCopy(client->ps.viewangles, pos->vangles);
  pos->isValid = true;
  pos->isLatest = true;

  if (client->ps.eFlags & EF_PRONE) {
    pos->stance = Prone;
  } else {
    pos->stance = client->ps.eFlags & EF_CROUCHING ? Crouch : Stand;
  }
}

int SaveSystem::getLatestSaveSlot(gclient_s *client) {
  const int clientNum = ClientNum(client);
  const auto teamSaves = client->sess.sessionTeam == TEAM_ALLIES
                             ? _clients[clientNum].alliesSaves
                             : _clients[clientNum].axisSaves;
  int slot = -1;

  for (int i = 0; i < MAX_SAVED_POSITIONS; i++) {
    if (teamSaves[i].isLatest) {
      slot = i;
      break;
    }
  }

  return slot;
}

void SaveSystem::resetLatestSaveSlot(gentity_t *ent) {
  const int clientNum = ClientNum(ent);
  const auto teamSaves = ent->client->sess.sessionTeam == TEAM_ALLIES
                             ? _clients[clientNum].alliesSaves
                             : _clients[clientNum].axisSaves;

  for (int i = 0; i < MAX_SAVED_POSITIONS; i++) {
    teamSaves[i].isLatest = false;
  }
}

void SaveSystem::sendClientCommands(gentity_t *ent, int position) {
  const int self = ClientNum(ent);
  int target;
  std::string saveMsg = va("savePrint %d\n", position);

  if (!g_cheats.integer && level.limitedSaves > 0) {
    saveMsg += va(" %d", ent->client->sess.saveLimit);
  }

  // Send the commands to ourselves and any following clients
  for (int i = 0; i < level.numConnectedClients; i++) {
    target = level.sortedClients[i];
    if (target == self ||
        (level.clients[target].sess.sessionTeam == TEAM_SPECTATOR &&
         level.clients[target].sess.spectatorState == SPECTATOR_FOLLOW &&
         level.clients[target].sess.spectatorClient == self)) {
      trap_SendServerCommand(target, "resetStrafeQuality\n");
      trap_SendServerCommand(target, "resetJumpSpeeds\n");
      trap_SendServerCommand(target, saveMsg.c_str());
    }
  }
}

void SaveSystem::teleportPlayer(gentity_t *ent, SavePosition *pos) {
  auto *client = ent->client;
  client->ps.eFlags ^= EF_TELEPORT_BIT;

  G_AddEvent(ent, EV_LOAD_TELEPORT, 0);
  G_SetOrigin(ent, pos->origin);
  VectorClear(client->ps.velocity);

  if (client->pers.loadViewAngles) {
    SetClientViewAngle(ent, pos->vangles);
  }

  client->ps.pm_time = 1; // Crashland + instant load bug fix.
}
} // namespace ETJump
