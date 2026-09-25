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

#include "etj_portal_prediction.h"
#include "cg_local.h"
#include "etj_cgame.h"
#include "etj_trace_utils.h"

#include "../game/etj_entity_utilities_shared.h"

namespace ETJump {
// max distance between a predicted and a server portal origin,
// for them to be considered the same portal
inline constexpr float CONFIRMED_PORTAL_MAX_DIST = 1.0f;

void PortalPrediction::beginPrediction() {
  prevPortals = portals;
  portals = {};
  serverPortals = {};

  active = etj_portalPredict.integer || cgame.sharedWSKeys.portalPredict;
  clientNum = cg.snap->ps.clientNum;

  if (!active) {
    prevPortals = {};
    confirmedPortals = {};
    return;
  }

  for (int32_t i = 0; i < cg.snap->numEntities; i++) {
    const entityState_t &es = cg.snap->entities[i];

    if (isOwnPortal(es)) {
      serverPortals[typeIndex(es.eType)] = es;
    }
  }
}

void PortalPrediction::endPrediction() {
  if (!active) {
    return;
  }

  // predicted portals which were fired by a command the server has processed
  // are no longer replayed, the snapshot now contains the server's result
  for (int32_t i = 0; i < NUM_PORTAL_TYPES; i++) {
    const auto &prev = prevPortals[i];

    if (!prev || prev->fireTime > cg.snap->ps.commandTime) {
      continue;
    }

    ConfirmedPortal confirmed{};
    confirmed.eType = prev->es.eType;
    VectorCopy(prev->es.origin, confirmed.origin);
    confirmed.spawnTime = prev->es.effect1Time;
    confirmedPortals[i] = confirmed;
  }

  prevPortals = {};
}

void PortalPrediction::clear() {
  active = false;
  portals = {};
  prevPortals = {};
  serverPortals = {};
  confirmedPortals = {};
}

void PortalPrediction::checkFire(const playerState_t &before,
                                 const playerState_t &after,
                                 const usercmd_t &cmd) {
  if (!active) {
    return;
  }

  // these match the early outs in 'FireWeapon'
  if (after.pm_type == PM_DEAD || after.persistant[PERS_HWEAPON_USE] ||
      after.weapon != WP_PORTAL_GUN) {
    return;
  }

  // same event range as 'ClientEvents' checks on server
  int32_t eventSequence =
      std::max(before.eventSequence, after.eventSequence - MAX_EVENTS);
  bool fired = false;

  for (; eventSequence < after.eventSequence; eventSequence++) {
    switch (after.events[eventSequence & (MAX_EVENTS - 1)]) {
      case EV_FIRE_WEAPON:
      case EV_FIRE_WEAPONB:
      case EV_FIRE_WEAPON_LASTSHOT:
        fired = true;
        break;
      default:
        break;
    }
  }

  if (!fired) {
    return;
  }

  const int32_t eType =
      cmd.wbuttons & WBUTTON_ATTACK2 ? ET_PORTAL_RED : ET_PORTAL_BLUE;
  const int32_t fireTime = after.commandTime;
  const auto &prev = prevPortals[typeIndex(eType)];

  // we've already predicted this shot on a previous frame,
  // keep the portal as it was, so it doesn't jump around or re-animate
  if (prev && prev->fireTime == fireTime) {
    portals[typeIndex(eType)] = prev;
    return;
  }

  PortalgunShared::Placement placement{};
  int32_t hitEntityNum = ENTITYNUM_NONE;

  if (!tryPlacePortal(after, eType, placement, hitEntityNum)) {
    return;
  }

  PredictedPortal portal{};
  portal.fireTime = fireTime;
  portal.parentEntityNum = hitEntityNum;

  entityState_t &es = portal.es;
  es.number = ENTITYNUM_NONE;
  es.eType = eType;
  es.pos.trType = TR_STATIONARY;
  VectorCopy(placement.origin, es.origin);
  VectorCopy(placement.origin, es.pos.trBase);
  VectorCopy(placement.angles, es.angles);
  VectorCopy(placement.angles, es.apos.trBase);
  es.onFireStart = PortalgunShared::sizeFromScale(placement.scale);
  es.effect1Time = cg.time;
  es.otherEntityNum = clientNum;
  es.teamNum = etj_portalTeam.integer;

  portals[typeIndex(eType)] = portal;

  // first time we see this shot, draw the trail
  // (also handle the command time going backwards, e.g. on map restart)
  if (fireTime > lastTrailFireTime || lastTrailFireTime > cg.time + 1000) {
    vec3_t muzzle;
    PortalgunShared::muzzlePoint(after.origin, after.viewheight,
                                 after.viewangles, muzzle);
    portalTrail(placement.surfacePoint, muzzle,
                eType == ET_PORTAL_BLUE ? portalBlueTrail : portalRedTrail);

    lastTrailFireTime = fireTime;
  }
}

bool PortalPrediction::isActive() const { return active; }

bool PortalPrediction::resolveServerPortal(const entityState_t &es,
                                           entityState_t &out) const {
  out = es;

  if (!active || !isOwnPortal(es)) {
    return true;
  }

  // the server frees this portal once it processes the shot
  if (portals[typeIndex(es.eType)]) {
    return false;
  }

  const int32_t other = otherType(es.eType);

  // destination is still the server's view of our other portal
  if (!portals[typeIndex(other)]) {
    return true;
  }

  if (!getOwnPortalPosition(other, out.origin2, out.angles2)) {
    VectorClear(out.origin2);
    VectorClear(out.angles2);
  }

  return true;
}

bool PortalPrediction::isSuperseded(const entityState_t &es) const {
  return active && isOwnPortal(es) && portals[typeIndex(es.eType)];
}

int32_t PortalPrediction::spawnAnimTime(const entityState_t &es) const {
  if (!active || !isOwnPortal(es)) {
    return es.effect1Time;
  }

  const auto &confirmed = confirmedPortals[typeIndex(es.eType)];

  if (confirmed && confirmed->eType == es.eType &&
      Distance(confirmed->origin, es.origin) < CONFIRMED_PORTAL_MAX_DIST) {
    return confirmed->spawnTime;
  }

  return es.effect1Time;
}

void PortalPrediction::addToScene() {
  if (!active) {
    return;
  }

  forEachPortal([](const entityState_t &es, int32_t &) {
    CG_DrawPortalGate(&es, es.effect1Time);
  });
}

int32_t PortalPrediction::typeIndex(const int32_t eType) {
  return eType == ET_PORTAL_BLUE ? 0 : 1;
}

int32_t PortalPrediction::otherType(const int32_t eType) {
  return eType == ET_PORTAL_BLUE ? ET_PORTAL_RED : ET_PORTAL_BLUE;
}

bool PortalPrediction::isOwnPortal(const entityState_t &es) const {
  return (es.eType == ET_PORTAL_BLUE || es.eType == ET_PORTAL_RED) &&
         es.otherEntityNum == clientNum;
}

bool PortalPrediction::isAlive(const PredictedPortal &portal) const {
  if (portal.parentEntityNum < 0 ||
      portal.parentEntityNum >= ENTITYNUM_MAX_NORMAL) {
    return true;
  }

  // server removes portals attached to 'func_static_client'
  // when the entity is toggled off for the portal owner
  const entityState_t &parent =
      cg_entities[portal.parentEntityNum].currentState;

  return !(parent.eType == ET_STATIC_CLIENT &&
           EntityUtilsShared::funcStaticClientIsHidden(&parent, clientNum));
}

const PortalPrediction::PredictedPortal *
PortalPrediction::getPortal(const int32_t eType) const {
  const auto &portal = portals[typeIndex(eType)];

  if (!portal || !isAlive(*portal)) {
    return nullptr;
  }

  return &*portal;
}

bool PortalPrediction::getOwnPortalPosition(const int32_t eType,
                                            vec3_t origin,
                                            vec3_t angles) const {
  const int32_t index = typeIndex(eType);

  if (portals[index]) {
    const PredictedPortal *portal = getPortal(eType);

    if (!portal) {
      return false;
    }

    VectorCopy(portal->es.origin, origin);
    VectorCopy(portal->es.angles, angles);
    return true;
  }

  if (serverPortals[index]) {
    VectorCopy(serverPortals[index]->origin, origin);
    VectorCopy(serverPortals[index]->angles, angles);
    return true;
  }

  // the portal might be outside our PVS, but our other portal
  // in the snapshot still knows where it is, since it's the destination
  const auto &other = serverPortals[typeIndex(otherType(eType))];

  if (other && (!VectorCompare(other->origin2, vec3_origin) ||
                !VectorCompare(other->angles2, vec3_origin))) {
    VectorCopy(other->origin2, origin);
    VectorCopy(other->angles2, angles);
    return true;
  }

  return false;
}

bool PortalPrediction::resolvePredictedPortal(
    const std::optional<PredictedPortal> &portal, entityState_t &out) const {
  if (!portal || !isAlive(*portal)) {
    return false;
  }

  out = portal->es;

  if (!getOwnPortalPosition(otherType(out.eType), out.origin2, out.angles2)) {
    VectorClear(out.origin2);
    VectorClear(out.angles2);
  }

  return true;
}

bool PortalPrediction::tryPlacePortal(const playerState_t &ps,
                                      const int32_t eType,
                                      PortalgunShared::Placement &placement,
                                      int32_t &hitEntityNum) const {
  vec3_t traceStart;
  vec3_t traceEnd;
  trace_t tr;

  PortalgunShared::traceEndpoints(ps.origin, ps.viewheight, ps.viewangles,
                                  traceStart, traceEnd);

  cgame.utils.trace->filteredTrace(clientNum, &tr, traceStart, nullptr,
                                   nullptr, traceEnd, clientNum, MASK_PORTAL);

  if (!PortalgunShared::surfaceAllowsPortal(
          tr, cgame.sharedWSKeys.portalSurfaces)) {
    return false;
  }

  PortalgunShared::PortalTarget target{};
  bool hitTarget = false;

  if (tr.entityNum >= 0 && tr.entityNum < ENTITYNUM_MAX_NORMAL) {
    const entityState_t &hitEs = cg_entities[tr.entityNum].currentState;

    if (PortalgunShared::isPortalTarget(hitEs)) {
      PortalgunShared::getPortalTarget(hitEs, target);
      hitTarget = true;
    }
  }

  PortalgunShared::computePlacement(tr, hitTarget ? &target : nullptr,
                                    placement);

  if (overlapsOtherPortals(placement, eType)) {
    return false;
  }

  hitEntityNum = tr.entityNum;
  return true;
}

bool PortalPrediction::overlapsOtherPortals(
    const PortalgunShared::Placement &placement, const int32_t eType) const {
  // we can never overlap our own portals
  const int32_t other = otherType(eType);
  const int32_t otherIndex = typeIndex(other);

  if (portals[otherIndex]) {
    const PredictedPortal *portal = getPortal(other);

    if (portal &&
        PortalgunShared::portalsOverlap(placement, portal->es.origin,
                                        portal->es.angles,
                                        portal->es.onFireStart)) {
      return true;
    }
  } else if (serverPortals[otherIndex]) {
    const entityState_t &es = *serverPortals[otherIndex];

    if (PortalgunShared::portalsOverlap(placement, es.origin, es.angles,
                                        es.onFireStart)) {
      return true;
    }
  }

  const auto portalTeam = static_cast<PortalTeamOpts>(etj_portalTeam.integer);

  if (portalTeam != PortalTeamOpts::FIRETEAM &&
      portalTeam != PortalTeamOpts::ALL) {
    return false;
  }

  for (int32_t i = 0; i < cg.snap->numEntities; i++) {
    const entityState_t &es = cg.snap->entities[i];

    if (es.eType != ET_PORTAL_BLUE && es.eType != ET_PORTAL_RED) {
      continue;
    }

    // our own portals are already handled
    if (es.otherEntityNum == clientNum) {
      continue;
    }

    if (portalTeam == PortalTeamOpts::FIRETEAM &&
        !CG_IsOnSameFireteam(clientNum, es.otherEntityNum)) {
      continue;
    }

    if (PortalgunShared::portalsOverlap(placement, es.origin, es.angles,
                                        es.onFireStart)) {
      return true;
    }
  }

  return false;
}
} // namespace ETJump
