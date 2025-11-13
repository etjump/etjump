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

#include <string>
#include <vector>
#include <bitset>
#include <unordered_map>

#include "etj_local.h"
#include "etj_levels.h"
#include "etj_database.h"
#include "etj_iauthentication.h"

class Session {
public:
  static const unsigned MAX_COMMANDS = 256;
  Session(std::shared_ptr<IAuthentication> database);
  ~Session();
  void ResetClient(int clientNum);

  struct Client {
    Client();
    std::string guid;
    std::string hwid;
    std::bitset<MAX_COMMANDS> permissions;
    std::string ip;
    int sessionStartTime; // FIXME: 32-bit time
    const User_s *user;
    const Levels::Level *level;
  };

  void Init(int clientNum);
  void ReadSessionData(int clientNum);
  void WriteSessionData(int clientNum);
  void GetUserAndLevelData(int clientNum);
  bool GuidReceived(gentity_t *ent);
  void OnGuidReceived(gentity_t *ent);
  void PrintGreeting(gentity_t *ent);
  void OnClientDisconnect(int clientNum);
  std::string Guid(const gentity_t *ent) const;
  std::bitset<256> Permissions(gentity_t *ent) const;
  int GetLevel(gentity_t *ent) const;
  int GetId(gentity_t *ent) const;
  int GetId(int clientNum) const;
  gentity_t *gentityFromId(unsigned id);
  bool SetLevel(gentity_t *target, int level);
  bool SetLevel(int id, int level);
  int GetLevelById(unsigned id) const;
  bool UserExists(unsigned id);
  std::string GetMessage() const;
  void PrintAdmintest(gentity_t *ent);
  void PrintFinger(gentity_t *ent, gentity_t *target);
  bool Ban(gentity_t *ent, gentity_t *player, unsigned expires,
           std::string reason);
  bool IsIpBanned(int clientNum);
  void ParsePermissions(int clientNum);
  bool HasPermission(gentity_t *ent, char flag);
  void NewName(gentity_t *ent);
  // Returns the amount of users with that level
  int LevelDeleted(int level);
  std::vector<Session::Client *> FindUsersByLevel(int level);
  int getSessionStartTime(int clientNum) const;

private:
  std::shared_ptr<IAuthentication> database_;

  void UpdateLastSeen(int clientNum);
  Client clients_[MAX_CLIENTS];
  std::string message_;
};
