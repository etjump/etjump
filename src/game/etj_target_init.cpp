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

#include <algorithm>
#include <array>

#include "g_local.h"
#include "etj_target_init.h"

namespace ETJump {
std::vector<weapon_t> allowedWeapons = {
    WP_NONE,   WP_KNIFE,      WP_MEDKIT, WP_MEDIC_SYRINGE, WP_MEDIC_ADRENALINE,
    WP_PLIERS, WP_BINOCULARS,
};

void TargetInit::spawn(gentity_t *self) { self->use = TargetInit::use; }

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
    if (!(COM_BitCheck(activator->client->ps.weapons, weapon))) {
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
      COM_BitSet(activator->client->ps.weapons, WP_PORTAL_GUN);
    }

    // remove any disallowed weapons we might have restored if timerun is active
    if (activator->client->sess.timerunActive) {
      for (auto i = 0; i < WP_NUM_WEAPONS; i++) {
        if (BG_WeaponDisallowedInTimeruns(i)) {
          COM_BitClear(activator->client->ps.weapons, i);
        }
      }
    }
    // swap to the restored weapon if our current weapon was removed
    if (!(COM_BitCheck(activator->client->ps.weapons,
                       activator->client->ps.weapon))) {
      activator->client->ps.weapon = weapon;
    }
  }

  if (!(self->spawnflags & static_cast<int>(SpawnFlags::KeepPortalgun))) {
    // swap weapons in case portalgun was equipped
    // if we have ammo: primary > secondary > knife
    if (activator->client->ps.weapon == WP_PORTAL_GUN) {
      if (BG_WeaponHasAmmo(&activator->client->ps,
                           activator->client->sess.playerWeapon)) {
        activator->client->ps.weapon = activator->client->sess.playerWeapon;
      } else if (BG_WeaponHasAmmo(&activator->client->ps,
                                  activator->client->sess.playerWeapon2)) {
        activator->client->ps.weapon = activator->client->sess.playerWeapon2;
      } else {
        activator->client->ps.weapon = WP_KNIFE;
      }
    }

    COM_BitClear(activator->client->ps.weapons, WP_PORTAL_GUN);
  }

  if (!(self->spawnflags & static_cast<int>(SpawnFlags::KeepHoldable))) {
    // we can just utilize target_remove_powerups here
    Use_target_remove_powerups(self, other, activator);
  }

  if (!(self->spawnflags & static_cast<int>(SpawnFlags::KeepIdent))) {
    activator->client->sess.clientMapProgression = 0;
  }

  if (!(self->spawnflags & static_cast<int>(SpawnFlags::KeepTracker))) {
    for (auto &tracker : activator->client->sess.progression) {
      tracker = 0;
    }
  }

  if (self->spawnflags & static_cast<int>(SpawnFlags::RemoveStartingWeapons) &&
      !(self->spawnflags & static_cast<int>(SpawnFlags::KeepWeapons))) {
    for (int i = 0; i < WP_NUM_WEAPONS; i++) {
      if (std::find(allowedWeapons.begin(), allowedWeapons.end(), i) !=
          allowedWeapons.end()) {
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
