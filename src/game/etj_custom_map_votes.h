/*
 * MIT License
 *
 * Copyright (c) 2026 ETJump team <zero@etjump.com>
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

#include <vector>
#include <string>
#include "etj_log.h"

namespace ETJump {
class MapStatistics;

class CustomMapVotes {
public:
  struct MapType {
    std::string type;
    std::string callvoteText;
    std::vector<std::string> mapsOnServer;
    std::vector<std::string> otherMaps;
  };

  struct TypeInfo {
    TypeInfo(bool typeExists, const std::string &callvoteText)
        : typeExists(typeExists), callvoteText(callvoteText) {}
    TypeInfo() : typeExists(false) {}
    bool typeExists;
    std::string callvoteText;
  };

  CustomMapVotes(const std::shared_ptr<MapStatistics> &mapStats,
                 std::unique_ptr<Log> log);
  ~CustomMapVotes() = default;

  // if 'init' is true, clients aren't forced to invalidate cached customvotes
  // any calls made outside of game init should call this with 'false'
  void loadCustomvotes(bool init);
  TypeInfo getTypeInfo(const std::string &type) const;
  std::string randomMap(const std::string &type);
  static bool isValidMap(const std::string &mapName);
  [[nodiscard]] std::vector<std::string> getCustomVoteTypes() const;
  std::string listInfo(const std::string &type);
  std::vector<std::string> getMapsOnList(const std::string &name);
  void generateVotesFile();
  size_t getNumVotelists() const;
  // returns nullptr on invalid index
  CustomMapVotes::MapType *getVotelistByIndex(int index);

  void addCustomvoteList(int clientNum, const std::string &name,
                         const std::string &fullName, const std::string &maps);
  void deleteCustomvoteList(int clientNum, const std::string &name);

  void editCustomvoteList(int clientNum, const std::string &list,
                          const std::string &name, const std::string &fullName,
                          const std::string &addMaps,
                          const std::string &removeMaps);

private:
  std::vector<MapType> customMapVotes_;
  const std::vector<std::string> *_currentMapsOnServer{};
  const std::shared_ptr<MapStatistics> &_mapStats;
  std::unique_ptr<Log> logger;
  std::string errors;
};
} // namespace ETJump
