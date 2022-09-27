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

#include "etj_strafe_quality_drawable.h"
#include "etj_cvar_update_handler.h"
#include "etj_client_commands_handler.h"
#include "etj_utilities.h"
#include "etj_snaphud.h"
#include "etj_cgaz.h"
#include "etj_pmove_utils.h"
#include "etj_player_events_handler.h"

namespace ETJump {
StrafeQuality::StrafeQuality() {
  parseColor();
  startListeners();
}

void StrafeQuality::startListeners() {
  // only subscribe to cvars whose parsing would be inefficient each
  // frame
  cvarUpdateHandler->subscribe(&etj_strafeQualityColor,
                               [&](const vmCvar_t *cvar) { parseColor(); });

  consoleCommandsHandler->subscribe(
      "resetStrafeQuality",
      [&](const std::vector<std::string> &args) { resetStrafeQuality(); });
  playerEventsHandler->subscribe(
      "respawn",
      [&](const std::vector<std::string> &args) { resetStrafeQuality(); });
}

void StrafeQuality::parseColor() {
  parseColorString(etj_strafeQualityColor.string, _color);
}

void StrafeQuality::resetStrafeQuality() {
  // reset underlying vars, but not what is rendered until actually
  // needed
  _totalFrames = _goodFrames = _oldSpeed = 0;
}

void StrafeQuality::beforeRender() {
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
  if (_team != ps.persistant[PERS_TEAM]) {
    _team = ps.persistant[PERS_TEAM];
    // reset strafe quality upon team change
    // note: not handled by consoleCommandsHandler because _team
    // is needed in render() either ways
    resetStrafeQuality();
  }

  // check if current frame should count towards strafe quality
  // we check for framerate dependency here by comparing current time
  // to last update time, using commandTime for clients for 100%
  // accuracy and cg.time for spectators/demos as an approximation note:
  // this will be wrong for clients running < 125FPS... oh well
  const auto frameTime = (cg.snap->ps.pm_flags & PMF_FOLLOW || cg.demoPlayback)
                             ? cg.time
                             : pm->ps->commandTime;

  if (canSkipUpdate(cmd, frameTime)) {
    return;
  }

  // count this frame towards strafe quality
  _lastUpdateTime = frameTime;
  ++_totalFrames;

  // check whether user input is good
  const float speed = VectorLength2(ps.velocity);
  vec3_t wishvel;
  const float wishspeed = PmoveUtils::PM_GetWishspeed(
      wishvel, pm->pmext->scale, cmd, pm->pmext->forward, pm->pmext->right,
      pm->pmext->up, ps, pm);
  if (speed < wishspeed) {
    // possibly good frame under ground speed if speed increased
    // note that without speed increased you could go forward in
    // a "ps.speed - 1" angle endlessly because of velocity
    // snapping
    if (speed > _oldSpeed) {
      // good frame if no upmove and either only
      // forwardmove or only rightmove
      if (cmd.upmove == 0 && ((cmd.forwardmove != 0 && cmd.rightmove == 0) ||
                              (cmd.forwardmove == 0 && cmd.rightmove != 0))) {
        ++_goodFrames;
      }
      // otherwise only half as good because not optimal
      else {
        _goodFrames += 0.5;
      }
    }
  } else {
    // good frame above ground speed if no upmove and in main
    // accel zone
    if (cmd.upmove == 0 && Snaphud::inMainAccelZone(ps, pm)) {
      ++_goodFrames;
    }
    // or if speed increased, half as good because not optimal
    else if (speed > _oldSpeed) {
      _goodFrames += 0.5;
    }
  }

  // update strafe quality
  _strafeQuality = 100 * _goodFrames / _totalFrames;

  // update vars for next frame
  _oldSpeed = speed;
}

void StrafeQuality::render() const {
  // check whether to skip render
  if (canSkipDraw()) {
    return;
  }

  // get coordinates and size
  float x = _x + etj_strafeQualityX.value;
  float y = _y + etj_strafeQualityY.value;
  const float size = 0.1f * etj_strafeQualitySize.value;
  ETJump_AdjustPosition(&x);

  // get hud text
  std::string str;
  std::string qualityStr = std::to_string(_strafeQuality);
  qualityStr.resize(_digits + 1);
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
  CG_Text_Paint_Ext(x, y, size, size, _color, str, 0, 0, textStyle,
                    &cgs.media.limboFont1);
}

bool StrafeQuality::canSkipUpdate(usercmd_t cmd, int frameTime) {
  // only count this frame if it's relevant to pmove
  // this makes sure that if clients FPS > 125
  // we only count frames at pmove_msec intervals
  if (_lastUpdateTime + pm->pmove_msec > frameTime) {
    return true;
  }

  // not strafing
  if (cmd.forwardmove == 0 && cmd.rightmove == 0) {
    return true;
  }

  // don't update if not in air or on ice
  if (cg.snap->ps.groundEntityNum != ENTITYNUM_NONE &&
      !(pm->pmext->groundTrace.surfaceFlags & SURF_SLICK)) {
    return true;
  }

  if (cg.snap->ps.pm_type == PM_NOCLIP || cg.snap->ps.pm_type == PM_DEAD) {
    return true;
  }

  if (BG_PlayerMounted(cg.snap->ps.eFlags) ||
      cg.snap->ps.weapon == WP_MOBILE_MG42_SET ||
      cg.snap->ps.weapon == WP_MORTAR_SET) {
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

  if (_team == TEAM_SPECTATOR) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
