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

#include "etj_client_rtv_handler.h"
#include "cg_local.h"

namespace ETJump {
ClientRtvHandler::ClientRtvHandler() {
  rtvMaps.clear();
  rtvVoteYes = 0;
  isRtvVote = false;
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
  char value[3];       // can't exceed MAX_CLIENTS + null terminator

  rtvMaps.clear();

  while (cs != nullptr) {
    Info_NextPair(&cs, key, value);

    if (!key[0]) {
      break;
    }

    rtvMaps.emplace_back(key, Q_atoi(value));
  }
}

void ClientRtvHandler::countRtvVotes() {
  rtvVoteYes = 0;

  for (const auto &votes : rtvMaps) {
    rtvVoteYes += votes.second;
  }
}

const std::vector<std::pair<std::string, int>> *ClientRtvHandler::getRtvMaps() {
  return &rtvMaps;
}

int ClientRtvHandler::getRtvYesVotes() const { return rtvVoteYes; }

void ClientRtvHandler::setRtvVoteStatus() {
  isRtvVote = !Q_stricmpn(cgs.voteString, "Rock The Vote", 13);
}

bool ClientRtvHandler::rtvVoteActive() const {
  return isRtvVote && cgs.voteTime;
}

void ClientRtvHandler::resetRtvEventHandler() {
  CG_EventHandling(CGAME_EVENT_NONE, qfalse);
}
} // namespace ETJump
