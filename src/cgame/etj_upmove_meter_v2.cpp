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

#include "etj_upmove_meter_v2.h"
#include "etj_client_commands_handler.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_player_events_handler.h"
#include "etj_utilities.h"

namespace ETJump {
inline constexpr float UPMOVEMETER_POS_X = 8.0f;
inline constexpr float UPMOVEMETER_POS_Y = 8.0f;

UpmoveMeterV2::UpmoveMeterV2(
    const std::shared_ptr<CvarUpdateHandler> &cvarUpdate,
    const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
    const std::shared_ptr<PlayerEventsHandler> &playerEvents)
    : cvarUpdate(cvarUpdate), consoleCommands(consoleCommands),
      playerEvents(playerEvents) {
  cgame.utils.colorParser->parseColorString(etj_upmoveMeterGraphColor.string,
                                            graph.colorBg);
  cgame.utils.colorParser->parseColorString(
      etj_upmoveMeterGraphOnGroundColor.string, graph.colorOnGround);
  cgame.utils.colorParser->parseColorString(
      etj_upmoveMeterGraphPreJumpColor.string, graph.colorPreJump);
  cgame.utils.colorParser->parseColorString(
      etj_upmoveMeterGraphPostJumpColor.string, graph.colorPostJump);
  cgame.utils.colorParser->parseColorString(
      etj_upmoveMeterGraphOutlineColor.string, graph.colorOutline);

  cgame.utils.colorParser->parseColorString(etj_upmoveMeterTextColor.string,
                                            colorText);

  setTextSize(etj_upmoveMeterTextSize);
  startListeners();
}

UpmoveMeterV2::~UpmoveMeterV2() {
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

void UpmoveMeterV2::startListeners() {
  cvarUpdate->subscribe(
      &etj_upmoveMeterGraphColor, [this](const vmCvar_t *cvar) {
        cgame.utils.colorParser->parseColorString(cvar->string, graph.colorBg);
      });

  cvarUpdate->subscribe(&etj_upmoveMeterGraphOnGroundColor,
                        [this](const vmCvar_t *cvar) {
                          cgame.utils.colorParser->parseColorString(
                              cvar->string, graph.colorOnGround);
                        });

  cvarUpdate->subscribe(&etj_upmoveMeterGraphPreJumpColor,
                        [this](const vmCvar_t *cvar) {
                          cgame.utils.colorParser->parseColorString(
                              cvar->string, graph.colorPreJump);
                        });

  cvarUpdate->subscribe(&etj_upmoveMeterGraphPostJumpColor,
                        [this](const vmCvar_t *cvar) {
                          cgame.utils.colorParser->parseColorString(
                              cvar->string, graph.colorPostJump);
                        });

  cvarUpdate->subscribe(&etj_upmoveMeterGraphOutlineColor,
                        [this](const vmCvar_t *cvar) {
                          cgame.utils.colorParser->parseColorString(
                              cvar->string, graph.colorOutline);
                        });

  cvarUpdate->subscribe(
      &etj_upmoveMeterTextColor, [this](const vmCvar_t *cvar) {
        cgame.utils.colorParser->parseColorString(cvar->string, colorText);
      });

  cvarUpdate->subscribe(&etj_upmoveMeterTextSize,
                        [this](const vmCvar_t *cvar) { setTextSize(*cvar); });

  consoleCommands->subscribe(
      "resetUpmoveMeter",
      [this](const std::vector<std::string> &) { resetUpmoveMeter(); });

  playerEvents->subscribe("respawn", [this](const std::vector<std::string> &) {
    resetUpmoveMeter();
  });
}

void UpmoveMeterV2::setTextSize(const vmCvar_t &cvar) {
  textSize = CvarValueParser::parse<CvarValue::Size>(cvar);
  textSize.x *= 0.1f;
  textSize.y *= 0.1f;

  textHeightOffset = static_cast<float>(CG_Text_Height_Ext(
                         "0", textSize.y, 0, &cgs.media.limboFont1)) *
                     0.5f;
}

void UpmoveMeterV2::resetUpmoveMeter() {
  graph.preDelay = 0;
  graph.postDelay = 0;
  graph.fullDelay = 0;

  jumpState = JumpState::AIR_NOJUMP;
}

void UpmoveMeterV2::updateJumpState() {
  switch (lastJumpState) {
    case JumpState::AIR_NOJUMP:
    case JumpState::AIR_JUMP:
      if (inAir) {
        jumpState = jumping ? JumpState::AIR_JUMP : JumpState::AIR_NOJUMP;
      } else {
        jumpState = jumping ? JumpState::GROUND_JUMP : JumpState::GROUND_NOJUMP;
      }
      break;
    case JumpState::GROUND_NOJUMP:
    case JumpState::GROUND_JUMP:
    case JumpState::AIR_JUMP_NORELEASE:
      if (inAir) {
        jumpState =
            jumping ? JumpState::AIR_JUMP_NORELEASE : JumpState::AIR_NOJUMP;
      } else {
        jumpState = jumping ? JumpState::GROUND_JUMP : JumpState::GROUND_NOJUMP;
      }
      break;
    default:
      jumpState = JumpState::GROUND_NOJUMP;
      break;
  }
}

void UpmoveMeterV2::updateUpmoveMeter() {
  switch (jumpState) {
    case JumpState::AIR_NOJUMP:
      if (lastJumpState == JumpState::GROUND_NOJUMP) {
        graph.preDelay = jumpPreGroundTime - lastUpdateTime;
        graph.postDelay = 0;
        graph.fullDelay = 0;
      } else if (lastJumpState == JumpState::AIR_JUMP) {
        graph.postDelay = 0;
        graph.fullDelay = graph.preDelay;
      } else if (lastJumpState == JumpState::AIR_JUMP_NORELEASE) {
        graph.fullDelay = graph.postDelay;

        if (graph.preDelay > 0) {
          graph.fullDelay += graph.preDelay;
        }
      }

      break;
    case JumpState::AIR_JUMP:
      if (lastJumpState == JumpState::AIR_NOJUMP) {
        jumpPreGroundTime = lastUpdateTime;
      }

      graph.preDelay = lastUpdateTime - jumpPreGroundTime;
      break;
    case JumpState::GROUND_JUMP:
      groundTouchTime = lastUpdateTime;
      break;
    case JumpState::GROUND_NOJUMP:
      if (lastJumpState == JumpState::AIR_JUMP ||
          lastJumpState == JumpState::GROUND_JUMP) {
        graph.postDelay = 0;
        graph.fullDelay = graph.preDelay;
      } else if (lastJumpState == JumpState::AIR_NOJUMP) {
        jumpPreGroundTime = lastUpdateTime;
      }

      graph.preDelay = jumpPreGroundTime - lastUpdateTime;
      groundTouchTime = lastUpdateTime;
      break;
    case JumpState::AIR_JUMP_NORELEASE:
      if (lastJumpState == JumpState::GROUND_NOJUMP) {
        graph.preDelay = jumpPreGroundTime - lastUpdateTime;
      }

      graph.postDelay = lastUpdateTime - groundTouchTime;
      break;
  }

  graph.preDelay = std::clamp(graph.preDelay, -etj_upmoveMeterMaxDelay.integer,
                              etj_upmoveMeterMaxDelay.integer);
  graph.postDelay = std::min(graph.postDelay, etj_upmoveMeterMaxDelay.integer);
}

bool UpmoveMeterV2::beforeRender() {
  const PmoveUtilsV2::State &s = cgame.utils.pmoveV2->getState();

  if (!s.pm.ps) {
    return false;
  }

  if (team != s.pm.ps->persistant[PERS_TEAM]) {
    team = static_cast<team_t>(s.pm.ps->persistant[PERS_TEAM]);
    resetUpmoveMeter();
  }

  if (canSkipDraw()) {
    return false;
  }

  // never lerp this, it would just produce unrealistic jump timings
  if (PmoveUtilsV2::skipUpdate(lastUpdateTime, std::nullopt, s.pm)) {
    return true;
  }

  if (canSkipUpdate(s)) {
    return true;
  }

  // FIXME: should use 's.pm.ps->groundEntityNum' but currently broken,
  // see 'PmoveUtilsV2::groundTrace' for details
  inAir = ps->groundEntityNum == ENTITYNUM_NONE;
  jumping = s.pm.cmd.upmove > 0;

  updateJumpState();
  updateUpmoveMeter();
  lastJumpState = jumpState;

  graph.rect.x =
      std::clamp(UPMOVEMETER_POS_X + etj_upmoveMeterGraphX.value, 0.0f, 640.0f);
  graph.rect.y = std::clamp(UPMOVEMETER_POS_Y + etj_upmoveMeterGraphY.value,
                            0.0f, static_cast<float>(SCREEN_HEIGHT));
  graph.rect.w = etj_upmoveMeterGraphW.value;
  graph.rect.h = etj_upmoveMeterGraphH.value;

  // TODO: A/B testing, remove
  graph.rect.y += graph.rect.h + 5;

  graph.upHeight =
      static_cast<float>(graph.postDelay) /
      std::max(static_cast<float>(etj_upmoveMeterMaxDelay.integer), 1.0f);
  graph.upHeight *= graph.rect.h * 0.5f;

  graph.downHeight =
      static_cast<float>(std::abs(graph.preDelay)) /
      std::max(static_cast<float>(etj_upmoveMeterMaxDelay.integer), 1.0f);
  graph.downHeight *= graph.rect.h * 0.5f;

  textX = std::clamp(graph.rect.x + graph.rect.w + etj_upmoveMeterTextX.value,
                     0.0f, 640.0f);
  textH = etj_upmoveMeterTextH.value;

  // NOTE: apply widescreen adjustment to 'rect.x' only after 'textX'
  // has been calculated as well
  ETJump_AdjustPosition(&graph.rect.x);
  ETJump_AdjustPosition(&textX);

  textStyle = etj_upmoveMeterTextShadow.integer ? ITEM_TEXTSTYLE_SHADOWED
                                                : ITEM_TEXTSTYLE_NORMAL;

  return true;
}

void UpmoveMeterV2::render() const {
  const float graphHH = graph.rect.h * 0.5f;
  const float graphM = graph.rect.y + graphHH;

  // graph
  if (etj_drawUpmoveMeter.integer & 1) {
    CG_FillRect(graph.rect, graph.colorBg);

    CG_FillRect(graph.rect.x, graph.rect.y + graphHH, graph.rect.w,
                graph.downHeight,
                graph.preDelay < 0 ? graph.colorOnGround : graph.colorPreJump);
    CG_FillRect(graph.rect.x, graphM - graph.upHeight, graph.rect.w,
                graph.upHeight, graph.colorPostJump);

    CG_DrawRect(graph.rect, etj_upmoveMeterGraphOutlineW.value,
                graph.colorOutline);
  }

  // text
  if (etj_drawUpmoveMeter.integer & 2) {
    CG_Text_Paint_Ext(textX, graphM - textH + textHeightOffset, textSize.x,
                      textSize.y, colorText, std::to_string(graph.postDelay), 0,
                      0, textStyle, &cgs.media.limboFont1);

    CG_Text_Paint_Ext(textX, graphM + textHeightOffset, textSize.x, textSize.y,
                      colorText, std::to_string(graph.fullDelay), 0, 0,
                      textStyle, &cgs.media.limboFont1);

    CG_Text_Paint_Ext(textX, graphM + textH + textHeightOffset, textSize.x,
                      textSize.y, colorText, std::to_string(graph.preDelay), 0,
                      0, textStyle, &cgs.media.limboFont1);
  }
}

bool UpmoveMeterV2::canSkipUpdate(const PmoveUtilsV2::State &s) const {
  if (ps->pm_type == PM_NOCLIP || ps->pm_type == PM_DEAD) {
    return true;
  }

  if (BG_PlayerMounted(ps->eFlags) || ps->weapon == WP_MOBILE_MG42_SET ||
      ps->weapon == WP_MORTAR_SET) {
    return true;
  }

  if (s.pm.waterlevel > 1 || s.pml.ladder) {
    return true;
  }

  return false;
}

bool UpmoveMeterV2::canSkipDraw() const {
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
