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

#include <functional>
#include <string>
#include <vector>

#include "etj_portalgun_shared.h"
#include "etj_save_system.h"
#include "etj_shared.h"

namespace ETJump {
class Worldspawn {
public:
  struct Keys {
    static constexpr char NO_DROP[] = "nodrop";
    static constexpr char NO_EXPLOSIVES[] = "noexplosives";
    static constexpr char NO_FALL_DAMAGE[] = "nofalldamage";
    static constexpr char NO_FT_NO_GHOST[] = "noftnoghost";
    static constexpr char NO_FT_SAVE_LIMIT[] = "noftsavelimit";
    static constexpr char NO_FT_TJ_MODE[] = "noftteamjumpmode";
    static constexpr char NO_GHOST[] = "noghost";
    static constexpr char NO_GOD[] = "nogod";
    static constexpr char NO_GOTO[] = "nogoto";
    static constexpr char NO_JUMP_DELAY[] = "nojumpdelay";
    static constexpr char NO_NOCLIP[] = "nonoclip";
    static constexpr char NO_SAVE[] = "nosave";
    static constexpr char NO_OVERBOUNCE[] = "nooverbounce";
    static constexpr char NO_PRONE[] = "noprone";
    static constexpr char NO_WALLBUG[] = "nowallbug";
    static constexpr char OVERBOUCNE_PLAYERS[] = "overbounce_players";
    static constexpr char PORTALGUN_SPAWN[] = "portalgun_spawn";
    static constexpr char PORTAL_SURFACES[] = "portalsurfaces";
    static constexpr char PORTAL_PREDICT[] = "portalpredict";
    static constexpr char PORTAL_TEAM[] = "portalteam";
    static constexpr char LIMITED_SAVES[] = "limitedsaves";
    static constexpr char STRICT_SAVE_LOAD[] = "strictsaveload";
  };

  enum class NoExplosives {
    OFF = 0,
    NO_EXPLOSIVE_WEAPONS = 1,
    NO_DYNAMITE = 2,
  };

  enum class NoFallDamage {
    OFF = 0,
    ON = 1,
    FORCE_ON = 2,
  };

  enum class OverbouncePlayers {
    OFF = 0,
    ALWAYS = 1,
    NEVER = 2,
  };

  NoExplosives noExplosives{};
  NoFallDamage noFallDamage{};
  OverbouncePlayers overbouncePlayers{};
  PortalTeam portalTeam{};
  EnumBitset<SaveSystem::SaveLoadRestrictions> strictSaveLoad;

  int32_t limitedSaves{};

  bool noDrop{};
  bool noFTNoGhost{};
  bool noFTSaveLimit{};
  bool noFTTeamjumpMode{};
  bool noGhost{};
  bool noGod{};
  bool noGoto{};
  bool noJumpDelay{};
  bool noNoclip{};
  bool noSave{};
  bool noOverbounce{};
  bool noProne{};
  bool noWallbug{};
  bool portalgunSpawn{};
  bool portalSurfaces{};
  bool portalPredict{};

  Worldspawn();
  ~Worldspawn() = default;

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

  static void printKeyValue(const std::string &key, const std::string &value);

  std::vector<std::pair<std::string, std::function<void()>>> keys = {
      {Keys::NO_DROP, [this]() { initNoDrop(Keys::NO_DROP); }},
      {Keys::NO_EXPLOSIVES,
       [this]() { initNoExplosives(Keys::NO_EXPLOSIVES); }},
      {Keys::NO_FALL_DAMAGE,
       [this]() { initNoFallDamage(Keys::NO_FALL_DAMAGE); }},
      {Keys::NO_FT_NO_GHOST,
       [this]() { initNoFTNoGhost(Keys::NO_FT_NO_GHOST); }},
      {Keys::NO_FT_SAVE_LIMIT,
       [this]() { initNoFTSaveLimit(Keys::NO_FT_SAVE_LIMIT); }},
      {Keys::NO_FT_TJ_MODE,
       [this]() { initNoFTTeamjumpMode(Keys::NO_FT_TJ_MODE); }},
      {Keys::NO_GHOST, [this]() { initNoGhost(Keys::NO_GHOST); }},
      {Keys::NO_GOD, [this]() { initNoGod(Keys::NO_GOD); }},
      {Keys::NO_GOTO, [this]() { initNoGoto(Keys::NO_GOTO); }},
      {Keys::NO_JUMP_DELAY, [this]() { initNoJumpDelay(Keys::NO_JUMP_DELAY); }},
      {Keys::NO_NOCLIP, [this]() { initNoNoclip(Keys::NO_NOCLIP); }},
      {Keys::NO_SAVE, [this]() { initNoSave(Keys::NO_SAVE); }},
      {Keys::NO_OVERBOUNCE,
       [this]() { initNoOverbounce(Keys::NO_OVERBOUNCE); }},
      {Keys::NO_PRONE, [this]() { initNoProne(Keys::NO_PRONE); }},
      {Keys::NO_WALLBUG, [this]() { initNoWallbug(Keys::NO_WALLBUG); }},
      {Keys::OVERBOUCNE_PLAYERS,
       [this]() { initOverbouncePlayers(Keys::OVERBOUCNE_PLAYERS); }},
      {Keys::PORTALGUN_SPAWN,
       [this]() { initPortalgunSpawn(Keys::PORTALGUN_SPAWN); }},
      {Keys::PORTAL_SURFACES,
       [this]() { initPortalSurfaces(Keys::PORTAL_SURFACES); }},
      {Keys::PORTAL_PREDICT,
       [this]() { initPortalPredict(Keys::PORTAL_PREDICT); }},
      {Keys::PORTAL_TEAM, [this]() { initPortalTeam(Keys::PORTAL_TEAM); }},
      {Keys::LIMITED_SAVES,
       [this]() { initLimitedSaves(Keys::LIMITED_SAVES); }},
      {Keys::STRICT_SAVE_LOAD,
       [this]() { initStrictSaveLoad(Keys::STRICT_SAVE_LOAD); }},
  };
};
} // namespace ETJump
