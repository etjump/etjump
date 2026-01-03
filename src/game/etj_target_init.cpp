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
#include <array>
#include <cstring>

#include "g_local.h"
#include "etj_target_init.h"
#include "etj_utilities.h"

namespace ETJump {
static constexpr std::array<weapon_t, 7> allowedWeapons = {
    WP_NONE,   WP_KNIFE,      WP_MEDKIT, WP_MEDIC_SYRINGE, WP_MEDIC_ADRENALINE,
    WP_PLIERS, WP_BINOCULARS,
};

void TargetInit::spawn(gentity_t *self) { self->use = use; }

void TargetInit::use(gentity_t *self, gentity_t *other, gentity_t *activator) {
  if (!activator || !activator->client) {
    return;
  }

  if (activator->client->sess.sessionTeam == TEAM_SPECTATOR) {
    return;
  }

  if (!(self->spawnflags & static_cast<int>(SpawnFlags::KeepHealth))) {
    activator->health = activator->client->ps.stats[STAT_MAX_HEALTH];
  }

  if (!(self->spawnflags & static_cast<int>(SpawnFlags::KeepAmmo))) {
    ResetPlayerAmmo(activator->client, activator);
  }

  if (!(self->spawnflags & static_cast<int>(SpawnFlags::KeepWeapons))) {
    const int weapon = activator->client->sess.playerWeapon;

    // we might have picked up the portal gun and are not removing it
    bool keepPortalgun = false;

    if (self->spawnflags & static_cast<int>(SpawnFlags::KeepPortalgun) &&
        COM_BitCheck(activator->client->ps.weapons, WP_PORTAL_GUN)) {
      keepPortalgun = true;
    }

    // restore the ammo we spawned with in case we don't have our weapon anymore
    if (!COM_BitCheck(activator->client->ps.weapons, weapon)) {
      activator->client->ps.ammo[weapon] =
          activator->client->sess.ammoOnSpawn[weapon];
      activator->client->ps.ammoclip[weapon] =
          activator->client->sess.ammoclipOnSpawn[weapon];
    }

    memcpy(activator->client->ps.weapons,
           activator->client->sess.weaponsOnSpawn,
           sizeof(activator->client->sess.weaponsOnSpawn));

    if (keepPortalgun &&
        !COM_BitCheck(activator->client->ps.weapons, WP_PORTAL_GUN)) {
      AddWeaponToPlayer(activator->client, static_cast<weapon_t>(WP_PORTAL_GUN),
                        0, 1, qfalse);
    }

    // remove any disallowed weapons we might have restored if timerun is active
    if (activator->client->sess.timerunActive) {
      for (auto i = 0; i < WP_NUM_WEAPONS; i++) {
        if (BG_WeaponDisallowedInTimeruns(i)) {
          COM_BitClear(activator->client->ps.weapons, i);
        }
      }
    }

    // swap to a valid weapon if our current weapon was removed
    if (!COM_BitCheck(activator->client->ps.weapons,
                      activator->client->ps.weapon)) {
      Utilities::selectValidWeapon(activator);
    }
  }

  if (!(self->spawnflags & static_cast<int>(SpawnFlags::KeepPortalgun))) {
    // swap weapons in case portalgun was equipped
    if (activator->client->ps.weapon == WP_PORTAL_GUN) {
      Utilities::selectValidWeapon(activator);
    }

    COM_BitClear(activator->client->ps.weapons, WP_PORTAL_GUN);
    activator->client->ps.ammo[WP_PORTAL_GUN] = 0;
    activator->client->ps.ammoclip[WP_PORTAL_GUN] = 0;
  }

  if (!(self->spawnflags & static_cast<int>(SpawnFlags::KeepHoldable))) {
    // we can just utilize target_remove_powerups here
    Use_target_remove_powerups(self, other, activator);
  }

  if (!(self->spawnflags & static_cast<int>(SpawnFlags::KeepIdent))) {
    activator->client->sess.clientMapProgression = 0;
  }

  if (!(self->spawnflags & static_cast<int>(SpawnFlags::KeepTracker))) {
    std::memset(activator->client->pers.progression, 0,
                sizeof(activator->client->pers.progression));
  }

  if (self->spawnflags & static_cast<int>(SpawnFlags::RemoveStartingWeapons) &&
      !(self->spawnflags & static_cast<int>(SpawnFlags::KeepWeapons))) {
    for (int i = WP_NONE; i < WP_NUM_WEAPONS; i++) {
      if (std::find(allowedWeapons.cbegin(), allowedWeapons.cend(), i) !=
          allowedWeapons.cend()) {
        continue;
      }

      // skip portal gun if 'KeepPortalgun' is set
      if (i == WP_PORTAL_GUN &&
          self->spawnflags & static_cast<int>(SpawnFlags::KeepPortalgun)) {
        continue;
      }

      COM_BitClear(activator->client->ps.weapons, i);
      // swap to knife if our weapon was removed
      if (activator->client->ps.weapon == i) {
        activator->client->ps.weapon = WP_KNIFE;
      }
    }

    activator->client->ps.grenadeTimeLeft = 0;
  }
}
} // namespace ETJump
