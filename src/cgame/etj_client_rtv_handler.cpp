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

#include "etj_client_rtv_handler.h"
#include "etj_client_commands_handler.h"

namespace ETJump {
ClientRtvHandler::ClientRtvHandler(
    const std::shared_ptr<ClientCommandsHandler> &serverCommandsHandler)
    : rtvVoteYes({0, 0}), isRtvVote(false),
      serverCommandsHandler(serverCommandsHandler) {
  rtvMaps.clear();

  serverCommandsHandler->subscribe(
      "openRtvMenu",
      [](const std::vector<std::string> &) {
        trap_SendConsoleCommand("openRtvMenu");
      },
      false);
}

void ClientRtvHandler::initialize() {
  setRtvVoteStatus();

  if (!rtvVoteActive()) {
    return;
  }

  setRtvConfigStrings(CG_ConfigString(CS_VOTE_YES));
  countRtvVotes();
}

void ClientRtvHandler::setRtvConfigStrings(const char *cs) {
  char key[MAX_QPATH]; // these are map names so MAX_QPATH is sufficient
  char value[6]; // can't exceed "MAX_CLIENTS,MAX_CLIENTS" + null terminator

  rtvMaps.clear();

  while (cs != nullptr) {
    Info_NextPair(&cs, key, value);

    if (!key[0]) {
      break;
    }

    // Parse "playerCount,spectatorCount"
    int playerCount = 0;
    int spectatorCount = 0;
    char *comma = strchr(value, ',');
    if (comma) {
      *comma = '\0';
      playerCount = Q_atoi(value);
      spectatorCount = Q_atoi(comma + 1);
    } else {
      // fallback: treat all as player votes if no comma
      // this happens on old demos, where we don't have players/specs separated
      playerCount = Q_atoi(value);
    }

    rtvMaps.emplace_back(
        RtvMapVoteInfo{key, RtvVoteCountInfo{playerCount, spectatorCount}});
  }
}

int ClientRtvHandler::getTotalVotesForMap(int mapIndex) {
  return rtvMaps[mapIndex].voteCountInfo.playerCount +
         rtvMaps[mapIndex].voteCountInfo.spectatorCount;
}

void ClientRtvHandler::countRtvVotes() {
  rtvVoteYes = {0, 0};

  for (size_t i = 0; i < rtvMaps.size(); ++i) {
    rtvVoteYes.playerCount += rtvMaps[i].voteCountInfo.playerCount;
    rtvVoteYes.spectatorCount += rtvMaps[i].voteCountInfo.spectatorCount;
  }
}

const std::vector<RtvMapVoteInfo> *ClientRtvHandler::getRtvMaps() {
  return &rtvMaps;
}

RtvVoteCountInfo ClientRtvHandler::getRtvYesVotes() const { return rtvVoteYes; }

void ClientRtvHandler::setRtvVoteStatus() {
  isRtvVote = !Q_stricmpn(cgs.voteString, "Rock The Vote", 13);
}

bool ClientRtvHandler::rtvVoteActive() const {
  return isRtvVote && cgs.voteTime;
}

void ClientRtvHandler::resetRtvEventHandler() {
  if (cgs.eventHandling == CGAME_EVENT_RTV) {
    CG_EventHandling(CGAME_EVENT_NONE, qfalse);
  }
}
} // namespace ETJump
