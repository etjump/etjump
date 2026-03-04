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

#include <algorithm>

#include "etj_worldspawn.h"
#include "etj_save_system.h"
#include "etj_string_utilities.h"
#include "g_local.h"

namespace ETJump {
Worldspawn::Worldspawn() {
  G_Printf("^g--------------------------------------------------\n"
           "^gETJump: ^7initializing worldspawn keys\n"
           "^g--------------------------------------------------\n");

  shared.integer = 0;

  for (const auto &[key, initFn] : keys) {
    initFn();
  }

  G_Printf("^g--------------------------------------------------\n");
  trap_Cvar_Set("shared", std::to_string(shared.integer).c_str());
}

void Worldspawn::initNoDrop(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    noDrop = value;
    shared.integer |= BG_LEVEL_NO_DROP;
  }

  printKeyValue(key, noDrop ? "1" : "0");
}

void Worldspawn::initNoExplosives(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    value = std::clamp(value, 0, 2);
    noExplosives = static_cast<NoExplosives>(value);
  }

  printKeyValue(key, std::to_string(static_cast<int32_t>(noExplosives)));
}

void Worldspawn::initNoFallDamage(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    value = std::clamp(value, 0, 2);
    noFallDamage = static_cast<NoFallDamage>(value);

    if (noFallDamage == NoFallDamage::ON) {
      shared.integer |= BG_LEVEL_NO_FALLDAMAGE;
    } else if (noFallDamage == NoFallDamage::FORCE_ON) {
      shared.integer |= BG_LEVEL_NO_FALLDAMAGE_FORCE;
    }
  }

  printKeyValue(key, std::to_string(static_cast<int32_t>(noFallDamage)));
}

void Worldspawn::initNoFTNoGhost(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    noFTNoGhost = value;
  }

  printKeyValue(key, noFTNoGhost ? "1" : "0");
}

void Worldspawn::initNoFTSaveLimit(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    noFTSaveLimit = value;
  }

  printKeyValue(key, noFTSaveLimit ? "1" : "0");
}

void Worldspawn::initNoFTTeamjumpMode(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    noFTTeamjumpMode = value;
  }

  printKeyValue(key, noFTTeamjumpMode ? "1" : "0");
}

/*
 * The value of 'noghost' key will be set to 'g_ghostPlayers' cvar using
 * bitwise operations, to the second bit. This means that the value of this key
 * can be derived from the value of 'g_ghostPlayers' cvar, since it is always
 * forced to be either 0 or 1.
 *
 * - g_ghostPlayers 0 = g_ghostPlayers 0, noghost 0
 * - g_ghostPlayers 1 = g_ghostPlayers 1, noghost 0
 * - g_ghostPlayers 2 = g_ghostPlayers 0, noghost 1
 * - g_ghostPlayers 3 = g_ghostPlayers 1, noghost 1
 *
 * Why? I don't know, but I guess it's neat.
 */
void Worldspawn::initNoGhost(const char *key) {
  int32_t value = 0;
  int32_t currentValue = g_ghostPlayers.integer;

  if (G_SpawnInt(key, "0", &value)) {
    currentValue |= 2;
    noGhost = value;
  } else {
    currentValue &= ~2;
  }

  trap_Cvar_Set("g_ghostPlayers", std::to_string(currentValue).c_str());
  trap_Cvar_Update(&g_ghostPlayers);

  printKeyValue(key, noGhost ? "1" : "0");
}

void Worldspawn::initNoGod(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    noGod = value;
  }

  printKeyValue(key, noGod ? "1" : "0");
}

void Worldspawn::initNoGoto(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    noGoto = value;
  }

  printKeyValue(key, noGoto ? "1" : "0");
}

void Worldspawn::initNoJumpDelay(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    noJumpDelay = value;
    shared.integer |= BG_LEVEL_NO_JUMPDELAY;
  }

  printKeyValue(key, noJumpDelay ? "1" : "0");
}

void Worldspawn::initNoNoclip(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    noNoclip = value;
    shared.integer |= BG_LEVEL_NO_NOCLIP;
  }

  printKeyValue(key, noNoclip ? "1" : "0");
}

void Worldspawn::initNoSave(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    noSave = value;
    shared.integer |= BG_LEVEL_NO_SAVE;
  }

  printKeyValue(key, noSave ? "1" : "0");
}

void Worldspawn::initNoOverbounce(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    noOverbounce = value;
    shared.integer |= BG_LEVEL_NO_OVERBOUNCE;
  }

  printKeyValue(key, noOverbounce ? "1" : "0");
}

void Worldspawn::initNoProne(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    noProne = value;
    shared.integer |= BG_LEVEL_NO_PRONE;
  }

  printKeyValue(key, noProne ? "1" : "0");
}

void Worldspawn::initNoWallbug(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    noWallbug = value;
    shared.integer |= BG_LEVEL_NO_WALLBUG;
  }

  printKeyValue(key, noWallbug ? "1" : "0");
}

void Worldspawn::initOverbouncePlayers(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    value = std::clamp(value, 0, 2);
    overbouncePlayers = static_cast<OverbouncePlayers>(value);

    if (overbouncePlayers == OverbouncePlayers::ALWAYS) {
      shared.integer |= BG_LEVEL_BODY_OB_ALWAYS;
    } else if (overbouncePlayers == OverbouncePlayers::NEVER) {
      shared.integer |= BG_LEVEL_BODY_OB_NEVER;
    }
  }

  printKeyValue(key, std::to_string(static_cast<int32_t>(overbouncePlayers)));
}

void Worldspawn::initPortalgunSpawn(const char *key) {
  int32_t value = 0;

  G_SpawnInt(key, "1", &value);
  portalgunSpawn = value;

  printKeyValue(key, portalgunSpawn ? "1" : "0");
}

void Worldspawn::initPortalSurfaces(const char *key) {
  int32_t value = 0;

  G_SpawnInt(key, "1", &value);
  portalSurfaces = value;

  printKeyValue(key, portalSurfaces ? "1" : "0");
}

void Worldspawn::initPortalPredict(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    portalPredict = value;
    shared.integer |= BG_LEVEL_PORTAL_PREDICT;
  }

  printKeyValue(key, portalPredict ? "1" : "0");
}

void Worldspawn::initPortalTeam(const char *key) {
  int32_t value = 0;

  if (G_SpawnInt(key, "0", &value)) {
    value = std::clamp(value, 0, 2);
    portalTeam = static_cast<PortalTeam>(value);
  }

  trap_Cvar_Set("g_portalTeam",
                std::to_string(static_cast<int32_t>(portalTeam)).c_str());
  printKeyValue(key, std::to_string(static_cast<int32_t>(portalTeam)));
}

void Worldspawn::initLimitedSaves(const char *key) {
  G_SpawnInt(key, "0", &limitedSaves);
  printKeyValue(key, std::to_string(limitedSaves));
}

void Worldspawn::initStrictSaveLoad(const char *key) {
  const std::unordered_map<std::string, SaveSystem::SaveLoadRestrictions>
      allowedStrictValues{
          {"none", SaveSystem::SaveLoadRestrictions::NONE},
          {"move", SaveSystem::SaveLoadRestrictions::MOVE},
          {"dead", SaveSystem::SaveLoadRestrictions::DEAD},
      };

  char *buf = nullptr;

  if (G_SpawnString(key, "0", &buf)) {
    if (std::isdigit(buf[0])) {
      strictSaveLoad |=
          static_cast<SaveSystem::SaveLoadRestrictions>(Q_atoi(buf));
    } else {
      auto restrictions = StringUtils::split(buf, "|");

      for (const auto &restriction : restrictions) {
        const auto token =
            StringUtils::toLowerCase(StringUtils::trim(restriction));

        if (allowedStrictValues.find(token) != allowedStrictValues.cend()) {
          strictSaveLoad |= allowedStrictValues.at(token);
        }
      }
    }
  }

  std::vector<std::string> values;

  for (const auto &[name, value] : allowedStrictValues) {
    if (strictSaveLoad & value) {
      values.emplace_back(name);
    }
  }

  if (values.empty()) {
    printKeyValue(key, "none");
  } else {
    printKeyValue(key, StringUtils::join(values, ", "));
  }
}

void Worldspawn::printKeyValue(const std::string &key,
                               const std::string &value) {
  G_Printf(" %s: %s\n", key.c_str(), value.c_str());
}
} // namespace ETJump
