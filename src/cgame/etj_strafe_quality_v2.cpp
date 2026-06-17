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

#include "etj_strafe_quality_v2.h"
#include "cg_local.h"
#include "etj_client_commands_handler.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_local.h"
#include "etj_player_events_handler.h"
#include "etj_snaphud_data.h"
#include "etj_utilities.h"

inline constexpr float POS_X = 100;
inline constexpr float POS_Y = 100;
inline constexpr float SQ_SIZE_MIN = 0.1f;
inline constexpr float SQ_SIZE_MAX = 10.0f;

namespace ETJump {
StrafeQualityV2::StrafeQualityV2(
    const std::shared_ptr<SnaphudData> &snaphudData,
    const std::shared_ptr<CvarUpdateHandler> &cvarUpdate,
    const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
    const std::shared_ptr<PlayerEventsHandler> &playerEvents)
    : snaphudData(snaphudData), cvarUpdate(cvarUpdate),
      consoleCommands(consoleCommands), playerEvents(playerEvents) {
  cgame.utils.colorParser->parseColorString(etj_strafeQualityColor.string,
                                            color);
  setSize(etj_strafeQualitySize);
  startListeners();
}

StrafeQualityV2::~StrafeQualityV2() {
  cvarUpdate->unsubscribe(&etj_strafeQualityColor);
  cvarUpdate->unsubscribe(&etj_strafeQualitySize);

  consoleCommands->unsubscribe("resetStrafeQuality");

  playerEvents->unsubscribe("respawn");
}

void StrafeQualityV2::startListeners() {
  cvarUpdate->subscribe(&etj_strafeQualityColor, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, color);
  });

  cvarUpdate->subscribe(&etj_strafeQualitySize,
                        [this](const vmCvar_t *cvar) { setSize(*cvar); });

  consoleCommands->subscribe(
      "resetStrafeQuality",
      [this](const std::vector<std::string> &) { resetStrafeQuality(); });

  playerEvents->subscribe("respawn", [this](const std::vector<std::string> &) {
    resetStrafeQuality();
  });
}

void StrafeQualityV2::setSize(const vmCvar_t &cvar) {
  size =
      CvarValueParser::parse<CvarValue::Size>(cvar, SQ_SIZE_MIN, SQ_SIZE_MAX);
  size.x *= 0.1f;
  size.y *= 0.1f;
}

void StrafeQualityV2::resetStrafeQuality() {
  totalFrames = 0;
  score = 0;
  oldSpeed = 0;
}

float StrafeQualityV2::updateOptAngle(const PmoveUtilsV2::State &s) {
  const float num = s.wishspeed - s.a;
  return num >= s.vf ? 0 : std::acos(num / s.vf);
}

void StrafeQualityV2::updateStrafeQuality(const PmoveUtilsV2::State &s) {
  if (s.vf < s.wishspeed) {
    // possibly good frame under ground speed if speed increased
    // note that without speed increased you could go forward in
    // a "ps.speed - 1" angle endlessly because of velocity snapping
    if (s.vf > oldSpeed) {
      // good frame if no upmove and either only forwardmove or only rightmove
      if (s.pm.cmd.upmove == 0 &&
          ((s.pm.cmd.forwardmove != 0 && s.pm.cmd.rightmove == 0) ||
           (s.pm.cmd.forwardmove == 0 && s.pm.cmd.rightmove != 0))) {
        score++;
      }
      // otherwise only half as good because not optimal
      else {
        score += 0.5;
      }
    }
  } else {
    // good frame above ground speed if no upmove and in main accel zone
    if (s.pm.cmd.upmove == 0 &&
        snaphudData->inMainAccelZone(s.wishvel, s.wishspeed, s.velAngle,
                                     optAngle, s.pm)) {
      score++;
    }
    // or if speed increased, half as good because not optimal
    else if (s.vf > oldSpeed) {
      score += 0.5;
    }
  }

  totalFrames++;
  strafeQuality = 100 * (score / static_cast<double>(totalFrames));
  oldSpeed = s.vf;
}

bool StrafeQualityV2::beforeRender() {
  const PmoveUtilsV2::State &s = cgame.utils.pmoveV2->getState();

  // because we don't calculate the state if strafe quality drawing is disabled,
  // we get a null playerstate here
  if (!s.pm.ps) {
    return false;
  }

  // reset on team switch
  if (team != s.pm.ps->persistant[PERS_TEAM]) {
    team = static_cast<team_t>(s.pm.ps->persistant[PERS_TEAM]);
    resetStrafeQuality();
  }

  if (canSkipDraw()) {
    return false;
  }

  if (PmoveUtilsV2::skipUpdate(lastUpdateTime, HUDLerpFlags::STRAFE_QUALITY,
                               s.pm)) {
    return true;
  }

  x = std::clamp(POS_X + etj_strafeQualityX.value, 0.0f, 640.0f);
  ETJump_AdjustPosition(&x);
  y = std::clamp(POS_Y + etj_strafeQualityY.value, 0.0f,
                 static_cast<float>(SCREEN_HEIGHT));

  // TODO: A/B testing, remove
  y += 15;

  textStyle = etj_strafeQualityShadow.integer ? ITEM_TEXTSTYLE_SHADOWED
                                              : ITEM_TEXTSTYLE_NORMAL;
  style = static_cast<Style>(etj_strafeQualityStyle.integer);

  // return true so we still draw
  if (canSkipUpdate(s)) {
    return true;
  }

  optAngle = updateOptAngle(s);
  updateStrafeQuality(s);

  return true;
}

void StrafeQualityV2::render() const {
  // we only want 5 chars max (100.0, 99.99, 9.999 etc)
  const std::string strafeQualityStr =
      std::to_string(strafeQuality).substr(0, 5);
  std::string s;

  switch (style) {
    case Style::FULL:
    default:
      s = "Strafe Quality: " + strafeQualityStr + "%";
      break;
    case Style::PERCENT:
      s = strafeQualityStr + "%";
      break;
    case Style::NUMBER:
      s = strafeQualityStr;
      break;
  }

  CG_Text_Paint_Ext(x, y, size.x, size.y, color, s, 0, 0, textStyle,
                    &cgs.media.limboFont1);
}

bool StrafeQualityV2::canSkipUpdate(const PmoveUtilsV2::State &s) {
  // not strafing
  if (!s.pm.cmd.forwardmove && !s.pm.cmd.rightmove) {
    return true;
  }

  // only air or slick movement is important
  if (s.pm.ps->groundEntityNum != ENTITYNUM_NONE &&
      !(s.pml.groundTrace.surfaceFlags & SURF_SLICK)) {
    return true;
  }

  if (s.pm.ps->pm_type == PM_NOCLIP || s.pm.ps->pm_type == PM_DEAD) {
    return true;
  }

  if (BG_PlayerMounted(s.pm.ps->eFlags) ||
      s.pm.ps->weapon == WP_MOBILE_MG42_SET ||
      s.pm.ps->weapon == WP_MORTAR_SET) {
    return true;
  }

  if (s.pm.waterlevel > 1 || s.pml.ladder) {
    return true;
  }

  return false;
}

bool StrafeQualityV2::canSkipDraw() const {
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
