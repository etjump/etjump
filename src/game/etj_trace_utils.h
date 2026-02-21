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

#include "q_shared.h"

namespace ETJump {
class TraceUtils {
public:
  // 'G_Trace'
  static void filteredTrace(int32_t clientNum, trace_t *results,
                            const vec3_t start, const vec3_t mins,
                            const vec3_t maxs, const vec3_t end,
                            int32_t passEntityNum, int32_t contentmask);

  // 'G_HistoricalTrace'
  static void filteredHistoricalTrace(int32_t clientNum, trace_t *results,
                                      const vec3_t start, const vec3_t mins,
                                      const vec3_t maxs, const vec3_t end,
                                      int32_t passEntityNum,
                                      int32_t contentmask);

  // 'trap_Trace'
  static void filteredTraceNoBody(int32_t clientNum, trace_t *results,
                                  const vec3_t start, const vec3_t mins,
                                  const vec3_t maxs, const vec3_t end,
                                  int32_t passEntityNum, int32_t contentmask);

  // special trace for bullets to account for them traveling
  // through 'func_explosive' entities when they are shot and destroyed
  // any regular shot will have the same 'sourceNum' and 'attackerNum',
  // but if the shot breaks a 'func_explosive' entity, 'sourceNum'
  // will be the entityNum of the entity that was broken
  // see 'Bullet_Fire_Extended' for more details
  // NOTE: this doesn't take mins/maxs as it's always 'nullptr' for bullets
  static void filteredBulletTrace(int32_t sourceNum, int32_t attackerNum,
                                  trace_t *results, const vec3_t start,
                                  const vec3_t end, int32_t passEntityNum,
                                  int32_t contentmask);

  // projectiles might be spawned by entities in the map,
  // e.g. shooter entities or 'props_flamethrower', which don't need filtering
  static void projectileTrace(int32_t clientNum, trace_t *results,
                              const vec3_t start, const vec3_t mins,
                              const vec3_t maxs, const vec3_t end,
                              int32_t passEntityNum, int32_t contentmask);

  static void setupIgnoredEntities(int32_t clientNum);
};
} // namespace ETJump
