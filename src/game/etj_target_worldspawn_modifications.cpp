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

#include "etj_target_worldspawn_modifications.h"
#include "etj_worldspawn.h"
#include "bg_public.h"

namespace ETJump {
void TargetWorldspawnModifications::spawn(gentity_t *ent) {
  const std::vector<const char *> supportedKeys = {
      Worldspawn::Keys::NO_GOD,
      Worldspawn::Keys::NO_GOTO,
      Worldspawn::Keys::NO_JUMP_DELAY,
      Worldspawn::Keys::NO_NOCLIP,
      Worldspawn::Keys::NO_SAVE,
      Worldspawn::Keys::NO_OVERBOUNCE,
      Worldspawn::Keys::NO_PRONE,
      Worldspawn::Keys::NO_WALLBUG,
      Worldspawn::Keys::OVERBOUCNE_PLAYERS,
      Worldspawn::Keys::PORTALGUN_SPAWN,
  };

  EnumBitset<WSModificationsFlags> spawnflags(ent->spawnflags);

  // at least one team must be selected
  if (!(spawnflags & WSModificationsFlags::IGNORE_AXIS) &&
      !(spawnflags & WSModificationsFlags::IGNORE_ALLIES) &&
      !(spawnflags & WSModificationsFlags::IGNORE_SPEC)) {
    G_Error("%s: no team selected.", ent->classname);
  }

  // can't choose to ignore both timerun states
  if (spawnflags & WSModificationsFlags::IGNORE_TIMERUN_ONLY &&
      spawnflags & WSModificationsFlags::IGNORE_NO_TIMERUN) {
    G_Error("%s: TIMERUN_ONLY and NO_TIMERUN cannot be set on the same entity.",
            ent->classname);
  }

  int32_t value = 0;

  for (const auto &key : supportedKeys) {
    if (G_SpawnInt(key, "0", &value)) {
      Worldspawn::KeyModification keyMod{};
      keyMod.key = key;
      keyMod.value = static_cast<int32_t>(spawnflags);

      game.worldspawn->addKeyModification(keyMod);
    }
  }

  // we don't need to keep this entity around, all the necessary data is stored
  // in 'Worldspawn' object, so just free it to not take up an entity slot
  G_FreeEntity(ent);
}
} // namespace ETJump
