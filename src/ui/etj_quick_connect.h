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

#include "ui_local.h"

namespace ETJump {
inline constexpr char QUICKCONNECT_FILE[] = "quickconnect.dat";
inline constexpr int MAX_LABEL_LINE_CHARS = 30;

class QuickConnect {
public:
  QuickConnect();
  ~QuickConnect() = default;

  void refreshServers(bool force);
  void addServer(const std::string &address, const std::string &password,
                 const std::string &customName);
  void deleteServer();
  void editServer();
  void setEditData(int index);
  [[nodiscard]] bool isFull() const;
  [[nodiscard]] int getServerCount() const;

  // if successful, returns a newline-terminated string
  // if index is invalid, returns an empty string
  [[nodiscard]] std::string buildConnectCommand(int index) const;

  // label texts are not fixed sized arrays, they are normally allocated
  // to the UI string pool with 'String_Alloc'. Because we're potentially
  // editing the label texts multiple times on one session, we hold the
  // label texts in a separate locations here, to avoid gradually filling
  // up the string pool when the server infos get updated,
  // since there's no way to free a string from the pool.
  std::array<char[MAX_CVAR_VALUE_STRING], MAX_QUICKCONNECT_SERVERS>
      serverLabels{};

  bool initialRefreshDone;

private:
  struct QuickConnectServer {
    std::string serverName;
    std::string customName;
    std::string ip;
    std::string map;
    std::string password;
    uint8_t players;
    uint8_t maxClients;
    bool valid;      // true once info has been fetched
    int nextRefresh; // 0 if not refreshing
  };

  bool parseServers(); // true if parsing was successful
  static bool getServerInfo(QuickConnectServer &info);
  static uint8_t getPlayerCount(const std::string &serverStatus);
  [[nodiscard]] bool serverExists(const std::string &address) const;
  void updateServerNames() const;
  void updateLabels();

  // NOTE: performs no validation for whether the index is valid,
  // ensure the server exists before calling this!
  void buildLabelString(int index);

  std::vector<QuickConnectServer> servers;
  int selectedServer; // the currently selected server for editing/deletion
};
} // namespace ETJump
