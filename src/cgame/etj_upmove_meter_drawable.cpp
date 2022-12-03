/*
 * MIT License
 *
 * Copyright (c) 2022 ETJump team <zero@etjump.com>
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

#include "etj_upmove_meter_drawable.h"
#include "etj_cvar_update_handler.h"
#include "etj_client_commands_handler.h"
#include "etj_utilities.h"
#include "etj_pmove_utils.h"
#include "etj_player_events_handler.h"

namespace ETJump {
UpmoveMeter::UpmoveMeter() {
  parseAllColors();
  startListeners();

  team_ = 0;
}

void UpmoveMeter::startListeners() {
  // only subscribe to cvars whose parsing would be inefficient each
  // frame
  cvarUpdateHandler->subscribe(
      &etj_upmoveMeterGraphColor, [&](const vmCvar_t *cvar) {
        parseColorString(etj_upmoveMeterGraphColor.string, jump_.graph_rgba);
      });
  cvarUpdateHandler->subscribe(
      &etj_upmoveMeterGraphOnGroundColor, [&](const vmCvar_t *cvar) {
        parseColorString(etj_upmoveMeterGraphOnGroundColor.string,
                         jump_.graph_rgbaOnGround);
      });
  cvarUpdateHandler->subscribe(
      &etj_upmoveMeterGraphPreJumpColor, [&](const vmCvar_t *cvar) {
        parseColorString(etj_upmoveMeterGraphPreJumpColor.string,
                         jump_.graph_rgbaPreJump);
      });
  cvarUpdateHandler->subscribe(
      &etj_upmoveMeterGraphPostJumpColor, [&](const vmCvar_t *cvar) {
        parseColorString(etj_upmoveMeterGraphPostJumpColor.string,
                         jump_.graph_rgbaPostJump);
      });
  cvarUpdateHandler->subscribe(
      &etj_upmoveMeterGraphOutlineColor, [&](const vmCvar_t *cvar) {
        parseColorString(etj_upmoveMeterGraphOutlineColor.string,
                         jump_.graph_outline_rgba);
      });
  cvarUpdateHandler->subscribe(
      &etj_upmoveMeterTextColor, [&](const vmCvar_t *cvar) {
        parseColorString(etj_upmoveMeterTextColor.string, jump_.text_rgba);
      });

  consoleCommandsHandler->subscribe(
      "resetUpmoveMeter",
      [&](const std::vector<std::string> &args) { resetUpmoveMeter(); });

  playerEventsHandler->subscribe(
      "respawn",
      [&](const std::vector<std::string> &args) { resetUpmoveMeter(); });
}

void UpmoveMeter::parseAllColors() {
  parseColorString(etj_upmoveMeterGraphColor.string, jump_.graph_rgba);
  parseColorString(etj_upmoveMeterGraphOnGroundColor.string,
                   jump_.graph_rgbaOnGround);
  parseColorString(etj_upmoveMeterGraphPreJumpColor.string,
                   jump_.graph_rgbaPreJump);
  parseColorString(etj_upmoveMeterGraphPostJumpColor.string,
                   jump_.graph_rgbaPostJump);
  parseColorString(etj_upmoveMeterGraphOutlineColor.string,
                   jump_.graph_outline_rgba);
  parseColorString(etj_upmoveMeterTextColor.string, jump_.text_rgba);
}

void UpmoveMeter::resetUpmoveMeter() {
  jump_.preDelay = jump_.postDelay = jump_.fullDelay = 0;
  jump_.lastState = AIR_NOJUMP;
}

void UpmoveMeter::beforeRender() {
  // get player state
  const playerState_t &ps = cg.predictedPlayerState;

  // get usercmd
  // cmdScale is only checked here to be 0 or !0
  // so we can just use CMDSCALE_DEFAULT
  const int8_t ucmdScale = CMDSCALE_DEFAULT;
  const usercmd_t cmd = PmoveUtils::getUserCmd(ps, ucmdScale);

  // get correct pmove
  pm = PmoveUtils::getPmove(cmd);

  // update team
  if (team_ != ps.persistant[PERS_TEAM]) {
    team_ = ps.persistant[PERS_TEAM];
    // reset upon team change
    // note: not handled by consoleCommandsHandler because team
    // is needed in render() either ways
    resetUpmoveMeter();
  }

  if (canSkipUpdate()) {
    return;
  }

  int now;
  if (cg.snap->ps.pm_flags & PMF_FOLLOW || cg.demoPlayback) {
    now = ((cg.time >> 3) << 3) + ((cg.time % 8) >= 4 ? 8 : 0);
  } else {
    now = pm->ps->commandTime;
  }
  const bool inAir = ps.groundEntityNum == ENTITYNUM_NONE;
  const bool jumping = cmd.upmove > 0;

  // determine current state
  state_t state;
  switch (jump_.lastState) {
    case AIR_JUMP:
    case AIR_NOJUMP:
      if (inAir) {
        state = jumping ? AIR_JUMP : AIR_NOJUMP;
      } else {
        state = jumping ? GROUND_JUMP : GROUND_NOJUMP;
      }
      break;

    // edge case at end of cycle
    case GROUND_NOJUMP:
    case GROUND_JUMP:
    case AIR_JUMPNORELEASE:
      if (inAir) {
        state = jumping ? AIR_JUMPNORELEASE : AIR_NOJUMP;
      } else {
        state = jumping ? GROUND_JUMP : GROUND_NOJUMP;
      }
      break;

    default:
      state = GROUND_NOJUMP;
      break;
  }

  // act on current state
  switch (state) {
    case AIR_NOJUMP: // we spend the most time in this state
                     // that is why here we show the last jump
                     // stats
      if (jump_.lastState == GROUND_NOJUMP) {
        jump_.preDelay = jump_.t_jumpPreGround - now;
        jump_.postDelay = 0;
        jump_.fullDelay = 0;
      } else if (jump_.lastState == AIR_JUMP) {
        jump_.postDelay = 0;
        jump_.fullDelay = jump_.preDelay;
      } else if (jump_.lastState == AIR_JUMPNORELEASE) {
        jump_.fullDelay = jump_.postDelay;
        if (jump_.preDelay > 0) {
          jump_.fullDelay += jump_.preDelay;
        }
      }
      break;

    case AIR_JUMP:
      if (jump_.lastState == AIR_NOJUMP)
        jump_.t_jumpPreGround = now;
      jump_.preDelay = now - jump_.t_jumpPreGround; // ms
      break;

    case GROUND_JUMP:
      jump_.t_groundTouch = now;
      break;

    case GROUND_NOJUMP:
      if (jump_.lastState == AIR_JUMP || jump_.lastState == GROUND_JUMP) {
        jump_.postDelay = 0;
        jump_.fullDelay = jump_.preDelay;
      } else if (jump_.lastState == AIR_NOJUMP) {
        jump_.t_jumpPreGround = now; // groundtime
      }
      jump_.preDelay = jump_.t_jumpPreGround - now;
      jump_.t_groundTouch = now;
      break;

    case AIR_JUMPNORELEASE:
      if (jump_.lastState == GROUND_NOJUMP) {
        jump_.preDelay = jump_.t_jumpPreGround - now;
      }
      jump_.postDelay = now - jump_.t_groundTouch; // ms
      break;

    default:
      break;
  }

  if (jump_.preDelay > etj_upmoveMeterMaxDelay.integer)
    jump_.preDelay = etj_upmoveMeterMaxDelay.integer;
  if (jump_.preDelay < -etj_upmoveMeterMaxDelay.integer)
    jump_.preDelay = -etj_upmoveMeterMaxDelay.integer;
  if (jump_.postDelay > etj_upmoveMeterMaxDelay.integer)
    jump_.postDelay = etj_upmoveMeterMaxDelay.integer;

  jump_.lastState = state;
}

void UpmoveMeter::render() const {
  // check whether to skip render
  if (canSkipDraw()) {
    return;
  }

  const int textStyle =
      (etj_upmoveMeterTextShadow.integer != 0 ? ITEM_TEXTSTYLE_SHADOWED
                                              : ITEM_TEXTSTYLE_NORMAL);
  const float textSize = 0.1f * etj_upmoveMeterTextSize.value;
  const float textHeightOffset =
      0.5f * CG_Text_Height_Ext("0", textSize, 0, &cgs.media.limboFont1);

  jump_.graph_xywh[0] = graphX_ + etj_upmoveMeterGraphX.value;
  ETJump_AdjustPosition(&jump_.graph_xywh[0]);
  jump_.graph_xywh[1] = graphY_ + etj_upmoveMeterGraphY.value;
  jump_.graph_xywh[2] = etj_upmoveMeterGraphW.value;
  jump_.graph_xywh[3] = etj_upmoveMeterGraphH.value;

  jump_.text_xh[0] = etj_upmoveMeterTextX.value;
  ETJump_AdjustPosition(&jump_.text_xh[0]);
  jump_.text_xh[1] = etj_upmoveMeterTextH.value;

  const float graph_hh = jump_.graph_xywh[3] / 2.0f; // half height
  const float graph_m = jump_.graph_xywh[1] + graph_hh;

  const float upHeight =
      (jump_.postDelay / std::max(etj_upmoveMeterMaxDelay.value, 1.0f)) *
      graph_hh;
  const float downHeight = (std::abs(jump_.preDelay) /
                            std::max(etj_upmoveMeterMaxDelay.value, 1.0f)) *
                           graph_hh;

  if (etj_drawUpmoveMeter.integer & 1) {
    CG_FillRect(jump_.graph_xywh[0], jump_.graph_xywh[1], jump_.graph_xywh[2],
                jump_.graph_xywh[3], jump_.graph_rgba);
    CG_FillRect(jump_.graph_xywh[0], graph_m, jump_.graph_xywh[2], downHeight,
                jump_.preDelay < 0 ? jump_.graph_rgbaOnGround
                                   : jump_.graph_rgbaPreJump);
    CG_FillRect(jump_.graph_xywh[0], graph_m - upHeight, jump_.graph_xywh[2],
                upHeight, jump_.graph_rgbaPostJump);
    CG_DrawRect(jump_.graph_xywh[0], jump_.graph_xywh[1], jump_.graph_xywh[2],
                jump_.graph_xywh[3], etj_upmoveMeterGraphOutlineW.value,
                jump_.graph_outline_rgba);
  }
  if (etj_drawUpmoveMeter.integer & 2) {
    CG_Text_Paint_Ext(jump_.graph_xywh[0] + jump_.graph_xywh[2] +
                          jump_.text_xh[0],
                      graph_m - jump_.text_xh[1] + textHeightOffset, textSize,
                      textSize, jump_.text_rgba, va("%i", jump_.postDelay), 0,
                      0, textStyle, &cgs.media.limboFont1);
    CG_Text_Paint_Ext(
        jump_.graph_xywh[0] + jump_.graph_xywh[2] + jump_.text_xh[0],
        graph_m + textHeightOffset, textSize, textSize, jump_.text_rgba,
        va("%i", jump_.fullDelay), 0, 0, textStyle, &cgs.media.limboFont1);
    CG_Text_Paint_Ext(jump_.graph_xywh[0] + jump_.graph_xywh[2] +
                          jump_.text_xh[0],
                      graph_m + jump_.text_xh[1] + textHeightOffset, textSize,
                      textSize, jump_.text_rgba, va("%i", jump_.preDelay), 0, 0,
                      textStyle, &cgs.media.limboFont1);
  }
}

bool UpmoveMeter::canSkipUpdate() const {
  if (cg.snap->ps.pm_type == PM_NOCLIP || cg.snap->ps.pm_type == PM_DEAD) {
    return true;
  }

  if (BG_PlayerMounted(cg.snap->ps.eFlags) ||
      cg.snap->ps.weapon == WP_MOBILE_MG42_SET ||
      cg.snap->ps.weapon == WP_MORTAR_SET) {
    return true;
  }

  if (pm->pmext->waterlevel > 1 || pm->pmext->ladder) {
    return true;
  }

  return false;
}

bool UpmoveMeter::canSkipDraw() const {
  if (!etj_drawUpmoveMeter.integer) {
    return true;
  }

  if (team_ == TEAM_SPECTATOR) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
