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

#pragma once

#include <unordered_map>
#include <string>
#include <deque>

#include "etj_local.h"

namespace ETJump {
class SaveSystem {
public:
  explicit SaveSystem(const std::shared_ptr<Session> &session)
      : _session(session) {}
  ~SaveSystem() = default;

  static constexpr int MAX_SAVED_POSITIONS = 3;
  static constexpr int MAX_BACKUP_POSITIONS = 3;

  struct SavePosition {
    SavePosition()
        : isValid(false), isLatest(false), origin{0, 0, 0}, vangles{0, 0, 0},
          stance(PlayerStance::Stand), isTimerunSave(false) {}
    bool isValid;
    bool isLatest;
    vec3_t origin;
    vec3_t vangles;
    PlayerStance stance;
    bool isTimerunSave;
  };

  struct Client {
    Client();

    SavePosition alliesSaves[MAX_SAVED_POSITIONS];
    std::deque<SavePosition> alliesBackups;
    SavePosition alliesLastLoadPos;

    SavePosition axisSaves[MAX_SAVED_POSITIONS];
    std::deque<SavePosition> axisBackups;
    SavePosition axisLastLoadPos;

    // contains a couple of extra positions for TEAM_SPEC and
    // TEAM_FREE, but simplifies the accessing code
    SavePosition quickDeployPositions[TEAM_NUM_TEAMS];
  };

  struct DisconnectedClient {
    DisconnectedClient();

    // Allies saved positions at the time of disconnect
    SavePosition alliesSaves[MAX_SAVED_POSITIONS];
    // Axis saved positions at the time of disconnect
    SavePosition axisSaves[MAX_SAVED_POSITIONS];

    // Last load positions
    SavePosition axisLastLoadPos;
    SavePosition alliesLastLoadPos;

    // So called "map ident"
    // FIXME: why tf is this here, it has nothing to do with save positions
    int progression;

    // Global map savelimit
    int saveLimit;
  };

  enum class SaveLoadRestrictions {
    Default = 0,
    Move = 1 << 0,
    Dead = 1 << 1,
  };

  // Saves current position
  void save(gentity_t *ent);

  // Loads position
  void load(gentity_t *ent);

  // Saves position, does not check for anything
  void forceSave(const gentity_t *location, gentity_t *ent);

  // Loads backup position
  void loadBackupPosition(gentity_t *ent);

  // UnLoad - revert last "load" command
  void unload(gentity_t *ent);

  // Saves last position client loaded from
  void saveLastLoadPos(gentity_t *ent);

  // Resets targets positions
  void resetSavedPositions(gentity_t *ent);

  // invalidates any save slots that are marked as timerun saves
  // called at the start of a timerun to reset saves from previous run
  void clearTimerunSaves(gentity_t *ent);

  // Saves positions to db on disconnect
  void savePositionsToDatabase(gentity_t *ent);

  // Loads positions from db on dc
  void loadPositionsFromDatabase(gentity_t *ent);

  void storeTeamQuickDeployPosition(gentity_t *ent, team_t team);
  void loadOnceTeamQuickDeployPosition(gentity_t *ent, team_t team);

private:
  // Saves backup position
  void saveBackupPosition(gentity_t *ent, const SavePosition *pos);

  // copies player positional info to target position
  static void storePosition(const gclient_s *client, SavePosition *pos);

  // returns the latest save slot number that client used in their current team
  // -1 if no slots found (no saved positions in current team)
  int getLatestSaveSlot(gentity_t *ent);

  // marks all save slots as 'not latest', called before storing a new save pos
  // this does not touch backup positions as we don't care about them
  // since 'isLatest' check is only done to save slots, not backup slots
  void resetLatestSaveSlot(gentity_t *ent);

  // Teleports player to the saved position
  static void teleportPlayer(gentity_t *ent, SavePosition *pos);

  // true if '_clients' contains data for the ent
  inline bool clientHasSaveData(gentity_t *ent);

  SavePosition *getValidTeamQuickDeploySave(gentity_t *ent, team_t team);

  static void restoreStanceFromSave(gentity_t *ent, const SavePosition *pos);

  SavePosition *getValidTeamSaveForSlot(gentity_t *ent, team_t team, int slot);

  SavePosition *getValidTeamUnloadPos(gentity_t *ent, team_t team);

  // Commands that are sent to client when they successfully save a position
  static void sendClientCommands(gentity_t *ent, int position);

  // All clients' save related data
  std::unordered_map<std::string, Client> _clients;

  // Disconnected clients saved position data
  std::unordered_map<std::string, DisconnectedClient> _savedPositions;

  // Interface to get player guid
  const std::shared_ptr<Session> _session;
};
} // namespace ETJump
