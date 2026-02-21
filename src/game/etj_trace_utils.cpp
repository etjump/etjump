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

#include "etj_trace_utils.h"

#include "g_local.h"
#include "etj_entity_utilities.h"
#include "etj_entity_utilities_shared.h"

namespace ETJump {
void TraceUtils::filteredTrace(const int32_t clientNum, trace_t *results,
                               const vec3_t start, const vec3_t mins,
                               const vec3_t maxs, const vec3_t end,
                               const int32_t passEntityNum,
                               const int32_t contentmask) {
  setupIgnoredEntities(clientNum);

  G_Trace(g_entities + clientNum, results, start, mins, maxs, end,
          passEntityNum, contentmask);

  G_ResetTempTraceIgnoreEnts();
}

void TraceUtils::filteredHistoricalTrace(const int32_t clientNum,
                                         trace_t *results, const vec3_t start,
                                         const vec3_t mins, const vec3_t maxs,
                                         const vec3_t end,
                                         const int32_t passEntityNum,
                                         const int32_t contentmask) {
  setupIgnoredEntities(clientNum);

  G_HistoricalTrace(g_entities + clientNum, results, start, mins, maxs, end,
                    passEntityNum, contentmask);

  G_ResetTempTraceIgnoreEnts();
}

void TraceUtils::filteredTraceNoBody(const int32_t clientNum, trace_t *results,
                                     const vec3_t start, const vec3_t mins,
                                     const vec3_t maxs, const vec3_t end,
                                     const int32_t passEntityNum,
                                     const int32_t contentmask) {
  setupIgnoredEntities(clientNum);

  trap_Trace(results, start, mins, maxs, end, passEntityNum, contentmask);

  G_ResetTempTraceIgnoreEnts();
}

void TraceUtils::filteredBulletTrace(const int32_t sourceNum,
                                     const int32_t attackerNum,
                                     trace_t *results, const vec3_t start,
                                     const vec3_t end,
                                     const int32_t passEntityNum,
                                     const int32_t contentmask) {
  setupIgnoredEntities(attackerNum);

  G_Trace(g_entities + sourceNum, results, start, nullptr, nullptr, end,
          passEntityNum, contentmask);

  G_ResetTempTraceIgnoreEnts();
}

void TraceUtils::projectileTrace(const int32_t clientNum, trace_t *results,
                                 const vec3_t start, const vec3_t mins,
                                 const vec3_t maxs, const vec3_t end,
                                 const int32_t passEntityNum,
                                 const int32_t contentmask) {
  // projectiles not owned by players don't need filtering
  if (!EntityUtilities::isPlayer(g_entities + clientNum)) {
    trap_Trace(results, start, mins, maxs, end, passEntityNum, contentmask);
    return;
  }

  filteredTraceNoBody(clientNum, results, start, mins, maxs, end, passEntityNum,
                      contentmask);
}

void TraceUtils::setupIgnoredEntities(const int32_t clientNum) {
  for (int32_t i = 0; i < level.num_entities; i++) {
    gentity_t *const other = &g_entities[i];
    const entityState_t *es = &g_entities[i].s;

    switch (es->eType) {
      case ET_PLAYER:
        if (!EntityUtilities::playerIsSolid(clientNum, es->number)) {
          G_TempTraceIgnoreEntity(other);
        }

        break;
      case ET_STATIC_CLIENT:
        if (EntityUtilsShared::funcStaticClientIsHidden(es, clientNum)) {
          G_TempTraceIgnoreEntity(other);
        }

        break;
      default:
        break;
    }
  }
}
} // namespace ETJump
