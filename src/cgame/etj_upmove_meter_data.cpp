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

#include "etj_upmove_meter_data.h"
#include "etj_client_commands_handler.h"
#include "etj_player_events_handler.h"

namespace ETJump {
UpmoveMeterData::UpmoveMeterData(
    const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
    const std::shared_ptr<PlayerEventsHandler> &playerEvents)
    : consoleCommands(consoleCommands), playerEvents(playerEvents) {
  startListeners();
}

UpmoveMeterData::~UpmoveMeterData() {
  consoleCommands->unsubscribe("resetUpmoveMeter");
  playerEvents->unsubscribe("respawn");
}

void UpmoveMeterData::startListeners() {
  consoleCommands->subscribe(
      "resetUpmoveMeter",
      [this](const std::vector<std::string> &) { reset(); });

  playerEvents->subscribe(
      "respawn", [this](const std::vector<std::string> &) { reset(); });
}

bool UpmoveMeterData::check() {
  // we're not checking for individual CHS info values here,
  // this is trivial enough the compute anyway
  return etj_drawUpmoveMeter.integer || etj_drawCHS1.integer ||
         etj_drawCHS2.integer || etj_drawCHS3.integer;
}

void UpmoveMeterData::runFrame() {
  const auto &pmove = cgame.utils.pmoveV2->getState();

  if (!pmove.pm.ps) {
    return;
  }

  if (team != pmove.pm.ps->persistant[PERS_TEAM]) {
    team = static_cast<team_t>(pmove.pm.ps->persistant[PERS_TEAM]);
    reset();
  }

  if (PmoveUtilsV2::skipUpdate(s.lastUpdateTime, std::nullopt, pmove.pm)) {
    return;
  }

  if (canSkipUpdate(pmove)) {
    return;
  }

  // FIXME: should use 's.pm.ps->groundEntityNum' but currently broken,
  // see 'PmoveUtilsV2::groundTrace' for details
  s.inAir = ps->groundEntityNum == ENTITYNUM_NONE;
  s.jumping = pmove.pm.cmd.upmove > 0;

  updateJumpState();
  updateUpmoveValues();
  s.lastJumpState = s.jumpState;
}

const UpmoveMeterData::State &UpmoveMeterData::getState() const { return s; }

void UpmoveMeterData::updateJumpState() {
  switch (s.lastJumpState) {
    case JumpState::AIR_NOJUMP:
    case JumpState::AIR_JUMP:
      if (s.inAir) {
        s.jumpState = s.jumping ? JumpState::AIR_JUMP : JumpState::AIR_NOJUMP;
      } else {
        s.jumpState =
            s.jumping ? JumpState::GROUND_JUMP : JumpState::GROUND_NOJUMP;
      }
      break;
    case JumpState::GROUND_NOJUMP:
    case JumpState::GROUND_JUMP:
    case JumpState::AIR_JUMP_NORELEASE:
      if (s.inAir) {
        s.jumpState =
            s.jumping ? JumpState::AIR_JUMP_NORELEASE : JumpState::AIR_NOJUMP;
      } else {
        s.jumpState =
            s.jumping ? JumpState::GROUND_JUMP : JumpState::GROUND_NOJUMP;
      }
      break;
    default:
      s.jumpState = JumpState::GROUND_NOJUMP;
      break;
  }
}

void UpmoveMeterData::updateUpmoveValues() {
  switch (s.jumpState) {
    case JumpState::AIR_NOJUMP:
      if (s.lastJumpState == JumpState::GROUND_NOJUMP) {
        s.preDelay = s.jumpPreGroundTime - s.lastUpdateTime;
        s.postDelay = 0;
        s.fullDelay = 0;
      } else if (s.lastJumpState == JumpState::AIR_JUMP) {
        s.postDelay = 0;
        s.fullDelay = s.preDelay;
      } else if (s.lastJumpState == JumpState::AIR_JUMP_NORELEASE) {
        s.fullDelay = s.postDelay;

        if (s.preDelay > 0) {
          s.fullDelay += s.preDelay;
        }
      }

      break;
    case JumpState::AIR_JUMP:
      if (s.lastJumpState == JumpState::AIR_NOJUMP) {
        s.jumpPreGroundTime = s.lastUpdateTime;
      }

      s.preDelay = s.lastUpdateTime - s.jumpPreGroundTime;
      break;
    case JumpState::GROUND_JUMP:
      s.groundTouchTime = s.lastUpdateTime;
      break;
    case JumpState::GROUND_NOJUMP:
      if (s.lastJumpState == JumpState::AIR_JUMP ||
          s.lastJumpState == JumpState::GROUND_JUMP) {
        s.postDelay = 0;
        s.fullDelay = s.preDelay;
      } else if (s.lastJumpState == JumpState::AIR_NOJUMP) {
        s.jumpPreGroundTime = s.lastUpdateTime;
      }

      s.preDelay = s.jumpPreGroundTime - s.lastUpdateTime;
      s.groundTouchTime = s.lastUpdateTime;
      break;
    case JumpState::AIR_JUMP_NORELEASE:
      if (s.lastJumpState == JumpState::GROUND_NOJUMP) {
        s.preDelay = s.jumpPreGroundTime - s.lastUpdateTime;
      }

      s.postDelay = s.lastUpdateTime - s.groundTouchTime;
      break;
  }

  s.preDelay = std::clamp(s.preDelay, -MAX_UPMOVE_TIME, MAX_UPMOVE_TIME);
  s.postDelay = std::min(s.postDelay, MAX_UPMOVE_TIME);
}

void UpmoveMeterData::reset() {
  s.preDelay = 0;
  s.postDelay = 0;
  s.fullDelay = 0;

  s.lastJumpState = JumpState::AIR_NOJUMP;
}

bool UpmoveMeterData::canSkipUpdate(const PmoveUtilsV2::State &pmove) {
  if (pmove.pm.ps->pm_type == PM_NOCLIP || pmove.pm.ps->pm_type == PM_DEAD) {
    return true;
  }

  if (BG_PlayerMounted(pmove.pm.ps->eFlags) ||
      pmove.pm.ps->weapon == WP_MOBILE_MG42_SET ||
      pmove.pm.ps->weapon == WP_MORTAR_SET) {
    return true;
  }

  if (pmove.pm.waterlevel > 1 || pmove.pml.ladder) {
    return true;
  }

  return false;
}
} // namespace ETJump
