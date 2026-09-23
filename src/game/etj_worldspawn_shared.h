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

#include <algorithm>
#include <vector>

#include "bg_public.h"
#include "etj_shared.h"
#include "q_shared.h"

// shared worldspawn data & methods needed on both client and server
namespace ETJump::WorldspawnShared {
inline constexpr char NO_OVERBOUNCE_CS[] = "ob";
inline constexpr char NO_JUMP_DELAY_CS[] = "njd";
inline constexpr char NO_SAVE_CS[] = "save";
inline constexpr char NO_PRONE_CS[] = "prone";
inline constexpr char NO_DROP_CS[] = "drop";
inline constexpr char NO_WALLBUG_CS[] = "wbug";
inline constexpr char NO_NOCLIP_CS[] = "noclip";
inline constexpr char PORTAL_PREDICT_CS[] = "ppred";
inline constexpr char NO_FALL_DAMAGE_CS[] = "fdmg";
inline constexpr char OVERBOUNCE_PLAYERS_CS[] = "obp";

enum class Keys {
  NO_DROP,
  NO_EXPLOSIVES,
  NO_FALL_DAMAGE,
  NO_FT_NO_GHOST,
  NO_FT_SAVE_LIMIT,
  NO_FT_TJ_MODE,
  NO_GHOST,
  NO_GOD,
  NO_GOTO,
  NO_JUMP_DELAY,
  NO_NOCLIP,
  NO_SAVE,
  NO_OVERBOUNCE,
  NO_PRONE,
  NO_WALLBUG,
  OVERBOUCNE_PLAYERS,
  PORTALGUN_SPAWN,
  PORTAL_SURFACES,
  PORTAL_PREDICT,
  PORTAL_TEAM,
  LIMITED_SAVES,
  STRICT_SAVE_LOAD,

  NUM_KEYS,
};

struct WSKeyDescriptor {
  WorldspawnShared::Keys key;
  const char *keyName;
  const char *csName;
  bool allowOverride;
};

struct KeyDescriptors {
  std::array<WSKeyDescriptor,
             static_cast<size_t>(WorldspawnShared::Keys::NUM_KEYS)>
      data;

  constexpr const WSKeyDescriptor &operator[](WorldspawnShared::Keys k) const {
    return data[static_cast<size_t>(k)];
  }

  [[nodiscard]] auto begin() const { return data.begin(); }
  [[nodiscard]] auto end() const { return data.end(); }
};

inline constexpr KeyDescriptors keyDescriptors = {{{
    {WorldspawnShared::Keys::NO_DROP, "nodrop", NO_DROP_CS},
    {WorldspawnShared::Keys::NO_EXPLOSIVES, "noexplosives", nullptr},
    {WorldspawnShared::Keys::NO_FALL_DAMAGE, "nofalldamage", NO_FALL_DAMAGE_CS},
    {WorldspawnShared::Keys::NO_FT_NO_GHOST, "noftnoghost", nullptr},
    {WorldspawnShared::Keys::NO_FT_SAVE_LIMIT, "noftsavelimit", nullptr},
    {WorldspawnShared::Keys::NO_FT_TJ_MODE, "noftteamjumpmode", nullptr},
    {WorldspawnShared::Keys::NO_GHOST, "noghost", nullptr},
    {WorldspawnShared::Keys::NO_GOD, "nogod", nullptr},
    {WorldspawnShared::Keys::NO_GOTO, "nogoto", nullptr},
    {WorldspawnShared::Keys::NO_JUMP_DELAY, "nojumpdelay", NO_JUMP_DELAY_CS},
    {WorldspawnShared::Keys::NO_NOCLIP, "nonoclip", NO_NOCLIP_CS, true},
    {WorldspawnShared::Keys::NO_SAVE, "nosave", NO_SAVE_CS, true},
    {WorldspawnShared::Keys::NO_OVERBOUNCE, "nooverbounce", NO_OVERBOUNCE_CS},
    {WorldspawnShared::Keys::NO_PRONE, "noprone", NO_PRONE_CS, true},
    {WorldspawnShared::Keys::NO_WALLBUG, "nowallbug", NO_WALLBUG_CS},
    {WorldspawnShared::Keys::OVERBOUCNE_PLAYERS, "overbounce_players",
     OVERBOUNCE_PLAYERS_CS},
    {WorldspawnShared::Keys::PORTALGUN_SPAWN, "portalgun_spawn", nullptr},
    {WorldspawnShared::Keys::PORTAL_SURFACES, "portalsurfaces", nullptr},
    {WorldspawnShared::Keys::PORTAL_PREDICT, "portalpredict",
     PORTAL_PREDICT_CS},
    {WorldspawnShared::Keys::PORTAL_TEAM, "portalteam", nullptr},
    {WorldspawnShared::Keys::LIMITED_SAVES, "limitedsaves", nullptr},
    {WorldspawnShared::Keys::STRICT_SAVE_LOAD, "strictsaveload", nullptr},
}}};

// ensure the init order matches the enum
static_assert(
    [] {
      for (size_t i = 0;
           i < static_cast<size_t>(WorldspawnShared::Keys::NUM_KEYS); i++) {
        if (static_cast<size_t>(keyDescriptors.data[i].key) != i) {
          return false;
        }
      }

      return true;
    }(),
    "keyDescriptors must be listed in the same order as Keys");

enum class KeyOverrideFlags {
  AXIS = 1 << 0,
  ALLIES = 1 << 1,
  SPECTATORS = 1 << 2,
  TIMERUN_ONLY = 1 << 3,
  NO_TIMERUN = 1 << 4,
};

struct KeyOverride {
  Keys key{};
  int32_t value{};
  EnumBitset<KeyOverrideFlags> flags;
};

inline bool areaAllowsAction(AreaOpts areaOpts, const trace_t &trace) {
  switch (areaOpts) {
    default: // AreaOpts::FORBID_INSIDE
      return trace.fraction == 1.0f;
    case AreaOpts::FORBID_OUTSIDE:
      return trace.fraction != 1.0f;
    case AreaOpts::ALLOW_EVERYWHERE:
      return true;
    case AreaOpts::FORBID_EVERYWHERE:
      return false;
  }
}

// Resolves effective worldspawn key values for a client, taking override
// values into account. This *should* be called on init, whenever team changes,
// and whenever timerun state changes, but realistically it's cheap enough to be
// called every frame.
inline SharedWorldspawnKeys
resolveSharedWSKeys(const SharedWorldspawnKeys &global,
                    const std::vector<KeyOverride> &overrides,
                    const team_t team, const bool timerunActive) {
  // globals are the baseline
  SharedWorldspawnKeys keys = global;

  if (overrides.empty()) {
    return keys;
  }

  EnumBitset<KeyOverrideFlags> teamBit;

  switch (team) {
    case TEAM_AXIS:
      teamBit |= KeyOverrideFlags::AXIS;
      break;
    case TEAM_ALLIES:
      teamBit |= KeyOverrideFlags::ALLIES;
      break;
    case TEAM_SPECTATOR:
      teamBit |= KeyOverrideFlags::SPECTATORS;
      break;
    default:
      break;
  }

  // this should be generalized into a lookup function perhaps,
  // if the number of supported keys grows significantly
  // (it also only "supports" AreaOpts currently)
  const auto setOverrideKey = [&keys](const KeyOverride &o) {
    const auto value =
        std::clamp(static_cast<AreaOpts>(o.value), AreaOpts::FORBID_INSIDE,
                   AreaOpts::FORBID_EVERYWHERE);

    switch (o.key) {
      case Keys::NO_NOCLIP:
        keys.noNoclip = value;
        break;
      case Keys::NO_SAVE:
        keys.noSave = value;
        break;
      case Keys::NO_PRONE:
        keys.noProne = value;
        break;
      default:
        break;
    }
  };

  // Overrides are applied in two passes because a timerun-stateless override
  // and a state-specific one for the same key/team can legally coexist (e.g.
  // "Allies can save anywhere, except while timerunning"). Both match a running
  // player, so the state-specific one must win. Applying in vector order
  // instead would make the result depend on entity spawn order, which isn't
  // deterministic. This is why we iterate the vector twice, and check for
  // stateless overrides first.

  for (const auto &o : overrides) {
    // team is mandatory in the entity,
    // so there's never an override without a team
    if (!(o.flags & teamBit)) {
      continue;
    }

    // not a timerun-stateless override
    if ((o.flags & KeyOverrideFlags::TIMERUN_ONLY) ||
        (o.flags & KeyOverrideFlags::NO_TIMERUN)) {
      continue;
    }

    setOverrideKey(o);
  }

  const auto timerunScope = timerunActive ? KeyOverrideFlags::TIMERUN_ONLY
                                          : KeyOverrideFlags::NO_TIMERUN;

  for (const auto &o : overrides) {
    // team is mandatory in the entity,
    // so there's never an override without a team
    if (!(o.flags & teamBit)) {
      continue;
    }

    // not applicable to current state
    if (!(o.flags & timerunScope)) {
      continue;
    }

    setOverrideKey(o);
  }

  return keys;
}
} // namespace ETJump::WorldspawnShared
