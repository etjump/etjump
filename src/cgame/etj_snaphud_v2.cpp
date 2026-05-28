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

#include "etj_snaphud_v2.h"
#include "cg_local.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_utilities.h"

inline constexpr float SNAPHUD_MIN_FOV = 1.0f;
inline constexpr float SNAPHUD_MAX_FOV = 179.0f;

namespace ETJump {
SnaphudV2::SnaphudV2(const std::shared_ptr<SnaphudData> &snaphudData,
                     const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : snaphudData(snaphudData), cvarUpdate(cvarUpdate) {

  cgame.utils.colorParser->parseColorString(etj_snapHUDColor1.string,
                                            snaphud.colors[0]);
  cgame.utils.colorParser->parseColorString(etj_snapHUDColor2.string,
                                            snaphud.colors[1]);

  cgame.utils.colorParser->parseColorString(etj_snapHUDHLColor1.string,
                                            snaphud.colors[2]);
  cgame.utils.colorParser->parseColorString(etj_snapHUDHLColor2.string,
                                            snaphud.colors[3]);
  startListeners();
}

SnaphudV2::~SnaphudV2() {
  cvarUpdate->unsubscribe(&etj_snapHUDColor1);
  cvarUpdate->unsubscribe(&etj_snapHUDColor2);
  cvarUpdate->unsubscribe(&etj_snapHUDHLColor1);
  cvarUpdate->unsubscribe(&etj_snapHUDHLColor2);
}

void SnaphudV2::startListeners() {
  cvarUpdate->subscribe(&etj_snapHUDColor1, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, snaphud.colors[0]);
  });

  cvarUpdate->subscribe(&etj_snapHUDColor2, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, snaphud.colors[1]);
  });

  cvarUpdate->subscribe(&etj_snapHUDHLColor1, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, snaphud.colors[2]);
  });

  cvarUpdate->subscribe(&etj_snapHUDHLColor2, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, snaphud.colors[3]);
  });
}

void SnaphudV2::updateSnaphud(const SnaphudData::State &s) {
  snaphud.yaw = std::atan2(s.wishvel[1], s.wishvel[0]);
  snaphud.y = SCREEN_CENTER_Y + std::clamp(etj_snapHUDOffsetY.value,
                                           -SCREEN_CENTER_Y, SCREEN_CENTER_Y);
  snaphud.h = std::clamp(etj_snapHUDHeight.value, 0.0f,
                         static_cast<float>(SCREEN_HEIGHT));
  snaphud.fov =
      etj_snapHUDFov.value > 0
          ? std::clamp(etj_snapHUDFov.value, SNAPHUD_MIN_FOV, SNAPHUD_MAX_FOV)
          : cg.refdef.fov_x;

  // TODO: A/B testing, remove
  snaphud.y += snaphud.h + 1;

  snaphud.style = static_cast<SnaphudStyle>(etj_drawSnapHUD.integer);

  switch (snaphud.style) {
    case SnaphudStyle::EDGE:
      // FIXME: the unit of this cvar no longer really makes sense,
      // but by doing SHORT2RAD here, it behaves the same as with old hud
      snaphud.edgeThickness =
          SHORT2RAD(std::clamp(etj_snapHUDEdgeThickness.integer, 0, 128));
      break;
    case SnaphudStyle::BORDER:
      snaphud.borderOnly = true;
      snaphud.borderThickness =
          std::clamp(etj_snapHUDBorderThickness.value, 0.1f,
                     std::min(etj_snapHUDHeight.value * 2, 10.0f));
      break;
    default:
      snaphud.borderOnly = false;
      break;
  }

  buildSnapZones(s);
}

void SnaphudV2::buildSnapZones(const SnaphudData::State &s) {
  snaphud.zones.clear();
  snaphud.isCurrentAlt = false;

  for (int32_t i = 0; i < s.snapAngles.size() - 1; i++) {
    for (int32_t q = 0; q < 4; q++) {
      const float offset = static_cast<float>(q) * M_PI_2f;
      const float start = s.snapAngles[i] + offset;
      const float end = s.snapAngles[i + 1] + offset;

      const bool active = AngleNormalizePI(snaphud.yaw - start) >= 0 &&
                          (AngleNormalizePI(snaphud.yaw - start) <=
                           AngleNormalizePI(end - start));
      const bool alt = i % 2;
      snaphud.isCurrentAlt |= active && alt;

      snaphud.zones.push_back({start, end, alt, active});
    }
  }
}

bool SnaphudV2::beforeRender() {
  const SnaphudData::State &s = snaphudData->getState();

  if (canSkipDraw(s)) {
    return false;
  }

  updateSnaphud(s);

  return true;
}

void SnaphudV2::render() const {
  for (const auto &zone : snaphud.zones) {
    int8_t colorIndex = zone.alt ? 1 : 0;

    if (etj_snapHUDActiveIsPrimary.integer && snaphud.isCurrentAlt) {
      colorIndex ^= 1;
    }

    if (etj_snapHUDHLActive.integer && zone.active) {
      colorIndex += 2;
    }

    if (snaphud.style == SnaphudStyle::EDGE) {
      CG_FillAngleYaw(zone.start, zone.start + snaphud.edgeThickness,
                      snaphud.yaw, snaphud.y, snaphud.h, snaphud.fov,
                      snaphud.colors[colorIndex]);
      CG_FillAngleYaw(zone.end, zone.end - snaphud.edgeThickness, snaphud.yaw,
                      snaphud.y, snaphud.h, snaphud.fov,
                      snaphud.colors[colorIndex]);
    } else {
      CG_FillAngleYawExt(zone.start, zone.end, snaphud.yaw, snaphud.y,
                         snaphud.h, snaphud.fov, snaphud.colors[colorIndex],
                         snaphud.borderOnly, snaphud.borderThickness);
    }
  }
}

bool SnaphudV2::canSkipDraw(const SnaphudData::State &s) const {
  if (!etj_drawSnapHUD.integer) {
    return true;
  }

  if (s.result != PmoveUtilsV2::PmoveSingleResult::WALK &&
      s.result != PmoveUtilsV2::PmoveSingleResult::AIR) {
    return true;
  }

  // this will be true at the very beginning of the game, before the server
  // has ran a client think at least once, or with very low speeds
  // ('target_scale_velocity' with base scaling < 1)
  if (s.a == 0 || s.snapAngles.empty()) {
    return true;
  }

  if (s.pm.ps->persistant[PERS_TEAM] == TEAM_SPECTATOR ||
      s.pm.ps->pm_type == PM_NOCLIP || s.pm.ps->pm_type == PM_DEAD) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  if (cg.zoomedBinoc || BG_IsScopedWeapon(weapnumForClient())) {
    return true;
  }

  if (BG_PlayerMounted(s.pm.ps->eFlags) ||
      s.pm.ps->weapon == WP_MOBILE_MG42_SET ||
      s.pm.ps->weapon == WP_MORTAR_SET) {
    return true;
  }

  return false;
}
} // namespace ETJump
