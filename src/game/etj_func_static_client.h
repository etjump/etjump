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

#include "g_local.h"

namespace ETJump {
class FuncStaticClient {
public:
  class Spawnflags {
  public:
    static constexpr int32_t START_INVIS = 1 << 0;
    static constexpr int32_t PAIN = 1 << 1;
    static constexpr int32_t GIB_INSIDE = 1 << 2;
    static constexpr int32_t FT_TEAMJUMP_SYNC = 1 << 3;
    static constexpr int32_t CONSUME_PORTALS = 1 << 4;
    static constexpr int32_t PORTAL_TARGET = 1 << 5;
  };

  static void spawn(gentity_t *ent);
  static void use(gentity_t *self, gentity_t *other, gentity_t *activator);
  static void pain(gentity_t *self, gentity_t *attacker, int32_t damage,
                   vec3_t point);

  static void syncToFireteamLeaderState(int32_t clientNum, int32_t leaderNum);

private:
  static void turnOn(gentity_t *self, int32_t clientNum);
  static void turnOff(gentity_t *self, int32_t clientNum);
  static bool activatorIsInsideEnt(const gentity_t *self, int32_t clientNum);
  static void deleteTouchingPortals(const gentity_t *self, int32_t clientNum);
  static void scriptEvent(gentity_t *self, const char *trigger);
};
} // namespace ETJump
