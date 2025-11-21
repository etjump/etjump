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

#ifdef NEW_AUTH

  #include "etj_session_v2.h"
  #include "etj_session_result_types.h"
  #include "g_local.h"
  #include "etj_printer.h"
  #include "etj_crypto.h"
  #include "etj_filesystem.h"
  #include "etj_game.h"
  #include "etj_local.h"
  #include "etj_timerun_v2.h"
  #include "etj_motd.h"
  #include "utilities.hpp"
  #include "etj_json_utilities.h"

namespace ETJump {
inline constexpr char SESSION_FILE[] = "session/client_%02i.json";

SessionV2::SessionV2(
    std::unique_ptr<UserRepository> userRepository, std::unique_ptr<Log> log,
    std::unique_ptr<SynchronizationContext> synchronizationContext)
    : repository(std::move(userRepository)), logger(std::move(log)),
      sc(std::move(synchronizationContext)) {
  sc->startWorkerThreads(1);
}

SessionV2::~SessionV2() {
  repository = nullptr;
  logger = nullptr;
  sc->stopWorkerThreads();
  sc = nullptr;
}

void SessionV2::runFrame() const { sc->processCompletedTasks(); }

void SessionV2::initClientSession(const int clientNum) {
  resetClient(clientNum);

  char userinfo[MAX_INFO_STRING]{};
  trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));
  const std::string ip =
      Utilities::stripIPAddressPort(Info_ValueForKey(userinfo, "ip"));

  // treat listen servers as IPv4, as that's more likely correct
  // there's no way to know for sure whether localhost is IPv4 or IPv6,
  // but this really doesn't matter
  if (ip.find('.') != std::string::npos || ip == "localhost") {
    clients[clientNum].ipv4 = ip;
  } else {
    clients[clientNum].ipv6 = ip;
  }

  time_t t;
  t = std::time(&t);
  clients[clientNum].sessionStartTime = t;
}

void SessionV2::resetClient(const int clientNum) {
  clients[clientNum].guid.clear();
  clients[clientNum].platform = Constants::OS_DEFAULT;
  clients[clientNum].hwid.clear();
  clients[clientNum].permissions.reset();
  clients[clientNum].ipv4.clear();
  clients[clientNum].ipv6.clear();
  clients[clientNum].sessionStartTime = 0;
  clients[clientNum].permissions.reset();

  clients[clientNum].user = nullptr;
  clients[clientNum].level = nullptr;
}

void SessionV2::onClientConnect(const int clientNum, const bool firstTime) {
  G_DPrintf("%s called for %i\n", __func__, clientNum);

  if (firstTime) {
    initClientSession(clientNum);
    checkIPBan(clientNum);
  } else {
    if (readSessionData(clientNum)) {
      onAuthSuccess(clientNum);
    }
  }
}

void SessionV2::onClientDisconnect(const int clientNum) {
  G_DPrintf("%s called for %i\n", __func__, clientNum);
  updateLastSeen(clientNum);
  resetClient(clientNum);

  // TODO: this is probably unnecessary and could be removed,
  //  there will only ever be max 64 of these files anyway
  FileSystem::remove(stringFormat(SESSION_FILE, clientNum));
}

void SessionV2::onClientBegin(const gentity_t *ent) {
  const int32_t clientNum = ClientNum(ent);

  G_DPrintf("%s called by %i\n", __func__, clientNum);

  if (clients[clientNum].guid.empty()) {
    G_DPrintf("Requesting authentication from %i\n", clientNum);
    Printer::command(clientNum, Constants::Authentication::AUTH_REQUEST);
  }

  if (!ent->client->sess.motdPrinted) {
    game.motd->printMotd(ent);
    ent->client->sess.motdPrinted = qtrue;
  }

  ETJump::Log::processMessages();
}

void SessionV2::onAuthSuccess(const int32_t clientNum) {
  const gentity_t *ent = g_entities + clientNum;
  const std::string guid = clients[clientNum].guid;
  const bool firstTime = ent->client->sess.firstTime;
  const std::string func = __func__;

  sc->postTask(
      [this, clientNum, guid] {
        const auto user = repository->getUserData(clients[clientNum].guid);

        // this should never happen, because we have authenticated already
        if (user.id == 0) {
          throw std::runtime_error(
              stringFormat("Failed to get user data for GUID %s", guid));
        }

        return std::make_unique<GetUserDataResult>(user);
      },
      [this, clientNum, ent, firstTime](
          std::unique_ptr<SynchronizationContext::ResultBase> userData) {
        auto *const r = dynamic_cast<GetUserDataResult *>(userData.get());

        if (r == nullptr) {
          throw std::runtime_error("GetUserDataResult is null.");
        }

        clients[clientNum].user = std::make_unique<UserV2>();
        clients[clientNum].user->id = r->user.id;
        clients[clientNum].user->name = r->user.name;
        clients[clientNum].user->greeting = r->user.greeting;
        clients[clientNum].user->title = r->user.title;
        clients[clientNum].user->lastSeen = r->user.lastSeen;
        clients[clientNum].user->commands = r->user.commands;

        clients[clientNum].level = game.levels->GetLevel(r->user.level);

        if (firstTime) {
          printGreeting(clientNum);
          storeNewName(ent);

          if (r->user.lastSeen > 0) {
            Printer::popup(
                clientNum,
                stringFormat(
                    "^5Your last visit was on %s.",
                    Time::fromInt(r->user.lastSeen).toDateTimeString()));
          }

          // mutes expire once map changes, so it only makes sense to check
          // this if the client is connecting for the first time
          if (isMuted(clientNum)) {
            ent->client->sess.muted = qtrue;
          }
        }

        parsePermissions(clientNum);
        game.timerunV2->clientConnect(clientNum, r->user.id);
      },
      [this, clientNum, func](const std::runtime_error &e) {
        logger->error("%s failed: %s", func, e.what());
      });
}

bool SessionV2::authenticate(const gentity_t *ent) {
  const int argc = trap_Argc();
  const int clientNum = ClientNum(ent);
  const std::string cleanName = sanitize(ent->client->pers.netname);
  const std::string spoofAttempt = stringFormat(
      "authentication: Potential GUID/HWID spoof attempt by %i %s (%s)",
      clientNum, cleanName, getIP(clientNum));

  // auth response is 'auth <GUID> <OS> <HWIDs>'
  // the amount of HWIDs is dependent on OS
  char osBuf[8]{};
  trap_Argv(2, osBuf, sizeof(osBuf));
  clients[clientNum].platform = Q_atoi(osBuf);

  int numHWIDs = 0;

  switch (clients[clientNum].platform) {
    case Constants::OS_WIN_X86:
    case Constants::OS_WIN_X86_64:
      numHWIDs = Constants::Authentication::HWID_SIZE_WIN;
      break;
    case Constants::OS_LINUX_X86:
    case Constants::OS_LINUX_X86_64:
      numHWIDs = Constants::Authentication::HWID_SIZE_LINUX;
      break;
    case Constants::OS_MACOS_X86_64:
    case Constants::OS_MACOS_AARCH64:
      numHWIDs = Constants::Authentication::HWID_SIZE_MAC;
      break;
    default:
      break;
  }

  const int NUM_EXPECTED_ARGS = 3 + numHWIDs;

  if (argc != NUM_EXPECTED_ARGS) {
    Printer::logAdminLn(spoofAttempt);
    return false;
  }

  std::vector<std::string> hwid{};
  hwid.reserve(numHWIDs);
  bool hwidValid = true;

  char guidBuf[MAX_CVAR_VALUE_STRING]{};
  char hwidBuf[MAX_CVAR_VALUE_STRING]{};

  for (int i = 0; i < numHWIDs; i++) {
    trap_Argv(3 + i, hwidBuf, sizeof(hwidBuf));

    if (!Crypto::isValidSHA2(hwidBuf)) {
      hwidValid = false;
      break;
    }

    hwid.emplace_back(Crypto::sha2(hwidBuf));
  }

  trap_Argv(1, guidBuf, sizeof(guidBuf));

  if (!hwidValid || !Crypto::isValidSHA2(guidBuf)) {
    Printer::logAdminLn(spoofAttempt);
    return false;
  }

  clients[clientNum].guid = Crypto::sha2(guidBuf);
  clients[clientNum].hwid = hwid;

  const std::string func = __func__;

  sc->postTask(
      [this, clientNum] {
        const auto user = repository->getUserData(clients[clientNum].guid);

        if (user.guid.empty()) {
          // user.id will be 0 here as it returns an empty user
          return std::make_unique<AuthenticationResult>(user.id, false);
        }

        // check after user data is successfully fetched so we have valid GUID
        if (isBanned(clientNum, user.id)) {
          return std::make_unique<AuthenticationResult>(0, true);
        }

        updateHWID(clientNum, user.id);
        updateLastKnownIP(clientNum, user.id);

        return std::make_unique<AuthenticationResult>(user.id, false);
      },
      [this, clientNum, cleanName,
       func](std::unique_ptr<SynchronizationContext::ResultBase> userData) {
        const auto result =
            dynamic_cast<AuthenticationResult *>(userData.get());

        if (result == nullptr) {
          throw std::runtime_error("AuthenticationResult is null.");
        }

        if (result->isBanned) {
          dropBannedClient(clientNum);
          return;
        }

        if (result->userID == 0) {
          logger->info("Client %i %s with GUID %s not found in the database, "
                       "sending migration request",
                       clientNum, cleanName, clients[clientNum].guid);

          Printer::command(clientNum,
                           Constants::Authentication::GUID_MIGRATE_REQUEST);
        } else {
          onAuthSuccess(clientNum);
        }
      },
      [this, clientNum, cleanName](const std::runtime_error &e) {
        logger->error("Failed to get data for user %i %s - %s", clientNum,
                      cleanName, e.what());
      });

  return true;
}

bool SessionV2::migrateGuid(const gentity_t *ent) {
  const int argc = trap_Argc();
  constexpr int NUM_EXPECTED_ARGS = 3;
  const int clientNum = ClientNum(ent);
  const std::string cleanName = sanitize(ent->client->pers.netname);
  const std::string spoofAttempt = stringFormat(
      "authentication: Potential GUID migration spoof attempt by %i %s (%s)",
      clientNum, cleanName.c_str(), getIP(clientNum));

  // client sends 'guid_migrate <oldGuid> <migrationType>'
  if (argc != NUM_EXPECTED_ARGS) {
    Printer::logAdminLn(spoofAttempt);
    return false;
  }

  char guidBuf[MAX_TOKEN_CHARS]{};
  trap_Argv(1, guidBuf, sizeof(guidBuf));

  if (!Crypto::isValidSHA1(guidBuf)) {
    Printer::logAdminLn(spoofAttempt);
    return false;
  }

  const std::string oldGuid = Crypto::sha1(guidBuf);

  char typeBuf[2]{};
  trap_Argv(2, typeBuf, sizeof(typeBuf));
  const auto migrationType =
      static_cast<Constants::Authentication::MigrationType>(Q_atoi(typeBuf));

  sc->postTask(
      [this, oldGuid, clientNum, migrationType] {
        const auto legacyAuth = repository->getLegacyAuthData(oldGuid);
        const auto user = repository->getUserData(legacyAuth.userID);

        // this can happen if the user tries to authenticate using
        // an old GUID that has never been registered on the server,
        // and is also visiting for the first time (new GUID not in the db)
        if (user.id == 0) {
          return std::make_unique<GuidMigrationResult>(user.id, false, "");
        }

        if (isBanned(clientNum, user.id, oldGuid)) {
          return std::make_unique<GuidMigrationResult>(user.id, true, "");
        }

        // forced manual migration overrides the GUID in the database,
        // otherwise we only perform migration if the GUID in database is empty
        if (migrationType !=
                Constants::Authentication::MigrationType::MANUAL_FORCE &&
            !user.guid.empty()) {
          return std::make_unique<GuidMigrationResult>(
              user.id, false,
              "Already authenticated with new GUID. You may force a migration "
              "with 'migrateGuid -f'.\nNote that this will delete the data "
              "associated with your current GUID.\n");
        }

        repository->migrateGuid(user.id, clients[clientNum].guid);

        // update HWID + last know IP, since we didn't do this during auth
        updateHWID(clientNum, user.id);
        updateLastKnownIP(clientNum, user.id);

        return std::make_unique<GuidMigrationResult>(
            user.id, false, "Successfully migrated old GUID.\n");
      },
      [this, clientNum, cleanName](
          std::unique_ptr<SynchronizationContext::ResultBase> legacyAuthData) {
        const auto result =
            dynamic_cast<GuidMigrationResult *>(legacyAuthData.get());

        if (result == nullptr) {
          throw std::runtime_error("GuidMigrationResults is null.");
        }

        if (result->isBanned) {
          dropBannedClient(clientNum);
          return;
        }

        if (result->userID == 0) {
          addNewUser(clientNum);
        } else {
          logger->info(
              "GUID migration successfully performed for client %i %s.",
              clientNum, cleanName);
          Printer::console(clientNum, result->message);
          onAuthSuccess(clientNum);
        }
      },
      [this, clientNum, cleanName](const std::runtime_error &e) {
        Printer::console(
            clientNum, stringFormat("Failed to migrate GUID: %s\n", e.what()));
        logger->error("GUID migration failed for client %i %s: %s", clientNum,
                      cleanName, e.what());
      });

  return true;
}

void SessionV2::addNewUser(const int32_t clientNum) {
  const std::string name = (g_entities + clientNum)->client->pers.netname;
  const std::string guid = clients[clientNum].guid;
  const std::string ipv4 = clients[clientNum].ipv4;
  const std::string ipv6 = clients[clientNum].ipv6;

  sc->postTask(
      [this, clientNum, name, guid, ipv4, ipv6] {
        UserModels::User user{};
        user.name = name;
        user.guid = guid;
        user.ipv4 = ipv4;
        user.ipv6 = ipv6;

        repository->addNewUser(user);

        // now that the user is in the db, we have a valid ID,
        // so we can store the HWIDs for the current user ID
        const int userID = repository->getUserID(guid);

        // TODO: remove the entry from DB if this fails?
        if (userID == 0) {
          throw std::runtime_error(
              stringFormat("Failed to get user id for GUID '%s'.", guid));
        }

        updateHWID(clientNum, userID);
        updateLastKnownIP(clientNum, userID);

        return std::make_unique<AddUserResult>(
            userID, "Successfully added new user to the database.");
      },
      [this, clientNum](
          std::unique_ptr<SynchronizationContext::ResultBase> addUserResult) {
        const auto result = dynamic_cast<AddUserResult *>(addUserResult.get());

        if (result == nullptr) {
          throw std::runtime_error("AddUserResult is null.");
        }

        logger->info(result->message);
        onAuthSuccess(clientNum);
      },
      [this](const std::runtime_error &e) {
        logger->error("Failed to add new user: %s", e.what());
      });
}

// NOTE: this performs database operations, don't call this on the main thread!
void SessionV2::updateHWID(const int clientNum, const int userID) const {
  const int platform = clients[clientNum].platform;
  // since we're potentially updating HWID, we only need to check
  // the existing ones for the users current platform
  const auto userHWIDs = repository->getHWIDsForUsersPlatform(userID, platform);
  const std::string currentHWID =
      StringUtil::join(clients[clientNum].hwid, ",");

  const auto addHWID = [&] {
    UserModels::UserHWID hwid{};
    hwid.userID = userID;
    hwid.platform = platform;
    hwid.hwid = currentHWID;
    repository->addHwid(hwid);
  };

  // nothing in the database, add the current one
  if (userHWIDs.empty()) {
    addHWID();
    return;
  }

  // add the current one if it's not found in the database
  for (const auto &hwid : userHWIDs) {
    if (hwid.hwid != currentHWID) {
      addHWID();
      break;
    }
  }
}

// NOTE: this performs database operations, don't call this on the main thread!
void SessionV2::updateLastKnownIP(const int clientNum, const int userID) const {
  // a client connects using either IPv4 or IPv6,
  // so only one of these is filled during a session
  // TODO: do we want to exclude 'localhost' here?
  if (!clients[clientNum].ipv4.empty()) {
    repository->updateIPv4(userID, clients[clientNum].ipv4);
  } else {
    repository->updateIPv6(userID, clients[clientNum].ipv6);
  }
}

void SessionV2::updateLastSeen(const int32_t clientNum) {
  if (clients[clientNum].user == nullptr) {
    logger->error("%s: clientNum %i has a null 'user'.", __func__, clientNum);
    return;
  }

  time_t t = 0;
  t = std::time(&t);

  // capture this in the lambdas, as we reset the client right after calling
  // this, so the id might not be valid if we try to use it directly
  const int32_t id = clients[clientNum].user->id;
  const std::string func = __func__;

  sc->postTask(
      [this, clientNum, id, t] {
        repository->updateLastSeen(id, t);
        return std::make_unique<UpdateLastSeenResult>(
            stringFormat("Updated last seen timestamp for user %i", id));
      },
      [this, id](std::unique_ptr<SynchronizationContext::ResultBase>
                     updateLastSeenResult) {
        auto *const r =
            dynamic_cast<UpdateLastSeenResult *>(updateLastSeenResult.get());

        if (r == nullptr) {
          throw std::runtime_error("UpdateLastSeenResult is null.");
        }

        logger->info(r->message);
      },
      [this, id, func](const std::runtime_error &e) {
        logger->error("%s: %s", func, e.what());
      });
}

void SessionV2::parsePermissions(const int32_t clientNum) {
  clients[clientNum].permissions.reset();
  // any per-user commands will override the commands defined by the level
  std::string commands =
      clients[clientNum].level->commands + clients[clientNum].user->commands;
  bool allow = true;

  for (size_t i = 0; i < commands.length(); i++) {
    char c = commands.at(i);

    if (allow) {
      if (c == '*') {
        // allow all commands
        clients[clientNum].permissions.set();
      } else if (c == '+') {
        // ignore '+'
        continue;
      } else if (c == '-') {
        allow = false;
      } else {
        clients[clientNum].permissions.set(c, true);
      }
    } else {
      if (c == '*') {
        // ignore '*' if 'allow' is false
        continue;
      }

      if (c == '+') {
        allow = true;
      } else {
        clients[clientNum].permissions.set(c, false);
      }
    }
  }
}

std::bitset<MAX_COMMANDS>
SessionV2::getPermissions(const gentity_t *ent) const {
  // null 'ent' means console, give all permissions
  if (!ent) {
    return std::bitset<MAX_COMMANDS>().set();
  }

  return clients[ClientNum(ent)].permissions;
}

bool SessionV2::hasPermission(const gentity_t *ent, const char flag) const {
  return clients[ClientNum(ent)].permissions[flag];
}

int32_t SessionV2::getID(const int32_t clientNum) const {
  if (!clients[clientNum].user) {
    return -1;
  }

  return clients[clientNum].user->id;
}

int32_t SessionV2::getLevel(const gentity_t *ent) const {
  if (!clients[ClientNum(ent)].level) {
    return -1;
  }

  return clients[ClientNum(ent)].level->level;
}

const UserV2 *SessionV2::getUser(const int32_t clientNum) const {
  return clients[clientNum].user.get();
}

const Levels::Level *SessionV2::getLevel(const int32_t clientNum) const {
  return clients[clientNum].level;
}

int64_t SessionV2::getSessionStartTime(const int32_t clientNum) const {
  return clients[clientNum].sessionStartTime;
}

void SessionV2::storeNewName(const gentity_t *ent) const {
  const int32_t clientNum = ClientNum(ent);
  const int32_t id = clients[clientNum].user->id;
  const std::string clientName = ent->client->pers.netname;
  const std::string func = __func__;

  sc->postTask(
      [this, id, clientName] {
        UserModels::Name name{};
        name.name = clientName;
        name.cleanName = sanitize(clientName, true);
        name.userID = id;

        std::string message;

        if (repository->addNewName(name)) {
          message = stringFormat("Added new nickname '%s' for user %i",
                                 name.name, name.userID);
        }

        return std::make_unique<AddNewNameResult>(message);
      },
      [this](std::unique_ptr<SynchronizationContext::ResultBase>
                 addNewNameResult) {
        auto *const r =
            dynamic_cast<AddNewNameResult *>(addNewNameResult.get());

        if (r == nullptr) {
          throw std::runtime_error("AddNewNameResult is null");
        }

        if (!r->message.empty()) {
          logger->info(r->message);
        }
      },
      [this, func](const std::runtime_error &e) {
        logger->error("%s: %s", func, e.what());
      });
}

void SessionV2::listUsers(const gentity_t *ent, const int32_t page) const {
  const int32_t clientNum =
      ent ? ClientNum(ent) : Printer::CONSOLE_CLIENT_NUMBER;
  const std::string func = __func__;

  sc->postTask(
      [this] {
        const auto users = repository->getUsers();
        return std::make_unique<GetUsersResult>(users);
      },
      [this, clientNum, page](
          std::unique_ptr<SynchronizationContext::ResultBase> getUsersResult) {
        auto *const r = dynamic_cast<GetUsersResult *>(getUsersResult.get());

        if (r == nullptr) {
          throw std::runtime_error("GetUsersResult is null.");
        }

        constexpr int32_t USERS_PER_PAGE = 20;
        const auto size = static_cast<uint32_t>(r->users.size());
        const uint32_t pages = (size / USERS_PER_PAGE) + 1;

        if (page < 1 || page > pages) {
          Printer::chat(clientNum, "^3listusers: ^7invalid page.");
          return;
        }

        Printer::chat(clientNum,
                      "^3listusers: ^7check console for more information.");

        time_t t = std::time(&t);
        const uint32_t start = (page - 1) * USERS_PER_PAGE;
        const uint32_t end = std::min(start + USERS_PER_PAGE, size);

        std::string msg =
            stringFormat("Listing page ^3%i ^7of ^3%i\n", page, pages);
        msg += stringFormat("^7%-6s %-10s %-15s %-36s\n", "ID", "Level",
                            "Last seen", "Name");

        for (uint32_t i = start; i < end; i++) {
          msg += ETJump::stringFormat(
              "^7%-6i %-10i %-15s %-36s\n", r->users[i].id, r->users[i].level,
              r->users[i].lastSeen > 0
                  ? TimeStampDifferenceToString(
                        static_cast<int32_t>(t - r->users[i].lastSeen)) +
                        " ago"
                  : "N/A",
              r->users[i].name);
        }

        Printer::console(clientNum, msg);
      },
      [this, func](const std::runtime_error &e) {
        logger->error("%s: %s", func, e.what());
      });
}

void SessionV2::listUsernames(const gentity_t *ent, const int32_t id) const {
  const int32_t clientNum =
      ent ? ClientNum(ent) : Printer::CONSOLE_CLIENT_NUMBER;
  const std::string func = __func__;

  sc->postTask(
      [this, id] {
        const auto usernames = repository->getUserNames(id);
        return std::make_unique<GetUserNamesResult>(usernames);
      },
      [this, clientNum, id](std::unique_ptr<SynchronizationContext::ResultBase>
                                getUserNamesResult) {
        auto *const r =
            dynamic_cast<GetUserNamesResult *>(getUserNamesResult.get());

        if (r == nullptr) {
          throw std::runtime_error("GetUserNamesResult is null.");
        }

        if (r->usernames.empty()) {
          Printer::chat(
              clientNum,
              stringFormat(
                  "^3listusernames: ^7couldn't find any names with id ^3%i",
                  id));
          return;
        }

        Printer::chat(clientNum,
                      "^3listusernames: ^7check console for more information.");

        std::string msg =
            stringFormat("Found ^3%i ^7usernames for ID ^3%i^7:\n",
                         static_cast<int32_t>(r->usernames.size()), id);

        for (const auto &name : r->usernames) {
          msg += name + "\n^7";
        }

        Printer::console(clientNum, msg);
      },
      [this, func](const std::runtime_error &e) {
        logger->error("%s: %s", func, e.what());
      });
}

void SessionV2::findUser(const gentity_t *ent, const std::string &name) const {
  // query is case-insensitive, so we just need to strip color codes
  const std::string cleanName = sanitize(name);
  const int32_t clientNum =
      ent ? ClientNum(ent) : Printer::CONSOLE_CLIENT_NUMBER;
  const std::string func = __func__;

  sc->postTask(
      [this, cleanName] {
        const auto users = repository->getUsersByName(cleanName);
        return std::make_unique<GetUsersByNameResult>(users);
      },
      [this, clientNum,
       cleanName](std::unique_ptr<SynchronizationContext::ResultBase>
                      getUsersByNameResult) {
        auto *const r =
            dynamic_cast<GetUsersByNameResult *>(getUsersByNameResult.get());

        if (r == nullptr) {
          throw std::runtime_error("GetUsersByNameResult is null.");
        }

        if (r->users.empty()) {
          Printer::chat(
              clientNum,
              stringFormat("^3finduser: ^7no users found with name ^3'%s'^7.",
                           cleanName));
          return;
        }

        Printer::chat(clientNum,
                      "^3finduser: ^7check console for more information.");
        std::string msg = stringFormat("Listing users matching the name ^3'%s' "
                                       "^7(max 20 results)\n\nID       Name\n",
                                       cleanName);

        for (const auto &[id, name] : r->users) {
          msg += ETJump::stringFormat("%-8d %-36s^7\n", id, name);
        }

        Printer::console(clientNum, msg);
      },
      [this, func](const std::runtime_error &e) {
        logger->error("%s: %s", func, e.what());
      });
}

void SessionV2::userInfo(const gentity_t *ent, const int32_t id) const {
  const int32_t clientNum =
      ent ? ClientNum(ent) : Printer::CONSOLE_CLIENT_NUMBER;
  const std::string func = __func__;

  sc->postTask(
      [this, id] {
        const UserModels::User user = repository->getUserData(id);

        // if this user hasn't migrated from old GUID yet,
        // grab legacy auth data as well so we can display the old GUID
        std::optional<UserModels::LegacyAuth> legacyAuth;

        if (user.guid.empty()) {
          legacyAuth = repository->getLegacyAuthData(id);
        }

        return std::make_unique<GetUserDataResult>(user, legacyAuth);
      },
      [this, clientNum, id](std::unique_ptr<SynchronizationContext::ResultBase>
                                getUserDataResult) {
        auto *const r =
            dynamic_cast<GetUserDataResult *>(getUserDataResult.get());

        if (r == nullptr) {
          throw std::runtime_error("GetUserDataResult is null.");
        }

        if (r->user.id == 0) {

          Printer::chat(clientNum, "^3userinfo: ^7no user found with id ^3" +
                                       std::to_string(id));
          return;
        }

        Printer::chat(clientNum,
                      "^3userinfo: ^7check console for more information.");

        std::string guid = r->user.guid;

        if (guid.empty() && r->legacyAuth.has_value()) {
          guid = !r->legacyAuth.value().guid.empty()
                     ? r->legacyAuth.value().guid
                     : "N/A";
        }

        const std::string ipv4 = !r->user.ipv4.empty() ? r->user.ipv4 : "N/A";
        const std::string ipv6 = !r->user.ipv6.empty() ? r->user.ipv6 : "N/A";
        const std::string lastSeen =
            r->user.lastSeen > 0
                ? Time::fromInt(r->user.lastSeen).toDateTimeString()
                : "N/A";

        std::string msg = stringFormat(
            "^5ID: ^7%d\n"
            "^5GUID: ^7%s\n"
            "^5Last IPv4: ^7%s\n"
            "^5Last IPv6: ^7%s\n"
            "^5Level: ^7%d\n"
            "^5Last seen:^7 %s\n"
            "^5Name: ^7%s\n"
            "^5Title: ^7%s\n"
            "^5Commands: ^7%s\n"
            "^5Greeting: ^7%s\n",
            r->user.id, guid, ipv4, ipv6, r->user.level, lastSeen, r->user.name,
            r->user.title, r->user.commands, r->user.greeting);

        Printer::console(clientNum, msg);
      },
      [this, func](const std::runtime_error &e) {
        logger->error("%s: %s", func, e.what());
      });
}

void SessionV2::editUser(const gentity_t *ent,
                         const UserModels::EditUserParams &params) {
  const int32_t clientNum =
      ent ? ClientNum(ent) : Printer::CONSOLE_CLIENT_NUMBER;
  const std::string func = __func__;

  sc->postTask(
      [this, params] {
        if (!repository->userExists(params.id)) {
          return std::make_unique<EditUserResult>(
              stringFormat("^3edituser: ^7no user found with id ^3%i",
                           params.id),
              CLIENT_NOT_CONNECTED);
        }

        repository->editUser(params);

        return std::make_unique<EditUserResult>(
            stringFormat("^3edituser: ^7successfully updated user ^3%i",
                         params.id),
            clientNumFromID(params.id));
      },
      [this, clientNum, params](
          std::unique_ptr<SynchronizationContext::ResultBase> editUserResult) {
        auto *const r = dynamic_cast<EditUserResult *>(editUserResult.get());

        if (r == nullptr) {
          throw std::runtime_error("EditUserResult is null.");
        }

        Printer::chat(clientNum, r->message);

        // update the client's in-memory data
        if (r->clientNum != CLIENT_NOT_CONNECTED) {
          if (params.commands.has_value()) {
            clients[r->clientNum].user->commands = params.commands.value();
            parsePermissions(r->clientNum);
          }

          if (params.greeting.has_value()) {
            clients[r->clientNum].user->greeting = params.greeting.value();
          }

          if (params.title.has_value()) {
            clients[r->clientNum].user->title = params.title.value();
          }
        }
      },
      [this, func](const std::runtime_error &e) {
        logger->error("%s: %s", func, e.what());
      });
}

void SessionV2::listBans(const gentity_t *ent, const int32_t page) const {
  const int32_t clientNum =
      ent ? ClientNum(ent) : Printer::CONSOLE_CLIENT_NUMBER;
  const std::string func = __func__;

  sc->postTask(
      [this] {
        // this returns a lot of unneeded data for now, but in the future
        // we probably want to use it for better output
        const auto bans = repository->getBans();
        return std::make_unique<GetBansResult>(bans);
      },
      [this, clientNum, page](
          std::unique_ptr<SynchronizationContext::ResultBase> getBansResult) {
        auto *const r = dynamic_cast<GetBansResult *>(getBansResult.get());

        if (r == nullptr) {
          throw std::runtime_error("GetBanResult is null.");
        }

        if (r->bans.empty()) {
          Printer::chat(clientNum,
                        "^3listbans: ^7no bans found in the database.");
          return;
        }

        constexpr int32_t BANS_PER_PAGE = 3;
        const auto size = static_cast<uint32_t>(r->bans.size());
        const uint32_t pages = (size / BANS_PER_PAGE) + 1;

        if (page < 1 || page > pages) {
          Printer::chat(clientNum, "^3listbans: ^7invalid page.");
          return;
        }

        const uint32_t start = (page - 1) * BANS_PER_PAGE;
        const uint32_t end = std::min(start + BANS_PER_PAGE, size);

        Printer::chat(clientNum,
                      "^3listbans: ^7check console for more information.");

        std::string msg =
            stringFormat("^7Showing page ^3%i ^7of ^3%i\n", page, pages);

        // FIXME: this formatting is terrible, but it cannot really be fixed
        // properly because we run out of space on a single line.
        // We should display a bit less information here, and have a separate
        // '!baninfo <id>' command to display more detailed information
        // about a specific ban.
        for (uint32_t i = start; i < end; i++) {
          msg += stringFormat(
              "^7%-3i %s ^7%s %s ^7%s %s\n", r->bans[i].banID, r->bans[i].name,
              r->bans[i].banDate, r->bans[i].bannedBy,
              r->bans[i].expires > 0 ? TimeStampToString(r->bans[i].expires)
                                     : "Never",
              r->bans[i].reason);
        }

        Printer::console(clientNum, msg);
      },
      [this, func](const std::runtime_error &e) {
        logger->error("%s: %s", func, e.what());
      });
}

/*
 * When this function gets called via '!setlevel' command,
 * the following checks should have already been done:
 * - 'params.level' is a valid number
 * - 'params.level' is a valid level that exists
 * - if 'params.id' is set, it's a valid number
 * - if 'params.targetClientNum' is set, it's a valid client
 * - if executing this as a client (not console),
 *   'params.level' isn't higher than your current level
 */
void SessionV2::setLevel(const gentity_t *ent,
                         const UserModels::SetLevelParams &params) {
  const int32_t clientNum =
      ent ? ClientNum(ent) : Printer::CONSOLE_CLIENT_NUMBER;
  const std::string func = __func__;

  sc->postTask(
      [this, clientNum, params] {
        int32_t targetUserLevel = 0;
        int32_t targetClientNum = CLIENT_NOT_CONNECTED;

        // if 'id' is set, ensure it's a valid user
        if (params.id.has_value()) {
          // if the user is connected, we don't need to check the database
          targetClientNum = clientNumFromID(params.id.value());

          if (targetClientNum == CLIENT_NOT_CONNECTED) {
            const auto targetUser = repository->getUserData(params.id.value());

            if (targetUser.id == 0) {
              return std::make_unique<SetLevelResult>(stringFormat(
                  "^3setlevel: user with id ^3%i ^7does not exist.",
                  params.id.value()));
            }

            targetUserLevel = targetUser.level;
          } else {
            targetUserLevel = clients[targetClientNum].level->level;
          }
        } else if (params.targetClientNum.has_value()) {
          targetClientNum = params.targetClientNum.value();
          targetUserLevel = clients[targetClientNum].level->level;
        } else {
          // shouldn't happen, 'id' or 'targetClientNum' should always be set
          Printer::chat(
              clientNum,
              "^3setlevel: ^7no valid target found. This is a bug, please "
              "report this to the developers.");
          throw std::runtime_error(stringFormat(
              "called by client %i with no valid target! (id: %s "
              "targetClientNum: %s)",
              clientNum,
              params.id.has_value() ? std::to_string(params.id.value())
                                    : "null",
              params.targetClientNum.has_value()
                  ? std::to_string(params.targetClientNum.value())
                  : "null"));
        }

        if (params.level == targetUserLevel) {
          return std::make_unique<SetLevelResult>(stringFormat(
              "^3setlevel: ^7targeted user is already a level ^3%i ^7user.",
              params.level));
        }

        // server can set any level to anyone
        if (clientNum != Printer::CONSOLE_CLIENT_NUMBER) {
          if (targetUserLevel > clients[clientNum].level->level) {
            return (std::make_unique<SetLevelResult>(
                "^3setlevel: ^7you can't set the level of a fellow admin."));
          }
        }

        if (params.id.has_value()) {
          repository->setLevel(params.id.value(), params.level);
          std::optional<std::string> messageOther;

          // if the targeted user is online, update their in-memory
          // level and permissions, and send a message to them
          if (targetClientNum != CLIENT_NOT_CONNECTED) {
            clients[targetClientNum].level =
                game.levels->GetLevel(params.level);
            parsePermissions(targetClientNum);

            messageOther = stringFormat(
                "^3setlevel: ^7you are now a level ^3%i ^7user (%s^7).",
                params.level, clients[targetClientNum].level->name);
          }

          return std::make_unique<SetLevelResult>(
              stringFormat("^3setlevel: ^7user with id ^3%i ^7is now a level "
                           "^3%i ^7user.",
                           params.id.value(), params.level),
              messageOther.has_value() ? messageOther : std::nullopt);
        }

        repository->setLevel(clients[targetClientNum].user->id, params.level);

        clients[targetClientNum].level = game.levels->GetLevel(params.level);
        parsePermissions(targetClientNum);

        return std::make_unique<SetLevelResult>(
            stringFormat("^3setlevel: ^7%s^7 is now a level ^3%i ^7user.",
                         (g_entities + targetClientNum)->client->pers.netname,
                         params.level),
            stringFormat(
                "^3setlevel: ^7you are now a level ^3%i ^7user (%s^7).",
                params.level, clients[targetClientNum].level->name));
      },
      [this, clientNum, params, func](
          std::unique_ptr<SynchronizationContext::ResultBase> setLevelResult) {
        const auto *const r =
            dynamic_cast<SetLevelResult *>(setLevelResult.get());

        if (r == nullptr) {
          throw std::runtime_error("SetLevelResult is null.");
        }

        Printer::chat(clientNum, r->messageSelf);

        if (r->messageOther.has_value()) {
          // this shouldn't happen, but let's not crash the server in case
          // there's a logic bug somewhere in the task, as it is quite complex
          if (!params.id.has_value() && !params.targetClientNum.has_value()) {
            logger->error("%s: callback reached with no valid 'id' or "
                          "'targetClientNum'!",
                          func);
          } else {
            // NOTE: don't use .value_or() here!
            // either 'params.targetClientNum' or 'params.id' is always null
            Printer::chat(params.targetClientNum.has_value()
                              ? params.targetClientNum.value()
                              : clientNumFromID(params.id.value()),
                          r->messageOther.value());
          }
        }
      },
      [this, func](const std::runtime_error &e) {
        logger->error("%s: %s", func, e.what());
      });
}

void SessionV2::addMute(const int32_t clientNum) {
  if (clients[clientNum].guid.empty()) {
    logger->error("Unable to add mute for client %i - empty GUID!", clientNum);
    return;
  }

  if (std::find(mutedClients.cbegin(), mutedClients.cend(),
                clients[clientNum].guid) == mutedClients.cend()) {
    mutedClients.emplace_back(clients[clientNum].guid);
    logger->info("Added mute for client %i with GUID %s", clientNum,
                 clients[clientNum].guid);
  }
}

void SessionV2::removeMute(const int32_t clientNum) {
  if (clients[clientNum].guid.empty()) {
    logger->error("Unable to remove mute from client %i - empty GUID!",
                  clientNum);
    return;
  }

  const auto it = std::find(mutedClients.cbegin(), mutedClients.cend(),
                            clients[clientNum].guid);

  if (it != mutedClients.cend()) {
    mutedClients.erase(it);
    logger->info("Removed mute from client %i with GUID %s", clientNum,
                 clients[clientNum].guid);
  }
}

bool SessionV2::isMuted(const int32_t clientNum) const {
  return std::find(mutedClients.cbegin(), mutedClients.cend(),
                   clients[clientNum].guid) != mutedClients.cend();
}

bool SessionV2::readSessionData(const int clientNum) {
  Json::Value root;
  std::string err;

  const auto parsingFailed = [&] {
    logger->error("Failed to read session data for client %i: %s", clientNum,
                  err);

    // re-initialize the session completely
    initClientSession(clientNum);
    Printer::command(clientNum, Constants::Authentication::AUTH_REQUEST);
  };

  // if we can't read the data for some reason, send authentication request
  if (!JsonUtils::readFile(stringFormat(SESSION_FILE, clientNum), root, &err)) {
    parsingFailed();
    return false;
  }

  if (!JsonUtils::parseValue(clients[clientNum].guid, root["guid"], &err,
                             "guid")) {
    parsingFailed();
    return false;
  }

  const Json::Value hwid = root["hwid"];

  for (int i = 0; i < hwid.size(); ++i) {
    std::string value;

    if (!JsonUtils::parseValue(value, root["hwid"][i], &err, "hwid")) {
      parsingFailed();
      return false;
    }

    clients[clientNum].hwid.emplace_back(std::move(value));
  }

  if (!JsonUtils::parseValue(clients[clientNum].sessionStartTime,
                             root["sessionStartTime"], &err,
                             "sessionStartTime")) {
    parsingFailed();
    return false;
  }

  return true;
}

// TODO: do we want to write other fields of the Client struct here too?
//  we need GUID and session start time for proper session persistence,
//  HWID is unnecessary I think, but it was written on the old system too
//  the other data might be usedful to reduce db access, e.g. IPv4/6
void SessionV2::writeSessionData() const {
  for (int i = 0; i < level.numConnectedClients; i++) {
    const int clientNum = level.sortedClients[i];
    Json::Value root;

    root["guid"] = clients[clientNum].guid;

    root["hwid"] = Json::arrayValue;
    for (const auto &hwid : clients[clientNum].hwid) {
      root["hwid"].append(hwid);
    }

    root["sessionStartTime"] = clients[clientNum].sessionStartTime;

    std::string err;

    if (!JsonUtils::writeFile(stringFormat(SESSION_FILE, clientNum), root,
                              &err)) {
      logger->error("Failed to write session data for client %i: %s", clientNum,
                    err);
    }
  }
}

std::string SessionV2::getIP(const int32_t clientNum) const {
  if (!clients[clientNum].ipv4.empty()) {
    return clients[clientNum].ipv4;
  }

  if (!clients[clientNum].ipv6.empty()) {
    return clients[clientNum].ipv6;
  }

  return "Unknown IP";
}

void SessionV2::checkIPBan(const int clientNum) const {
  sc->postTask(
      [this, clientNum] {
        const std::vector<UserModels::BannedIPAddresses> bannedIPs =
            repository->getBannedIPAddresses();

        if (bannedIPs.empty()) {
          return std::make_unique<IPBanResult>(false, "");
        }

        // we check for both IP addresses as we ban both last known
        // IP address (old bans won't have any IPv6 addresses though)
        for (const auto &ban : bannedIPs) {
          if (!ban.ipv4.empty() && ban.ipv4 == clients[clientNum].ipv4) {
            return std::make_unique<IPBanResult>(true, ban.ipv4);
          }

          if (!ban.ipv6.empty() && ban.ipv6 == clients[clientNum].ipv6) {
            return std::make_unique<IPBanResult>(true, ban.ipv6);
          }
        }

        return std::make_unique<IPBanResult>(false, "");
      },
      [clientNum](
          std::unique_ptr<SynchronizationContext::ResultBase> ipBanResult) {
        const auto result = dynamic_cast<IPBanResult *>(ipBanResult.get());

        if (result == nullptr) {
          throw std::runtime_error("IPBanResult is null.");
        }

        if (result->isBanned) {
          const std::string cleanName =
              sanitize(g_entities[clientNum].client->pers.netname);
          Printer::logAdminLn(
              stringFormat("authentication: Rejected connection from IP banned "
                           "client %i %s (%s)",
                           clientNum, cleanName, result->ip));
          trap_DropClient(clientNum, "You are banned.", 0);
        }
      },
      [this](const std::exception &e) {
        logger->error("Failed to check IPBanResult: %s", e.what());
      });
}

// NOTE: this performs database operations, don't call this on the main thread!
bool SessionV2::isBanned(const int clientNum, const int userID,
                         const std::string &legacyGuid) const {
  const auto bans = repository->getBanData();
  const std::string userHwid = StringUtil::join(clients[clientNum].hwid, ",");

  UserModels::LegacyAuth legacyAuth{};
  std::vector<std::string> legacyAuthHWIDs{};

  if (!legacyGuid.empty()) {
    legacyAuth = repository->getLegacyAuthData(legacyGuid);

    if (legacyAuth.userID == userID) {
      legacyAuthHWIDs = StringUtil::split(legacyAuth.hwid, ",");
    }
  }

  for (const auto &ban : bans) {
    if (clients[clientNum].guid == ban.guid) {
      return true;
    }

    for (const auto &hwid : ban.hwids) {
      // TODO: this should not be a boolean operation
      //  https://github.com/etjump/etjump/issues/1559
      if (userHwid == hwid) {
        return true;
      }
    }

    if (!legacyGuid.empty() && legacyGuid == ban.legacyGUID) {
      return true;
    }

    // if we have legacy auth data, we can check for legacy hwid bans
    if (legacyAuth.userID == userID) {
      // legacy data may have multiple HWIDs, but bans only ever
      // stored the HWID at the time of the ban, so check all of them
      for (const auto &hwid : legacyAuthHWIDs) {
        if (hwid == ban.legacyHWID) {
          return true;
        }
      }
    }
  }

  return false;
}

void SessionV2::dropBannedClient(const int clientNum) const {
  const std::string cleanName =
      sanitize(g_entities[clientNum].client->pers.netname);

  Printer::logAdminLn(
      stringFormat("authentication: Banned player %s tried to connect with "
                   "GUID '%s' and HWID '%s'",
                   cleanName, clients[clientNum].guid,
                   StringUtil::join(clients[clientNum].hwid, ",")));
  Printer::popupAll(stringFormat("Banned player %s ^7tried to connect.",
                                 g_entities[clientNum].client->pers.netname));
  trap_DropClient(clientNum, "You are banned.", 0);
}

void SessionV2::printGreeting(const int32_t clientNum) const {
  const gentity_t *ent = g_entities + clientNum;

  // print custom greeting if the client has one
  if (!clients[clientNum].user->greeting.empty()) {
    std::string greeting = clients[clientNum].user->greeting;
    clients[clientNum].user->formatGreeting(ent, greeting);
    Printer::chatAll(greeting);
  } else {
    // see if the clients admin level has a greeting associated with it
    if (!clients[clientNum].level) {
      logger->error("No level associated with user %i",
                    clients[clientNum].user->id);
      return;
    }

    if (clients[clientNum].level->greeting.empty()) {
      return;
    }

    std::string greeting = clients[clientNum].level->greeting;
    // TODO: might wanna move this away from 'User'?
    clients[clientNum].user->formatGreeting(ent, greeting);
    Printer::chatAll(greeting);
  }
}

int32_t SessionV2::clientNumFromID(const int32_t id) const {
  for (int32_t i = 0; i < level.numConnectedClients; i++) {
    const int32_t clientNum = level.sortedClients[i];

    if (clients[clientNum].user && clients[clientNum].user->id == id) {
      return clientNum;
    }
  }

  return CLIENT_NOT_CONNECTED;
}
} // namespace ETJump

#endif
