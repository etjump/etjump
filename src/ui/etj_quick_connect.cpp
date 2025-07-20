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

#include "etj_quick_connect.h"

#include "../game/etj_json_utilities.h"
#include "../game/etj_filesystem.h"

namespace ETJump {
QuickConnect::QuickConnect() : selectedServer(-1) {
  servers.reserve(MAX_QUICKCONNECT_SERVERS);

  if (parseServers()) {
    updateServerNames();
    updateLabels();
  }
}

bool QuickConnect::parseServers() {
  if (!FileSystem::exists(QUICKCONNECT_FILE)) {
    return false;
  }

  Json::Value root;
  std::string err;

  if (!JsonUtils::readFile(QUICKCONNECT_FILE, root, &err)) {
    uiInfo.uiDC.Print(S_COLOR_YELLOW
                      "%s: Failed to parse quick connect file:\n%s\n",
                      __func__, err.c_str());
    return false;
  }

  const int max =
      std::min(MAX_QUICKCONNECT_SERVERS, static_cast<int>(root.size()));
  servers.clear();

  for (int i = 0; i < max; i++) {
    QuickConnectServer server{};
    const Json::Value &current = root[i];

    if (!JsonUtils::parseValue(server.serverName, current["serverName"], &err,
                               "serverName") ||
        !JsonUtils::parseValue(server.ip, current["ip"], &err, "ip") ||
        !JsonUtils::parseValue(server.password, current["password"], &err,
                               "password") ||
        !JsonUtils::parseValue(server.customName, current["customName"], &err,
                               "customName")) {
      // + 1 for user-facing index
      uiInfo.uiDC.Print(
          S_COLOR_YELLOW
          "%s: Failed to parse server #%i from quick connect file:\n%s\n",
          __func__, i + 1, err.c_str());
      continue;
    }

    servers.push_back(server);
  }

  return true;
}

void QuickConnect::refreshServers(bool force) {
  bool updated = false;

  // reset all requests and invalidate data
  if (force) {
    trap_LAN_ServerStatus(nullptr, nullptr, 0);

    for (auto &server : servers) {
      server.valid = false;
    }

    updateLabels();
  }

  for (auto &server : servers) {
    if (server.ip.empty()) {
      continue;
    }

    if (!force) {
      if (!server.nextRefresh || server.nextRefresh > uiInfo.uiDC.realTime) {
        continue;
      }
    }

    if (getServerInfo(server)) {
      server.nextRefresh = 0;
      // reset the requests for this server
      trap_LAN_ServerStatus(server.ip.c_str(), nullptr, 0);
      updated = true;
    } else {
      server.nextRefresh = uiInfo.uiDC.realTime + 500;
    }
  }

  if (updated) {
    updateServerNames();
    updateLabels();
  }
}

bool QuickConnect::getServerInfo(QuickConnectServer &info) {
  char status[MAX_SERVERSTATUS_TEXT]{};

  // a status request is already pending, but response hasn't arrived
  if (!trap_LAN_ServerStatus(info.ip.c_str(), status, sizeof(status))) {
    return false;
  }

  info.serverName = Info_ValueForKey(status, "sv_hostname");
  info.map = Info_ValueForKey(status, "mapname");
  info.maxClients =
      static_cast<uint8_t>(Q_atoi(Info_ValueForKey(status, "sv_maxclients")));
  info.players = getPlayerCount(status);
  info.valid = true;

  return true;
}

uint8_t QuickConnect::getPlayerCount(const std::string &serverStatus) {
  uint8_t count = 0;

  // The "players" part of the server status response starts after a set of
  // double backslashes. Each player is then separated by a single backslash.
  // Therefore the simplest way to count the players on the server is to
  // just find the set of double backslashes, and then count the number of
  // single backslashes in the remaining string. If the server has no players,
  // the double backslash is the end of the string.
  std::string playersStr = serverStatus.substr(serverStatus.find("\\\\") + 2);
  size_t pos = 0;

  while ((pos = playersStr.find('\\', pos)) != std::string::npos) {
    count++;
    pos++;
  }

  return count;
}

bool QuickConnect::serverExists(const std::string &address) const {
  // this is very novel and will allow duplicates if using IP and domain,
  // or address with and without port, but it's not really dangerous
  // and really not worth the effort to make this more robust
  // we couldn't even distinguish domain from IP without implementing
  // out own custom solution, since there's no syscall to do that in engine
  return std::any_of(servers.cbegin(), servers.cend(),
                     [&address](const QuickConnectServer &server) {
                       return server.ip == address;
                     });
}

void QuickConnect::updateServerNames() const {
  Json::Value root{};
  std::string err;

  if (!FileSystem::exists(QUICKCONNECT_FILE)) {
    uiInfo.uiDC.Print(S_COLOR_YELLOW
                      "%s: Failed to update quick connect server names - file "
                      "'%s' does not exist.\n",
                      __func__, QUICKCONNECT_FILE);
    return;
  }

  if (!JsonUtils::readFile(QUICKCONNECT_FILE, root, &err)) {
    uiInfo.uiDC.Print(
        S_COLOR_YELLOW
        "%s: Failed to open quick connect file for reading:\n%s\n",
        __func__, err.c_str());
    return;
  }

  for (auto &server : root) {
    for (const auto &srv : servers) {
      if (srv.ip == server["ip"].asString()) {
        server["serverName"] = srv.serverName;
        break;
      }
    }
  }

  if (!JsonUtils::writeFile(QUICKCONNECT_FILE, root, &err)) {
    uiInfo.uiDC.Print(S_COLOR_YELLOW
                      "%s: Failed to write quick connect file:\n%s",
                      __func__, err.c_str());
  }
}

void QuickConnect::updateLabels() {
  const menuDef_t *menu = Menus_FindByName("main");

  if (!menu) {
    return;
  }

  int numParsed = 0;

  for (int i = 0; i < menu->itemCount; i++) {
    if (numParsed == static_cast<int>(servers.size())) {
      break;
    }

    auto *const item = menu->items[i];
    std::string name = item->window.name;

    // NOTE: only handles a single digit, if we were want to support
    // more than 9 quick connect servers, this needs to be updated!
    if (StringUtil::startsWith(name, "lblQuickConnectServer")) {
      // menu entries are 1-indexed
      char buf[2] = {name.back(), '\0'};
      int index = Q_atoi(buf) - 1;

      buildLabelString(index);
      item->text = serverLabels[index];
      numParsed++;
      continue;
    }
  }
}

void QuickConnect::buildLabelString(const int index) {
  // for safety
  if (index < 0 || index > MAX_QUICKCONNECT_SERVERS) {
    uiInfo.uiDC.Print(
        S_COLOR_RED
        "%s: Unable to build quick connect label - index '%i' out of range!\n",
        __func__, index);
    return;
  }

  bool infoValid = servers[index].valid;
  bool hasCustomName = !servers[index].customName.empty();

  const std::string line1 = StringUtil::truncate(
      hasCustomName ? servers[index].customName : servers[index].serverName,
      MAX_LABEL_LINE_CHARS);

  const std::string playerCountStr = stringFormat(
      " (%s/%s)", infoValid ? std::to_string(servers[index].players) : "-",
      infoValid ? std::to_string(servers[index].maxClients) : "-");
  const std::string mapStr =
      StringUtil::truncate(infoValid ? servers[index].map : "-",
                           MAX_LABEL_LINE_CHARS - playerCountStr.length());

  const std::string line2 = stringFormat("%s%s", mapStr, playerCountStr);

  Com_sprintf(serverLabels[index], sizeof(serverLabels[index]),
              stringFormat("%s\n%s", line1, line2).c_str());
}

void QuickConnect::addServer(const std::string &address,
                             const std::string &password,
                             const std::string &customName) {
  Json::Value root{};
  std::string err;

  // append to existing file if present
  if (FileSystem::exists(QUICKCONNECT_FILE)) {
    if (JsonUtils::readFile(QUICKCONNECT_FILE, root, &err)) {
      // this should already be taken care of by the button in UI,
      // but let's check so that we don't somehow end up with too many entires
      if (root.size() >= MAX_QUICKCONNECT_SERVERS) {
        uiInfo.uiDC.Print(
            S_COLOR_YELLOW
            "You may not add more than %i quick connect servers.\n",
            MAX_QUICKCONNECT_SERVERS);
        return;
      }

      if (serverExists(address)) {
        uiInfo.uiDC.Print(S_COLOR_YELLOW "Server '%s' is already added.\n",
                          address.c_str());
        return;
      }

    } else {
      uiInfo.uiDC.Print(
          S_COLOR_YELLOW
          "%s: Failed to open quick connect file for reading:\n%s\n",
          __func__, err.c_str());
      return;
    }
  }

  Json::Value server{};

  // we cache the server name here after update, so the menu can display
  // the server name for the user even if they haven't updated the list yet,
  // (e.g. after a fresh start) and custom name is blank
  server["serverName"] = "";
  server["password"] = password;
  server["customName"] = customName;
  server["ip"] = address;

  root.append(server);

  if (JsonUtils::writeFile(QUICKCONNECT_FILE, root, &err)) {
    uiInfo.uiDC.Print("Added new server to quick connect with IP '%s'\n",
                      address.c_str());
  } else {
    uiInfo.uiDC.Print(S_COLOR_YELLOW
                      "%s: Failed to write quick connect file:\n%s",
                      __func__, err.c_str());
  }

  if (parseServers()) {
    refreshServers(true);
  }
}

void QuickConnect::editServer() {
  Json::Value root{};
  std::string err;

  if (!FileSystem::exists(QUICKCONNECT_FILE)) {
    uiInfo.uiDC.Print(
        S_COLOR_YELLOW
        "%s: Failed to edit quick connect server - file '%s' does not exist.\n",
        __func__, QUICKCONNECT_FILE);
    return;
  }

  if (!JsonUtils::readFile(QUICKCONNECT_FILE, root, &err)) {
    uiInfo.uiDC.Print(
        S_COLOR_YELLOW
        "%s: Failed to open quick connect file for reading:\n%s\n",
        __func__, err.c_str());
    return;
  }

  char buf[MAX_CVAR_VALUE_STRING]{};

  trap_Cvar_VariableStringBuffer("ui_quickconnect_ip", buf, sizeof(buf));

  // TODO: maybe make this a popup window some day
  if (servers[selectedServer].ip != buf && serverExists(buf)) {
    uiInfo.uiDC.Print(S_COLOR_YELLOW "Server '%s' is already added.\n", buf);
    return;
  }

  root[selectedServer]["ip"] = buf;

  trap_Cvar_VariableStringBuffer("ui_quickconnect_password", buf, sizeof(buf));
  root[selectedServer]["password"] = buf;

  trap_Cvar_VariableStringBuffer("ui_quickconnect_customname", buf,
                                 sizeof(buf));
  root[selectedServer]["customName"] = buf;

  if (!JsonUtils::writeFile(QUICKCONNECT_FILE, root, &err)) {
    uiInfo.uiDC.Print(S_COLOR_YELLOW
                      "%s: Failed to write quick connect file:\n%s",
                      __func__, err.c_str());
    return;
  }

  if (parseServers()) {
    refreshServers(true);
  }

  selectedServer = -1;
}

void QuickConnect::deleteServer() {
  Json::Value root{};
  std::string err;

  if (!FileSystem::exists(QUICKCONNECT_FILE)) {
    uiInfo.uiDC.Print(S_COLOR_YELLOW "%s: Failed to delete quick connect "
                                     "server - file '%s' does not exist.\n",
                      __func__, QUICKCONNECT_FILE);
    return;
  }

  if (!JsonUtils::readFile(QUICKCONNECT_FILE, root, &err)) {
    uiInfo.uiDC.Print(
        S_COLOR_YELLOW
        "%s: Failed to open quick connect file for reading:\n%s\n",
        __func__, err.c_str());
    return;
  }

  // + 1 for user-facing index
  if (!root.removeIndex(selectedServer, nullptr)) {
    uiInfo.uiDC.Print(
        S_COLOR_YELLOW
        "%s: Failed to delete server - no server saved on slot '%i'\n",
        __func__, selectedServer + 1);
    selectedServer = -1;
    return;
  }

  if (!JsonUtils::writeFile(QUICKCONNECT_FILE, root, &err)) {
    uiInfo.uiDC.Print(S_COLOR_YELLOW
                      "%s: Failed to write quick connect file:\n%s",
                      __func__, err.c_str());
    return;
  }

  // clear all the labels to not display wrong info
  for (auto &label : serverLabels) {
    std::memset(label, 0, sizeof(label));
  }

  if (parseServers()) {
    refreshServers(true);
  }

  selectedServer = -1;
}

void QuickConnect::setEditData(const int index) {
  // for safety
  if (index < 0 || index > MAX_QUICKCONNECT_SERVERS) {
    uiInfo.uiDC.Print(S_COLOR_RED "%s: Unable to set quick connect edit data - "
                                  "index '%i' out of range!\n",
                      __func__, index);
    return;
  }

  // + 1 for user-facing index
  if (index > static_cast<int>(servers.size())) {
    uiInfo.uiDC.Print(S_COLOR_RED
                      "%s: No quick connect server saved at slot '%i'\n",
                      __func__, index + 1);
    return;
  }

  selectedServer = index;
  trap_Cvar_Set("ui_quickconnect_ip", servers[index].ip.c_str());
  trap_Cvar_Set("ui_quickconnect_password", servers[index].password.c_str());
  trap_Cvar_Set("ui_quickconnect_customname",
                servers[index].customName.c_str());
}

bool QuickConnect::isFull() const {
  return servers.size() >= MAX_QUICKCONNECT_SERVERS;
}

int QuickConnect::getServerCount() const {
  return static_cast<int>(servers.size());
}

std::string QuickConnect::buildConnectCommand(const int index) const {
  // for safety
  if (index < 0 || index > MAX_QUICKCONNECT_SERVERS) {
    uiInfo.uiDC.Print(S_COLOR_RED "%s: Unable to build quick connect command - "
                                  "index '%i' out of range!\n",
                      __func__, index);
    return "";
  }

  // + 1 for user-facing index
  if (index > static_cast<int>(servers.size())) {
    uiInfo.uiDC.Print(S_COLOR_RED
                      "%s: No quick connect server saved at slot '%i'\n",
                      __func__, index + 1);
    return "";
  }

  std::string command = "connect " + servers[index].ip;

  if (!servers[index].password.empty()) {
    command += ";password " + servers[index].password;
  }

  return command + '\n';
}
} // namespace ETJump
