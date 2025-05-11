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

#include <iostream>
#include <ctime>

#include "etj_session.h"
#include "utilities.hpp"
#include "etj_printer.h"
#include "etj_string_utilities.h"
#include "etj_levels.h"
#include "etj_save_system.h"
#include "etj_crypto.h"

Session::Session(std::shared_ptr<IAuthentication> database)
    : database_(database) {
  for (unsigned i = 0; i < MAX_CLIENTS; i++) {
    ResetClient(i);
  }
}

Session::~Session() {}

void Session::ResetClient(int clientNum) {
  clients_[clientNum].guid = "";
  clients_[clientNum].hwid = "";
  clients_[clientNum].user = NULL;
  clients_[clientNum].level = NULL;
  clients_[clientNum].sessionStartTime = 0;
  clients_[clientNum].permissions.reset();
}

void Session::Init(int clientNum) {
  G_DPrintf("Session::Init called for %d\n", clientNum);
  clients_[clientNum].guid = "";
  clients_[clientNum].hwid = "";
  clients_[clientNum].user = NULL;
  clients_[clientNum].level = NULL;
  clients_[clientNum].sessionStartTime = 0;
  clients_[clientNum].permissions.reset();

  std::string ip = ValueForKey(clientNum, "ip");
  const size_t pos = ip.find(':');
  clients_[clientNum].ip = ip.substr(0, pos);

  // FIXME: 32-bit time
  time_t t;
  t = std::time(&t);
  clients_[clientNum].sessionStartTime = static_cast<int>(t);
}

void Session::UpdateLastSeen(int clientNum) {
  unsigned lastSeen = 0;

  G_DPrintf("DEBUG: updating client %d last seen.\n", clientNum);

  if (clients_[clientNum].user) {
    time_t t;
    if (!time(&t)) {
      G_LogPrintf("ERROR: couldn't get current time.");
      return;
    }

    lastSeen = static_cast<unsigned>(t);

    G_DPrintf("Updating client's last seen to: %s\n",
              TimeStampToString(lastSeen).c_str());

    if (!database_->UpdateLastSeen(clients_[clientNum].user->id, lastSeen)) {
      G_LogPrintf("ERROR: %s\n", database_->GetMessage().c_str());
    }
  }
}

void Session::WriteSessionData(int clientNum) {
  G_DPrintf("DEBUG: Writing client %d etjump session data\n", clientNum);

  const char *sessionData = va("%s %s %i", clients_[clientNum].guid.c_str(),
                               clients_[clientNum].hwid.c_str(),
                               clients_[clientNum].sessionStartTime);

  trap_Cvar_Set(va("etjumpsession%i", clientNum), sessionData);
}

std::string Session::Guid(gentity_t *ent) const {
  return clients_[ClientNum(ent)].guid;
}

void Session::ReadSessionData(int clientNum) {
  G_DPrintf("Session::ReadSessionData called for %d\n", clientNum);

  char sessionData[MAX_CVAR_VALUE_STRING] = "\0";

  trap_Cvar_VariableStringBuffer(va("etjumpsession%i", clientNum), sessionData,
                                 sizeof(sessionData));

  char guidBuf[MAX_CVAR_VALUE_STRING] = "\0";
  char hwidBuf[MAX_CVAR_VALUE_STRING] = "\0";
  char sessStartTimeBuf[MAX_CVAR_VALUE_STRING] = "\0";

  sscanf(sessionData, "%s %s %s", guidBuf, hwidBuf, sessStartTimeBuf);

  clients_[clientNum].guid = guidBuf;
  clients_[clientNum].hwid = hwidBuf;
  clients_[clientNum].sessionStartTime = Q_atoi(sessStartTimeBuf);

  GetUserAndLevelData(clientNum);

  if (!clients_[clientNum].user) {
    G_Error("client %d has no user\n", clientNum);
  } else if (!clients_[clientNum].level) {
    G_Error("client %d has no level.\n", clientNum);
  }
}

void Session::OnGuidReceived(gentity_t *ent) {
#ifdef NEW_AUTH
  G_Printf("%s: [NEW_AUTH] TODO:\n- read/write from/to userdb\n- handle admin "
           "system\n- store nickname to userdb\n",
           __func__);
#else
  const int clientNum = ClientNum(ent);
  GetUserAndLevelData(clientNum);
  WriteSessionData(clientNum);
  ClientNameChanged(ent);
#endif

  if (g_save.integer) {
    ETJump::saveSystem->loadPositionsFromDatabase(ent);
  }
}

bool Session::GuidReceived(gentity_t *ent) {
  const int argc = trap_Argc();
  const int clientNum = ClientNum(ent);
  char guidBuf[MAX_CVAR_VALUE_STRING];
  const std::string cleanName = ETJump::sanitize(ent->client->pers.netname);
  const std::string spoofAttempt = ETJump::stringFormat(
      "authentication: Potential GUID/HWID spoof attempt by %i %s (%s)",
      clientNum, cleanName, ClientIPAddr(ent));

#ifdef NEW_AUTH
  // TODO: this should probably be refactored to smaller functions
  //  in fact, this entire function should be something like
  //  'authResponseReceived', since it deals with a lot more than just GUID

  // auth response is 'auth <GUID> <OS> <HWID>'
  // OS isn't used for anything now,
  // but it likely will be used with new database schema,
  // and/or to detect OS-specific HWID parse failures
  constexpr int NUM_EXPECTED_ARGS = 4;

  if (argc != NUM_EXPECTED_ARGS) {
    Printer::logAdminLn(spoofAttempt);
    return false;
  }

  char osBuf[8]{};
  trap_Argv(2, osBuf, sizeof(osBuf));
  const int OS = Q_atoi(osBuf);

  char hwidBuf[MAX_TOKEN_CHARS]{};

  trap_Argv(1, guidBuf, sizeof(guidBuf));
  trap_Argv(3, hwidBuf, sizeof(hwidBuf));

  bool hwidValid = true;
  std::vector<std::string> hwid = ETJump::StringUtil::split(hwidBuf, ",");
  std::vector<std::string> hwidHashed{};
  hwidHashed.reserve(hwid.size());

  // go through each component and make sure they are valid
  for (const auto &component : hwid) {
    if (!ValidGuid(component)) {
      hwidValid = false;
      break;
    }

    hwidHashed.emplace_back(ETJump::Crypto::sha1(component));
  }

  if (!hwidValid || !ValidGuid(guidBuf)) {
    Printer::logAdminLn(spoofAttempt);
    return false;
  }

  clients_[clientNum].guid = ETJump::Crypto::sha1(guidBuf);
  clients_[clientNum].hwid = ETJump::StringUtil::join(hwidHashed, ",");

  G_DPrintf("%s: %d GUID: %s OS: %i HWID: %s\n", __func__, clientNum,
            clients_[clientNum].guid.c_str(), OS,
            clients_[clientNum].hwid.c_str());
  G_Printf("%s [NEW_AUTH] TODO:\n- handle ban checks from database\n",
           __func__);

  // TODO: check bans from database

#else
  // Client sends 'AUTHENTICATE guid hwid'
  constexpr int ARGC = 3;
  if (argc != ARGC) {
    Printer::logAdminLn(ETJump::stringFormat(
        "authentication: Potential GUID/HWID spoof attempt by %i %s (%s)",
        clientNum, cleanName, ClientIPAddr(ent)));
    return false;
  }

  char hwidBuf[MAX_CVAR_VALUE_STRING]{};

  trap_Argv(1, guidBuf, sizeof(guidBuf));
  trap_Argv(2, hwidBuf, sizeof(hwidBuf));

  if (!ValidGuid(guidBuf) || !ValidGuid(hwidBuf)) {
    Printer::logAdminLn(ETJump::stringFormat(
        "authentication: Potential GUID/HWID spoof attempt by %i %s (%s)",
        clientNum, cleanName, ClientIPAddr(ent)));
    return false;
  }

  clients_[clientNum].guid = ETJump::Crypto::sha1(guidBuf);
  clients_[clientNum].hwid = ETJump::Crypto::sha1(hwidBuf);

  G_DPrintf("GuidReceived: %d GUID: %s HWID: %s\n", clientNum,
            clients_[clientNum].guid.c_str(), clients_[clientNum].hwid.c_str());

  if (database_->IsBanned(clients_[clientNum].guid, clients_[clientNum].hwid)) {
    Printer::logAdminLn(ETJump::stringFormat(
        "authentication: Banned player %s tried to connect with GUID '%s' and "
        "HWID '%s'",
        cleanName, clients_[clientNum].guid, clients_[clientNum].hwid));
    Printer::popupAll(
        va("Banned player %s ^7tried to connect.", ent->client->pers.netname));
    trap_DropClient(clientNum, "You are banned.", 0);
    return false;
  }
#endif

  OnGuidReceived(ent);

  return true;
}

void Session::GetUserAndLevelData(int clientNum) {
  gentity_t *ent = g_entities + clientNum;
  if (!database_->UserExists(clients_[clientNum].guid)) {
    if (!database_->AddUser(clients_[clientNum].guid, clients_[clientNum].hwid,
                            std::string(ent->client->pers.netname))) {
      G_LogPrintf("ERROR: failed to add user to database: %s\n",
                  database_->GetMessage().c_str());
    } else {
      G_DPrintf("New user connected. Added user to the "
                "user database\n");
      clients_[clientNum].user =
          database_->GetUserData(clients_[clientNum].guid);
    }
  } else {
    G_DPrintf("Old user connected. Getting user data from the "
              "database.\n");

    clients_[clientNum].user = database_->GetUserData(clients_[clientNum].guid);
    if (clients_[clientNum].user) {
      G_DPrintf("User data found: %s\n", clients_[clientNum].user->ToChar());

      if (find(clients_[clientNum].user->hwids.begin(),
               clients_[clientNum].user->hwids.end(),
               clients_[clientNum].hwid) ==
          clients_[clientNum].user->hwids.end()) {
        G_DPrintf("New HWID detected. Adding HWID %s "
                  "to list.\n",
                  clients_[clientNum].hwid.c_str());

        if (!database_->AddNewHardwareId(clients_[clientNum].user->id,
                                         clients_[clientNum].hwid)) {
          G_LogPrintf("Failed to add a "
                      "new hardware ID "
                      "to user %s\n",
                      ent->client->pers.netname);
        }
      }
    } else {
      G_LogPrintf("ERROR: couldn't get user's data (%s)\n",
                  ent->client->pers.netname);
    }
  }

  clients_[clientNum].level =
      game.levels->GetLevel(clients_[clientNum].user->level);

  if (ent->client->sess.firstTime) {
    PrintGreeting(ent);
    Printer::popup(ent, "^5Your last visit was on " +
                            clients_[clientNum].user->GetLastSeenString() +
                            ".");
  }

  if (!clients_[clientNum].user) {
    // Debugging purposes, should be never executed
    G_Error("Client doesn't have db::user.\n");
  }

  ParsePermissions(clientNum);
}

void Session::ParsePermissions(int clientNum) {
  clients_[clientNum].permissions.reset();
  // First parse level commands then user commands (as user commands
  // override level ones)
  std::string commands =
      clients_[clientNum].level->commands + clients_[clientNum].user->commands;

  const int STATE_ALLOW = 1;
  const int STATE_DENY = 2;
  int state = STATE_ALLOW;
  for (unsigned i = 0; i < commands.length(); i++) {
    char c = commands.at(i);
    if (state == STATE_ALLOW) {
      if (c == '*') {
        // Allow all commands
        for (size_t i = 0; i < MAX_COMMANDS; i++) {
          clients_[clientNum].permissions.set(i, true);
        }
      } else if (c == '+') {
        // ignore +
        continue;
      } else if (c == '-') {
        state = STATE_DENY;
      } else {
        clients_[clientNum].permissions.set(c, true);
      }
    } else {
      if (c == '*') {
        // Ignore * while in deny-mode
        continue;
      }

      if (c == '+') {
        state = STATE_ALLOW;
      } else {
        clients_[clientNum].permissions.set(c, false);
      }
    }
  }
}

void Session::OnClientDisconnect(int clientNum) {
  WriteSessionData(clientNum);
  UpdateLastSeen(clientNum);

  clients_[clientNum].user = NULL;
  clients_[clientNum].level = NULL;
  clients_[clientNum].sessionStartTime = 0;
  clients_[clientNum].permissions.reset();
}

void Session::PrintGreeting(gentity_t *ent) {
  int clientNum = ClientNum(ent);
  Client *cl = &clients_[clientNum];

  // If user has own greeting, print it
  if (cl->user->greeting.length() > 0) {
    std::string greeting = cl->user->greeting;
    ETJump::StringUtil::replaceAll(greeting, "[n]", ent->client->pers.netname);
    ETJump::StringUtil::replaceAll(greeting, "[t]",
                                   cl->user->GetLastSeenString());
    ETJump::StringUtil::replaceAll(greeting, "[d]",
                                   cl->user->GetLastVisitString());
    G_DPrintf("Printing greeting %s\n", greeting.c_str());
    Printer::chatAll(greeting);
  } else {
    if (!cl->level) {
      return;
    }
    // if user has a level greeting, print it
    if (cl->level->greeting.length() > 0) {
      std::string greeting = cl->level->greeting;
      ETJump::StringUtil::replaceAll(greeting, "[n]",
                                     ent->client->pers.netname);
      ETJump::StringUtil::replaceAll(greeting, "[t]",
                                     cl->user->GetLastSeenString());
      ETJump::StringUtil::replaceAll(greeting, "[d]",
                                     cl->user->GetLastVisitString());
      G_DPrintf("Printing greeting %s\n", greeting.c_str());
      Printer::chatAll(greeting);
    }
  }
}

bool Session::SetLevel(gentity_t *target, int level) {
  if (!clients_[ClientNum(target)].user) {
    message_ = "you must wait until user has connected.";
    return false;
  }

  if (!database_->SetLevel(clients_[ClientNum(target)].user->id, level)) {
    message_ = database_->GetMessage();
    return false;
  }

  clients_[ClientNum(target)].level = game.levels->GetLevel(level);

  ParsePermissions(ClientNum(target));
  return true;
}

bool Session::SetLevel(int id, int level) {
  if (!database_->SetLevel(id, level)) {
    message_ = database_->GetMessage();
    return false;
  }

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients_[i].user && clients_[i].user->id == id) {
      ParsePermissions(i);
      Printer::chat(g_entities + i,
                    va("^3setlevel: ^7you are now a level %d user.", level));
    }
  }
  return true;
}

bool Session::UserExists(unsigned id) { return database_->UserExists(id); }

int Session::GetLevelById(unsigned id) const {
  return database_->GetUserData(id)->level;
}

int Session::GetId(gentity_t *ent) const {
  if (!clients_[ClientNum(ent)].user) {
    return -1;
  }

  return clients_[ClientNum(ent)].user->id;
}

int Session::GetId(int clientNum) const {
  if (!clients_[clientNum].user) {
    return -1;
  }

  return clients_[clientNum].user->id;
}

gentity_t *Session::gentityFromId(unsigned id) {
  if (!UserExists(id)) {
    return nullptr;
  }

  for (int i = 0; i < level.numConnectedClients; i++) {
    const int clientNum = level.sortedClients[i];

    if (GetId(clientNum) == id) {
      return g_entities + clientNum;
    }
  }

  return nullptr;
}

int Session::GetLevel(gentity_t *ent) const {
  int num = ClientNum(ent);
  if (ent && clients_[num].user) {
    return clients_[num].user->level;
  }

  return 0;
}

bool Session::IsIpBanned(int clientNum) {
  return database_->IsIpBanned(clients_[clientNum].ip);
}

bool Session::Ban(gentity_t *ent, gentity_t *player, unsigned expires,
                  std::string reason) {
  int clientNum = ClientNum(player);
  time_t t;
  time(&t);

  if (clients_[clientNum].guid.length() == 0) {
    message_ = "User doesn't have a guid. Are you sure he's "
               "connected?";
    return false;
  }

  std::string ipport = ValueForKey(player, "ip");
  std::string::size_type pos = ipport.find(":");
  std::string ip = ipport.substr(0, pos);

  return database_->BanUser(
      std::string(player->client->pers.netname), clients_[clientNum].guid,
      clients_[clientNum].hwid, ip,
      std::string(ent ? ent->client->pers.netname : "Console"),
      TimeStampToString(static_cast<unsigned>(t)), expires, reason);
}

void Session::PrintFinger(gentity_t *ent, gentity_t *target) {
  int num = ClientNum(target);

  if (!clients_[num].user) {
    message_ = "you must wait until user has connected.";
    return;
  }

  Printer::chat(ent, "^3finger: ^7check console for more information.");

  const auto user = clients_[num].user;
  Printer::console(
      ent, ETJump::stringFormat(
               "^7Name: %s\n"
               "^7Original name: %s\n"
               "^7ID: %d\n"
               "^7Level: %d\n"
               "^7Title: %s\n",
               target->client->pers.netname, user->name, user->id, user->level,
               user->title.empty() ? clients_[num].level->name : user->title));
}

void Session::PrintAdmintest(gentity_t *ent) {
  int clientNum = ClientNum(ent);

  if (!clients_[ClientNum(ent)].user) {
    // TODO: what is this for?
    message_ = "you must wait until user has connected.";
    return;
  }

  if (ent && clients_[clientNum].user && clients_[clientNum].level) {
    std::string message =
        va("^3admintest: ^7%s^7 is a level %d user (%s^7).",
           ent->client->pers.netname, clients_[clientNum].user->level,
           clients_[clientNum].user->title.length() > 0
               ? clients_[clientNum].user->title.c_str()
               : clients_[clientNum].level->name.c_str());

    Printer::chatAll(message);
  }
}

std::string Session::GetMessage() const { return message_; }

std::bitset<256> Session::Permissions(gentity_t *ent) const {
  if (!ent) {
    std::bitset<MAX_COMMANDS> all;
    all.set();
    return all;
  }
  return clients_[ClientNum(ent)].permissions;
}

Session::Client::Client() : guid(""), hwid("") {}

std::vector<Session::Client *> Session::FindUsersByLevel(int userLevel) {
  std::vector<Session::Client *> matchingClients;
  for (int i = 0; i < level.numConnectedClients; i++) {
    int clientNum = level.sortedClients[i];
    if (clients_[clientNum].level->level == userLevel) {
      matchingClients.push_back(&clients_[clientNum]);
    }
  }
  return matchingClients;
}

int Session::LevelDeleted(int adminLevel) {
  int usersReseted = database_->ResetUsersWithLevel(adminLevel);

  for (int i = 0; i < level.numConnectedClients; i++) {
    int clientNum = level.sortedClients[i];
    clients_[clientNum].level =
        game.levels->GetLevel(clients_[clientNum].user->level);
    ParsePermissions(clientNum);
  }
  return usersReseted;
}

void Session::NewName(gentity_t *ent) {
  auto clientNum = ClientNum(ent);
  if (!clients_[clientNum].user) {
    G_LogPrintf("Error: Couldn't store new user nick -- user "
                "is null.\n");
    return;
  }
  database_->NewName(clients_[ClientNum(ent)].user->id,
                     ent->client->pers.netname);
}

bool Session::HasPermission(gentity_t *ent, char flag) {
  return clients_[ClientNum(ent)].permissions[flag];
}

int Session::getSessionStartTime(const int clientNum) const {
  return clients_[clientNum].sessionStartTime;
}
