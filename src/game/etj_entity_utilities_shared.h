/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
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

#ifdef CGAMEDLL
  #include "../cgame/cg_local.h"
#else
  #include "g_local.h"
#endif

namespace ETJump {
class EntityUtilsShared {
public:
  static void touchPusher(playerState_t *ps, int time,
                          const entityState_t *pusher);

  static void setPushVelocity(const playerState_t *ps, const vec3_t origin2,
                              const int &spawnflags, vec3_t pushVel);

  static void teleportPlayer(playerState_t *ps, entityState_t *player,
                             entityState_t *teleporter, usercmd_t *cmd,
                             const vec3_t origin, vec3_t angles);

  static void setViewAngles(playerState_t *ps, entityState_t *es,
                            usercmd_t *cmd, const vec3_t angle);

  static void portalTeleport(playerState_t *ps, entityState_t *player,
                             const entityState_t *portal, usercmd_t *cmd,
                             int time);

  static void setPortalBBox(vec3_t mins, vec3_t maxs, const vec3_t angles,
                            float scale);
};
} // namespace ETJump
