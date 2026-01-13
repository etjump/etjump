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

#include <random>

#include "etj_target_random.h"

namespace ETJump {
void TargetRandom::spawn(gentity_t *ent) {
  G_SpawnInt("chance", "1", &ent->count);
  G_SpawnInt("total", "1", &ent->count2);

  if (ent->count < 1) {
    G_Error("target_random: 'chance' must be over 1.\n");
  }

  if (ent->count2 < 1) {
    G_Error("target_random: 'total' must be over 1.\n");
  }

  if (ent->count2 < ent->count) {
    G_Error("target_random: 'total' (%i) must be higher than 'chance' (%i).",
            ent->count2, ent->count);
  }

  ent->use = [](gentity_t *self, gentity_t *other, gentity_t *activator) {
    use(self, activator);
  };
}

void TargetRandom::use(gentity_t *self, gentity_t *activator) {
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<int32_t> dist(1, self->count2);

  const int32_t result = dist(rng);
  const bool success = self->spawnflags & SF_EXACT_NUMBER
                           ? result == self->count
                           : result <= self->count;

  if (!success) {
    return;
  }

  // if scriptname is set, call the 'activate' script trigger
  if (self->scriptName) {
    const team_t team = activator->client->sess.sessionTeam;
    self->activator = activator;

    // send team to script too to support 'activate axis/allies'
    // this simplifies usage for mappers so they don't have to create
    // separate entities to fire individual script triggers for either team
    if (team == TEAM_AXIS || team == TEAM_ALLIES) {
      G_Script_ScriptEvent(self, "activate",
                           team == TEAM_AXIS ? "axis" : "allies");
    } else {
      G_Script_ScriptEvent(self, "activate", nullptr);
    }
  }

  G_UseTargets(self, activator);
}
} // namespace ETJump
