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

#include <algorithm>

#include "etj_trace_utils.h"
#include "cg_local.h"
#include "etj_utilities.h"

#include "../game/etj_entity_utilities_shared.h"

namespace ETJump {
void TraceUtils::filteredTrace(const int32_t clientNum, trace_t *result,
                               const vec3_t start, const vec3_t mins,
                               const vec3_t maxs, const vec3_t end,
                               const int32_t skipNumber, const int32_t mask) {
  setupIgnoredEntities(clientNum);

  CG_Trace(result, start, mins, maxs, end, skipNumber, mask);

  tempTraceIgnoredEnts.clear();
}

void TraceUtils::flamechunkTrace(const int32_t clientNum, trace_t *result,
                                 const vec3_t start, const vec3_t mins,
                                 const vec3_t maxs, const vec3_t end,
                                 const int32_t skipNumber, const int32_t mask) {
  // 'props_flamethrower' chunks do no need filtering
  if (!isValidClientNum(clientNum)) {
    CG_Trace(result, start, mins, maxs, end, skipNumber, mask);
    return;
  }

  filteredTrace(clientNum, result, start, mins, maxs, end, skipNumber, mask);
}

void TraceUtils::setupIgnoredEntities(const int32_t clientNum) {
  for (int32_t i = 0; i < cg.snap->numEntities; i++) {
    const entityState_t *es = &cg.snap->entities[i];

    switch (es->eType) {
      case ET_PLAYER:
        if (!playerIsSolid(clientNum, es->number) ||
            playerIsNoclipping(es->number)) {
          tempTraceIgnoredEnts.emplace_back(es->number);
        }

        break;
      case ET_STATIC_CLIENT:
        if (EntityUtilsShared::funcStaticClientIsHidden(es, clientNum)) {
          tempTraceIgnoredEnts.emplace_back(es->number);
        }

        break;
      default:
        break;
    }
  }
}

void TraceUtils::resetIgnoredEntities() { tempTraceIgnoredEnts.clear(); }

bool TraceUtils::entityIsIgnored(const int32_t entityNum) {
  return std::find(tempTraceIgnoredEnts.cbegin(), tempTraceIgnoredEnts.cend(),
                   entityNum) != tempTraceIgnoredEnts.cend();
}
} // namespace ETJump
