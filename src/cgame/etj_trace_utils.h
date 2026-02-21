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

#include <vector>

#include "../game/q_shared.h"

namespace ETJump {
class TraceUtils {
public:
  TraceUtils() = default;
  ~TraceUtils() = default;

  void filteredTrace(int32_t clientNum, trace_t *result, const vec3_t start,
                     const vec3_t mins, const vec3_t maxs, const vec3_t end,
                     int32_t skipNumber, int32_t mask);

  // flamechunks might be owned by players or 'props_flamethrower' entity,
  // which means we don't need to always do conditional filtering
  void flamechunkTrace(int32_t clientNum, trace_t *result, const vec3_t start,
                       const vec3_t mins, const vec3_t maxs, const vec3_t end,
                       int32_t skipNumber, int32_t mask);

  // prediction code gets pretty awkward & inefficient if we set/reset
  // entities for every single trace, so expose these so they can be
  // called directly in 'CG_PredictPlayerState' before/after 'Pmove'
  void setupIgnoredEntities(int32_t clientNum);
  void resetIgnoredEntities();

  bool entityIsIgnored(int32_t entityNum);

private:
  std::vector<int32_t> tempTraceIgnoredEnts;
};
} // namespace ETJump
