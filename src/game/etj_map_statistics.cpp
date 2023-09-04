/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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

#include "etj_map_statistics.h"
#include <sqlite3.h>
#include <algorithm>
#include "etj_utilities.h"
#include "etj_string_utilities.h"
#include "g_local.h"

MapStatistics::MapStatistics()
    : _previousLevelTime(0), _currentMillisecondsPlayed(0),
      _currentMillisecondsOnServer(0), _currentMap(nullptr) {}

std::vector<const MapStatistics::MapInformation *>
MapStatistics::getMostPlayed() {
  std::vector<const MapInformation *> mostPlayed;
  for (auto &map : _maps) {
    mostPlayed.push_back(&map);
  }

  std::sort(mostPlayed.begin(), mostPlayed.end(),
            [](const MapInformation *lhs, const MapInformation *rhs) {
              return lhs->secondsPlayed > rhs->secondsPlayed;
            });

  return mostPlayed;
}

std::vector<const MapStatistics::MapInformation *>
MapStatistics::getLeastPlayed() {
  std::vector<const MapInformation *> mostPlayed;
  for (auto &map : _maps) {
    mostPlayed.push_back(&map);
  }

  std::sort(mostPlayed.begin(), mostPlayed.end(),
            [](const MapInformation *lhs, const MapInformation *rhs) {
              return lhs->secondsPlayed < rhs->secondsPlayed;
            });

  return mostPlayed;
}

const std::vector<std::string> *MapStatistics::getCurrentMaps() {
  return &_currentMaps;
}

std::vector<std::string> MapStatistics::getMaps() {
  std::vector<std::string> maps;

  for (auto &map : _maps) {
    if (map.isOnServer && !MapStatistics::isBlockedMap(map.name)) {
      maps.push_back(map.name);
    }
  }

  return maps;
}

const MapStatistics::MapInformation *
MapStatistics::getMapInformation(const std::string &mapName) {
  const MapInformation *mi = nullptr;
  if (mapName.length() == 0) {
    mi = _currentMap;
  } else {
    auto it = std::find_if(_maps.begin(), _maps.end(),
                           [&mapName](const MapInformation &mapInformation) {
                             return mapInformation.name == mapName;
                           });

    if (it == _maps.end()) {
      return nullptr;
    }

    mi = &(*it);
  }
  return mi;
}

void MapStatistics::increasePassedCount(const char *mapName) {
  auto it = std::find_if(
      _maps.begin(), _maps.end(),
      [&mapName](const MapInformation &mi) { return mi.name == mapName; });

  if (it == _maps.end()) {
    Utilities::Error(std::string("Error: Could not find map ") + mapName);
    return;
  }

  it->changed = true;
  ++it->votesPassed;
}

void MapStatistics::increaseCallvoteCount(const char *mapName) {
  auto it = std::find_if(
      _maps.begin(), _maps.end(),
      [&mapName](const MapInformation &mi) { return mi.name == mapName; });

  if (it == _maps.end()) {
    Utilities::Error(std::string("Error: Could not find map ") + mapName);
    return;
  }

  it->changed = true;
  ++it->callvoted;
}

void MapStatistics::saveChanges() {
  if (!_currentMap) {
    Utilities::Log("Notification: Map was changed before a new "
                   "map was loaded. "
                   "No map statistics were saved.");
    return;
  }
  _currentMap->changed = true;
  _currentMap->secondsPlayed =
      _originalSecondsPlayed + (_currentMillisecondsPlayed / 1000);
  _currentMap->timesPlayed += 1;
  time_t t;
  time(&t);
  _currentMap->lastPlayed = static_cast<int>(t);

  sqlite3 *db = nullptr;
  auto rc = sqlite3_open(Utilities::getPath(_databaseName).c_str(), &db);
  if (rc != SQLITE_OK) {
    Utilities::Error(ETJump::stringFormat("MapStatistics::saveChanges: Error: "
                                          "Failed to open database. (%d) %s.\n",
                                          rc, sqlite3_errmsg(db)));
    return;
  }

  rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
  if (rc != SQLITE_OK) {
    Utilities::Error(
        ETJump::stringFormat("MapStatistics::saveChanges: Error: Failed to "
                             "start transaction. (%d) %s.\n",
                             rc, sqlite3_errmsg(db)));
    return;
  }

  for (auto &map : _maps) {
    if (map.changed) {
      rc = sqlite3_exec(db,
                        ETJump::stringFormat(
                            "UPDATE map_statistics SET "
                            "seconds_played=%d, callvoted=%d, "
                            "votes_passed=%d, times_played=%d, "
                            "last_played=%d WHERE id=%d;",
                            map.secondsPlayed, map.callvoted, map.votesPassed,
                            map.timesPlayed, map.lastPlayed, map.id)
                            .c_str(),
                        nullptr, nullptr, nullptr);
      if (rc != SQLITE_OK) {
        Utilities::Error(ETJump::stringFormat("MapStatistics::saveChanges: "
                                              "Error: Failed to "
                                              "update map. (%d) %s.\n",
                                              rc, sqlite3_errmsg(db)));
        return;
      }
    }
  }

  rc = sqlite3_exec(db, "END TRANSACTION;", nullptr, nullptr, nullptr);
  if (rc != SQLITE_OK) {
    Utilities::Error(
        ETJump::stringFormat("MapStatistics::saveChanges: Error: Failed to end "
                             "transaction. (%d) %s.\n",
                             rc, sqlite3_errmsg(db)));
    return;
  }

  rc = sqlite3_close(db);
  if (rc != SQLITE_OK) {
    Utilities::Error(ETJump::stringFormat(
        "MapStatistics::saveChanges: Error: Failed to close "
        "sqlite db. (%d) %s",
        rc, sqlite3_errmsg(db)));
  }
}

void MapStatistics::runFrame(int levelTime) {
  auto diff = levelTime - _previousLevelTime;
  _previousLevelTime = levelTime;

  _currentMillisecondsOnServer += diff;
  if (Utilities::anyonePlaying()) {
    _currentMillisecondsPlayed += diff;
    _currentMap->secondsPlayed =
        _originalSecondsPlayed + (_currentMillisecondsPlayed / 1000);
  }
}

void MapStatistics::resetFields() {
  _maps.clear();
  _currentMap = nullptr;
  _currentMillisecondsOnServer = 0;
  _currentMillisecondsPlayed = 0;
  _databaseName = "";
  _originalSecondsPlayed = 0;
  _previousLevelTime = 0;
}

bool MapStatistics::initialize(std::string database,
                               const std::string &currentMap) {
  resetFields();

  if (database.length() == 0) {
    database = "maps_database.db";
  }

  _databaseName = database;
  if (!loadFromDatabase()) {
    // TODO: error handling
    return false;
  }

  addNewMaps();

  setCurrentMap(currentMap);

  _originalSecondsPlayed = _currentMap->secondsPlayed;

  return true;
}

const MapStatistics::MapInformation *MapStatistics::getCurrentMap() const {
  return _currentMap;
}

void MapStatistics::setCurrentMap(const std::string currentMap) {
  auto it = std::find_if(_maps.begin(), _maps.end(),
                         [&currentMap](const MapInformation &iteratee) {
                           return iteratee.name == currentMap;
                         });

  if (it == _maps.end()) {
    Utilities::Error(ETJump::stringFormat(
        "Error: Failed to set the current map to %s. Map could "
        "not be found in "
        "the maps vector. Map count: %d\n",
        currentMap, _maps.size()));
    return;
  }

  _currentMap = &(*it);
}

void MapStatistics::addNewMaps() {
  auto maps = Utilities::getMaps();
  auto mapCount = 0;
  std::vector<std::string> newMaps;
  _currentMaps = maps;
  std::sort(_currentMaps.begin(), _currentMaps.end());

  for (auto &map : maps) {
    ++mapCount;
    auto it = std::find_if(
        _maps.begin(), _maps.end(),
        [&map](const MapInformation &mi) { return mi.name == map; });

    if (it != _maps.end()) {
      it->isOnServer = true;
      // skip maps that are already in the array
      continue;
    }

    Utilities::Console(
        ETJump::stringFormat("Notification: New map %s found.\n", map));

    MapInformation mapInformation;
    mapInformation.name = map;
    mapInformation.isOnServer = true;

    _maps.push_back(std::move(mapInformation));
    newMaps.push_back(map);
  }

  auto count = newMaps.size();
  saveNewMaps(std::move(newMaps));

  Utilities::Console(ETJump::stringFormat(
      "%d maps on the server. Added %d new maps.\n", mapCount, count));
}

void MapStatistics::saveNewMaps(std::vector<std::string> newMaps) {
  sqlite3 *db = nullptr;
  auto rc = sqlite3_open(Utilities::getPath(_databaseName).c_str(), &db);
  if (rc != SQLITE_OK) {
    Utilities::Error(ETJump::stringFormat(
        "MapStatistics::saveNewMaps: Error: Could not open map "
        "database %s\n",
        _databaseName));
    return;
  }

  for (auto &newMap : newMaps) {
    std::string sqlEscapedMapName = newMap;
    ETJump::StringUtil::replaceAll(sqlEscapedMapName, "'", "''");
    // Map names won't be doing any SQL injection
    rc = sqlite3_exec(db,
                      ETJump::stringFormat(
                          "INSERT INTO map_statistics (name, seconds_played, "
                          "callvoted, votes_passed, times_played, "
                          "last_played) "
                          "VALUES ('%s', 0, 0, 0, 0, 0);",
                          sqlEscapedMapName)
                          .c_str(),
                      nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
      Utilities::Error(
          ETJump::stringFormat("MapStatistics::saveNewMaps: Error: Failed "
                               "to "
                               "execute statement: (%d) %s",
                               rc, sqlite3_errmsg(db)));
      sqlite3_close(db);
      return;
    }

    auto newMapIter = std::find_if(
        _maps.begin(), _maps.end(),
        [&newMap](const MapInformation &mi) { return mi.name == newMap; });

    if (newMapIter == _maps.end()) {
      Utilities::Error("MapStatistics::saveNewMaps: Error: "
                       "newMapIter == _maps.end()");
      return;
    }
    newMapIter->id = sqlite3_last_insert_rowid(db);
  }

  sqlite3_close(db);
}

bool MapStatistics::loadFromDatabase() {
  if (!createDatabase()) {
    return false;
  }

  if (!loadMaps()) {
    return false;
  }

  return true;
}

bool MapStatistics::loadMaps() {
  sqlite3 *db = nullptr;

  auto rc = sqlite3_open(Utilities::getPath(_databaseName).c_str(), &db);
  if (rc != SQLITE_OK) {
    Utilities::Error(ETJump::stringFormat("MapStatistics::loadMaps: Error: "
                                          "Failed to open database %s\n",
                                          _databaseName));
    return false;
  }

  // id INTEGER PRIMARY KEY AUTOINCREMENT, seconds_played INTEGER NOT
  // NULL, callvoted INTEGER NOT NULL, votes_passed INTEGER NOT NULL,
  // times_played INTEGER NOT NULL
  sqlite3_stmt *stmt = nullptr;
  rc = sqlite3_prepare(db,
                       "SELECT id, name, seconds_played, callvoted, "
                       "votes_passed, times_played, "
                       "last_played FROM map_statistics;",
                       -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    Utilities::Error(
        ETJump::stringFormat("MapStatistics::loadMaps: Error: Failed to "
                             "prepare statement: (%d) %s\n",
                             rc, sqlite3_errmsg(db)));
    sqlite3_close(db);
    return false;
  }

  rc = sqlite3_step(stmt);
  while (rc == SQLITE_ROW) {
    MapInformation mi;

    mi.id = sqlite3_column_int(stmt, 0);
    auto name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    mi.name = name ? name : "";
    mi.secondsPlayed = sqlite3_column_int(stmt, 2);
    mi.callvoted = sqlite3_column_int(stmt, 3);
    mi.votesPassed = sqlite3_column_int(stmt, 4);
    mi.timesPlayed = sqlite3_column_int(stmt, 5);
    mi.lastPlayed = sqlite3_column_int(stmt, 6);
    mi.isOnServer = false;

    _maps.push_back(mi);
    rc = sqlite3_step(stmt);
  }

  if (rc != SQLITE_DONE) {
    Utilities::Error(
        ETJump::stringFormat("MapStatistics::loadMaps: Error: Reading map "
                             "statistics failed. (%d) %s\n",
                             rc, sqlite3_errmsg(db)));
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return false;
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  return true;
}

bool MapStatistics::createDatabase() {
  sqlite3 *db = nullptr;
  auto rc = sqlite3_open(Utilities::getPath(_databaseName).c_str(), &db);
  if (rc != SQLITE_OK) {
    Utilities::Error(ETJump::stringFormat(
        "MapStatistics::createDatabase: Error: Failed to open "
        "database %s\n",
        _databaseName));
    return false;
  }

  char *errorMessage = nullptr;
  rc = sqlite3_exec(db,
                    "CREATE TABLE IF NOT EXISTS map_statistics (id "
                    "INTEGER PRIMARY KEY "
                    "AUTOINCREMENT, name TEXT UNIQUE, seconds_played "
                    "INTEGER NOT NULL, "
                    "callvoted INTEGER NOT NULL, votes_passed INTEGER "
                    "NOT NULL, times_played "
                    "INTEGER NOT NULL, last_played INTEGER NOT NULL);",
                    nullptr, nullptr, &errorMessage);
  if (rc != SQLITE_OK) {
    sqlite3_free(errorMessage);
    sqlite3_close(db);
    Utilities::Error(
        ETJump::stringFormat("MapStatistics::createDatabase: Error: Failed to "
                             "create database %s. (%d) %s\n",
                             _databaseName, rc, errorMessage));
    return false;
  }
  sqlite3_free(errorMessage);
  sqlite3_close(db);
  return true;
}

const char *MapStatistics::randomMap() const {
  static char mapName[256];
  const int MAX_TRIES = 15;
  std::random_device rd;
  std::mt19937 re(rd());
  std::uniform_int_distribution<int> ui(0, _maps.size() - 1);

  mapName[0] = 0;

  int mapIdx = -1;
  // try to select a valid random map
  for (int tries = 0; tries < MAX_TRIES; tries++) {
    int testIdx = ui(re);
    if (isValidMap(&_maps[testIdx])) {
      mapIdx = testIdx;
    }
  }
  // fallback, iterate map list and select first valid map
  if (mapIdx < 0) {
    for (int i = 0; i < static_cast<int>(_maps.size()); i++) {
      if (isValidMap(&_maps[i])) {
        mapIdx = i;
        break;
      }
    }
  }

  if (mapIdx >= 0) {
    Q_strncpyz(mapName, _maps[mapIdx].name.c_str(), sizeof(mapName));
  }

  return mapName;
}

std::vector<std::string> MapStatistics::blockedMaps() {
  std::string blockedMapsStr =
      ETJump::StringUtil::toLowerCase(g_blockedMaps.string);
  return ETJump::StringUtil::split(blockedMapsStr, " ");
}

bool MapStatistics::isBlockedMap(const std::string &mapName) {
  bool isBlocked = false;
  auto blockedMaps = MapStatistics::blockedMaps();
  auto numBlockedMaps = blockedMaps.size();

  for (int i = 0; i < numBlockedMaps; i++) {
    if (ETJump::StringUtil::matches(blockedMaps[i], mapName)) {
      isBlocked = true;
      break;
    }
  }

  return isBlocked;
}

bool MapStatistics::isValidMap(const MapInformation *mapInfo) const {
  return mapInfo != _currentMap && mapInfo->isOnServer &&
         !MapStatistics::isBlockedMap(mapInfo->name);
}

MapStatistics::~MapStatistics() {}
