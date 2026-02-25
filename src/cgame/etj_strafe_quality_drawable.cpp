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

#include "etj_strafe_quality_drawable.h"
#include "etj_client_commands_handler.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_player_events_handler.h"
#include "etj_pmove_utils.h"
#include "etj_utilities.h"
#include "etj_snaphud.h"

namespace ETJump {
StrafeQuality::StrafeQuality(
    const std::shared_ptr<CvarUpdateHandler> &cvarUpdate,
    const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
    const std::shared_ptr<PlayerEventsHandler> &playerEvents)
    : cvarUpdate(cvarUpdate), consoleCommands(consoleCommands),
      playerEvents(playerEvents) {
  parseColor(&etj_strafeQualityColor);
  setSize(&etj_strafeQualitySize);
  startListeners();
}

StrafeQuality::~StrafeQuality() {
  cvarUpdate->unsubscribe(&etj_strafeQualityColor);
  cvarUpdate->unsubscribe(&etj_strafeQualitySize);

  consoleCommands->unsubscribe("resetStrafeQuality");

  playerEvents->unsubscribe("respawn");
}

void StrafeQuality::startListeners() {
  // only subscribe to cvars whose parsing would be inefficient each frame
  cvarUpdate->subscribe(&etj_strafeQualityColor,
                        [this](const vmCvar_t *cvar) { parseColor(cvar); });

  cvarUpdate->subscribe(&etj_strafeQualitySize,
                        [this](const vmCvar_t *cvar) { setSize(cvar); });

  consoleCommands->subscribe(
      "resetStrafeQuality",
      [this](const std::vector<std::string> &) { resetStrafeQuality(); });

  playerEvents->subscribe("respawn", [this](const std::vector<std::string> &) {
    resetStrafeQuality();
  });
}

void StrafeQuality::parseColor(const vmCvar_t *cvar) {
  cgame.utils.colorParser->parseColorString(cvar->string, color);
}

void StrafeQuality::setSize(const vmCvar_t *cvar) {
  size = CvarValueParser::parse<CvarValue::Size>(*cvar, 0, 10);
  size.x *= 0.1f;
  size.y *= 0.1f;
}

void StrafeQuality::resetStrafeQuality() {
  // reset underlying vars, but not what is rendered until actually
  // needed
  totalFrames = goodFrames = oldSpeed = 0;
}

bool StrafeQuality::beforeRender() {
  // update team before checking if we should draw or not,
  // since we don't draw for spectators
  if (team != ps->persistant[PERS_TEAM]) {
    team = ps->persistant[PERS_TEAM];
    // reset strafe quality upon team change
    // note: not handled by consoleCommandsHandler because _team
    // is needed in render() either ways
    resetStrafeQuality();
  }

  if (canSkipDraw()) {
    return false;
  }

  // get correct pmove
  pm = cgame.utils.pmove->getPmove();

  if (cgame.utils.pmove->skipUpdate(lastUpdateTime,
                                    HUDLerpFlags::STRAFE_QUALITY)) {
    return true;
  }

  // just skip update, but return true so that we actually still draw
  if (canSkipUpdate(pm->cmd)) {
    return true;
  }

  // count this frame towards strafe quality
  ++totalFrames;

  // check whether user input is good
  const float speed = VectorLength2(ps->velocity);
  vec3_t wishvel;
  const float wishspeed = cgame.utils.pmove->getWishspeed(
      wishvel, pm->pmext->scale, pm->pmext->forward, pm->pmext->right,
      pm->pmext->up);
  if (speed < wishspeed) {
    // possibly good frame under ground speed if speed increased
    // note that without speed increased you could go forward in
    // a "ps.speed - 1" angle endlessly because of velocity snapping
    if (speed > oldSpeed) {
      // good frame if no upmove and either only forwardmove or only rightmove
      if (pm->cmd.upmove == 0 &&
          ((pm->cmd.forwardmove != 0 && pm->cmd.rightmove == 0) ||
           (pm->cmd.forwardmove == 0 && pm->cmd.rightmove != 0))) {
        ++goodFrames;
      }
      // otherwise only half as good because not optimal
      else {
        goodFrames += 0.5;
      }
    }
  } else {
    // good frame above ground speed if no upmove and in main accel zone
    if (pm->cmd.upmove == 0 && Snaphud::inMainAccelZone(*ps, pm)) {
      ++goodFrames;
    }
    // or if speed increased, half as good because not optimal
    else if (speed > oldSpeed) {
      goodFrames += 0.5;
    }
  }

  // update strafe quality
  strafeQuality = 100 * goodFrames / totalFrames;

  // update vars for next frame
  oldSpeed = speed;

  return true;
}

void StrafeQuality::render() const {
  // get coordinates and size
  float x = posX + etj_strafeQualityX.value;
  const float y = posY + etj_strafeQualityY.value;
  ETJump_AdjustPosition(&x);

  // get hud text
  std::string str;
  std::string qualityStr = std::to_string(strafeQuality);
  qualityStr.resize(digits + 1);
  switch (etj_strafeQualityStyle.integer) {
    case 1:
      // percent
      str = qualityStr + "%";
      break;
    case 2:
      // number
      str = qualityStr;
      break;
    default:
      // full
      str = "Strafe Quality: " + qualityStr + "%";
      break;
  }

  // get hud shadow
  const auto textStyle =
      (etj_strafeQualityShadow.integer != 0 ? ITEM_TEXTSTYLE_SHADOWED
                                            : ITEM_TEXTSTYLE_NORMAL);

  // draw quality on screen
  CG_Text_Paint_Ext(x, y, size.x, size.y, color, str, 0, 0, textStyle,
                    &cgs.media.limboFont1);
}

bool StrafeQuality::canSkipUpdate(usercmd_t cmd) {
  // not strafing
  if (cmd.forwardmove == 0 && cmd.rightmove == 0) {
    return true;
  }

  // don't update if not in air or on ice
  if (pm->ps->groundEntityNum != ENTITYNUM_NONE &&
      !(pm->pmext->groundTrace.surfaceFlags & SURF_SLICK)) {
    return true;
  }

  if (pm->ps->pm_type == PM_NOCLIP || pm->ps->pm_type == PM_DEAD) {
    return true;
  }

  if (BG_PlayerMounted(pm->ps->eFlags) ||
      pm->ps->weapon == WP_MOBILE_MG42_SET || pm->ps->weapon == WP_MORTAR_SET) {
    return true;
  }

  // no updates underwater or on ladders
  if (pm->pmext->waterlevel > 1 || pm->pmext->ladder) {
    return true;
  }

  return false;
}

bool StrafeQuality::canSkipDraw() const {
  if (!etj_drawStrafeQuality.integer) {
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
