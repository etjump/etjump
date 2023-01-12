/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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

#include "g_local.h"
#include "etj_missilepad.h"

namespace ETJump {
void Missilepad::touch(gentity_t *self, gentity_t *other) {
  // could check for classnames "m7_grenade" and "gpg40_grenade" here
  // but MOD is likely the simplest and marginally faster than string compare
  // we CANNOT check for ent->s.weapon here
  // because a client might be holding the weapon
  if (!(other->methodOfDeath == MOD_GPG40 || other->methodOfDeath == MOD_M7)) {
    return;
  }

  G_UseTargets(self, other);
}

void Missilepad::use(gentity_t *ent) {
  if (ent->r.linked) {
    trap_UnlinkEntity(ent);
  } else {
    trap_LinkEntity(ent);
  }
}

void Missilepad::spawn(gentity_t *ent) {
  trap_SetBrushModel(ent, ent->model);
  InitMover(ent);
  VectorCopy(ent->s.origin, ent->s.pos.trBase);
  VectorCopy(ent->s.origin, ent->r.currentOrigin);
  ent->use = [](gentity_t *ent, gentity_t *other, gentity_t *activator) {
    Missilepad::use(ent);
  };

  if (ent->spawnflags & static_cast<int>(Spawnflags::StartInvis)) {
    trap_UnlinkEntity(ent);
  }

  ent->touch = [](gentity_t *ent, gentity_t *activator, trace_t *trace) {
    Missilepad::touch(ent, activator);
  };
}
} // namespace ETJump

void SP_func_missilepad(gentity_t *ent) { ETJump::Missilepad::spawn(ent); }
