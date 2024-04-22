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

#include "etj_entity_utilities_shared.h"

// Entity helper functions used by both games (client side predicted entities)

namespace ETJump {
void EntityUtilsShared::touchPusher(playerState_t *ps, const int time,
                                    const entityState_t *pusher) {
  if (ps->pm_type != PM_NORMAL) {
    return;
  }

  // 'pusher->frame' hold the wait key value
  if (ps->powerups[PW_PUSHERPREDICT] + pusher->frame > time) {
    return;
  }

  ps->powerups[PW_PUSHERPREDICT] = time;

  vec3_t dir;
  float s;

  // first 8 bits holds noise_index
  if (pusher->constantLight & 0xff) {
    VectorNormalize2(pusher->origin2, dir);
    s = DotProduct(ps->velocity, dir);

    if (s < 500) {
      BG_AddPredictableEventToPlayerstate(EV_GENERAL_SOUND,
                                          pusher->constantLight & 0xff, ps);
    }
  }

  if (pusher->eType == ET_VELOCITY_PUSH_TRIGGER) {
    vec3_t pushVel{};

    // bits 8-23 holds spawnflags
    const int spawnflags = (pusher->constantLight >> 8) & 0xff;
    setPushVelocity(ps, pusher->origin2, spawnflags, pushVel);
    VectorCopy(pushVel, ps->velocity);
  } else {
    VectorCopy(pusher->origin2, ps->velocity);
  }
}

void EntityUtilsShared::setPushVelocity(const playerState_t *ps,
                                        const vec_t *origin2,
                                        const int &spawnflags, vec_t *pushVel) {
  VectorCopy(ps->velocity, pushVel);

  if (spawnflags & static_cast<int>(PusherSpawnFlags::AddXY) &&
      spawnflags & static_cast<int>(PusherSpawnFlags::AddZ)) {
    VectorAdd(origin2, ps->velocity, pushVel);
  } else {
    if (spawnflags & static_cast<int>(PusherSpawnFlags::AddXY)) {
      pushVel[0] += origin2[0];
      pushVel[1] += origin2[1];
      pushVel[2] = origin2[2];
    }

    if (spawnflags & static_cast<int>(PusherSpawnFlags::AddZ)) {
      pushVel[0] = origin2[0];
      pushVel[1] = origin2[1];
      pushVel[2] += origin2[2];
    }
  }
}

} // namespace ETJump
