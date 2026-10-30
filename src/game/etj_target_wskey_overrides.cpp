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

#include "etj_target_wskey_overrides.h"
#include "etj_shared.h"
#include "etj_worldspawn.h"

namespace ETJump {
void TargetWSKeyOverrides::spawn(gentity_t *ent) {
  EnumBitset<WorldspawnShared::KeyOverrideFlags> spawnflags(ent->spawnflags);

  // one team must be selected
  if (!(spawnflags & WorldspawnShared::KeyOverrideFlags::AXIS) &&
      !(spawnflags & WorldspawnShared::KeyOverrideFlags::ALLIES) &&
      !(spawnflags & WorldspawnShared::KeyOverrideFlags::SPECTATORS)) {
    G_Error("%s: no team selected.", ent->classname);
  }

  // can't choose to ignore both timerun states
  if (spawnflags & WorldspawnShared::KeyOverrideFlags::TIMERUN_ONLY &&
      spawnflags & WorldspawnShared::KeyOverrideFlags::NO_TIMERUN) {
    G_Error("%s: TIMERUN_ONLY and NO_TIMERUN cannot be set on the same entity.",
            ent->classname);
  }

  for (const auto &desc : WorldspawnShared::keyDescriptors) {
    int32_t value = 0;

    // '0' is a throwaway value, not the real default: 'G_SpawnInt' returns
    // whether the key was present, and we only apply the value in that case.
    // Omitted keys mean "no override present", so defaulting the value to 0
    // does not actually add an override with 0 as value.
    if (G_SpawnInt(desc.keyName, "0", &value)) {
      if (!desc.allowOverride) {
        G_Printf(S_COLOR_YELLOW "%s: key '%s' does not allow overriding.\n",
                 ent->classname, desc.keyName);
        continue;
      }

      game.worldspawn->addKeyOverride({desc.key, value, spawnflags});
    }
  }

  // we don't need to keep this entity around, worldspawn object holds the data
  G_FreeEntity(ent);
}
} // namespace ETJump
