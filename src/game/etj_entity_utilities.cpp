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

#include "etj_entity_utilities.h"

bool ETJump::isPlayer(gentity_t *ent) {
  auto cnum = ClientNum(ent);
  return cnum >= 0 && cnum < MAX_CLIENTS;
}

void ETJump::drawRailBox(const vec_t *origin, const vec_t *mins,
                         const vec_t *maxs, const vec_t *color, int index) {
  vec3_t b1;
  vec3_t b2;
  gentity_t *temp;

  VectorCopy(origin, b1);
  VectorCopy(origin, b2);
  VectorAdd(b1, mins, b1);
  VectorAdd(b2, maxs, b2);

  temp = G_TempEntity(b1, EV_RAILTRAIL);

  VectorCopy(b2, temp->s.origin2);
  VectorCopy(color, temp->s.angles);
  temp->s.dmgFlags = 1;

  temp->s.angles[0] = color[0] * 255;
  temp->s.angles[1] = color[1] * 255;
  temp->s.angles[2] = color[2] * 255;

  temp->s.effect1Time = index + 1;
}
