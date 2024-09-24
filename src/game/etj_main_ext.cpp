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

#include "etj_local.h"
#include "etj_game.h"
#include "etj_session.h"
#include "etj_commands.h"
#include "etj_levels.h"
#include "etj_database.h"
#include "etj_custom_map_votes.h"
#include "etj_utilities.h"
#include "etj_motd.h"
#include "etj_map_statistics.h"
#include "etj_tokens.h"
#include "etj_shared.h"
#include "etj_string_utilities.h"
#include "etj_timerun_repository.h"
#include "etj_timerun_v2.h"
#include "etj_rtv.h"
#include "etj_chat_replay.h"
#include "etj_filesystem.h"

Game game;

void OnClientConnect(int clientNum, qboolean firstTime, qboolean isBot) {
  // Do not do g_entities + clientNum here, entity is not initialized
  // yet
  G_DPrintf("OnClientConnect called by %d\n", clientNum);

  if (firstTime) {
    ETJump::session->Init(clientNum);

    G_DPrintf("Requesting guid from %d\n", clientNum);

    trap_SendServerCommand(
        clientNum, ETJump::Constants::Authentication::GUID_REQUEST.c_str());
  } else {
    ETJump::session->ReadSessionData(clientNum);
    game.timerunV2->clientConnect(clientNum, ETJump::session->GetId(clientNum));
  }

  if (ETJump::session->IsIpBanned(clientNum)) {
    G_LogPrintf("Kicked banned client: %d\n", clientNum);
    trap_DropClient(clientNum, "You are banned.", 0);
  }
}

void OnClientBegin(gentity_t *ent) {
  G_DPrintf("OnClientBegin called by %d\n", ClientNum(ent));
  if (!ent->client->sess.motdPrinted) {
    game.motd->PrintMotd(ent);
    ent->client->sess.motdPrinted = qtrue;
  }
  ETJump::Log::processMessages();
}

void OnClientDisconnect(gentity_t *ent) {
  G_DPrintf("OnClientDisconnect called by %d\n", ClientNum(ent));

  ETJump::session->OnClientDisconnect(ClientNum(ent));
  ETJump::Log::processMessages();
  game.timerunV2->clientDisconnect(ClientNum(ent));
}

void WriteSessionData() {
  for (int i = 0; i < level.numConnectedClients; i++) {
    int clientNum = level.sortedClients[i];
    ETJump::session->WriteSessionData(clientNum);
  }
}

namespace ETJump {
bool checkCheatCvars(gclient_s *client, int flags) {
  bool cheatCvarsEnabled = false;
  const int clientNum = ClientNum(client);
  std::string message =
      "^gThe following cvars are not allowed on this server:\n";

  if (flags & static_cast<int>(CheatCvarFlags::LookYaw)) {
    message += "^3cl_yawspeed != 0\n"
               "^3cl_freelook 0\n";

    if (client->pers.clientFlags & CGF_CHEATCVARSON) {
      cheatCvarsEnabled = true;
    }
  }

  if (flags & static_cast<int>(CheatCvarFlags::PmoveFPS)) {
    message += "^3pmove_fixed 0 ^gwith:\n"
               "^3com_maxfps ^goutside of ^325-125\n\"";

    if ((client->pers.maxFPS > 125 || client->pers.maxFPS < 25) &&
        !client->pers.pmoveFixed) {
      cheatCvarsEnabled = true;
    }
  }

  if (cheatCvarsEnabled) {
    trap_SendServerCommand(clientNum, va("cheatCvarsOff %i", flags));
    Printer::chat(clientNum, "^gCheat cvars are not allowed on this server, "
                             "check console for more information.\n");
    Printer::console(clientNum, message);
  }

  return cheatCvarsEnabled;
}
} // namespace ETJump

void RunFrame(int levelTime) {
  game.mapStatistics->runFrame(levelTime);
  game.timerunV2->runFrame();

  if (game.rtv->checkAutoRtv()) {
    game.rtv->callAutoRtv();
  }

  ETJump::Log::processMessages();
}

void OnGameInit() {
  game.levels = std::make_shared<Levels>();
  game.commands = std::make_shared<Commands>();
  game.mapStatistics = std::make_shared<MapStatistics>();
  game.customMapVotes =
      std::make_shared<CustomMapVotes>(game.mapStatistics.get());
  game.motd = std::make_shared<Motd>();
  game.tokens = std::make_shared<ETJump::Tokens>();
  game.rtv = std::make_shared<ETJump::RockTheVote>();
  game.chatReplay = std::make_unique<ETJump::ChatReplay>();

  if (strlen(g_levelConfig.string)) {
    if (!game.levels->ReadFromConfig()) {
      G_LogPrintf("Error while reading admin config: %s\n",
                  game.levels->ErrorMessage().c_str());
    } else {
      G_Printf("Successfully loaded levels from "
               "config: %s\n",
               g_levelConfig.string);
    }
  }

  if (strlen(g_userConfig.string) > 0) {
    if (!ETJump::database->InitDatabase(g_userConfig.string)) {
      G_LogPrintf("DATABASE ERROR: %s\n",
                  ETJump::database->GetMessage().c_str());
    } else {
      G_LogPrintf("Users loaded successfully from "
                  "database: %s\n",
                  g_userConfig.string);
    }
  }

  game.timerunV2 = std::make_shared<ETJump::TimerunV2>(
      level.rawmapname,
      std::make_unique<ETJump::TimerunRepository>(
          std::make_unique<ETJump::DatabaseV2>(
              "timerunv2",
              ETJump::FileSystem::Path::getPath(g_timeruns2Database.string)),
          std::make_unique<ETJump::DatabaseV2>(
              "timerunv1",
              ETJump::FileSystem::Path::getPath(g_timerunsDatabase.string))),
      std::make_unique<ETJump::Log>("timerunv2"),
      std::make_unique<ETJump::SynchronizationContext>());

  game.mapStatistics->initialize(std::string(g_mapDatabase.string),
                                 level.rawmapname);
  game.customMapVotes->Load();
  game.motd->Initialize();
  game.timerunV2->initialize();

  if (g_tokensMode.integer) {
    // Utilities::WriteFile handles the correct path
    // (etjump/...)
    auto path = std::string(g_tokensPath.string) + "/" +
                std::string(level.rawmapname) + ".json";
    game.tokens->loadTokens(path);
  }

  game.mapStatistics->writeMapsToDisk("maps.json");

  ETJump::Log::processMessages();
}

void OnGameShutdown() {
  WriteSessionData();
  //    ETJump::database->ExecuteQueuedOperations();
  // these may be null, e.g. when G_Alloc or stack unwinding fails
  // after engine calls longjmp on errors
  if (ETJump::database != nullptr) {
    ETJump::database->CloseDatabase();
  }
  if (game.mapStatistics != nullptr) {
    game.mapStatistics->saveChanges();
  }
  if (game.tokens != nullptr) {
    game.tokens->reset();
  }
  if (game.timerunV2) {
    game.timerunV2->shutdown();
  }

  game.levels = nullptr;
  game.commands = nullptr;
  game.customMapVotes = nullptr;
  game.motd = nullptr;
  game.mapStatistics = nullptr;
  game.tokens = nullptr;
  game.timerunV2 = nullptr;
  game.rtv = nullptr;
  game.chatReplay = nullptr;
  ETJump::Log::processMessages();
}

qboolean OnConnectedClientCommand(gentity_t *ent) {
  G_DPrintf("OnClientCommand called for %d (%s): %s\n", ClientNum(ent),
            ConcatArgs(0), ent->client->pers.netname);

  auto argv = GetArgs();
  auto command = ETJump::StringUtil::toLowerCase((*argv)[0]);

  if (ent->client->pers.connected != CON_CONNECTED) {
    return qfalse;
  }

  if (game.commands->ClientCommand(ent, command)) {
    return qtrue;
  }

  if (game.commands->AdminCommand(ent)) {
    return qtrue;
  }

  return qfalse;
}

// Returning qtrue means no other commands will be checked
qboolean OnClientCommand(gentity_t *ent) {
  G_DPrintf("OnClientCommand called for %d (%s): %s\n", ClientNum(ent),
            ConcatArgs(0), ent->client->pers.netname);

  auto argv = GetArgs();
  auto command = ETJump::StringUtil::toLowerCase((*argv)[0]);

  if (command == ETJump::Constants::Authentication::AUTHENTICATE) {
    ETJump::session->GuidReceived(ent);
    game.timerunV2->clientConnect(ClientNum(ent),
                                  ETJump::session->GetId(ClientNum(ent)));
    return qtrue;
  }

  return qfalse;
}

/*
=======================
Server console commands
=======================
*/
qboolean OnConsoleCommand() {
  G_DPrintf("OnConsoleCommand called: %s.\n", ConcatArgs(0));

  auto argv = GetArgs();
  auto command = ETJump::StringUtil::toLowerCase((*argv)[0]);

  if (command == "generatemotd") {
    game.motd->GenerateMotdFile();
    return qtrue;
  }

  if (command == "generatecustomvotes") {
    game.customMapVotes->GenerateVotesFile();
    return qtrue;
  }

  if (command == "readcustomvotes") {
    game.customMapVotes->Load();
    // force voteflag re-check so UI can turn on/off custom vote button
    G_voteFlags();
    return qtrue;
  }

  if (command == "logstate") {
    LogServerState();
    return qtrue;
  }

  if (game.commands->AdminCommand(NULL)) {
    return qtrue;
  }

  return qfalse;
}

const char *GetRandomMap() { return game.mapStatistics->randomMap(); }

std::vector<std::string> getMapsOnList(const std::string &name) {
  return game.customMapVotes->getMapsOnList(name);
}

const char *G_MatchOneMap(const char *arg) {
  auto currentMaps = game.mapStatistics->getCurrentMaps();
  std::vector<std::string> matchingMaps;
  std::string mapName = arg ? ETJump::StringUtil::toLowerCase(arg) : "";

  for (auto &map : *(currentMaps)) {
    if (map.find(mapName) != std::string::npos) {
      matchingMaps.push_back(map);
    }
  }

  static char matchingMap[MAX_STRING_TOKENS] = "\0";
  if (matchingMaps.size() == 1) {
    Q_strncpyz(matchingMap, matchingMaps[0].c_str(), sizeof(matchingMap));
    return matchingMap;
  }

  if (matchingMaps.size() > 1) {
    auto match = std::find(matchingMaps.begin(), matchingMaps.end(), mapName);
    if (match != matchingMaps.end()) {
      Q_strncpyz(matchingMap, match->c_str(), sizeof(matchingMap));
      return matchingMap;
    }
  }

  return NULL;
}

std::vector<std::string> G_MatchAllMaps(const char *arg) {
  auto currentMaps = game.mapStatistics->getCurrentMaps();
  std::vector<std::string> matchingMaps;
  std::string mapName = arg ? ETJump::StringUtil::toLowerCase(arg) : "";

  for (auto &map : *(currentMaps)) {
    if (map.find(mapName) != std::string::npos) {
      matchingMaps.push_back(map);
    }
  }

  return matchingMaps;
}

const char *GetRandomMapByType(const char *customType) {
  static char buf[MAX_TOKEN_CHARS] = "\0";

  if (!customType) {
    G_Error("customType is NULL.");
  }

  Q_strncpyz(buf, game.customMapVotes->RandomMap(customType).c_str(),
             sizeof(buf));
  return buf;
}

// returns null if map type doesnt exists.
const char *CustomMapTypeExists(const char *mapType) {
  static char buf[MAX_TOKEN_CHARS] = "\0";
  if (!mapType) {
    G_Error("mapType is NULL.");
  }

  CustomMapVotes::TypeInfo info = game.customMapVotes->GetTypeInfo(mapType);

  if (info.typeExists) {
    Q_strncpyz(buf, info.callvoteText.c_str(), sizeof(buf));
    return buf;
  }

  return NULL;
}

void ClientNameChanged(gentity_t *ent) { ETJump::session->NewName(ent); }

std::string GetTeamString(int clientNum) {
  gentity_t *ent = g_entities + clientNum;

  if (ent->client->sess.sessionTeam == TEAM_ALLIES) {
    return "ALLIES";
  }
  if (ent->client->sess.sessionTeam == TEAM_AXIS) {
    return "AXIS";
  }
  return "SPECTATOR";
}

void LogServerState() {
  time_t t;
  time(&t);
  std::string state =
      "Server state at " + TimeStampToString(static_cast<int>(t)) + ":\n";

  for (int i = 0; i < level.numConnectedClients; i++) {
    int clientNum = level.sortedClients[i];

    state +=
        ETJump::stringFormat("%i %s %s\n", clientNum, GetTeamString(clientNum),
                             (g_entities + clientNum)->client->pers.netname);
  }

  if (level.numConnectedClients == 0) {
    state += "No players on the server.\n";
  }

  Printer::log(state);
}

void TimerunConnectNotify(gentity_t *ent) {
  game.timerunV2->connectNotify(ClientNum(ent));
}

void InterruptRun(gentity_t *ent) {
  if (!ent) {
    return;
  }

  game.timerunV2->interrupt(ClientNum(ent));
}

void G_increaseCallvoteCount(const char *mapName) {
  game.mapStatistics->increaseCallvoteCount(mapName);
}

void G_increasePassedCount(const char *mapName) {
  game.mapStatistics->increasePassedCount(mapName);
}

namespace ETJump {
bool checkEntsForScriptname() {
  // clients and bodyqueue don't have scriptname set
  const int startEnt = MAX_CLIENTS + BODY_QUEUE_SIZE;

  for (int i = startEnt; i < level.num_entities; i++) {
    gentity_t *current = &g_entities[i];
    if (!current->scriptName) {
      continue;
    } else {
      G_Printf("^2FOUND\n");
      return true; // ent with scriptname found, we can exit
    }
  }

  G_Printf("^1NOT FOUND\n");
  return false;
}
} // namespace ETJump
