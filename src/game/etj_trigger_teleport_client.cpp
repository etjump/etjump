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

#include "g_local.h"
#include "etj_trigger_teleport_client.h"
#include "etj_numeric_utilities.h"
#include "etj_entity_utilities_shared.h"

namespace ETJump {
void TriggerTeleportClient::touch(gentity_t *self, gentity_t *other) {
  if (!other->client || other->client->ps.pm_type == PM_DEAD) {
    return;
  }

  EntityUtilsShared::teleportPlayer(&other->client->ps, &other->s, &self->s,
                                    &other->client->pers.cmd, self->s.origin2,
                                    self->s.angles2);
}

void TriggerTeleportClient::think(gentity_t *self) {
  gentity_t *target = nullptr;
  std::vector<gentity_t *> foundTargets{};

  const auto hash = static_cast<int>(BG_StringHashValue(self->target));

  while (true) {
    target = G_FindByTargetnameFast(target, self->target, hash);
    if (!target) {
      break;
    }

    foundTargets.emplace_back(target);

    if (foundTargets.size() > 1) {
      G_Printf(S_COLOR_YELLOW
               "WARNING: 'trigger_teleport_client' with multiple targets "
               "found, using the first available target\n");
      break;
    }
  }

  if (foundTargets.empty()) {
    G_Printf(S_COLOR_YELLOW
             "WARNING: 'trigger_teleport_client' without a target.\n");
    G_FreeEntity(self);
  } else {
    VectorCopy(foundTargets[0]->s.origin, self->s.origin2);
    VectorCopy(foundTargets[0]->s.angles, self->s.angles2);

    // massive hack incoming
    // set portal flag to force destination to be in PVS for prediction to work
    // when the teleporter destination is not currently in our PVS
    // entity with SVF_PORTAL flag will force the bsp leaf that origin2 lies
    // in to be included in the current PVS, and since we store
    // the destination origin there, this way we can force it to be "visible"
    // and prediction functions correctly even if the destination is in
    // a completely separate, enclosed area
    self->r.svFlags |= SVF_PORTAL;
  }

  self->nextthink = 0;
}

void TriggerTeleportClient::spawn(gentity_t *self) {
  InitTrigger(self);

  // 'think' re-checks this in scenarios where 'target' key points to
  // an entity that does not actually exist
  if (!self->target) {
    G_Printf(S_COLOR_YELLOW
             "WARNING: 'trigger_teleport_client' without a target.\n");
    G_FreeEntity(self);
    return;
  }

  // send to client for prediction
  self->r.svFlags &= ~SVF_NOCLIENT;

  char *noise;
  G_SpawnString("noise", "", &noise);
  self->noise_index = G_SoundIndex(noise);

  G_SpawnFloat("outspeed", "0", &self->speed);
  Numeric::clamp(self->speed, 0, UINT16_MAX);

  // 16 bits for outspeed, should be more than enough
  self->s.frame = static_cast<int>(self->speed);

  // 8 bits for sound, 24 bits for spawnflags
  // if we ever need more than 24 spawnflags, we can move the sound elsewhere,
  // there's plenty of fields free to reuse for teleporters,
  // but for now lets conserve the available resources
  self->s.constantLight |= self->noise_index & 0xff;
  self->s.constantLight |= (self->spawnflags << 8) & 0xffffff;

  self->s.eType = ET_TELEPORT_TRIGGER_CLIENT;
  self->touch = [](gentity_t *self, gentity_t *other, trace_t *touch) {
    TriggerTeleportClient::touch(self, other);
  };

  self->think = think;
  self->nextthink = level.time + FRAMETIME;

  trap_LinkEntity(self);
}

} // namespace ETJump
