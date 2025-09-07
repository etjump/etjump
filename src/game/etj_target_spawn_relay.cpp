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

#include "etj_target_spawn_relay.h"

#include "etj_string_utilities.h"

namespace ETJump {
std::vector<TargetSpawnRelay::SpawnRelayEntity> spawnRelayEntities{};

void TargetSpawnRelay::spawn(gentity_t *ent) {
  setTeams(ent);

  ent->use = [](gentity_t *self, gentity_t *other, gentity_t *activator) {
    use(self, activator);
  };
}

void TargetSpawnRelay::use(gentity_t *self, gentity_t *activator) {
  if (!activator || !activator->client) {
    G_Error("target_spawn_relay: activator is not a client.");
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

void TargetSpawnRelay::setTeams(gentity_t *self) {
  const auto registerEntity = [&](const team_t team) {
    spawnRelayEntities.emplace_back(SpawnRelayEntity({self, team}));
  };

  if (!self->team) {
    // use TEAM_FREE for 'any' team relays so we can check this easily
    registerEntity(TEAM_FREE);
    return;
  }

  const auto teams = StringUtil::split(self->team, ",");

  for (const auto &team : teams) {
    if (StringUtil::iEqual(team, "axis")) {
      registerEntity(TEAM_AXIS);
    } else if (StringUtil::iEqual(team, "allies")) {
      registerEntity(TEAM_ALLIES);
    } else if (StringUtil::iEqual(team, "spectator")) {
      registerEntity(TEAM_SPECTATOR);
    }
  }
}

// Because we have no control of the order in which the entities are spawned,
// we iterate the entities twice - first check for global relays,
// and then check again to ensure there are no duplicate team relays.
// We could verify the global relay already while spawning,
// but it makes more sense to have the validation logic in one place,
// even if that means that we do two loops here.
void TargetSpawnRelay::validateSpawnRelayEntities() {
  const char *err = "%s: only one %s spawn relay entity can be spawned.";

  for (const auto &entity : spawnRelayEntities) {
    if (entity.team != TEAM_FREE) {
      continue;
    }

    if (level.spawnRelayEntities.axisRelay ||
        level.spawnRelayEntities.alliesRelay ||
        level.spawnRelayEntities.spectatorRelay) {
      G_Error(err, __func__, "global");
    }

    level.spawnRelayEntities.axisRelay = entity.ent;
    level.spawnRelayEntities.alliesRelay = entity.ent;
    level.spawnRelayEntities.spectatorRelay = entity.ent;
  }

  for (const auto &entity : spawnRelayEntities) {
    switch (entity.team) {
      case TEAM_AXIS:
        if (level.spawnRelayEntities.axisRelay) {
          G_Error(err, __func__, "Axis");
        }

        level.spawnRelayEntities.axisRelay = entity.ent;
        break;
      case TEAM_ALLIES:
        if (level.spawnRelayEntities.alliesRelay) {
          G_Error(err, __func__, "Allies");
        }

        level.spawnRelayEntities.alliesRelay = entity.ent;
        break;
      case TEAM_SPECTATOR:
        if (level.spawnRelayEntities.spectatorRelay) {
          G_Error(err, __func__, "Spectator");
        }

        level.spawnRelayEntities.spectatorRelay = entity.ent;
        break;
      case TEAM_FREE:
      default:
        break;
    }
  }

  // save the bytes :)
  spawnRelayEntities.erase(spawnRelayEntities.begin(),
                           spawnRelayEntities.end());
}

void TargetSpawnRelay::invalidateSpawnRelayPointers(const gentity_t *ent) {
  if (!ent->team) {
    level.spawnRelayEntities.alliesRelay = nullptr;
    level.spawnRelayEntities.axisRelay = nullptr;
    level.spawnRelayEntities.spectatorRelay = nullptr;
  } else {
    const auto teams = StringUtil::split(ent->team, ",");

    for (const auto &team : teams) {
      if (StringUtil::iEqual(team, "allies")) {
        level.spawnRelayEntities.alliesRelay = nullptr;
      } else if (StringUtil::iEqual(team, "axis")) {
        level.spawnRelayEntities.axisRelay = nullptr;
      } else if (StringUtil::iEqual(team, "spectator")) {
        level.spawnRelayEntities.spectatorRelay = nullptr;
      }
    }
  }
}
} // namespace ETJump
