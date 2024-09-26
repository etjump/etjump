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

#pragma once

#include <string>
#include <vector>
#include <random>

namespace ETJump {
class MapStatistics {
public:
  MapStatistics();
  ~MapStatistics() = default;

  struct MapInformation {
    MapInformation()
        : id(0), secondsPlayed(0), callvoted(0), votesPassed(0), timesPlayed(0),
          isOnServer(false), changed(true) {}
    long long id;
    std::string name;
    int secondsPlayed;
    int callvoted;
    int votesPassed;
    int timesPlayed;
    int lastPlayed;
    bool isOnServer;
    bool changed;
  };

  const char *randomMap() const;
  bool isValidMap(const MapInformation *mapInfo) const;

  bool createDatabase();
  bool loadMaps();
  bool loadFromDatabase();
  void saveNewMaps(std::vector<std::string> newMaps);
  void addNewMaps();
  void setCurrentMap(const std::string currentMap);
  const MapInformation *getCurrentMap() const;
  bool initialize(std::string database, const std::string &currentMap);
  void runFrame(int levelTime);
  void resetFields();
  void saveChanges();
  void increaseCallvoteCount(const char *map_name);
  void increasePassedCount(const char *map_name);
  const MapInformation *getMapInformation(const std::string &mapName);
  std::vector<const MapInformation *> getMostPlayed();
  std::vector<const MapInformation *> getLeastPlayed();
  std::vector<std::string> getMaps();
  const std::vector<std::string> *getCurrentMaps();
  static std::vector<std::string> blockedMaps();
  static bool isBlockedMap(const std::string &mapName);
  bool mapExists(const std::string &mapName);
  void writeMapsToDisk(const std::string &fileName);

private:
  std::vector<MapInformation> _maps;
  std::vector<std::string> _currentMaps;
  int _previousLevelTime;
  // How many milliseconds have elapsed with atleast 1 player on team
  int _currentMillisecondsPlayed;
  // How many milliseconds have elapsed since the map was changed
  int _currentMillisecondsOnServer;
  // What the current map on the server is
  MapInformation *_currentMap;
  std::string _databaseName;
  int _originalSecondsPlayed;
};
} // namespace ETJump
