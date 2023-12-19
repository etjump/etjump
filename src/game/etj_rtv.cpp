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

#include "etj_rtv.h"
#include "g_local.h"
#include "etj_string_utilities.h"

namespace ETJump {

RockTheVote::RockTheVote() {
  rtvMaps.clear();
  isRtvVote = false;
}

bool RockTheVote::checkRtvWinner() {
  std::vector<std::string> mostVotedMaps{};
  int previousVoteCount = 0;
  int voteCount;

  for (const auto &map : rtvMaps) {
    voteCount = map.second;

    if (voteCount > previousVoteCount) {
      mostVotedMaps.clear();
      mostVotedMaps.push_back(map.first);
      previousVoteCount = map.second;
    } else if (voteCount == previousVoteCount) {
      mostVotedMaps.push_back(map.first);
      previousVoteCount = map.second;
    }
  }

  return mostVotedMaps.size() == 1;
}

void RockTheVote::setRtvWinner() {
  std::string mostVotedMap;
  int previousVoteCount = 0;
  int voteCount;

  for (const auto &map : rtvMaps) {
    voteCount = map.second;

    // we don't need to care about tied votes here as vote timer
    // will be extended in a tie scenario, and we'll always end up
    // with a single map with most votes
    if (voteCount > previousVoteCount) {
      mostVotedMap = map.first;
      previousVoteCount = map.second;
    }
  }

  Q_strncpyz(level.voteInfo.vote_value, mostVotedMap.c_str(),
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
} // namespace ETJump
