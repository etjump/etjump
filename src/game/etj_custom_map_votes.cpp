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

#include <fstream>
#include <algorithm>

#include "etj_custom_map_votes.h"
#include "etj_map_statistics.h"
#include "etj_string_utilities.h"
#include "utilities.hpp"
#include "etj_printer.h"
#include "etj_json_utilities.h"

CustomMapVotes::CustomMapVotes(MapStatistics *mapStats) : _mapStats(mapStats) {}

CustomMapVotes::~CustomMapVotes() {}

CustomMapVotes::TypeInfo
CustomMapVotes::GetTypeInfo(std::string const &type) const {
  for (unsigned i = 0; i < customMapVotes_.size(); i++) {
    if (customMapVotes_[i].type == type) {
      return TypeInfo(true, customMapVotes_[i].callvoteText);
    }
  }
  return TypeInfo(false, "");
}

bool CustomMapVotes::Load() {
  if (strlen(g_customMapVotesFile.string) == 0) {
    return false;
  }
  _currentMapsOnServer = _mapStats->getCurrentMaps();

  customMapVotes_.clear();
  std::string path = GetPath(g_customMapVotesFile.string);
  std::ifstream f(path.c_str());

  if (!f) {
    G_LogPrintf("Couldn't open \"%s\". Use /rcon "
                "generatecustomvotes to "
                "generate an example file.\n",
                g_customMapVotesFile.string);
    return false;
  }
  std::string content((std::istreambuf_iterator<char>(f)),
                      std::istreambuf_iterator<char>());

  Json::Value root;
  Json::Reader reader;

  if (!reader.parse(content, root)) {
    G_LogPrintf("There was a parsing error in the %s: %s\n",
                reader.getFormattedErrorMessages().c_str(),
                g_customMapVotesFile.string);
    return false;
  }

  try {
    for (int i = 0; i < static_cast<int>(root.size()); i++) {
      customMapVotes_.push_back(MapType());
      unsigned curr = customMapVotes_.size() - 1;
      Json::Value maps = root[i]["maps"];
      customMapVotes_[curr].type = root[i]["name"].asString();
      customMapVotes_[curr].callvoteText = root[i]["callvote_text"].asString();
      for (int j = 0; j < static_cast<int>(maps.size()); j++) {
        auto mapName = maps[j].asString();
        if (std::binary_search(_currentMapsOnServer->begin(),
                               _currentMapsOnServer->end(), mapName)) {
          customMapVotes_[curr].mapsOnServer.push_back(mapName);
        } else {
          customMapVotes_[curr].otherMaps.push_back(mapName);
        }
      }
    }
  } catch (...) {
    G_LogPrintf("There was a read error in %s parser\n",
                g_customMapVotesFile.string);
    return false;
  }

  G_LogPrintf("Successfully initialized custom votes.\n");
  return true;
}

std::string CustomMapVotes::ListTypes() const {
  std::string buf;
  for (unsigned i = 0; i < customMapVotes_.size(); i++) {
    if (i != (customMapVotes_.size() - 1)) {
      buf += customMapVotes_[i].type + ", ";
    } else {
      buf += customMapVotes_[i].type;
    }
  }
  return buf;
}

void CustomMapVotes::GenerateVotesFile() {
  Arguments argv = GetArgs();
  std::ifstream in(GetPath(g_customMapVotesFile.string).c_str());
  if (in.good()) {
    if (argv->size() == 1) {
      G_Printf("A custom map votes file exists. Are you "
               "sure you want to overwrite "
               "the write? Do /rcon generatecustomvotes -f "
               "if you are sure.");
      return;
    }
    if (argv->size() == 2 && argv->at(1) == "-f") {
      G_LogPrintf("Overwriting custom map votes file "
                  "with a default one.\n");
    } else {
      G_Printf("Unknown argument \"%s\".\n", argv->at(1).c_str());
      return;
    }
  }
  in.close();
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

  Json::StyledWriter writer;
  std::string output = writer.write(root);
  std::ofstream fOut(GetPath(g_customMapVotesFile.string).c_str());
  if (!fOut) {
    G_Printf("Couldn't open file \"%s\" defined in "
             "g_customMapVotesFile.\n",
             g_customMapVotesFile.string);
    return;
  }
  fOut << output;
  fOut.close();
  G_Printf("Generated new custom map votes file \"%s\"\n",
           g_customMapVotesFile.string);
  Load();
}

void CustomMapVotes::addCustomvoteList(int clientNum, const std::string &name,
                                       const std::string &fullName,
                                       const std::string &maps) {
  const std::string &customVotesFile = g_customMapVotesFile.string;
  Json::Value root;

  // read and append to existing file if present
  if (ETJump::JsonUtils::readFile(customVotesFile, root)) {

    // make sure we don't already have a list with this name
    for (const auto &lists : customMapVotes_) {
      if (name == lists.type) {
        Printer::SendChatMessage(clientNum,
                                 "^3add-customvote: ^7operation failed, check "
                                 "console for more information.");
        Printer::SendConsoleMessage(
            clientNum,
            ETJump::stringFormat("^3add-customvote: ^7a list with the name "
                                 "^3'%s' ^7already exists.\n",
                                 name));
        return;
      }
    }
  }

  Json::Value vote;
  vote["name"] = name;
  vote["callvote_text"] = fullName;
  vote["maps"] = Json::arrayValue;

  const auto mapList = ETJump::StringUtil::split(maps, " ");

  for (const auto &map : mapList) {
    vote["maps"].append(map);
  }

  root.append(vote);

  if (!ETJump::JsonUtils::writeFile(customVotesFile, root)) {
    Printer::SendChatMessage(clientNum, "^3add-customvote: ^7operation failed, "
                                        "check console for more information.");
    Printer::SendConsoleMessage(
        clientNum, ETJump::stringFormat("^3add-customvote: ^7couldn't open the "
                                        "file ^3'%s' ^7for writing.\n",
                                        customVotesFile));
    return;
  }

  Load();
  Printer::SendChatMessage(
      clientNum, ETJump::stringFormat("^3add-customvote: ^7successfully added "
                                      "a new custom vote list ^3'%s'",
                                      name));
}

void CustomMapVotes::deleteCustomvoteList(int clientNum,
                                          const std::string &name) {
  const std::string &customVotesFile = g_customMapVotesFile.string;
  Json::Value root;

  if (!ETJump::JsonUtils::readFile(customVotesFile, root)) {
    Printer::SendChatMessage(clientNum,
                             "^3delete-customvote: ^7operation failed, check "
                             "console for more information.");
    Printer::SendConsoleMessage(
        clientNum, ETJump::stringFormat("^3delete-customvote: ^7couldn't open "
                                        "the file ^3'%s' ^7for reading.\n",
                                        customVotesFile));
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
    Printer::SendChatMessage(
        clientNum, ETJump::stringFormat("^3delete-customvote: ^7a list with "
                                        "the name ^3'%s' ^7was not found.\n",
                                        name));
    return;
  }

  if (!ETJump::JsonUtils::writeFile(customVotesFile, root)) {
    Printer::SendChatMessage(clientNum,
                             "^3delete-customvote: ^7operation failed, check "
                             "console for more information.");
    Printer::SendConsoleMessage(
        clientNum, ETJump::stringFormat("^3delete-customvote: ^7couldn't open "
                                        "the file ^3'%s' ^7for writing.\n",
                                        customVotesFile));
    return;
  }

  Load();
  Printer::SendChatMessage(
      clientNum,
      ETJump::stringFormat(
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

  if (!ETJump::JsonUtils::readFile(customVotesFile, root)) {
    Printer::SendChatMessage(clientNum,
                             "^3edit-customvote: ^7operation failed, check "
                             "console for more information.");
    Printer::SendConsoleMessage(
        clientNum, ETJump::stringFormat("^3edit-customvote: ^7couldn't open "
                                        "the file ^3'%s' ^7for reading.\n",
                                        customVotesFile));
    return;
  }

  bool found = false;

  for (auto &object : root) {
    if (object["name"].asString() == list) {
      if (!name.empty()) {
        for (const auto &key : object) {
          if (key == object["name"].asString()) {
            object["name"] = name;
          }
        }
      }

      if (!fullName.empty()) {
        for (const auto &key : object) {
          if (key == object["callvote_text"].asString()) {
            object["callvote_text"] = fullName;
          }
        }
      }

      if (!addMaps.empty()) {
        const auto mapList = ETJump::StringUtil::split(addMaps, " ");

        for (const auto &map : mapList) {
          object["maps"].append(map);
        }
      }

      if (!removeMaps.empty()) {
        const auto mapList = ETJump::StringUtil::split(removeMaps, " ");

        for (const auto &map : mapList) {
          for (Json::ArrayIndex i = 0; i < object["maps"].size(); i++) {
            if (object["maps"][i].asString() == map) {
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
    Printer::SendChatMessage(
        clientNum, ETJump::stringFormat("^3edit-customvote: ^7a list with the "
                                        "name ^3'%s' ^7was not found.\n",
                                        list));
    return;
  }

  if (!ETJump::JsonUtils::writeFile(customVotesFile, root)) {
    Printer::SendChatMessage(clientNum,
                             "^3edit-customvote: ^7operation failed, check "
                             "console for more information.");
    Printer::SendConsoleMessage(
        clientNum, ETJump::stringFormat("^3edit-customvote: ^7couldn't open "
                                        "the file ^3'%s' ^7for writing.\n",
                                        customVotesFile));
    return;
  }

  Load();
  Printer::SendChatMessage(
      clientNum,
      ETJump::stringFormat(
          "^3edit-customvote: ^7successfully edited custom vote list ^3'%s'",
          list));
}

std::string CustomMapVotes::ListInfo(const std::string &type) {
  std::string buffer;

  for (auto &customMapVote : customMapVotes_) {
    if (customMapVote.type == type) {
      buffer += ETJump::stringFormat("^gMaps on the list ^3%s^g: \n", type);
      auto numMapsOnServer = customMapVote.mapsOnServer.size();

      int count = 0;
      for (auto &mapOnServer : customMapVote.mapsOnServer) {
        buffer += ETJump::stringFormat("^7%-30s", mapOnServer);

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
          buffer += ETJump::stringFormat("^9%-30s", mapNotOnServer);

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

std::string const CustomMapVotes::RandomMap(std::string const &type) {
  for (unsigned i = 0; i < customMapVotes_.size(); i++) {
    auto &customMapVote = customMapVotes_[i];
    if (customMapVote.type == type && customMapVote.mapsOnServer.size() > 0) {
      const int MAX_TRIES = 15;
      std::random_device rd;
      std::mt19937 re(rd());
      std::uniform_int_distribution<int> ui(
          0, customMapVote.mapsOnServer.size() - 1);

      // try to select a valid random map
      for (int tries = 0; tries < MAX_TRIES; tries++) {
        int testIdx = ui(re);
        if (isValidMap(customMapVote.mapsOnServer[testIdx])) {
          return customMapVote.mapsOnServer[testIdx];
        }
      }
      // fallback, iterate map list and select first
      // valid map
      for (int i = 0; i < static_cast<int>(customMapVote.mapsOnServer.size());
           i++) {
        if (isValidMap(customMapVote.mapsOnServer[i])) {
          return customMapVote.mapsOnServer[i];
        }
      }

      return "";
    }
  }
  return "";
}

// FIXME: this should just be nuked and custom map votes should use
//  MapStatistics::isValidMap but that takes MapInfo as argument
//  and isn't simple to refactor so bleh
bool CustomMapVotes::isValidMap(const std::string &mapName) {
  return G_MapExists(mapName.c_str()) && mapName != level.rawmapname &&
         !MapStatistics::isBlockedMap(mapName);
}
