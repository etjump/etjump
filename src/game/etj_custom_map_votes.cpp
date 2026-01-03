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

#include <fstream>
#include <algorithm>
#include <set>

#include "etj_custom_map_votes.h"
#include "etj_map_statistics.h"
#include "etj_string_utilities.h"
#include "utilities.hpp"
#include "etj_printer.h"
#include "etj_json_utilities.h"
#include "etj_filesystem.h"

namespace ETJump {
CustomMapVotes::CustomMapVotes(const std::shared_ptr<MapStatistics> &mapStats,
                               std::unique_ptr<Log> log)
    : _mapStats(mapStats), logger(std::move(log)) {}

CustomMapVotes::TypeInfo
CustomMapVotes::getTypeInfo(std::string const &type) const {
  for (const auto &customMapVote : customMapVotes_) {
    if (customMapVote.type == type) {
      return {true, customMapVote.callvoteText};
    }
  }
  return {false, ""};
}

void CustomMapVotes::loadCustomvotes(const bool init) {
  const std::string filename = g_customMapVotesFile.string;

  if (filename.empty()) {
    return;
  }

  _currentMapsOnServer = _mapStats->getCurrentMaps();
  customMapVotes_.clear();

  const auto parsingFailed = [&](const std::string &errMsg) {
    logger->error(errMsg);
    customMapVotes_.clear();

    if (!init) {
      Printer::commandAll("forceCustomvoteRefresh");
    }
  };

  if (!FileSystem::exists(filename)) {
    logger->info("Customvote file not found. Use '/generatecustomvotes' to "
                 "generate an example file.");
    return;
  }

  Json::Value root;

  if (!JsonUtils::readFile(filename, root, &errors)) {
    logger->error(errors);
    return;
  }

  for (const auto &list : root) {
    customMapVotes_.emplace_back();
    const size_t curr = customMapVotes_.size() - 1;

    if (!JsonUtils::parseValue(customMapVotes_[curr].type, list["name"],
                               &errors, "name")) {
      parsingFailed(errors);
      return;
    }

    if (!isValidVoteString(customMapVotes_[curr].type)) {
      logger->error(
          "Failed to parse custom vote - name '%s' contains invalid characters",
          customMapVotes_[curr].type);
      customMapVotes_.pop_back();
      continue;
    }

    customMapVotes_[curr].type = sanitize(customMapVotes_[curr].type, true);

    if (customMapVotes_[curr].type.empty()) {
      customMapVotes_.pop_back();
      logger->error("Failed to parse custom vote - 'name' is empty");
      continue;
    }

    if (!JsonUtils::parseValue(customMapVotes_[curr].callvoteText,
                               list["callvote_text"], &errors,
                               "callvote_text")) {
      parsingFailed(errors);
      return;
    }

    customMapVotes_[curr].callvoteText =
        sanitize(customMapVotes_[curr].callvoteText);

    if (customMapVotes_[curr].callvoteText.empty()) {
      customMapVotes_.pop_back();
      logger->error("Failed to parse custom vote - 'callvote_text' is empty");
      continue;
    }

    std::set<std::string> uniqueMapsOnServer{};
    std::set<std::string> uniqueMapsOther{};

    Json::Value maps = list["maps"];

    // clean up the list from potential duplicates
    for (const auto &map : maps) {
      std::string mapName;
      if (!JsonUtils::parseValue(mapName, map, &errors, "map")) {
        parsingFailed(errors);
        return;
      }

      mapName = sanitize(mapName, true);

      if (std::binary_search(_currentMapsOnServer->begin(),
                             _currentMapsOnServer->end(), mapName)) {
        uniqueMapsOnServer.insert(mapName);
      } else {
        uniqueMapsOther.insert(mapName);
      }
    }

    for (const auto &map : uniqueMapsOnServer) {
      customMapVotes_[curr].mapsOnServer.emplace_back(map);
    }

    for (const auto &map : uniqueMapsOther) {
      customMapVotes_[curr].otherMaps.emplace_back(map);
    }
  }

  if (!init) {
    // invalidate cached customvote lists for all clients
    Printer::commandAll("forceCustomvoteRefresh");
  }

  logger->info("Succesfully loaded %i custom votes from '%s'",
               static_cast<int>(customMapVotes_.size()), filename);
}

std::string CustomMapVotes::listTypes() const {
  std::string buf;
  for (int i = 0; i < customMapVotes_.size(); i++) {
    if (i != (customMapVotes_.size() - 1)) {
      buf += customMapVotes_[i].type + ", ";
    } else {
      buf += customMapVotes_[i].type;
    }
  }
  return buf;
}

void CustomMapVotes::generateVotesFile() {
  Arguments argv = GetArgs();

  const bool overwrite = argv->size() > 1 && argv->at(1) == "-f";
  const std::string filename = g_customMapVotesFile.string;

  if (argv->size() == 1 || !overwrite) {
    if (FileSystem::exists(filename)) {
      G_Printf("A custom map vote file '%s' already exists. Use "
               "'generatecustomvotes -f' to overwrite the existing file.\n",
               filename.c_str());
      return;
    }
  }

  if (overwrite) {
    G_Printf("Overwriting custom map vote file '%s' with a default one.\n",
             filename.c_str());
  }

  Json::Value root = Json::arrayValue;
  Json::Value vote;

  vote["name"] = "originals";
  vote["callvote_text"] = "Original 6 Maps";
  vote["maps"] = Json::arrayValue;
  vote["maps"].append("oasis");
  vote["maps"].append("fueldump");
  vote["maps"].append("radar");
  vote["maps"].append("goldrush");
  vote["maps"].append("railgun");
  vote["maps"].append("battery");

  root.append(vote);

  vote["name"] = "just_oasis";
  vote["callvote_text"] = "Just oasis";
  vote["maps"] = Json::arrayValue;
  vote["maps"].append("oasis");

  root.append(vote);

  if (!JsonUtils::writeFile(filename, root, &errors)) {
    G_Printf("%s\n", errors.c_str());
    return;
  }

  G_Printf("Generated new custom map votes file '%s'\n", filename.c_str());
  loadCustomvotes(false);
}

void CustomMapVotes::addCustomvoteList(int clientNum, const std::string &name,
                                       const std::string &fullName,
                                       const std::string &maps) {
  const std::string &customVotesFile = g_customMapVotesFile.string;
  Json::Value root;
  const std::string sanitizedName = sanitize(name, true);

  // read and append to existing file if present
  if (FileSystem::exists(customVotesFile)) {
    if (JsonUtils::readFile(customVotesFile, root, &errors)) {
      // make sure we don't already have a list with this name
      for (const auto &lists : customMapVotes_) {
        if (sanitizedName == lists.type) {
          Printer::chat(clientNum, "^3add-customvote: ^7operation failed, "
                                   "check console for more information.");
          Printer::console(clientNum,
                           stringFormat("^3add-customvote: ^7a list with the "
                                        "name ^3'%s' ^7already exists.\n",
                                        sanitizedName));
          return;
        }
      }
    } else {
      Printer::chat(clientNum, "^3add-customvote: ^7operation failed, check "
                               "console for more information.");
      Printer::console(clientNum,
                       stringFormat("^3add-customvote: ^7couldn't open the "
                                    "file ^3'%s' ^7for reading:\n",
                                    customVotesFile));
      Printer::console(clientNum, errors + '\n');
      return;
    }
  }

  Json::Value vote;
  vote["name"] = sanitizedName;
  vote["callvote_text"] = sanitize(fullName);
  vote["maps"] = Json::arrayValue;

  const auto mapList = StringUtil::split(maps, " ");

  for (const auto &map : mapList) {
    vote["maps"].append(sanitize(map, true));
  }

  root.append(vote);

  if (!JsonUtils::writeFile(customVotesFile, root, &errors)) {
    Printer::chat(clientNum, "^3add-customvote: ^7operation failed, check "
                             "console for more information.");
    Printer::console(clientNum,
                     stringFormat("^3add-customvote: ^7couldn't open the file "
                                  "^3'%s' ^7for writing:\n",
                                  customVotesFile));
    Printer::console(clientNum, errors + '\n');
    return;
  }

  loadCustomvotes(false);
  Printer::chat(clientNum, stringFormat("^3add-customvote: ^7successfully "
                                        "added a new custom vote list ^3'%s'",
                                        sanitizedName));
}

void CustomMapVotes::deleteCustomvoteList(int clientNum,
                                          const std::string &name) {
  const std::string &customVotesFile = g_customMapVotesFile.string;
  Json::Value root;

  if (!JsonUtils::readFile(customVotesFile, root, &errors)) {
    Printer::chat(clientNum, "^3delete-customvote: ^7operation failed, check "
                             "console for more information.");
    Printer::console(clientNum,
                     stringFormat("^3delete-customvote: ^7couldn't open the "
                                  "file ^3'%s' ^7for reading.\n",
                                  customVotesFile));
    Printer::console(clientNum, errors + '\n');
    return;
  }

  bool found = false;

  for (Json::ArrayIndex i = 0; i < root.size(); i++) {
    if (root[i]["name"].asString() == name) {
      Json::Value removed;
      root.removeIndex(i, &removed);
      found = true;
      break;
    }
  }

  if (!found) {
    Printer::chat(clientNum, stringFormat("^3delete-customvote: ^7a list with "
                                          "the name ^3'%s' ^7was not found.\n",
                                          name));
    return;
  }

  if (!JsonUtils::writeFile(customVotesFile, root, &errors)) {
    Printer::chat(clientNum, "^3delete-customvote: ^7operation failed, check "
                             "console for more information.");
    Printer::console(clientNum,
                     stringFormat("^3delete-customvote: ^7couldn't open the "
                                  "file ^3'%s' ^7for writing.\n",
                                  customVotesFile));
    Printer::console(clientNum, errors + '\n');
    return;
  }

  loadCustomvotes(false);
  Printer::chat(
      clientNum,
      stringFormat(
          "^3delete-customvote: ^7successfully deleted custom vote list ^3'%s'",
          name));
}

void CustomMapVotes::editCustomvoteList(int clientNum, const std::string &list,
                                        const std::string &name,
                                        const std::string &fullName,
                                        const std::string &addMaps,
                                        const std::string &removeMaps) {
  const std::string &customVotesFile = g_customMapVotesFile.string;
  Json::Value root;

  if (!JsonUtils::readFile(customVotesFile, root, &errors)) {
    Printer::chat(clientNum, "^3edit-customvote: ^7operation failed, check "
                             "console for more information.");
    Printer::console(clientNum,
                     stringFormat("^3edit-customvote: ^7couldn't open the file "
                                  "^3'%s' ^7for reading.\n",
                                  customVotesFile));
    Printer::console(clientNum, errors + '\n');
    return;
  }

  const std::string sanitizedList = sanitize(list, true);
  const std::string sanitizedName = sanitize(name, true);
  const std::string sanitizedFName = sanitize(fullName);

  bool found = false;

  const auto parsingFailed = [&](const std::string &errMsg) {
    Printer::chat(clientNum, "^3edit-customvote: ^7operation failed, check "
                             "console for more information.");
    Printer::console(clientNum, errMsg + "\n");
  };

  for (auto &object : root) {
    std::string temp;

    if (!JsonUtils::parseValue(temp, object["name"], &errors, "name")) {
      parsingFailed(errors);
      return;
    }

    if (sanitize(temp, true) == sanitizedList) {
      if (!sanitizedName.empty()) {
        for (const auto &key : object) {
          if (!JsonUtils::parseValue(temp, object["name"], &errors, "name")) {
            parsingFailed(errors);
            return;
          }

          if (key == temp) {
            object["name"] = sanitizedName;
          }
        }
      }

      if (!sanitizedFName.empty()) {
        for (const auto &key : object) {
          if (!JsonUtils::parseValue(temp, object["callvote_text"], &errors,
                                     "callvote_text")) {
            parsingFailed(errors);
            return;
          }

          if (key == temp) {
            object["callvote_text"] = sanitizedFName;
          }
        }
      }

      if (!addMaps.empty()) {
        const auto mapList = StringUtil::split(addMaps, " ");

        for (const auto &map : mapList) {
          object["maps"].append(sanitize(map, true));
        }
      }

      if (!removeMaps.empty()) {
        const auto mapList = StringUtil::split(removeMaps, " ");

        for (const auto &map : mapList) {
          for (Json::ArrayIndex i = 0; i < object["maps"].size(); i++) {
            if (!JsonUtils::parseValue(temp, object["maps"][i], &errors,
                                       "maps")) {
              parsingFailed(errors);
              return;
            }

            if (StringUtil::iEqual(temp, map)) {
              Json::Value removed;
              object["maps"].removeIndex(i, &removed);
              break;
            }
          }
        }
      }

      found = true;
      break;
    }
  }

  if (!found) {
    Printer::chat(clientNum, stringFormat("^3edit-customvote: ^7a list with "
                                          "the name ^3'%s' ^7was not found.\n",
                                          sanitizedList));
    return;
  }

  if (!JsonUtils::writeFile(customVotesFile, root, &errors)) {
    Printer::chat(clientNum, "^3edit-customvote: ^7operation failed, check "
                             "console for more information.");
    Printer::console(clientNum,
                     stringFormat("^3edit-customvote: ^7couldn't open the file "
                                  "^3'%s' ^7for writing.\n",
                                  customVotesFile));
    Printer::console(clientNum, errors + '\n');
    return;
  }

  loadCustomvotes(false);
  Printer::chat(
      clientNum,
      stringFormat(
          "^3edit-customvote: ^7successfully edited custom vote list ^3'%s'",
          sanitizedList));
}

std::string CustomMapVotes::listInfo(const std::string &type) {
  std::string buffer;

  for (const auto &customMapVote : customMapVotes_) {
    if (customMapVote.type == type) {
      buffer += stringFormat("^gMaps on the list ^3%s^g: \n", type);
      auto numMapsOnServer = customMapVote.mapsOnServer.size();

      int count = 0;
      for (auto &mapOnServer : customMapVote.mapsOnServer) {
        buffer += stringFormat("^7%-30s", mapOnServer);

        ++count;
        if (count % 3 == 0 || count == numMapsOnServer) {
          buffer += "\n";
        }
      }

      if (!customMapVote.otherMaps.empty()) {
        buffer += "\n^gMaps included on the list, but missing on server: \n";
        count = 0;
        auto numMapsNotOnServer = customMapVote.otherMaps.size();
        for (auto &mapNotOnServer : customMapVote.otherMaps) {
          buffer += stringFormat("^9%-30s", mapNotOnServer);

          ++count;
          if (count % 3 == 0 || count == numMapsNotOnServer) {
            buffer += "\n";
          }
        }
      }
    }
  }

  return buffer;
}

std::vector<std::string>
CustomMapVotes::getMapsOnList(const std::string &name) {
  std::vector<std::string> mapList{};

  for (const auto &list : customMapVotes_) {
    if (list.type == name) {
      mapList = list.mapsOnServer;
      break;
    }
  }

  return mapList;
}

size_t CustomMapVotes::getNumVotelists() const {
  return customMapVotes_.size();
}

CustomMapVotes::MapType *CustomMapVotes::getVotelistByIndex(const int index) {
  if (index < 0 || index >= customMapVotes_.size()) {
    return nullptr;
  }

  return &customMapVotes_[index];
}

std::string CustomMapVotes::randomMap(std::string const &type) {
  for (const auto &list : customMapVotes_) {
    if (list.type != type || list.mapsOnServer.empty()) {
      continue;
    }

    static constexpr int MAX_TRIES = 15;
    std::random_device rd;
    std::mt19937 re(rd());
    std::uniform_int_distribution<int> ui(
        0, static_cast<int>(list.mapsOnServer.size() - 1));

    // try to select a valid random map
    for (int tries = 0; tries < MAX_TRIES; tries++) {
      int testIdx = ui(re);
      if (isValidMap(list.mapsOnServer[testIdx])) {
        return list.mapsOnServer[testIdx];
      }
    }

    // fallback, iterate map list and select first valid map
    for (const auto &map : list.mapsOnServer) {
      if (isValidMap(map)) {
        return map;
      }
    }

    return "";
  }

  return "";
}

// FIXME: this should just be nuked and custom map votes should use
//  MapStatistics::isValidMap but that takes MapInfo as argument
//  and isn't simple to refactor so bleh
bool CustomMapVotes::isValidMap(const std::string &mapName) {
  return game.mapStatistics->mapExists(mapName) && mapName != level.rawmapname;
}
} // namespace ETJump
