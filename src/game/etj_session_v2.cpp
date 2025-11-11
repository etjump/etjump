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
  #include "etj_json_utilities.h"

namespace ETJump {
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
  clients[clientNum].user = nullptr;
}

void SessionV2::onClientConnect(const int clientNum, const bool firstTime) {
  G_DPrintf("%s called for %i\n", __func__, clientNum);

  if (firstTime) {
    initClientSession(clientNum);
    checkIPBan(clientNum);
  } else {
    readSessionData(clientNum);
    // TODO: timerun client connect
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
      [this, clientNum, firstTime](
          std::unique_ptr<SynchronizationContext::ResultBase> userData) {
        auto *const r = dynamic_cast<GetUserDataResult *>(userData.get());

        if (r == nullptr) {
          throw std::runtime_error("GetUserDataResult is null.");
        }

        clients[clientNum].user = std::make_unique<UserV2>();
        clients[clientNum].user->id = r->user.id;
        clients[clientNum].user->greeting = r->user.greeting;
        clients[clientNum].user->title = r->user.title;
        clients[clientNum].user->lastSeen = r->user.lastSeen;

        if (firstTime) {
          printGreeting(clientNum);
          Printer::popup(
              clientNum,
              stringFormat("^5Your last visit was on %s.",
                           Time::fromInt(r->user.lastSeen).toDateTimeString()));
        }

        game.timerunV2->clientConnect(clientNum, clients[clientNum].user->id);
      },
      [this, clientNum, func](const std::runtime_error &e) {
        logger->error("%s failed: %s", func, e.what());
      });
}

bool SessionV2::authenticate(gentity_t *ent) {
  const int argc = trap_Argc();
  const int clientNum = ClientNum(ent);
  const std::string cleanName = sanitize(ent->client->pers.netname);
  const std::string spoofAttempt = stringFormat(
      "authentication: Potential GUID/HWID spoof attempt by %i %s (%s)",
      clientNum, cleanName, ClientIPAddr(ent));

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

  G_DPrintf("%s: %d GUID: %s OS: %i HWID: %s\n", __func__, clientNum,
            clients[clientNum].guid.c_str(), clients[clientNum].platform,
            StringUtil::join(clients[clientNum].hwid, ",").c_str());

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
      [this, clientNum, cleanName, ent,
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
          Printer::console(
              Printer::CONSOLE_CLIENT_NUMBER,
              stringFormat("^3%s: [NEW_AUTH] ^7Client %i %s with GUID %s not "
                           "found in the database, sending migration request\n",
                           func, clientNum, cleanName,
                           clients[clientNum].guid));

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

  G_Printf("^3%s: [NEW_AUTH] TODO:\n  ^7- handle user database\n  - handle "
           "admin system\n",
           __func__);

  return true;
}

bool SessionV2::migrateGuid(gentity_t *ent) {
  const int argc = trap_Argc();
  constexpr int NUM_EXPECTED_ARGS = 3;
  const int clientNum = ClientNum(ent);
  const std::string cleanName = sanitize(ent->client->pers.netname);
  const std::string spoofAttempt = stringFormat(
      "authentication: Potential GUID migration spoof attempt by %i %s (%s)",
      clientNum, cleanName.c_str(), ClientIPAddr(ent));

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
      [this, clientNum, ent](
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
          addNewUser(ent);
        } else {
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

void SessionV2::addNewUser(gentity_t *ent) {
  const int clientNum = ClientNum(ent);
  const std::string name = ent->client->pers.netname;
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

        return std::make_unique<AddUserResult>(userID,
                                               "Successfully added new user.");
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

void SessionV2::readSessionData(const int clientNum) {
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
    return;
  }

  if (!JsonUtils::parseValue(clients[clientNum].guid, root["guid"], &err,
                             "guid")) {
    parsingFailed();
    return;
  }

  const Json::Value hwid = root["hwid"];

  for (int i = 0; i < hwid.size(); ++i) {
    std::string value;

    if (!JsonUtils::parseValue(value, root["hwid"][i], &err, "hwid")) {
      parsingFailed();
      return;
    }

    clients[clientNum].hwid.emplace_back(std::move(value));
  }

  if (!JsonUtils::parseValue(clients[clientNum].sessionStartTime,
                             root["sessionStartTime"], &err,
                             "sessionStartTime")) {
    parsingFailed();
  }
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
    G_Printf("^3%s: [NEW_AUTH] TODO:\n  ^7- print level-specific greeting\n",
             __func__);
  }
}
} // namespace ETJump

#endif
