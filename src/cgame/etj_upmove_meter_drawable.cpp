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

#include "etj_upmove_meter_drawable.h"
#include "etj_cvar_update_handler.h"
#include "etj_client_commands_handler.h"
#include "etj_utilities.h"
#include "etj_pmove_utils.h"
#include "etj_player_events_handler.h"

namespace ETJump {
UpmoveMeter::UpmoveMeter() {
  parseAllColors();
  setTextSize();
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

  cvarUpdateHandler->subscribe(&etj_upmoveMeterTextSize,
                               [&](const vmCvar_t *) { setTextSize(); });

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

void UpmoveMeter::setTextSize() {
  textSize = CvarValueParser::parse<CvarValue::Size>(etj_upmoveMeterTextSize);
  textSize.x *= 0.1f;
  textSize.y *= 0.1f;
}

void UpmoveMeter::resetUpmoveMeter() {
  jump_.preDelay = jump_.postDelay = jump_.fullDelay = 0;
  jump_.lastState = AIR_NOJUMP;
}

bool UpmoveMeter::beforeRender() {
  // update team before checking if we should draw or not,
  // since we don't draw for spectators
  if (team_ != ps->persistant[PERS_TEAM]) {
    team_ = ps->persistant[PERS_TEAM];
    // reset upon team change
    // note: not handled by consoleCommandsHandler because team
    // is needed in render() either ways
    resetUpmoveMeter();
  }

  if (canSkipDraw()) {
    return false;
  }

  // get correct pmove
  pm = pmoveUtils->getPmove();

  // never lerp this, because it would just produce unrealistic jump timings
  if (pmoveUtils->skipUpdate(lastUpdateTime, std::nullopt)) {
    return true;
  }

  // just skip update, but return true so that we actually still draw
  if (canSkipUpdate()) {
    return true;
  }

  const bool inAir = ps->groundEntityNum == ENTITYNUM_NONE;
  const bool jumping = pm->cmd.upmove > 0;

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
        jump_.preDelay = jump_.t_jumpPreGround - lastUpdateTime;
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
        jump_.t_jumpPreGround = lastUpdateTime;
      jump_.preDelay = lastUpdateTime - jump_.t_jumpPreGround; // ms
      break;

    case GROUND_JUMP:
      jump_.t_groundTouch = lastUpdateTime;
      break;

    case GROUND_NOJUMP:
      if (jump_.lastState == AIR_JUMP || jump_.lastState == GROUND_JUMP) {
        jump_.postDelay = 0;
        jump_.fullDelay = jump_.preDelay;
      } else if (jump_.lastState == AIR_NOJUMP) {
        jump_.t_jumpPreGround = lastUpdateTime; // groundtime
      }
      jump_.preDelay = jump_.t_jumpPreGround - lastUpdateTime;
      jump_.t_groundTouch = lastUpdateTime;
      break;

    case AIR_JUMPNORELEASE:
      if (jump_.lastState == GROUND_NOJUMP) {
        jump_.preDelay = jump_.t_jumpPreGround - lastUpdateTime;
      }
      jump_.postDelay = lastUpdateTime - jump_.t_groundTouch; // ms
      break;
  }

  if (jump_.preDelay > etj_upmoveMeterMaxDelay.integer)
    jump_.preDelay = etj_upmoveMeterMaxDelay.integer;
  if (jump_.preDelay < -etj_upmoveMeterMaxDelay.integer)
    jump_.preDelay = -etj_upmoveMeterMaxDelay.integer;
  if (jump_.postDelay > etj_upmoveMeterMaxDelay.integer)
    jump_.postDelay = etj_upmoveMeterMaxDelay.integer;

  jump_.lastState = state;

  return true;
}

void UpmoveMeter::render() const {
  const int textStyle =
      (etj_upmoveMeterTextShadow.integer != 0 ? ITEM_TEXTSTYLE_SHADOWED
                                              : ITEM_TEXTSTYLE_NORMAL);
  const float textHeightOffset =
      0.5f * CG_Text_Height_Ext("0", textSize.y, 0, &cgs.media.limboFont1);

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
                      graph_m - jump_.text_xh[1] + textHeightOffset, textSize.x,
                      textSize.y, jump_.text_rgba, va("%i", jump_.postDelay), 0,
                      0, textStyle, &cgs.media.limboFont1);
    CG_Text_Paint_Ext(
        jump_.graph_xywh[0] + jump_.graph_xywh[2] + jump_.text_xh[0],
        graph_m + textHeightOffset, textSize.x, textSize.y, jump_.text_rgba,
        va("%i", jump_.fullDelay), 0, 0, textStyle, &cgs.media.limboFont1);
    CG_Text_Paint_Ext(jump_.graph_xywh[0] + jump_.graph_xywh[2] +
                          jump_.text_xh[0],
                      graph_m + jump_.text_xh[1] + textHeightOffset, textSize.x,
                      textSize.y, jump_.text_rgba, va("%i", jump_.preDelay), 0,
                      0, textStyle, &cgs.media.limboFont1);
  }
}

bool UpmoveMeter::canSkipUpdate() const {
  if (pm->ps->pm_type == PM_NOCLIP || pm->ps->pm_type == PM_DEAD) {
    return true;
  }

  if (BG_PlayerMounted(pm->ps->eFlags) ||
      pm->ps->weapon == WP_MOBILE_MG42_SET || pm->ps->weapon == WP_MORTAR_SET) {
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
