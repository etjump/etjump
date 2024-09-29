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

#include "g_local.h"
#include "etj_local.h"
#include "etj_timerun_v2.h"
#include "etj_printer.h"

#include "etj_savepos_command_handler.h"

namespace ETJump {
void SavePosHandler::execSaveposCommand(gentity_t *ent,
                                        const std::vector<std::string> &args) {
  const SavePosData data = SavePosData::deserialize(args);

  if (!data.error.empty()) {
    Printer::console(ent, "Failed to deserialize savepos data: " + data.error);
    return;
  }

  if (ent->client->ps.stats[STAT_HEALTH] <= GIB_HEALTH) {
    respawn(ent);
  }

  game.timerunV2->interrupt(ClientNum(ent));

  saveposTeleport(ent, data);

  if (!data.timerunInfo.runName.empty()) {
    restoreSaveposTimerunState(ent, data);
  }
}

void SavePosHandler::saveposTeleport(gentity_t *ent, const SavePosData &data) {
  ent->client->ps.eFlags ^= EF_TELEPORT_BIT;

  VectorCopy(data.pos.origin, ent->client->ps.origin);
  SetClientViewAngle(ent, data.pos.angles);
  VectorCopy(data.pos.velocity, ent->client->ps.velocity);

  switch (data.pos.stance) {
    case PlayerStance::Stand:
      ent->client->ps.eFlags &= ~EF_PRONE;
      ent->client->ps.eFlags &= ~EF_PRONE_MOVING;
      ent->client->ps.pm_flags &= ~PMF_DUCKED;
      break;
    case PlayerStance::Crouch:
      ent->client->ps.eFlags &= ~EF_PRONE;
      ent->client->ps.eFlags &= ~EF_PRONE_MOVING;
      ent->client->ps.pm_flags |= PMF_DUCKED;
      break;
    case PlayerStance::Prone:
      ent->client->ps.eFlags |= EF_PRONE;
      break;
  }

  BG_PlayerStateToEntityState(&ent->client->ps, &ent->s, qtrue);
  // this can't be called if we are in spec, so always link the player
  VectorCopy(ent->client->ps.origin, ent->r.currentOrigin);
  trap_LinkEntity(ent);
}

void SavePosHandler::restoreSaveposTimerunState(gentity_t *ent,
                                                const SavePosData &data) {
  const int clientNum = ClientNum(ent);

  // we can't use the real startTime from the timerun info, as that is set to
  // whatever the server timestamp was at the time the position was saved,
  // so figure out a faked start time based off the current run timer
  const int startTime =
      ent->client->ps.commandTime - data.timerunInfo.currentRunTimer;

  game.timerunV2->startSaveposTimer(clientNum, ent->client->pers.netname,
                                    startTime, data);

  int checkpointNum = 0;

  for (int i = 0; i < MAX_TIMERUN_CHECKPOINTS; i++) {
    // skip any checkpoint indices we haven't hit
    if (!data.timerunInfo.checkpointIndicesHit[i]) {
      continue;
    }

    // these are relative times from the beginning of the run,
    // so we just add them to the adjusted startTime
    game.timerunV2->checkpoint(
        data.timerunInfo.runName, clientNum, i,
        startTime + data.timerunInfo.checkpoints[checkpointNum++]);
  }
}
} // namespace ETJump
