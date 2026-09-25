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
// portal trail colors
inline constexpr vec3_t portalBlueTrail = {0.0f, 0.0f, 1.0f};
inline constexpr vec3_t portalRedTrail = {1.0f, 0.1f, 0.0f};
inline constexpr vec3_t portalGreenTrail = {0.2f, 1.0f, 0.5f};
inline constexpr vec3_t portalYellowTrail = {1.0f, 1.0f, 0.0f};

// portalteam
enum class PortalTeamOpts {
  OFF = 0,
  FIRETEAM = 1,
  ALL = 2,
};

// cooldown between portal touch events (ms)
inline constexpr int PORTAL_TOUCH_COOLDOWN = 100;

// radius of portal bbox
inline constexpr float PORTAL_BBOX_RADIUS = 30.0f;
// depth for horizontal portals
inline constexpr float PORTAL_BBOX_HOR_DEPTH = 15.0f;
// depth for vertical portals
inline constexpr float PORTAL_BBOX_VERT_DEPTH = 5.0f;

// radius for portal shader drawing
inline constexpr float PORTAL_DRAW_RADIUS = 48.0f;
// scalar for drawing the portal shader proportional to the portal size
inline constexpr float PORTAL_DRAW_SCALAR =
    PORTAL_DRAW_RADIUS / (PORTAL_BBOX_RADIUS * 2);
// animation duration for portal spawning
inline constexpr int32_t PORTAL_SPAWN_ANIM_DURATION = 200;

// max range where you can place next portal gate
inline constexpr float MAX_PORTAL_RANGE = 2 << 16;
// min angle difference between two portals, used to avoid overlapping
inline constexpr float MIN_ANGLES_DIFF = 100.0f;
// min distance between two portal center points, used to avoid overlapping
inline constexpr float MIN_PORTALS_DIST = 75.0f / 2;
// max value of 'portalsize' key on portal target entities
inline constexpr int32_t MAX_PORTAL_TARGET_SIZE = 512;
// distance the portal origin is pushed out of the surface it's attached to
inline constexpr float PORTAL_SURFACE_OFFSET = 5.0f;

// Portal placement logic shared between server (authoritative spawning)
// and client (predicted spawning). Tracing and entity lookups are
// module-specific, so these operate on the results of those.
namespace PortalgunShared {
struct PortalTarget {
  // origin or bmodel center of the portal target entity
  vec3_t center{};
  // 'portalsize' key, 0 = default size
  int32_t size{};
};

struct Placement {
  // origin of the portal entity, slightly pushed out of the surface
  vec3_t origin{};
  // derived from the surface normal
  vec3_t angles{};
  // point on the surface the portal is attached to, trail end point
  vec3_t surfacePoint{};
  float scale = 1.0f;
};

// portal gun traces start from the eye position
void traceEndpoints(const vec3_t origin, int32_t viewheight,
                    const vec3_t viewangles, vec3_t start, vec3_t end);

// start point of the portal trail, matches 'CalcMuzzlePoint' on server
void muzzlePoint(const vec3_t origin, int32_t viewheight,
                 const vec3_t viewangles, vec3_t out);

// checks if the surface hit by the portal gun trace can hold a portal
[[nodiscard]] bool surfaceAllowsPortal(const trace_t &tr, bool portalSurfaces);

// 'target' is nullptr if the trace didn't hit a portal target entity
void computePlacement(const trace_t &tr, const PortalTarget *target,
                      Placement &out);

// 'otherSize' is the size of the other portal, as stored in 'onFireStart'
[[nodiscard]] bool portalsOverlap(const Placement &placement,
                                  const vec3_t otherOrigin,
                                  const vec3_t otherAngles, int32_t otherSize);

// portal size (diameter) stored in portal entities 'onFireStart'
[[nodiscard]] int32_t sizeFromScale(float scale);
[[nodiscard]] float scaleFromSize(int32_t size);

// Portal target entities ('func_portaltarget' and 'func_static_client' with
// portal target spawnflag) network their info in the entity state:
// - 'onFireEnd' - 'PORTAL_TARGET_FLAG' | 'portalsize'
// - 'origin2' - origin or bmodel center of the entity
// A flag is used rather than any non-zero value, because 'trigger_heal' and
// 'trigger_ammo' write their charge into 'onFireEnd' of the entity they target.
// This is only used by the client for prediction,
// server uses the entity itself to determine portal targets.
inline constexpr int32_t PORTAL_TARGET_FLAG = 1 << 24;

void setPortalTarget(entityState_t &es, int32_t size, const vec3_t center);
[[nodiscard]] bool isPortalTarget(const entityState_t &es);
void getPortalTarget(const entityState_t &es, PortalTarget &out);
[[nodiscard]] int32_t encodePortalTarget(int32_t size);
[[nodiscard]] int32_t portalTargetSize(const entityState_t &es);
} // namespace PortalgunShared
} // namespace ETJump
