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

#ifdef NEW_AUTH
  #include <memory>
  #include <bitset>

  #include "etj_user_repository.h"
  #include "etj_user_v2.h"
  #include "etj_log.h"
  #include "etj_synchronization_context.h"
  #include "q_shared.h"

namespace ETJump {
class SessionV2 {
public:
  SessionV2(std::unique_ptr<UserRepository> userRepository,
            std::unique_ptr<Log> log,
            std::unique_ptr<SynchronizationContext> synchronizationContext);
  ~SessionV2();

  void runFrame() const;

  void initClientSession(int clientNum);
  void resetClient(int clientNum);
  void onClientConnect(int clientNum, bool firstTime);
  void onClientDisconnect(int clientNum);
  void onClientBegin(const gentity_t *ent);
  void onAuthSuccess(int32_t clientNum);

  bool authenticate(gentity_t *ent);
  bool migrateGuid(gentity_t *ent);
  void addNewUser(gentity_t *ent);

  void readSessionData(int clientNum);
  void writeSessionData() const;

private:
  static constexpr int MAX_COMMANDS = 256;
  static constexpr char SESSION_FILE[] = "session/client_%02i.json";

  struct Client {
    std::string guid;
    int platform;
    std::vector<std::string> hwid;
    std::bitset<MAX_COMMANDS> permissions;
    std::string ipv4;
    std::string ipv6;
    int64_t sessionStartTime{};
    std::unique_ptr<UserV2> user;
  };

  void updateHWID(int clientNum, int userID) const;
  void updateLastKnownIP(int clientNum, int userID) const;

  void checkIPBan(int clientNum) const;
  bool isBanned(int clientNum, int userID,
                const std::string &legacyGuid = "") const;
  void dropBannedClient(int clientNum) const;

  void printGreeting(int32_t clientNum) const;

  // TODO: this could maybe be a map or a vector rather than an array,
  //  we don't actually need 64 copies of 'Client' like, ever
  std::array<Client, MAX_CLIENTS> clients{};

  std::unique_ptr<UserRepository> repository;
  std::unique_ptr<Log> logger;
  std::unique_ptr<SynchronizationContext> sc;
};
} // namespace ETJump

#endif
