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

#include <algorithm>

#include "etj_spectatorinfo_data.h"

namespace ETJump {
std::vector<int32_t> SpectatorInfoData::activeSpectators;
std::vector<int32_t> SpectatorInfoData::inactiveSpectators;

void SpectatorInfoData::updateSpectatorData(std::optional<int32_t> clientNum) {
  if (clientNum.has_value()) {
    const int32_t cnum = clientNum.value();
    const char *cs = CG_ConfigString(CS_PLAYERS + cnum);

    // the client we got an update from disconnected,
    // remove them from the lists if present
    if (cs[0] == '\0') {
      removeClientFromList(activeSpectators, cnum);
      removeClientFromList(inactiveSpectators, cnum);
    }

    return;
  }

  for (int32_t i = 0; i < cg.numScores; i++) {
    if (skipClient(cg.scores[i])) {
      continue;
    }

    if (cg.scores[i].followedClient != cg.snap->ps.clientNum) {
      removeClientFromList(activeSpectators, cg.scores[i].client);
      removeClientFromList(inactiveSpectators, cg.scores[i].client);
      continue;
    }

    const clientInfo_t *ci = &cgs.clientinfo[cg.scores[i].client];

    if (ci->clientIsInactive) {
      removeClientFromList(activeSpectators, cg.scores[i].client);

      if (std::find(inactiveSpectators.cbegin(), inactiveSpectators.cend(),
                    cg.scores[i].client) == inactiveSpectators.cend()) {
        inactiveSpectators.emplace_back(cg.scores[i].client);
      }
    } else {
      removeClientFromList(inactiveSpectators, cg.scores[i].client);

      if (std::find(activeSpectators.cbegin(), activeSpectators.cend(),
                    cg.scores[i].client) == activeSpectators.cend()) {
        activeSpectators.emplace_back(cg.scores[i].client);
      }
    }
  }
}

void SpectatorInfoData::removeClientFromList(std::vector<int32_t> &v,
                                             const int32_t clientNum) {
  v.erase(std::remove(v.begin(), v.end(), clientNum), v.end());
}

bool SpectatorInfoData::skipClient(const score_t &score) {
  if (score.client == cg.snap->ps.clientNum) {
    return true;
  }

  if (score.team != TEAM_SPECTATOR) {
    return true;
  }

  return false;
}
} // namespace ETJump
