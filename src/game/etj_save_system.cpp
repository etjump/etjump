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

#include "etj_save_system.h"
#include "utilities.hpp"
#include "etj_printer.h"
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
  const std::string guid = _session->Guid(ent);

  if (!client) {
    return;
  }

  if (!g_save.integer) {
    Printer::center(ent, "^3Save ^7is not enabled.");
    return;
  }

  if (!g_cheats.integer &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::Active)) &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::NoSave))) {
    Printer::center(ent, "^3Save ^7is disabled for this death run.");
    return;
  }

  if (!g_cheats.integer && level.saveLoadRestrictions &
                               static_cast<int>(SaveLoadRestrictions::Move)) {
    // comparing to zero vector
    if (!VectorCompare(client->ps.velocity, vec3_origin)) {
      Printer::center(ent, "^3Save ^7is disabled while moving on this map.");
      return;
    }
  }

  // No saving while dead if it's disabled by map
  if (!g_cheats.integer && client->ps.pm_type == PM_DEAD &&
      level.saveLoadRestrictions &
          static_cast<int>(SaveLoadRestrictions::Dead)) {
    Printer::center(ent, "^3Save ^7is disabled while dead on this map.");
    return;
  }

  const auto argv = GetArgs();
  int position = 0;

  if (argv->size() > 1) {
    ToInt((*argv)[1], position);

    if (position < 0 || position >= MAX_SAVED_POSITIONS) {
      Printer::center(ent, "Invalid position.");
      return;
    }

    if (!g_cheats.integer && position > 0 && client->sess.timerunActive &&
        client->sess.runSpawnflags &
            static_cast<int>(TimerunSpawnflags::NoBackups)) {
      Printer::center(ent, "Save slots are disabled for this timerun.");
      return;
    }
  }

  if (!client->sess.saveAllowed) {
    Printer::center(ent, "You are not allowed to save a position.");
    return;
  }

  if (client->sess.sessionTeam == TEAM_SPECTATOR) {
    Printer::center(ent, "^7You can not ^3save^7 as a spectator.");
    return;
  }

  if (!g_cheats.integer && client->sess.timerunActive &&
      client->sess.runSpawnflags &
          static_cast<int>(TimerunSpawnflags::NoSave)) {
    Printer::center(ent, "^3Save ^7is disabled for this timerun.");
    return;
  }

  trace_t trace;
  trap_TraceCapsule(&trace, client->ps.origin, ent->r.mins, ent->r.maxs,
                    client->ps.origin, ent->s.number, CONTENTS_NOSAVE);

  if (!g_cheats.integer) {
    if (level.noSave) {
      if (trace.fraction == 1.0f) {
        Printer::center(ent, "^7You can not ^3save ^7inside this area.");
        return;
      }
    } else {
      if (trace.fraction != 1.0f) {
        Printer::center(ent, "^7You can not ^3save ^7inside this area.");
        return;
      }
    }

    if (level.limitedSaves > 0) {
      if (client->sess.saveLimit == 0) {
        Printer::center(ent, "^7You've used all your saves.");
        return;
      }

      client->sess.saveLimit--;
    } else {
      if (client->pers.race.isRacing) {
        if (client->pers.race.saveLimit == 0) {
          Printer::center(ent, "^7You've used all your saves.");
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
              Printer::center(ent, "^7You've used all your fireteam saves.");
              return;
            }
          }
        }
      }
    }
  }

  const auto teamSaves = client->sess.sessionTeam == TEAM_ALLIES
                             ? _clients[guid].alliesSaves
                             : _clients[guid].axisSaves;
  SavePosition *pos = teamSaves + position;

  if (pos->isLatest) {
    saveBackupPosition(ent, pos);
  } else {
    const int latestSlot = getLatestSaveSlot(ent);

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
  const auto *client = ent->client;

  if (!client) {
    return;
  }

  if (!g_save.integer) {
    Printer::center(ent, "^3Load ^7is not enabled.");
    return;
  }

  if (!client->sess.saveAllowed) {
    Printer::center(ent, "You are not allowed to load a position.");
    return;
  }

  if (!g_cheats.integer &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::Active)) &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::NoSave))) {
    Printer::center(ent, "^3Load ^7is disabled for this death run.");
    return;
  }

  if (!g_cheats.integer && client->ps.pm_type == PM_DEAD &&
      level.saveLoadRestrictions &
          static_cast<int>(SaveLoadRestrictions::Dead)) {
    Printer::center(ent, "^3Load ^7is disabled while dead on this map.");
    return;
  }

  const auto argv = GetArgs();
  int slot = 0;

  if (argv->size() > 1) {
    ToInt((*argv)[1], slot);

    if (slot < 0 || slot >= MAX_SAVED_POSITIONS) {
      Printer::center(ent, "^7Invalid save slot.");
      return;
    }
  }

  if (client->sess.sessionTeam == TEAM_SPECTATOR) {
    Printer::center(ent, "^7You can not ^3load ^7as a spectator.");
    return;
  }

  if (!clientHasSaveData(ent)) {
    Printer::center(ent, "^7Use ^3save ^7first.");
    return;
  }

  const auto pos = getValidTeamSaveForSlot(ent, client->sess.sessionTeam, slot);

  if (pos) {
    if (pos->stance == PlayerStance::Prone &&
        client->ps.weapon == WP_MORTAR_SET) {
      Printer::center(
          ent,
          "You cannot ^3load ^7to this position while using a mortar set.");
      return;
    }

    if (!g_cheats.integer && slot > 0 && client->sess.timerunActive &&
        client->sess.runSpawnflags &
            static_cast<int>(TimerunSpawnflags::NoBackups)) {
      if (!pos->isTimerunSave) {
        InterruptRun(ent);
      } else {
        Printer::center(ent, "Save slots are disabled for this timerun.");
        return;
      }
    }

    saveLastLoadPos(ent); // store position for unload command
    restoreStanceFromSave(ent, pos);

    if (!g_cheats.integer && client->sess.timerunActive &&
        (client->sess.runSpawnflags &
             static_cast<int>(TimerunSpawnflags::NoSave) ||
         !pos->isTimerunSave)) {
      InterruptRun(ent);
    }

    // allow fast respawn + load if we got gibbed to skip death sequence
    if (ent->client->ps.stats[STAT_HEALTH] <= GIB_HEALTH) {
      respawn(ent);
    }

    teleportPlayer(ent, pos);
  } else {
    Printer::center(ent, "^7Use ^3save ^7first.");
  }
}

// Saves position, does not check for anything. Used for target_save
void SaveSystem::forceSave(const gentity_t *location, gentity_t *ent) {
  SavePosition *pos;
  const auto *client = ent->client;
  const int clientNum = ClientNum(ent);
  const std::string guid = _session->Guid(ent);

  if (!client || !location) {
    return;
  }

  if (client->sess.sessionTeam == TEAM_ALLIES) {
    pos = &_clients[guid].alliesSaves[0];
  } else if (client->sess.sessionTeam == TEAM_AXIS) {
    pos = &_clients[guid].axisSaves[0];
  } else {
    return;
  }

  if (pos->isLatest) {
    saveBackupPosition(ent, pos);
  } else {
    const int latestSlot = getLatestSaveSlot(ent);

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
    pos->stance = PlayerStance::Prone;
  } else if (client->ps.eFlags & (EF_CROUCHING | EF_DEAD)) {
    pos->stance = PlayerStance::Crouch;
  } else {
    pos->stance = PlayerStance::Stand;
  }

  pos->isTimerunSave = ent->client->sess.timerunActive;

  trap_SendServerCommand(clientNum, "savePrint");
}

// Loads backup position
void SaveSystem::loadBackupPosition(gentity_t *ent) {
  const auto *client = ent->client;
  const std::string guid = _session->Guid(ent);

  if (!client) {
    return;
  }

  if (!g_save.integer) {
    Printer::center(ent, "^3Load ^7is not enabled.");
    return;
  }

  if (!client->sess.saveAllowed) {
    Printer::center(ent, "You are not allowed to load a position.");
    return;
  }

  if (!g_cheats.integer && client->sess.timerunActive &&
      client->sess.runSpawnflags &
          static_cast<int>(TimerunSpawnflags::NoBackups)) {
    Printer::center(ent, "Backup is disabled for this timerun.");
    return;
  }

  if (!g_cheats.integer &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::Active)) &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::NoSave))) {
    Printer::center(ent, "^3Backup ^7is disabled for this death run.");
    return;
  }

  if (!g_cheats.integer && client->ps.pm_type == PM_DEAD &&
      level.saveLoadRestrictions &
          static_cast<int>(SaveLoadRestrictions::Dead)) {
    Printer::center(ent, "^3Backup ^7is disabled while dead on this map.");
    return;
  }

  if (!clientHasSaveData(ent)) {
    Printer::center(ent, "^7Use ^3save ^7first.");
    return;
  }

  const auto argv = GetArgs();
  int slot = 0;

  if (argv->size() > 1) {
    ToInt(argv->at(1), slot);

    if (slot < 1 || slot > MAX_SAVED_POSITIONS) {
      Printer::center(ent, "^7Invalid backup slot.");
      return;
    }

    if (slot > 0) {
      slot--;
    }
  }

  if (client->sess.sessionTeam == TEAM_SPECTATOR) {
    Printer::center(ent, "^7You can not ^3load ^7as a spectator.");
    return;
  }

  SavePosition *pos;

  if (client->sess.sessionTeam == TEAM_ALLIES) {
    pos = &_clients[guid].alliesBackups[slot];
  } else {
    pos = &_clients[guid].axisBackups[slot];
  }

  if (pos->stance == PlayerStance::Prone &&
      client->ps.weapon == WP_MORTAR_SET) {
    Printer::center(
        ent, "You cannot ^3load ^7to this position while using a mortar set.");
    return;
  }

  if (pos->isValid) {
    restoreStanceFromSave(ent, pos);

    if (client->sess.timerunActive &&
        (client->sess.runSpawnflags &
             static_cast<int>(TimerunSpawnflags::NoSave) ||
         !pos->isTimerunSave)) {
      InterruptRun(ent);
    }

    teleportPlayer(ent, pos);
  } else {
    Printer::center(ent, "^7Use ^3save ^7first.");
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
    Printer::center(ent, "^3Unload ^7is not enabled.");
    return;
  }

  if (!client->sess.saveAllowed) {
    Printer::center(ent, "^7You are not allowed to ^3unload ^7a position.");
    return;
  }

  if (!g_cheats.integer &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::Active)) &&
      (client->sess.deathrunFlags & static_cast<int>(DeathrunFlags::NoSave))) {
    Printer::center(ent, "^3unload ^7is disabled for this death run.");
    return;
  }

  if (!g_cheats.integer && client->ps.pm_type == PM_DEAD &&
      level.saveLoadRestrictions &
          static_cast<int>(SaveLoadRestrictions::Dead)) {
    Printer::center(ent, "^3unload ^7is disabled while dead on this map.");
    return;
  }

  if (client->sess.sessionTeam == TEAM_SPECTATOR) {
    Printer::center(ent, "^7You can not ^3unload ^7as a spectator.");
    return;
  }

  if (!g_cheats.integer && client->sess.timerunActive) {
    Printer::center(ent, "^3unload ^7is not available during timeruns.");
    return;
  }

  const auto pos = getValidTeamUnloadPos(ent, client->sess.sessionTeam);

  if (pos) {
    if (pos->stance == PlayerStance::Prone &&
        client->ps.weapon == WP_MORTAR_SET) {
      Printer::center(
          ent,
          "You cannot ^3unload ^7to this position while using a mortar set.");
      return;
    }

    if (!g_cheats.integer) {
      // check for nosave areas only if we have valid pos
      trace_t trace;
      trap_TraceCapsule(&trace, pos->origin, ent->r.mins, ent->r.maxs,
                        pos->origin, ent->s.number, CONTENTS_NOSAVE);

      if (level.noSave) {
        if (trace.fraction == 1.0f) {
          Printer::center(ent, "^7You can not ^3unload ^7to this area.");
          return;
        }
      } else {
        if (trace.fraction != 1.0f) {
          Printer::center(ent, "^7You can not ^3unload ^7to this area.");
          return;
        }
      }
    }

    restoreStanceFromSave(ent, pos);
    teleportPlayer(ent, pos);
  } else {
    Printer::center(ent, "^7Use ^3load ^7first.");
  }
}

// Saves position client loaded from. Executed on every successful load command,
// position validation is done later. This is to prevent unexpected behavior
// where the last load position is not a valid position, and client is
// teleported to a position that was valid before that.
void SaveSystem::saveLastLoadPos(gentity_t *ent) {
  SavePosition *pos;
  const auto *client = ent->client;
  const std::string guid = _session->Guid(ent);

  if (client->sess.sessionTeam == TEAM_ALLIES) {
    pos = &_clients[guid].alliesLastLoadPos;
  } else if (client->sess.sessionTeam == TEAM_AXIS) {
    pos = &_clients[guid].axisLastLoadPos;
  } else {
    return;
  }

  storePosition(client, pos);
}

void SaveSystem::resetSavedPositions(gentity_t *ent) {
  _clients.erase(_session->Guid(ent));
}

void SaveSystem::clearTimerunSaves(gentity_t *ent) {
  const std::string guid = _session->Guid(ent);

  for (int i = 0; i < MAX_SAVED_POSITIONS; i++) {
    if (_clients[guid].alliesSaves[i].isTimerunSave) {
      _clients[guid].alliesSaves[i].isValid = false;

      if (_clients[guid].alliesSaves[i].isLatest) {
        _clients[guid].alliesSaves[i].isLatest = false;
      }
    }

    if (_clients[guid].axisSaves[i].isTimerunSave) {
      _clients[guid].axisSaves[i].isValid = false;

      if (_clients[guid].axisSaves[i].isLatest) {
        _clients[guid].axisSaves[i].isLatest = false;
      }
    }
  }

  bool backupsHaveTimerunSaves = false;

  // check if we actually need to delete any old backups
  for (int i = 0; i < MAX_BACKUP_POSITIONS; i++) {
    if (_clients[guid].alliesBackups[i].isTimerunSave ||
        _clients[guid].axisBackups[i].isTimerunSave) {
      backupsHaveTimerunSaves = true;
    }
  }

  if (!backupsHaveTimerunSaves) {
    return;
  }

  std::deque<SavePosition> newAlliesBackups;
  std::deque<SavePosition> newAxisBackups;

  // clear the default constructed first slot, so we have a clean deque
  newAlliesBackups.clear();
  newAxisBackups.clear();

  // store any backup positions that were not from a timerun
  for (int i = 0; i < MAX_BACKUP_POSITIONS; i++) {
    if (!_clients[guid].alliesBackups[i].isTimerunSave) {
      newAlliesBackups.emplace_back(_clients[guid].alliesBackups[i]);
    }

    if (!_clients[guid].axisBackups[i].isTimerunSave) {
      newAxisBackups.emplace_back(_clients[guid].axisBackups[i]);
    }
  }

  // fill the remaining slots with default positions
  // storing backups always pushes front + pops back,
  // so this way we maintain the correct amount of slots

  while (newAxisBackups.size() < MAX_BACKUP_POSITIONS) {
    newAxisBackups.emplace_back();
  }

  while (newAlliesBackups.size() < MAX_BACKUP_POSITIONS) {
    newAlliesBackups.emplace_back();
  }

  _clients[guid].alliesBackups = std::move(newAlliesBackups);
  _clients[guid].axisBackups = std::move(newAxisBackups);
}

// Called on client disconnect. Stores saves for future sessions
void SaveSystem::savePositionsToDatabase(gentity_t *ent) {
  if (!ent->client) {
    return;
  }

  const std::string guid = _session->Guid(ent);
  DisconnectedClient client{};

  for (int i = 0; i < MAX_SAVED_POSITIONS; i++) {
    // Allied
    VectorCopy(_clients[guid].alliesSaves[i].origin,
               client.alliesSaves[i].origin);
    VectorCopy(_clients[guid].alliesSaves[i].vangles,
               client.alliesSaves[i].vangles);

    client.alliesSaves[i].isValid = _clients[guid].alliesSaves[i].isValid;
    client.alliesSaves[i].isLatest = _clients[guid].alliesSaves[i].isLatest;
    // Axis
    VectorCopy(_clients[guid].axisSaves[i].origin, client.axisSaves[i].origin);
    VectorCopy(_clients[guid].axisSaves[i].vangles,
               client.axisSaves[i].vangles);

    client.axisSaves[i].isValid = _clients[guid].axisSaves[i].isValid;
    client.axisSaves[i].isLatest = _clients[guid].axisSaves[i].isLatest;
  }

  client.progression = ent->client->sess.clientMapProgression;
  client.saveLimit = ent->client->sess.saveLimit;

  _savedPositions[guid] = client;
}

// Called when client has successfully authenticated.
// Loads saves from previous session
void SaveSystem::loadPositionsFromDatabase(gentity_t *ent) {
  if (!ent->client) {
    return;
  }

  const std::string guid = _session->Guid(ent);
  const auto it = _savedPositions.find(guid);

  if (it == _savedPositions.cend()) {
    return;
  }

  uint8_t validPositionsCount = 0;

  for (int i = 0; i < MAX_SAVED_POSITIONS; i++) {
    // Allied
    VectorCopy(it->second.alliesSaves[i].origin,
               _clients[guid].alliesSaves[i].origin);
    VectorCopy(it->second.alliesSaves[i].vangles,
               _clients[guid].alliesSaves[i].vangles);

    _clients[guid].alliesSaves[i].isValid = it->second.alliesSaves[i].isValid;
    _clients[guid].alliesSaves[i].isLatest = it->second.alliesSaves[i].isLatest;

    if (it->second.alliesSaves[i].isValid) {
      ++validPositionsCount;
    }

    // Axis
    VectorCopy(it->second.axisSaves[i].origin,
               _clients[guid].axisSaves[i].origin);
    VectorCopy(it->second.axisSaves[i].vangles,
               _clients[guid].axisSaves[i].vangles);

    _clients[guid].axisSaves[i].isValid = it->second.axisSaves[i].isValid;
    _clients[guid].axisSaves[i].isLatest = it->second.axisSaves[i].isLatest;

    if (it->second.axisSaves[i].isValid) {
      ++validPositionsCount;
    }
  }

  ent->client->sess.clientMapProgression = it->second.progression;
  ent->client->sess.saveLimit = it->second.saveLimit;

  if (validPositionsCount) {
    Printer::chat(ent,
                  "^gETJump: ^7loaded saved positions from previous session.");
  }
}

void SaveSystem::storeTeamQuickDeployPosition(gentity_t *ent, team_t team) {
  const auto lastValidSave = getValidTeamSaveForSlot(ent, team, 0);

  if (lastValidSave) {
    const auto client = &_clients[_session->Guid(ent)];
    const auto autoSave = &client->quickDeployPositions[team];
    *autoSave = *lastValidSave;
  }
}

void SaveSystem::loadOnceTeamQuickDeployPosition(gentity_t *ent,
                                                 const team_t team) {
  const auto validSave = getValidTeamQuickDeploySave(ent, team);

  if (validSave) {
    if (!g_cheats.integer && ent->client->sess.timerunActive &&
        (ent->client->sess.runSpawnflags &
             static_cast<int>(TimerunSpawnflags::NoSave) ||
         !validSave->isTimerunSave)) {
      InterruptRun(ent);
    }

    restoreStanceFromSave(ent, validSave);
    teleportPlayer(ent, validSave);
    validSave->isValid = false;
  }
}

SaveSystem::SavePosition *SaveSystem::getValidTeamSaveForSlot(gentity_t *ent,
                                                              const team_t team,
                                                              const int slot) {
  if (!ent || !ent->client) {
    return nullptr;
  }

  if (team != TEAM_ALLIES && team != TEAM_AXIS) {
    return nullptr;
  }

  const auto client = &_clients[_session->Guid(ent)];
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
                                                            const team_t team) {
  if (!ent || !ent->client) {
    return nullptr;
  }

  if (team != TEAM_ALLIES && team != TEAM_AXIS) {
    return nullptr;
  }

  const auto client = &_clients[_session->Guid(ent)];
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
SaveSystem::getValidTeamQuickDeploySave(gentity_t *ent, const team_t team) {
  if (!ent || !ent->client) {
    return nullptr;
  }

  if (team != TEAM_ALLIES && team != TEAM_AXIS) {
    return nullptr;
  }

  const std::string guid = _session->Guid(ent);

  // we might not have any save data stored yet (e.g. on first team join)
  if (!clientHasSaveData(ent)) {
    return nullptr;
  }

  const auto client = &_clients[guid];
  const auto pos = &client->quickDeployPositions[team];

  if (!pos->isValid) {
    return nullptr;
  }

  return pos;
}

void SaveSystem::restoreStanceFromSave(gentity_t *ent,
                                       const SavePosition *pos) {
  if (!ent || !ent->client) {
    return;
  }

  const auto client = ent->client;

  if (pos->stance == PlayerStance::Crouch) {
    client->ps.eFlags &= ~EF_PRONE;
    client->ps.eFlags &= ~EF_PRONE_MOVING;
    client->ps.pm_flags |= PMF_DUCKED;
  } else if (pos->stance == PlayerStance::Prone) {
    client->ps.eFlags |= EF_PRONE;
    SetClientViewAngle(ent, pos->vangles);
  } else {
    client->ps.eFlags &= ~EF_PRONE;
    client->ps.eFlags &= ~EF_PRONE_MOVING;
    client->ps.pm_flags &= ~PMF_DUCKED;
  }
}

// Saves backup position
void SaveSystem::saveBackupPosition(gentity_t *ent, const SavePosition *pos) {
  if (!ent->client) {
    return;
  }

  if (ent->client->sess.timerunActive &&
      ent->client->sess.runSpawnflags &
          static_cast<int>(TimerunSpawnflags::NoBackups)) {
    return;
  }

  const std::string guid = _session->Guid(ent);
  SavePosition backup;

  VectorCopy(pos->origin, backup.origin);
  VectorCopy(pos->vangles, backup.vangles);

  backup.isValid = pos->isValid;
  backup.isLatest = pos->isLatest;
  backup.stance = pos->stance;
  backup.isTimerunSave = pos->isTimerunSave;

  // Can never be spectator as this would not be called
  if (ent->client->sess.sessionTeam == TEAM_ALLIES) {
    _clients[guid].alliesBackups.pop_back();
    _clients[guid].alliesBackups.push_front(backup);
  } else {
    _clients[guid].axisBackups.pop_back();
    _clients[guid].axisBackups.push_front(backup);
  }
}

void SaveSystem::storePosition(const gclient_s *client, SavePosition *pos) {
  VectorCopy(client->ps.origin, pos->origin);
  VectorCopy(client->ps.viewangles, pos->vangles);
  pos->isValid = true;
  pos->isLatest = true;

  if (client->ps.eFlags & EF_PRONE) {
    pos->stance = PlayerStance::Prone;
  } else if (client->ps.eFlags & (EF_CROUCHING | EF_DEAD)) {
    pos->stance = PlayerStance::Crouch;
  } else {
    pos->stance = PlayerStance::Stand;
  }

  pos->isTimerunSave = client->sess.timerunActive;
}

int SaveSystem::getLatestSaveSlot(gentity_t *ent) {
  const std::string guid = _session->Guid(ent);
  const auto teamSaves = ent->client->sess.sessionTeam == TEAM_ALLIES
                             ? _clients[guid].alliesSaves
                             : _clients[guid].axisSaves;
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
  const std::string guid = _session->Guid(ent);
  const auto teamSaves = ent->client->sess.sessionTeam == TEAM_ALLIES
                             ? _clients[guid].alliesSaves
                             : _clients[guid].axisSaves;

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

inline bool SaveSystem::clientHasSaveData(gentity_t *ent) {
  const std::string guid = _session->Guid(ent);

  const auto it = _clients.find(guid);
  return it != _clients.end();
}

} // namespace ETJump
