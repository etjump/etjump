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

#include "g_local.h"

namespace ETJump {
class Portal {
public:
  enum class Type {
    PORTAL_BLUE = 1,
    PORTAL_RED = 2,
  };

  static void spawn(gentity_t *ent, float scale, Type type, const trace_t &tr,
                    vec3_t end, const vec3_t angles);
  static void think(gentity_t *self);
  static void touch(gentity_t *self, gentity_t *other);
};

class Portalgun {
public:
  static void spawn(gentity_t *ent);
  static void touch(gentity_t *self, gentity_t *other, trace_t *trace);

  static void fire(gentity_t *ent, Portal::Type type, vec3_t forward,
                   vec3_t right, vec3_t up, vec3_t muzzleEffect);

private:
  static void portalgunTrace(gentity_t *ent, trace_t *tr, vec3_t start,
                             vec3_t end);
};
} // namespace ETJump
