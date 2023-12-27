/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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

#include <random>
#include "etj_rtv.h"
#include "g_local.h"
#include "etj_string_utilities.h"
#include "etj_utilities.h"
#include "etj_printer.h"

namespace ETJump {

RockTheVote::RockTheVote() {
  rtvMaps.clear();
  isRtvVote = false;
  autoRtvStartTime = level.startTime;
  anyonePlayedSinceLastVote = false;
  twoMinWarningGiven = false;
}

std::vector<std::string> RockTheVote::getMostVotedMaps() {
  std::vector<std::string> mostVotedMaps{};
  int previousVoteCount = 0;
  int voteCount;

  for (const auto &map : rtvMaps) {
    if (map.second == 0) {
      continue;
    }

    voteCount = map.second;

    if (voteCount > previousVoteCount) {
      mostVotedMaps.clear();
      mostVotedMaps.push_back(map.first);
      previousVoteCount = map.second;
    } else if (voteCount == previousVoteCount) {
      mostVotedMaps.push_back((map.first));
    }
  }

  return mostVotedMaps;
}

void RockTheVote::setRtvWinner() {
  auto maps = getMostVotedMaps();

  if (maps.size() > 1) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::shuffle(maps.begin(), maps.end(), gen);
  } else {
    G_increasePassedCount(maps[0].c_str());
  }

  Q_strncpyz(level.voteInfo.vote_value, maps[0].c_str(),
             sizeof(level.voteInfo.vote_value));
}

void RockTheVote::setRtvConfigstrings() {
  const size_t maxMaps = rtvMaps.size();
  std::string newcs;

  for (size_t i = 0; i < maxMaps; ++i) {
    newcs += stringFormat("%s\\%i%s", rtvMaps[i].first, rtvMaps[i].second,
                          i == maxMaps - 1 ? "" : "\\");
  }

  trap_SetConfigstring(CS_VOTE_YES, newcs.c_str());
}

std::vector<std::pair<std::string, int>> *RockTheVote::getRtvMaps() {
  return &rtvMaps;
}

void RockTheVote::clearRtvMaps() { rtvMaps.clear(); }

bool RockTheVote::rtvVoteActive() const { return isRtvVote; }

void RockTheVote::setRtvStatus(bool status) { isRtvVote = status; }

bool RockTheVote::checkAutoRtv() {
  if (!level.numConnectedClients || g_autoRtv.integer <= 0) {
    // push the start time forward on empty servers or if auto rtv is off
    autoRtvStartTime = level.time;
    return false;
  }

  // wait for any ongoing vote finish
  if (level.voteInfo.voteTime) {
    return false;
  }

  if (Utilities::anyonePlaying()) {
    if (!anyonePlayedSinceLastVote) {
      // if this is the first time anyone has played
      // since last vote attempt, push back the start time
      autoRtvStartTime = level.time;
    }

    anyonePlayedSinceLastVote = true;
  } else {
    if (!anyonePlayedSinceLastVote) {
      autoRtvStartTime = level.time; // push back the start time
      return false;
    }
  }

  const int autoRtvMsec = g_autoRtv.integer * 1000 * 60;
  const int autoRtvCooldown = autoRtvStartTime + autoRtvMsec;

  if (!twoMinWarningGiven && level.time > autoRtvCooldown - (1000 * 60 * 2) &&
      g_autoRtv.integer >= 2) {
    Printer::BroadcastChatMessage(
        "^gServer: automatic Rock The Vote will be called in ^32 ^gminutes.");
    twoMinWarningGiven = true;
    return false;
  }

  return level.time > autoRtvCooldown;
}

void RockTheVote::callAutoRtv() {
  int i;
  char voteArg[MAX_STRING_TOKENS];

  Q_strncpyz(voteArg, "rtv", sizeof(voteArg));

  if ((i = G_voteCmdCheck(nullptr, voteArg, nullptr)) != G_OK) {
    if (i == G_NOTFOUND) {
      G_LogPrintf(
          "callAutoRtv: Could not find vote command for '%s'. This should not "
          "happen, please file a bug report at github.com/etjump/etjump\n",
          voteArg);
    }

    // if we fail here, it's because we don't have enough maps,
    // so push back auto rtv timer so that we don't flood the logs
    autoRtvStartTime = level.time;
    return;
  }

  Q_strncpyz(level.voteInfo.voteString, voteArg,
             sizeof(level.voteInfo.voteString));

  level.voteInfo.voteYes = 0;
  level.voteInfo.voteNo = 0;
  level.voteInfo.voteTime = level.time;
  level.voteInfo.voter_cn = -1;
  level.voteInfo.voter_team = TEAM_FREE;
  level.voteInfo.isAutoRtvVote = true;

  autoRtvStartTime = level.time; // reset cooldown in case this vote fails
  anyonePlayedSinceLastVote = false;
  twoMinWarningGiven = false;
  isRtvVote = true;

  std::string voteMsg = stringFormat("Server called an automatic %s.\n",
                                     level.voteInfo.voteString);

  Printer::BroadcastConsoleMessage(voteMsg);
  Printer::BroadcastCenterMessage("Server called an automatic vote.");
  G_LogPrintf("%s", voteMsg.c_str());

  G_globalSound("sound/misc/vote.wav");

  for (i = 0; i < level.numConnectedClients; i++) {
    level.clients[level.sortedClients[i]].ps.eFlags &= ~EF_VOTED;
  }

  trap_SetConfigstring(CS_VOTE_TIME, va("%i", level.voteInfo.voteTime));
  trap_SetConfigstring(CS_VOTE_STRING, level.voteInfo.voteString);
  setRtvConfigstrings();
  trap_SetConfigstring(CS_VOTE_NO, va("%i", level.voteInfo.voteNo));
}
} // namespace ETJump
