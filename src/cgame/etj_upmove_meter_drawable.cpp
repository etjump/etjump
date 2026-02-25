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
#include "etj_local.h"
#include "etj_utilities.h"

namespace ETJump {
UpmoveMeter::UpmoveMeter(
    const std::shared_ptr<CvarUpdateHandler> &cvarUpdate,
    const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
    const std::shared_ptr<PlayerEventsHandler> &playerEvents)
    : cvarUpdate(cvarUpdate), consoleCommands(consoleCommands),
      playerEvents(playerEvents) {
  parseAllColors();
  setTextSize(&etj_upmoveMeterTextSize);
  startListeners();
}

UpmoveMeter::~UpmoveMeter() {
  cvarUpdate->unsubscribe(&etj_upmoveMeterGraphColor);
  cvarUpdate->unsubscribe(&etj_upmoveMeterGraphOnGroundColor);
  cvarUpdate->unsubscribe(&etj_upmoveMeterGraphPreJumpColor);
  cvarUpdate->unsubscribe(&etj_upmoveMeterGraphPostJumpColor);
  cvarUpdate->unsubscribe(&etj_upmoveMeterGraphOutlineColor);
  cvarUpdate->unsubscribe(&etj_upmoveMeterTextColor);
  cvarUpdate->unsubscribe(&etj_upmoveMeterTextSize);

  consoleCommands->unsubscribe("resetUpmoveMeter");
  playerEvents->unsubscribe("respawn");
}

void UpmoveMeter::startListeners() {
  // only subscribe to cvars whose parsing would be inefficient each
  // frame
  cvarUpdate->subscribe(&etj_upmoveMeterGraphColor, [this](
                                                        const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, jump.graph_rgba);
  });

  cvarUpdate->subscribe(&etj_upmoveMeterGraphOnGroundColor,
                        [this](const vmCvar_t *cvar) {
                          cgame.utils.colorParser->parseColorString(
                              cvar->string, jump.graph_rgbaOnGround);
                        });

  cvarUpdate->subscribe(&etj_upmoveMeterGraphPreJumpColor,
                        [this](const vmCvar_t *cvar) {
                          cgame.utils.colorParser->parseColorString(
                              cvar->string, jump.graph_rgbaPreJump);
                        });

  cvarUpdate->subscribe(&etj_upmoveMeterGraphPostJumpColor,
                        [this](const vmCvar_t *cvar) {
                          cgame.utils.colorParser->parseColorString(
                              cvar->string, jump.graph_rgbaPostJump);
                        });

  cvarUpdate->subscribe(&etj_upmoveMeterGraphOutlineColor,
                        [this](const vmCvar_t *cvar) {
                          cgame.utils.colorParser->parseColorString(
                              cvar->string, jump.graph_outline_rgba);
                        });

  cvarUpdate->subscribe(
      &etj_upmoveMeterTextColor, [this](const vmCvar_t *cvar) {
        cgame.utils.colorParser->parseColorString(cvar->string, jump.text_rgba);
      });

  cvarUpdate->subscribe(&etj_upmoveMeterTextSize,
                        [this](const vmCvar_t *cvar) { setTextSize(cvar); });

  consoleCommands->subscribe(
      "resetUpmoveMeter",
      [this](const std::vector<std::string> &) { resetUpmoveMeter(); });

  playerEvents->subscribe("respawn", [this](const std::vector<std::string> &) {
    resetUpmoveMeter();
  });
}

void UpmoveMeter::parseAllColors() {
  cgame.utils.colorParser->parseColorString(etj_upmoveMeterGraphColor.string,
                                            jump.graph_rgba);
  cgame.utils.colorParser->parseColorString(
      etj_upmoveMeterGraphOnGroundColor.string, jump.graph_rgbaOnGround);
  cgame.utils.colorParser->parseColorString(
      etj_upmoveMeterGraphPreJumpColor.string, jump.graph_rgbaPreJump);
  cgame.utils.colorParser->parseColorString(
      etj_upmoveMeterGraphPostJumpColor.string, jump.graph_rgbaPostJump);
  cgame.utils.colorParser->parseColorString(
      etj_upmoveMeterGraphOutlineColor.string, jump.graph_outline_rgba);
  cgame.utils.colorParser->parseColorString(etj_upmoveMeterTextColor.string,
                                            jump.text_rgba);
}

void UpmoveMeter::setTextSize(const vmCvar_t *cvar) {
  textSize = CvarValueParser::parse<CvarValue::Size>(*cvar);
  textSize.x *= 0.1f;
  textSize.y *= 0.1f;
}

void UpmoveMeter::resetUpmoveMeter() {
  jump.preDelay = jump.postDelay = jump.fullDelay = 0;
  jump.lastState = AIR_NOJUMP;
}

bool UpmoveMeter::beforeRender() {
  // update team before checking if we should draw or not,
  // since we don't draw for spectators
  if (team != ps->persistant[PERS_TEAM]) {
    team = ps->persistant[PERS_TEAM];
    // reset upon team change
    // note: not handled by consoleCommandsHandler because team
    // is needed in render() either ways
    resetUpmoveMeter();
  }

  if (canSkipDraw()) {
    return false;
  }

  // get correct pmove
  pm = cgame.utils.pmove->getPmove();

  // never lerp this, because it would just produce unrealistic jump timings
  if (cgame.utils.pmove->skipUpdate(lastUpdateTime, std::nullopt)) {
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
  switch (jump.lastState) {
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
      if (jump.lastState == GROUND_NOJUMP) {
        jump.preDelay = jump.t_jumpPreGround - lastUpdateTime;
        jump.postDelay = 0;
        jump.fullDelay = 0;
      } else if (jump.lastState == AIR_JUMP) {
        jump.postDelay = 0;
        jump.fullDelay = jump.preDelay;
      } else if (jump.lastState == AIR_JUMPNORELEASE) {
        jump.fullDelay = jump.postDelay;
        if (jump.preDelay > 0) {
          jump.fullDelay += jump.preDelay;
        }
      }
      break;

    case AIR_JUMP:
      if (jump.lastState == AIR_NOJUMP)
        jump.t_jumpPreGround = lastUpdateTime;
      jump.preDelay = lastUpdateTime - jump.t_jumpPreGround; // ms
      break;

    case GROUND_JUMP:
      jump.t_groundTouch = lastUpdateTime;
      break;

    case GROUND_NOJUMP:
      if (jump.lastState == AIR_JUMP || jump.lastState == GROUND_JUMP) {
        jump.postDelay = 0;
        jump.fullDelay = jump.preDelay;
      } else if (jump.lastState == AIR_NOJUMP) {
        jump.t_jumpPreGround = lastUpdateTime; // groundtime
      }
      jump.preDelay = jump.t_jumpPreGround - lastUpdateTime;
      jump.t_groundTouch = lastUpdateTime;
      break;

    case AIR_JUMPNORELEASE:
      if (jump.lastState == GROUND_NOJUMP) {
        jump.preDelay = jump.t_jumpPreGround - lastUpdateTime;
      }
      jump.postDelay = lastUpdateTime - jump.t_groundTouch; // ms
      break;
  }

  if (jump.preDelay > etj_upmoveMeterMaxDelay.integer)
    jump.preDelay = etj_upmoveMeterMaxDelay.integer;
  if (jump.preDelay < -etj_upmoveMeterMaxDelay.integer)
    jump.preDelay = -etj_upmoveMeterMaxDelay.integer;
  if (jump.postDelay > etj_upmoveMeterMaxDelay.integer)
    jump.postDelay = etj_upmoveMeterMaxDelay.integer;

  jump.lastState = state;

  jump.graph_xywh[0] = graphX_ + etj_upmoveMeterGraphX.value;
  ETJump_AdjustPosition(&jump.graph_xywh[0]);
  jump.graph_xywh[1] = graphY_ + etj_upmoveMeterGraphY.value;
  jump.graph_xywh[2] = etj_upmoveMeterGraphW.value;
  jump.graph_xywh[3] = etj_upmoveMeterGraphH.value;

  jump.text_xh[0] = etj_upmoveMeterTextX.value;
  ETJump_AdjustPosition(&jump.text_xh[0]);
  jump.text_xh[1] = etj_upmoveMeterTextH.value;

  return true;
}

void UpmoveMeter::render() const {
  const int textStyle =
      (etj_upmoveMeterTextShadow.integer != 0 ? ITEM_TEXTSTYLE_SHADOWED
                                              : ITEM_TEXTSTYLE_NORMAL);
  const float textHeightOffset =
      0.5f * CG_Text_Height_Ext("0", textSize.y, 0, &cgs.media.limboFont1);

  const float graph_hh = jump.graph_xywh[3] / 2.0f; // half height
  const float graph_m = jump.graph_xywh[1] + graph_hh;

  const float upHeight =
      (jump.postDelay / std::max(etj_upmoveMeterMaxDelay.value, 1.0f)) *
      graph_hh;
  const float downHeight = (std::abs(jump.preDelay) /
                            std::max(etj_upmoveMeterMaxDelay.value, 1.0f)) *
                           graph_hh;

  if (etj_drawUpmoveMeter.integer & 1) {
    CG_FillRect(jump.graph_xywh[0], jump.graph_xywh[1], jump.graph_xywh[2],
                jump.graph_xywh[3], jump.graph_rgba);
    CG_FillRect(jump.graph_xywh[0], graph_m, jump.graph_xywh[2], downHeight,
                jump.preDelay < 0 ? jump.graph_rgbaOnGround
                                  : jump.graph_rgbaPreJump);
    CG_FillRect(jump.graph_xywh[0], graph_m - upHeight, jump.graph_xywh[2],
                upHeight, jump.graph_rgbaPostJump);
    CG_DrawRect(jump.graph_xywh[0], jump.graph_xywh[1], jump.graph_xywh[2],
                jump.graph_xywh[3], etj_upmoveMeterGraphOutlineW.value,
                jump.graph_outline_rgba);
  }
  if (etj_drawUpmoveMeter.integer & 2) {
    CG_Text_Paint_Ext(jump.graph_xywh[0] + jump.graph_xywh[2] + jump.text_xh[0],
                      graph_m - jump.text_xh[1] + textHeightOffset, textSize.x,
                      textSize.y, jump.text_rgba, va("%i", jump.postDelay), 0,
                      0, textStyle, &cgs.media.limboFont1);
    CG_Text_Paint_Ext(jump.graph_xywh[0] + jump.graph_xywh[2] + jump.text_xh[0],
                      graph_m + textHeightOffset, textSize.x, textSize.y,
                      jump.text_rgba, va("%i", jump.fullDelay), 0, 0, textStyle,
                      &cgs.media.limboFont1);
    CG_Text_Paint_Ext(jump.graph_xywh[0] + jump.graph_xywh[2] + jump.text_xh[0],
                      graph_m + jump.text_xh[1] + textHeightOffset, textSize.x,
                      textSize.y, jump.text_rgba, va("%i", jump.preDelay), 0, 0,
                      textStyle, &cgs.media.limboFont1);
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

  if (team == TEAM_SPECTATOR) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
