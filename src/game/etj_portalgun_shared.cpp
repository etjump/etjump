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
#include <cmath>

#include "etj_portalgun_shared.h"

namespace ETJump::PortalgunShared {
void traceEndpoints(const vec3_t origin, const int32_t viewheight,
                    const vec3_t viewangles, vec3_t start, vec3_t end) {
  vec3_t forward;

  VectorCopy(origin, start);
  start[2] += static_cast<float>(viewheight);

  AngleVectors(viewangles, forward, nullptr, nullptr);
  VectorMA(start, MAX_PORTAL_RANGE, forward, end);
}

void muzzlePoint(const vec3_t origin, const int32_t viewheight,
                 const vec3_t viewangles, vec3_t out) {
  vec3_t right;
  vec3_t up;

  AngleVectors(viewangles, nullptr, right, up);

  VectorCopy(origin, out);
  out[2] += static_cast<float>(viewheight);
  VectorMA(out, 2, right, out);
  VectorMA(out, -4, up, out);
}

bool surfaceAllowsPortal(const trace_t &tr, const bool portalSurfaces) {
  if (tr.surfaceFlags & SURF_NOIMPACT || tr.fraction == 1.0f) {
    return false;
  }

  // portalclip or player = no portal
  if (tr.contents & (CONTENTS_PORTALCLIP | CONTENTS_BODY)) {
    return false;
  }

  // 'portalsurfaces 1' - portals allowed everywhere, except on portalsurfaces
  // 'portalsurfaces 0' - portals allowed only on portalsurfaces
  const bool isPortalSurface = tr.surfaceFlags & SURF_PORTALSURFACE;
  return portalSurfaces ? !isPortalSurface : isPortalSurface;
}

void computePlacement(const trace_t &tr, const PortalTarget *target,
                      Placement &out) {
  vectoangles(tr.plane.normal, out.angles);
  out.scale = 1.0f;

  // we hit an entity that wants portals to be centered,
  // so project the entity center onto the surface plane
  if (target) {
    vec3_t delta;
    vec3_t normalScaled;

    VectorSubtract(target->center, tr.endpos, delta);
    const float dotProduct = DotProduct(delta, tr.plane.normal);
    VectorScale(tr.plane.normal, dotProduct, normalScaled);
    VectorSubtract(target->center, normalScaled, out.surfacePoint);

    if (target->size > 0) {
      out.scale = scaleFromSize(target->size);
    }
  } else {
    VectorCopy(tr.endpos, out.surfacePoint);
  }

  VectorMA(out.surfacePoint, PORTAL_SURFACE_OFFSET, tr.plane.normal,
           out.origin);
}

bool portalsOverlap(const Placement &placement, const vec3_t otherOrigin,
                    const vec3_t otherAngles, const int32_t otherSize) {
  const float otherScale = scaleFromSize(otherSize);
  const float minDist =
      (MIN_PORTALS_DIST * placement.scale) + (MIN_PORTALS_DIST * otherScale);

  return Distance(placement.angles, otherAngles) < MIN_ANGLES_DIFF &&
         Distance(placement.origin, otherOrigin) < minDist;
}

int32_t sizeFromScale(const float scale) {
  return static_cast<int32_t>(PORTAL_BBOX_RADIUS * 2 * scale);
}

float scaleFromSize(const int32_t size) {
  return static_cast<float>(size) / (PORTAL_BBOX_RADIUS * 2);
}

void setPortalTarget(entityState_t &es, const int32_t size,
                     const vec3_t center) {
  es.onFireEnd = encodePortalTarget(size);
  VectorCopy(center, es.origin2);
}

void getPortalTarget(const entityState_t &es, PortalTarget &out) {
  VectorCopy(es.origin2, out.center);
  out.size = portalTargetSize(es);
}

int32_t encodePortalTarget(const int32_t size) {
  return PORTAL_TARGET_FLAG | std::clamp(size, 0, MAX_PORTAL_TARGET_SIZE);
}

bool isPortalTarget(const entityState_t &es) {
  return (es.eType == ET_MOVER || es.eType == ET_STATIC_CLIENT) &&
         es.onFireEnd > 0 && (es.onFireEnd & PORTAL_TARGET_FLAG);
}

int32_t portalTargetSize(const entityState_t &es) {
  return std::clamp(es.onFireEnd & ~PORTAL_TARGET_FLAG, 0,
                    MAX_PORTAL_TARGET_SIZE);
}
} // namespace ETJump::PortalgunShared
