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

#pragma once

#include "etj_portalgun_shared.h"
#include "etj_save_system.h"
#include "etj_shared.h"
#include "etj_worldspawn_shared.h"

namespace ETJump {
class Worldspawn {
public:
  enum class NoExplosivesOpts {
    OFF = 0,
    NO_EXPLOSIVE_WEAPONS = 1,
    NO_DYNAMITE = 2,
  };

  NoExplosivesOpts noExplosives{};
  PortalTeamOpts portalTeam{};
  EnumBitset<SaveSystem::SaveLoadRestrictions> strictSaveLoad;

  int32_t limitedSaves{};

  bool noFTNoGhost{};
  bool noFTSaveLimit{};
  bool noFTTeamjumpMode{};
  bool noGhost{};
  bool noGod{};
  bool noGoto{};
  bool portalgunSpawn{};
  bool portalSurfaces{};

  // keys shared between server and client
  SharedWorldspawnKeys sharedKeys;
  std::vector<WorldspawnShared::KeyOverride> keyOverrides;

  Worldspawn();
  ~Worldspawn() = default;

  void setKeyOverrideCS() const;
  void addKeyOverride(const WorldspawnShared::KeyOverride &newOverride);
  SharedWorldspawnKeys resolvedKeysForClient(const gentity_t *ent) const;

private:
  void initNoDrop(const char *key);
  void initNoExplosives(const char *key);
  void initNoFallDamage(const char *key);
  void initNoFTNoGhost(const char *key);
  void initNoFTSaveLimit(const char *key);
  void initNoFTTeamjumpMode(const char *key);
  void initNoGhost(const char *key);
  void initNoGod(const char *key);
  void initNoGoto(const char *key);
  void initNoJumpDelay(const char *key);
  void initNoNoclip(const char *key);
  void initNoSave(const char *key);
  void initNoOverbounce(const char *key);
  void initNoProne(const char *key);
  void initNoWallbug(const char *key);
  void initOverbouncePlayers(const char *key);
  void initPortalgunSpawn(const char *key);
  void initPortalSurfaces(const char *key);
  void initPortalPredict(const char *key);
  void initPortalTeam(const char *key);
  void initLimitedSaves(const char *key);
  void initStrictSaveLoad(const char *key);

  void initKeys();
  void setWorldspawnCS() const;
  static void printKeyValue(const std::string &key, const std::string &value);
};
} // namespace ETJump
