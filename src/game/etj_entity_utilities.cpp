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

#include "etj_entity_utilities.h"

namespace ETJump {
bool EntityUtilities::isPlayer(gentity_t *ent) {
  auto cnum = ClientNum(ent);
  return cnum >= 0 && cnum < MAX_CLIENTS;
}

// vectors don't seem to like vec_t as their value,
// so using raw values for colors here instead of colorRed, colorGreen etc.
const std::vector<std::pair<std::string, std::vector<float>>> railBoxEnts{
    {"trigger_multiple_ext", {0.0f, 1.0f, 0.0f}},
    {"trigger_starttimer_ext", {0.0f, 0.0f, 1.0f}},
    {"trigger_stoptimer_ext", {0.0f, 0.0f, 1.0f}},
    {"trigger_checkpoint_ext", {1.0f, 0.0f, 1.0f}},
    {"func_fakebrush", {1.0f, 0.0f, 0.0f}}};

void EntityUtilities::checkForRailBox(gentity_t *ent) {
  for (const auto &railBoxEnt : railBoxEnts) {
    if (ent->classname == railBoxEnt.first) {
      drawRailBox(ent, railBoxEnt.second);
    }
  }
}

void EntityUtilities::drawRailBox(gentity_t *ent,
                                  const std::vector<float> &color) {
  vec3_t b1;
  vec3_t b2;
  gentity_t *temp;

  VectorCopy(ent->r.currentOrigin, b1);
  VectorCopy(ent->r.currentOrigin, b2);
  VectorAdd(b1, ent->r.mins, b1);
  VectorAdd(b2, ent->r.maxs, b2);

  temp = G_TempEntity(b1, EV_RAILTRAIL);

  VectorCopy(b2, temp->s.origin2);
  VectorCopy(color, temp->s.angles);
  temp->s.dmgFlags = 1;

  temp->s.angles[0] = color[0] * 255;
  temp->s.angles[1] = color[1] * 255;
  temp->s.angles[2] = color[2] * 255;

  temp->s.effect1Time = ent->s.number + 1;
}

bool EntityUtilities::playerIsSolid(const int self, const int other) {
  fireteamData_t *ftSelf, *ftOther;
  const gclient_t *selfClient = g_entities[self].client;
  const gclient_t *otherClient = g_entities[other].client;

  if (self == other) {
    return false;
  }

  if (selfClient->sess.sessionTeam == TEAM_SPECTATOR ||
      otherClient->sess.sessionTeam == TEAM_SPECTATOR) {
    return false;
  }

  if (selfClient->ps.pm_type == PM_NOCLIP ||
      otherClient->ps.pm_type == PM_NOCLIP) {
    return false;
  }

  if (g_ghostPlayers.integer == 1) {
    if (!G_IsOnFireteam(self, &ftSelf)) {
      return false;
    }

    // we're on same fireteam, but noghost isn't enabled
    if (G_IsOnFireteam(other, &ftOther) && ftSelf == ftOther) {
      if (!ftSelf->noGhost) {
        return false;
      }
    }

    // we're not in the same fireteam
    if (!G_IsOnFireteam(other, &ftOther) || ftSelf != ftOther) {
      return false;
    }
  }

  return true;
}
} // namespace ETJump
