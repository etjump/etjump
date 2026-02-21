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

#include "etj_portalgun.h"
#include "etj_entity_utilities.h"
#include "etj_entity_utilities_shared.h"
#include "etj_portalgun_shared.h"
#include "etj_trace_utils.h"

namespace ETJump {
// max range where you can place next portal gate
inline constexpr float MAX_PORTAL_RANGE = 2 << 16;
// min angle difference between two portals, used to avoid overlapping
inline constexpr float MIN_ANGLES_DIFF = 100.0f;
// min distance between two portal center points, used to avoid overlapping
inline constexpr float MIN_PORTALS_DIST = 75.0f / 2;

void Portal::spawn(gentity_t *ent, const float scale, const Type type,
                   const trace_t &tr, vec3_t end, const vec3_t angles) {
  gentity_t *portal = G_Spawn();
  portal->classname = "portal_gate";
  portal->s.onFireStart = static_cast<int>(PORTAL_BBOX_RADIUS * 2 * scale);

  // save the spawn timestamp so client can animate the spawning
  portal->s.effect1Time = level.time;

  // Assign ent to player as well as the portal type..
  if (type == Type::PORTAL_BLUE) {
    portal->s.eType = ET_PORTAL_BLUE; // Portal 1

    portal->linkedPortal = ent->portalRed;
    if (ent->portalRed) {
      ent->portalRed->linkedPortal = portal;
    }

    // Assign to client
    ent->portalBlue = portal;
  } else {
    portal->s.eType = ET_PORTAL_RED; // Portal 2

    portal->linkedPortal = ent->portalBlue;
    if (ent->portalBlue) {
      ent->portalBlue->linkedPortal = portal;
    }

    // Assign to client
    ent->portalRed = portal;
  }

  // Set origin (obviously)
  G_SetOrigin(portal, end);

  VectorCopy(end, portal->s.origin);
  VectorCopy(end, portal->r.currentOrigin);

  EntityUtilsShared::setPortalBBox(portal->r.mins, portal->r.maxs, angles,
                                   scale);

  portal->r.contents = CONTENTS_TRIGGER | CONTENTS_ITEM;
  portal->clipmask =
      CONTENTS_SOLID | CONTENTS_MISSILECLIP; // NOTE: _TELE just a test...
  portal->touch = [](gentity_t *self, gentity_t *other, trace_t *trace) {
    touch(self, other);
  };

  portal->think = think;

  // think once immediately, and also force the linked portal to think
  // this is so that we have valid origin2/angles2 set immediately
  // when we spawn the portal, otherwise there will be a single server
  // frame where the portal destination is at 0 0 0
  think(portal);

  if (portal->linkedPortal) {
    portal->linkedPortal->think(portal->linkedPortal);
  }

  portal->r.ownerNum = ent->s.number;
  portal->parent = ent;

  portal->s.pos.trType = TR_STATIONARY;
  portal->s.otherEntityNum =
      ent->s.clientNum; // HACK: Using this for render checks.....

  // Set angle of entity based on normal of plane....
  vectoangles(tr.plane.normal,
              portal->s.angles); // NOTE: RE-Enable angles...
  vectoangles(tr.plane.normal, portal->r.currentAngles);

  // rather than using the shared cvar, we can simply set 'portalteam'
  // value to an entitystate field that we can check on client
  portal->s.teamNum = level.portalTeam;

  trap_LinkEntity(portal);
}

void Portal::think(gentity_t *self) {
  // setup for prediction
  // we must do this here because 'linkedPortal' will be nullptr initially,
  // before a second portal is shot, so we need to make sure
  // origin/angles get regular updates on client
  if (self->linkedPortal) {
    VectorCopy(self->s.origin, self->linkedPortal->s.origin2);
    VectorCopy(self->s.angles, self->linkedPortal->s.angles2);

    // add/remove destination to PVS via SVF_PORTAL flag,
    // so entities are loaded in instantly when we teleport
    // and prediction works if enabled
    if (g_portalPredict.integer || level.portalPredict) {
      self->r.svFlags |= SVF_PORTAL;
    } else {
      self->r.svFlags &= ~SVF_PORTAL;
    }
  }

  // we should think *every* frame to ensure up-to-date positons
  self->nextthink = level.time + level.frameTime;
}

void Portal::touch(gentity_t *self, gentity_t *other) {
  // TODO: Add ability to teleport missiles...
  const gentity_t *dest = nullptr;

  // If this is not a player, then don't teleport it.
  // //NOTE: We'll probably want items to be
  // transferred through portal eventually...
  if (!other->client) {
    return;
  }

  if (other->client->ps.pm_type == PM_DEAD) {
    return;
  }

  // if this isn't our portal, and 'portalteam' isn't set to 2,
  // determine if we can use this portal at all
  if (self->r.ownerNum != other->s.number &&
      level.portalTeam != PORTAL_TEAM_ALL) {
    if (level.portalTeam == PORTAL_TEAM_NONE) {
      return;
    }

    if (level.portalTeam == PORTAL_TEAM_FT) {
      fireteamData_t *ftSelf{};
      fireteamData_t *ftOther{};

      if (!G_IsOnFireteam(ClientNum(other), &ftSelf) ||
          !G_IsOnFireteam(self->r.ownerNum, &ftOther) || ftSelf != ftOther) {
        return;
      }
    }
  }

  if (level.portalTeam == PORTAL_TEAM_NONE) {
    if (self->s.eType == ET_PORTAL_BLUE) {
      // Check that the 'other' portal exists and set it as dest
      if (other->portalRed != nullptr) {
        dest = other->portalRed;
      }

    } else if (self->s.eType == ET_PORTAL_RED) {
      // Check that the 'other' portal exists and set it as dest
      if (other->portalBlue != nullptr) {
        dest = other->portalBlue;
      }

    } else {
      G_Printf(S_COLOR_RED "ERROR: invalid portal entity hit, this is a bug! "
                           "Please report this to the developers.\n");
      return;
    }
  } else if (level.portalTeam == PORTAL_TEAM_FT ||
             level.portalTeam == PORTAL_TEAM_ALL) {
    if (self->linkedPortal != nullptr) {
      dest = self->linkedPortal;
    }
  }

  if (!dest) {
    return;
  }

  EntityUtilsShared::portalTeleport(&other->client->ps, &other->s, &self->s,
                                    &other->client->pers.cmd, level.time,
                                    other->client->teleportBitFlipped);
}

void Portalgun::spawn(gentity_t *ent) {
  gitem_t *item = BG_FindItemForWeapon(WP_PORTAL_GUN);

  char *noise{};

  if (G_SpawnString("noise", "", &noise)) {
    ent->noise_index = G_SoundIndex(noise);
  }

  ent->s.eType = ET_ITEM;
  // store item number in modelindex
  ent->s.modelindex = static_cast<int>(item - bg_itemlist);
  // this is taking modelindex2's place for a dropped item
  // TODO: do we need this for portalgun?
  ent->s.otherEntityNum2 = 1;

  ent->classname = item->classname;
  ent->item = item;

  VectorSet(ent->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, 0);
  VectorSet(ent->r.maxs, ITEM_RADIUS, ITEM_RADIUS, 2 * ITEM_RADIUS);
  ent->r.contents = CONTENTS_TRIGGER | CONTENTS_ITEM;
  ent->clipmask = CONTENTS_SOLID | CONTENTS_MISSILECLIP;

  ent->touch = touch;

  // TODO: can this be removed? does the portalgun even need to think?
  ent->think = [](gentity_t *self) {};
  ent->nextthink = level.time + FRAMETIME;

  if (ent->spawnflags & 2) {
    ent->s.eFlags |= EF_SPINNING;
  }

  if (ent->spawnflags & 4) {
    ent->s.eFlags |= EF_BOBBING;
  }

  G_SetOrigin(ent, ent->s.origin);
  G_SetAngle(ent, ent->s.angles);

  trap_LinkEntity(ent);
}

void Portalgun::touch(gentity_t *self, gentity_t *other, trace_t *trace) {
  if (!other->client) {
    return;
  }

  if (other->client->sess.timerunActive &&
      other->client->sess.runSpawnflags &
          static_cast<int>(TimerunSpawnflags::NoPortalgunPickup)) {
    return;
  }

  // check if player already had the weapon
  if (COM_BitCheck(other->client->ps.weapons, self->item->giTag)) {
    return;
  }

  // forcing pickup through same function as target_give uses
  Touch_Item_Give(self, other, trace);
}

void Portalgun::fire(gentity_t *ent, const Portal::Type type, vec3_t forward,
                     vec3_t right, vec3_t up, vec3_t muzzleEffect) {
  float scale = 1.0f;

  vec3_t t_endpos;

  // trace vars
  vec3_t start;       // Muzzle location
  vec3_t trace_start; // Actual trace start
  vec3_t trace_end;   // trace end point
  trace_t tr;         // trace results..
  vec3_t tr_end;      // trace end adjusted for possible func_portaltarget

  // BBox info
  vec3_t t_portalAngles; // Could be used for all angles conversions...

  // NOTE: NEW trace setup.... pulled from flamethrower
  // NOTE: Need this for +attack2 call...
  AngleVectors(ent->client->ps.viewangles, forward, right, up);

  VectorCopy(ent->r.currentOrigin, start);
  start[2] += static_cast<float>(ent->client->ps.viewheight);
  VectorCopy(start, trace_start);

  // Muzzle position
  VectorMA(start, -4, forward, start);
  VectorMA(start, 6, right, start);
  VectorMA(start, -4, up, start);

  // End pos
  VectorMA(trace_start, MAX_PORTAL_RANGE, forward, trace_end);

  ETJump::TraceUtils::filteredTrace(ent->s.number, &tr, trace_start, nullptr,
                                    nullptr, trace_end, ent->s.number,
                                    MASK_PORTAL);

  if (tr.surfaceFlags & SURF_NOIMPACT || tr.fraction == 1.0f) {
    return;
  }

  // portalclip or player = no portal
  if (tr.contents & (CONTENTS_PORTALCLIP | CONTENTS_BODY)) {
    return;
  }

  if (level.portalSurfaces && tr.surfaceFlags & SURF_PORTALSURFACE) {
    return;
  }

  if (!level.portalSurfaces && !(tr.surfaceFlags & SURF_PORTALSURFACE)) {
    return;
  }

  vectoangles(tr.plane.normal, t_portalAngles);

  // we hit a 'func_portaltarget'
  if (tr.entityNum > MAX_CLIENTS + BODY_QUEUE_SIZE &&
      tr.entityNum < ENTITYNUM_WORLD &&
      !Q_stricmp(g_entities[tr.entityNum].classname, "func_portaltarget")) {
    const gentity_t *brushEnt = &g_entities[tr.entityNum];

    vec3_t be_position; // brush ent position
    vec3_t delta;       // delta between brushent position and trace end
    vec3_t
        normalScaled; // plane normal scaled by dotproduct of delta and itself

    EntityUtilities::getOriginOrBmodelCenter(brushEnt, be_position);

    VectorSubtract(be_position, tr.endpos, delta);
    const float dotProduct = DotProduct(delta, tr.plane.normal);
    VectorScale(tr.plane.normal, dotProduct, normalScaled);
    VectorSubtract(be_position, normalScaled, tr_end);

    if (brushEnt->count > 0) {
      scale = static_cast<float>(brushEnt->count) / (PORTAL_BBOX_RADIUS * 2);
    }
  } else {
    VectorCopy(tr.endpos, tr_end);
  }

  VectorMA(tr_end, 5, tr.plane.normal, t_endpos);

  if (portalsOverlap(ent, type, scale, t_portalAngles, t_endpos)) {
    return;
  }

  // Free any previous instances of each portal if any
  if (type == Portal::Type::PORTAL_BLUE && ent->portalBlue) {

    G_FreeEntity(ent->portalBlue);
    ent->portalBlue = nullptr;

  } else if (type == Portal::Type::PORTAL_RED && ent->portalRed) {

    G_FreeEntity(ent->portalRed);
    ent->portalRed = nullptr;
  }

  // Railtrail
  // close enough for the barrel on most cases, realistically we should
  // grab the starting point from the weapon tags
  gentity_t *tent = G_TempEntity(muzzleEffect, EV_PORTAL_TRAIL);
  type == Portal::Type::PORTAL_BLUE
      ? VectorCopy(portalBlueTrail, tent->s.angles)
      : VectorCopy(portalRedTrail, tent->s.angles);

  SnapVectorTowards(tr_end, start);
  VectorCopy(tr_end, tent->s.origin2);
  tent->s.otherEntityNum2 = ent->s.number;
  // END - Rail

  // portal fired correctly, increment portal count
  ent->client->numPortals++;

  Portal::spawn(ent, scale, type, tr, t_endpos, t_portalAngles);
}

bool Portalgun::portalsOverlap(gentity_t *ent, Portal::Type type,
                               const float scale, vec3_t portalAngles,
                               vec3_t endPos) {
  std::vector<const gentity_t *> otherPortals;
  // * 2 for 2 portals per client
  otherPortals.reserve(level.numConnectedClients * 2L);
  fireteamData_t *ft = nullptr;

  // we can never overlap our own portals
  if (type == Portal::Type::PORTAL_BLUE && ent->portalRed) {
    otherPortals.push_back(ent->portalRed);
  } else if (type == Portal::Type::PORTAL_RED && ent->portalBlue) {
    otherPortals.push_back(ent->portalBlue);
  }

  switch (level.portalTeam) {
    case PORTAL_TEAM_FT:
      if (!G_IsOnFireteam(ClientNum(ent), &ft)) {
        break;
      }

      for (const char clientNum : ft->joinOrder) {
        if (clientNum == -1) {
          break;
        }

        // skip our portals since they are already added
        if (clientNum == ClientNum(ent)) {
          continue;
        }

        const auto *ftMember = static_cast<gentity_t *>(g_entities + clientNum);

        // we don't care about the portal type here
        // since these aren't our portals, so we can't ever overlap them
        if (ftMember->portalBlue) {
          otherPortals.push_back(ftMember->portalBlue);
        }

        if (ftMember->portalRed) {
          otherPortals.push_back(ftMember->portalRed);
        }
      }

      break;
    case PORTAL_TEAM_ALL:
      for (int i = 0; i < level.numConnectedClients; i++) {
        auto *const other =
            static_cast<gentity_t *>(g_entities + level.sortedClients[i]);

        // skip our portals
        if (other == ent) {
          continue;
        }

        if (other->portalBlue) {
          otherPortals.push_back(other->portalBlue);
        }

        if (other->portalRed) {
          otherPortals.push_back(other->portalRed);
        }
      }

      break;
    default:
      break;
  }

  return std::any_of(
      otherPortals.cbegin(), otherPortals.cend(),
      [scale, portalAngles, endPos](const gentity_t *otherPortal) {
        const float otherScale =
            static_cast<float>(otherPortal->s.onFireStart) /
            (PORTAL_BBOX_RADIUS * 2);
        const float min_dist =
            (MIN_PORTALS_DIST * scale) + (MIN_PORTALS_DIST * otherScale);

        return (Distance(portalAngles, otherPortal->s.angles) <
                    MIN_ANGLES_DIFF &&
                Distance(endPos, otherPortal->s.origin) < min_dist);
      });
}
} // namespace ETJump
