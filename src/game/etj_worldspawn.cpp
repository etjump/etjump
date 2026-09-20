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

  initKeys();

  G_Printf("^g--------------------------------------------------\n");

  setWorldspawnCS();
}

void Worldspawn::initKeys() {
  initNoDrop(Keys::NO_DROP);
  initNoExplosives(Keys::NO_EXPLOSIVES);
  initNoFallDamage(Keys::NO_FALL_DAMAGE);
  initNoFTNoGhost(Keys::NO_FT_NO_GHOST);
  initNoFTSaveLimit(Keys::NO_FT_SAVE_LIMIT);
  initNoFTTeamjumpMode(Keys::NO_FT_TJ_MODE);
  initNoGhost(Keys::NO_GHOST);
  initNoGod(Keys::NO_GOD);
  initNoGoto(Keys::NO_GOTO);
  initNoJumpDelay(Keys::NO_JUMP_DELAY);
  initNoNoclip(Keys::NO_NOCLIP);
  initNoSave(Keys::NO_SAVE);
  initNoOverbounce(Keys::NO_OVERBOUNCE);
  initNoProne(Keys::NO_PRONE);
  initNoWallbug(Keys::NO_WALLBUG);
  initOverbouncePlayers(Keys::OVERBOUCNE_PLAYERS);
  initPortalgunSpawn(Keys::PORTALGUN_SPAWN);
  initPortalSurfaces(Keys::PORTAL_SURFACES);
  initPortalPredict(Keys::PORTAL_PREDICT);
  initPortalTeam(Keys::PORTAL_TEAM);
  initLimitedSaves(Keys::LIMITED_SAVES);
  initStrictSaveLoad(Keys::STRICT_SAVE_LOAD);
}

void Worldspawn::setWorldspawnCS() const {
  std::string cs;

  cs += StringUtils::format(R"(\%s\%i)", NO_OVERBOUNCE_CS,
                            sharedKeys.noOverbounce ? 1 : 0);
  cs += StringUtils::format(R"(\%s\%i)", NO_JUMP_DELAY_CS,
                            sharedKeys.noJumpDelay ? 1 : 0);
  cs += StringUtils::format(R"(\%s\%i)", NO_SAVE_CS, sharedKeys.noSave ? 1 : 0);
  cs +=
      StringUtils::format(R"(\%s\%i)", NO_PRONE_CS, sharedKeys.noProne ? 1 : 0);
  cs += StringUtils::format(R"(\%s\%i)", NO_DROP_CS, sharedKeys.noDrop ? 1 : 0);
  cs += StringUtils::format(R"(\%s\%i)", NO_WALLBUG_CS,
                            sharedKeys.noWallbug ? 1 : 0);
  cs += StringUtils::format(R"(\%s\%i)", NO_NOCLIP_CS,
                            sharedKeys.noNoclip ? 1 : 0);
  cs += StringUtils::format(R"(\%s\%i)", PORTAL_PREDICT_CS,
                            sharedKeys.portalPredict ? 1 : 0);

  cs += StringUtils::format(R"(\%s\%i)", NO_FALL_DAMAGE_CS,
                            static_cast<int32_t>(sharedKeys.noFallDamage));
  cs += StringUtils::format(R"(\%s\%i)", OVERBOUNCE_PLAYERS_CS,
                            static_cast<int32_t>(sharedKeys.overbouncePlayers));

  trap_SetConfigstring(CS_ETJUMP_WS_KEYS, cs.c_str());
}

void Worldspawn::initNoDrop(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  sharedKeys.noDrop = value;

  printKeyValue(key, sharedKeys.noDrop ? "1" : "0");
}

void Worldspawn::initNoExplosives(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  noExplosives = static_cast<NoExplosivesOpts>(std::clamp(value, 0, 2));

  printKeyValue(key, std::to_string(static_cast<int32_t>(noExplosives)));
}

void Worldspawn::initNoFallDamage(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  sharedKeys.noFallDamage =
      static_cast<NoFallDamageOpts>(std::clamp(value, 0, 2));

  printKeyValue(key,
                std::to_string(static_cast<int32_t>(sharedKeys.noFallDamage)));
}

void Worldspawn::initNoFTNoGhost(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  noFTNoGhost = value;

  printKeyValue(key, noFTNoGhost ? "1" : "0");
}

void Worldspawn::initNoFTSaveLimit(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  noFTSaveLimit = value;

  printKeyValue(key, noFTSaveLimit ? "1" : "0");
}

void Worldspawn::initNoFTTeamjumpMode(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  noFTTeamjumpMode = value;

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
  G_SpawnInt(key, "0", &value);
  noGhost = value;

  if (noGhost) {
    currentValue |= 2;
  } else {
    currentValue &= ~2;
  }

  trap_Cvar_Set("g_ghostPlayers", std::to_string(currentValue).c_str());
  trap_Cvar_Update(&g_ghostPlayers);

  printKeyValue(key, noGhost ? "1" : "0");
}

void Worldspawn::initNoGod(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  noGod = value;

  printKeyValue(key, noGod ? "1" : "0");
}

void Worldspawn::initNoGoto(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  noGoto = value;

  printKeyValue(key, noGoto ? "1" : "0");
}

void Worldspawn::initNoJumpDelay(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  sharedKeys.noJumpDelay = value;

  printKeyValue(key, sharedKeys.noJumpDelay ? "1" : "0");
}

void Worldspawn::initNoNoclip(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  sharedKeys.noNoclip = value;

  printKeyValue(key, sharedKeys.noNoclip ? "1" : "0");
}

void Worldspawn::initNoSave(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  sharedKeys.noSave = value;

  printKeyValue(key, sharedKeys.noSave ? "1" : "0");
}

void Worldspawn::initNoOverbounce(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  sharedKeys.noOverbounce = value;

  printKeyValue(key, sharedKeys.noOverbounce ? "1" : "0");
}

void Worldspawn::initNoProne(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  sharedKeys.noProne = value;

  printKeyValue(key, sharedKeys.noProne ? "1" : "0");
}

void Worldspawn::initNoWallbug(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  sharedKeys.noWallbug = value;

  printKeyValue(key, sharedKeys.noWallbug ? "1" : "0");
}

void Worldspawn::initOverbouncePlayers(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  sharedKeys.overbouncePlayers =
      static_cast<OverbouncePlayersOpts>(std::clamp(value, 0, 2));

  printKeyValue(
      key, std::to_string(static_cast<int32_t>(sharedKeys.overbouncePlayers)));
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
  G_SpawnInt(key, "0", &value);
  sharedKeys.portalPredict = value;

  printKeyValue(key, sharedKeys.portalPredict ? "1" : "0");
}

void Worldspawn::initPortalTeam(const char *key) {
  int32_t value = 0;
  G_SpawnInt(key, "0", &value);
  portalTeam = static_cast<PortalTeamOpts>(std::clamp(value, 0, 2));

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
