/*
 * MIT License
 *
 * Copyright (c) 2024 ETJump team <zero@etjump.com>
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

#include "cg_local.h"
#include "etj_player_bbox.h"
#include "etj_cvar_update_handler.h"
#include "etj_utilities.h"

namespace ETJump {
PlayerBBox::PlayerBBox() {
  cvarUpdateHandler->subscribe(
      &etj_playerBBoxColorSelf, [&](const vmCvar_t *cvar) {
        parseColorString(etj_playerBBoxColorSelf.string, colorSelf);
      });

  cvarUpdateHandler->subscribe(
      &etj_playerBBoxColorOther, [&](const vmCvar_t *cvar) {
        parseColorString(etj_playerBBoxColorOther.string, colorOther);
      });

  cvarUpdateHandler->subscribe(
      &etj_playerBBoxColorFireteam, [&](const vmCvar_t *cvar) {
        parseColorString(etj_playerBBoxColorFireteam.string, colorFireteam);
      });

  parseColorString(etj_playerBBoxColorSelf.string, colorSelf);
  parseColorString(etj_playerBBoxColorOther.string, colorOther);
  parseColorString(etj_playerBBoxColorFireteam.string, colorFireteam);

  shader = trap_R_RegisterShader(etj_playerBBoxShader.string);
}

void PlayerBBox::drawBBox(clientInfo_t *ci, centity_t *cent) {
  if (canSkipDraw(cent, ci)) {
    return;
  }

  BBox box;
  int pType;

  const int drawFlags = etj_drawPlayerBBox.integer;
  const bool inFireteam =
      CG_IsOnSameFireteam(cg.snap->ps.clientNum, cent->currentState.clientNum);

  if (cent->currentState.clientNum == cg.snap->ps.clientNum) {
    if (!(drawFlags & static_cast<int>(DrawFlags::Self))) {
      return;
    }

    pType = static_cast<int>(PlayerType::Self);
  } else {
    if (inFireteam) {
      if (!(drawFlags & static_cast<int>(DrawFlags::Fireteam))) {
        return;
      }

      pType = static_cast<int>(PlayerType::Fireteam);
    } else {
      if (!(drawFlags & static_cast<int>(DrawFlags::Others))) {
        return;
      }

      pType = static_cast<int>(PlayerType::Other);
    }
  }

  setupBBoxExtents(cent, box);

  if (pType == static_cast<int>(PlayerType::Self)) {
    Vector4Copy(colorSelf, box.color);
  } else {
    Vector4Copy(inFireteam ? colorFireteam : colorOther, box.color);

    // adjust alpha to match etj_hideDistance + etj_hideFadeRange
    // we don't need to worry about hideme or dist < hideDistance here, the code
    // in CG_Player exits before we ever call this function in those scenarios
    if (etj_hide.integer) {
      float alpha = 1.0f;
      centity_t *self = &cg_entities[cg.snap->ps.clientNum];
      const vec_t playerDist = Distance(self->lerpOrigin, cent->lerpOrigin);
      const float fadeRange = etj_hideFadeRange.value;
      const float transZone = etj_hideDistance.value + fadeRange;

      if (playerDist < transZone) {
        const float diff = (transZone - playerDist) / fadeRange;
        alpha -= (alpha * diff);
      }

      box.color[3] *= alpha;
    }
  }

  // setup colors for polygon vertices
  for (auto &vert : box.verts) {
    for (int i = 0; i < 4; i++) {
      vert.modulate[i] = static_cast<byte>(box.color[i] * 255);
    }
  }

  // set the texture coordinates
  box.verts[0].st[0] = 0;
  box.verts[0].st[1] = 0;
  box.verts[1].st[0] = 0;
  box.verts[1].st[1] = 1;
  box.verts[2].st[0] = 1;
  box.verts[2].st[1] = 1;
  box.verts[3].st[0] = 1;
  box.verts[3].st[1] = 0;

  vec3_t corners[8];
  // layout for bbox corners
  //
  //             4 ----- 7
  //   z        /|      /|
  //   | _ y   5 ----- 6 |
  //  /        | |     | |
  // x         | |     | |
  //           | 0 ----| 3
  //           |/      |/
  //           1 ----- 2

  VectorAdd(cent->lerpOrigin, box.mins, corners[0]);

  // push bottom corners away from origin
  VectorCopy(corners[0], corners[1]);
  corners[1][0] += box.maxs[0] - box.mins[0];

  VectorCopy(corners[1], corners[2]);
  corners[2][1] += box.maxs[1] - box.mins[1];

  VectorCopy(corners[2], corners[3]);
  corners[3][0] -= box.maxs[0] - box.mins[0];

  // bottom
  VectorCopy(corners[0], box.verts[0].xyz);
  VectorCopy(corners[1], box.verts[1].xyz);
  VectorCopy(corners[2], box.verts[2].xyz);
  VectorCopy(corners[3], box.verts[3].xyz);
  trap_R_AddPolyToScene(shader, 4, box.verts);

  // skip the rest if we only draw bottom for this player
  if (bottomOnly(pType)) {
    return;
  }

  // push top corners to the bbox top
  for (int i = 0; i < 4; ++i) {
    VectorCopy(corners[i], corners[i + 4]);
    corners[i + 4][2] += box.maxs[2] - box.mins[2];
  }

  // top
  VectorCopy(corners[7], box.verts[0].xyz);
  VectorCopy(corners[6], box.verts[1].xyz);
  VectorCopy(corners[5], box.verts[2].xyz);
  VectorCopy(corners[4], box.verts[3].xyz);
  trap_R_AddPolyToScene(shader, 4, box.verts);

  // origin -> Y
  VectorCopy(corners[3], box.verts[0].xyz);
  VectorCopy(corners[2], box.verts[1].xyz);
  VectorCopy(corners[6], box.verts[2].xyz);
  VectorCopy(corners[7], box.verts[3].xyz);
  trap_R_AddPolyToScene(shader, 4, box.verts);

  // origin -> X
  VectorCopy(corners[2], box.verts[0].xyz);
  VectorCopy(corners[1], box.verts[1].xyz);
  VectorCopy(corners[5], box.verts[2].xyz);
  VectorCopy(corners[6], box.verts[3].xyz);
  trap_R_AddPolyToScene(shader, 4, box.verts);

  // origin -> -X
  VectorCopy(corners[0], box.verts[0].xyz);
  VectorCopy(corners[3], box.verts[1].xyz);
  VectorCopy(corners[7], box.verts[2].xyz);
  VectorCopy(corners[4], box.verts[3].xyz);
  trap_R_AddPolyToScene(shader, 4, box.verts);

  // origin -> -Y
  VectorCopy(corners[1], box.verts[0].xyz);
  VectorCopy(corners[0], box.verts[1].xyz);
  VectorCopy(corners[4], box.verts[2].xyz);
  VectorCopy(corners[5], box.verts[3].xyz);
  trap_R_AddPolyToScene(shader, 4, box.verts);
}

void PlayerBBox::setupBBoxExtents(centity_t *cent, BBox &box) {
  VectorCopy(playerMins, box.mins);
  VectorCopy(playerMaxs, box.maxs);

  if (cent->currentState.eFlags & EF_CROUCHING) {
    box.maxs[2] -= CROUCH_MAXS_OFFSET_Z;
  } else if (cent->currentState.eFlags & (EF_PRONE | EF_PRONE_MOVING)) {
    box.maxs[2] -= PRONE_MAXS_OFFSET_Z;
  }
}

bool PlayerBBox::bottomOnly(const int &pType) {
  switch (static_cast<PlayerType>(pType)) {
    case PlayerType::Self:
      return etj_playerBBoxBottomOnlySelf.integer;
    case PlayerType::Other:
      return etj_playerBBoxBottomOnlyOther.integer;
    case PlayerType::Fireteam:
      return etj_playerBBoxBottomOnlyFireteam.integer;
    default: // shouldn't happen
      return false;
  }
}

bool PlayerBBox::canSkipDraw(centity_t *cent, clientInfo_t *ci) const {
  if (ci->team == TEAM_SPECTATOR || cent->currentState.eFlags & EF_DEAD) {
    return true;
  }

  // bad/empty shader
  if (!shader) {
    return true;
  }

  return false;
}
} // namespace ETJump
