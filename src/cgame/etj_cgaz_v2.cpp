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

#include "etj_cgaz_v2.h"
#include "cg_local.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_snaphud_data.h"
#include "etj_utilities.h"

namespace ETJump {
inline constexpr float CGAZ_FOV_MIN = 1.0f;
inline constexpr float CGAZ_FOV_MAX = 179.0f;

CGazV2::CGazV2(const std::shared_ptr<CGazData> &cgazData,
               const std::shared_ptr<SnaphudData> &snaphudData,
               const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : cgazData(cgazData), snaphudData(snaphudData), cvarUpdate(cvarUpdate) {

  // CGaz 1
  cgame.utils.colorParser->parseColorString(etj_CGaz1Color1.string,
                                            cgaz1.colors[0]);
  cgame.utils.colorParser->parseColorString(etj_CGaz1Color2.string,
                                            cgaz1.colors[1]);
  cgame.utils.colorParser->parseColorString(etj_CGaz1Color3.string,
                                            cgaz1.colors[2]);
  cgame.utils.colorParser->parseColorString(etj_CGaz1Color4.string,
                                            cgaz1.colors[3]);

  cgame.utils.colorParser->parseColorString(etj_CGaz1MidlineColor.string,
                                            cgaz1.midlineColor);

  // CGaz 2
  cgame.utils.colorParser->parseColorString(etj_CGaz2Color1.string,
                                            cgaz2.colors[0]);
  cgame.utils.colorParser->parseColorString(etj_CGaz2Color2.string,
                                            cgaz2.colors[1]);

  setThickness(&etj_CGaz2Thickness1);
  setThickness(&etj_CGaz2Thickness2);
  startListeners();
}

CGazV2::~CGazV2() {
  cvarUpdate->unsubscribe(&etj_CGaz1Color1);
  cvarUpdate->unsubscribe(&etj_CGaz1Color2);
  cvarUpdate->unsubscribe(&etj_CGaz1Color3);
  cvarUpdate->unsubscribe(&etj_CGaz1Color4);
  cvarUpdate->unsubscribe(&etj_CGaz1MidlineColor);

  cvarUpdate->unsubscribe(&etj_CGaz2Color1);
  cvarUpdate->unsubscribe(&etj_CGaz2Color2);
  cvarUpdate->unsubscribe(&etj_CGaz2Thickness1);
  cvarUpdate->unsubscribe(&etj_CGaz2Thickness2);
}

void CGazV2::startListeners() {
  // CGaz 1
  cvarUpdate->subscribe(&etj_CGaz1Color1, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, cgaz1.colors[0]);
  });

  cvarUpdate->subscribe(&etj_CGaz1Color2, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, cgaz1.colors[1]);
  });

  cvarUpdate->subscribe(&etj_CGaz1Color3, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, cgaz1.colors[2]);
  });

  cvarUpdate->subscribe(&etj_CGaz1Color4, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, cgaz1.colors[3]);
  });

  cvarUpdate->subscribe(&etj_CGaz1MidlineColor, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, cgaz1.midlineColor);
  });

  // CGaz 2
  cvarUpdate->subscribe(&etj_CGaz2Color1, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, cgaz2.colors[0]);
  });

  cvarUpdate->subscribe(&etj_CGaz2Color2, [this](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, cgaz2.colors[1]);
  });

  cvarUpdate->subscribe(&etj_CGaz2Thickness1,
                        [this](const vmCvar_t *cvar) { setThickness(cvar); });

  cvarUpdate->subscribe(&etj_CGaz2Thickness2,
                        [this](const vmCvar_t *cvar) { setThickness(cvar); });
}

void CGazV2::setThickness(const vmCvar_t *cvar) {
  if (cvar == &etj_CGaz2Thickness1) {
    cgaz2.thickness[0] = std::clamp(cvar->value, 0.5f, 100.0f);
  } else if (cvar == &etj_CGaz2Thickness2) {
    cgaz2.thickness[1] = std::clamp(cvar->value, 0.5f, 100.0f);
  }
}

void CGazV2::updateCGaz1(const CGazData::State &s) {
  cgaz1.minAngle = updateMinAngle(s);
  cgaz1.optAngle = updateOptAngle(s);
  cgaz1.maxCosAngle = updateMaxCosAngle(s, cgaz1.optAngle);
  cgaz1.maxAngle = updateMaxAngle(s, cgaz1.maxCosAngle);

  assert(cgaz1.minAngle <= cgaz1.optAngle);
  assert(cgaz1.optAngle <= cgaz1.maxCosAngle);
  assert(cgaz1.maxCosAngle <= cgaz1.maxAngle);

  // no need to update this unless we're drawing midline
  if (etj_CGaz1DrawMidLine.integer) {
    cgaz1.midlineStart =
        cgaz1.optAngle + ((cgaz1.maxCosAngle - cgaz1.optAngle) / 2);
    cgaz1.midLineEnd =
        cgaz1.midlineStart + (((cgaz1.optAngle - cgaz1.minAngle) +
                               (cgaz1.maxAngle - cgaz1.maxCosAngle)) /
                              2);
  }

  cgaz1.yaw = std::atan2(s.wishvel[1], s.wishvel[0]) - s.velAngle;

  cgaz1.y =
      std::clamp(etj_CGazY.value, 0.0f, static_cast<float>(SCREEN_HEIGHT));
  cgaz1.h =
      std::clamp(etj_CGazHeight.value, 0.0f, static_cast<float>(SCREEN_HEIGHT));

  cgaz1.fov = etj_CGazFov.value > 0
                  ? std::clamp(etj_CGazFov.value, CGAZ_FOV_MIN, CGAZ_FOV_MAX)
                  : cg.refdef.fov_x;
}

void CGazV2::updateCGaz2(const CGazData::State &s) {
  cgaz2.velAngle = AngleNormalize180(s.pm.ps->viewangles[YAW] -
                                     AngleNormalize180(RAD2DEG(s.velAngle)));
  cgaz2.velAngle = DEG2RAD(cgaz2.velAngle);
  cgaz2.optAngle = updateOptAngle(s);
  cgaz2.velSize = etj_CGaz2FixedSpeed.value > 0
                      ? etj_CGaz2FixedSpeed.value / 5.0f
                      : std::min(s.vf / 5.0f, SCREEN_HEIGHT / 2.0f);

  cgaz2.y =
      std::clamp(etj_CGaz2Y.value, 0.0f, static_cast<float>(SCREEN_HEIGHT));

  cgaz2.forwardmove = s.pm.cmd.forwardmove;
  cgaz2.rightmove = s.pm.cmd.rightmove;

  cgaz2.highRes = etj_CGaz2HighRes.integer;
  cgaz2.drawSides = s.vf > s.wishspeed;
}

void CGazV2::updateDrawSnap(const CGazData::State &s) {
  // no meaningful values if speed is low
  if (s.vf < s.wishspeed) {
    cgaz1.drawSnap = std::nullopt;
    return;
  }

  const bool forwards =
      PmoveUtilsV2::strafingForwards(s.pm, s.wishspeed, s.wishvel);
  const bool rightStrafe = PmoveUtilsV2::rightStrafe(forwards, s.pm.cmd);

  // convert 'cgaz1.minAngle' to an absolute world-space angle, so we can
  // figure out which snap zone quadrant we are currently at
  float minAngleAbsolute =
      rightStrafe ? s.velAngle - cgaz1.minAngle : s.velAngle + cgaz1.minAngle;

  minAngleAbsolute = AngleNormalizePI(minAngleAbsolute);

  const SnaphudData::State &snap = snaphudData->getState();

  float snapEdge = 0.0f;
  bool found = false;

  // linear search is fine here, most of the time 'snapAngles' size is 8
  // (can go up to 85 in extreme edge cases, still fine for linear search)
  for (size_t i = 0; i < snap.snapAngles.size() - 1; i++) {
    for (int32_t q = 0; q < 4; q++) {
      const float offset = static_cast<float>(q) * M_PI_2f;
      const float start = snap.snapAngles[i] + offset;
      const float end = snap.snapAngles[i + 1] + offset;

      if (AngleNormalizePI(minAngleAbsolute - start) >= 0 &&
          AngleNormalizePI(minAngleAbsolute - start) <=
              AngleNormalizePI(end - start)) {

        snapEdge = rightStrafe ? start : end;
        found = true;
        break;
      }
    }
  }

  if (found) {
    cgaz1.drawSnap = cgaz1.minAngle +
                     std::abs(AngleNormalizePI(snapEdge - minAngleAbsolute));
  } else {
    cgaz1.drawSnap = std::nullopt;
  }
}

float CGazV2::updateMinAngle(const CGazData::State &s) {
#ifndef NDEBUG
  if (s.a == 0) {
    assert(s.vSquared - s.vfSquared == (2 * s.a * s.wishspeed) - s.aSquared);
  } else {
    assert(s.vSquared - s.vfSquared < (2 * s.a * s.wishspeed) - s.aSquared);
  }
#endif

  const float numSquared =
      s.vfSquared + std::pow(s.wishspeed, 2.0f) - s.vSquared + s.gSquared;
  assert(numSquared > 0);

  const float num = std::sqrt(numSquared);
  return num >= s.vf ? 0 : std::acos(num / s.vf);
}

float CGazV2::updateOptAngle(const CGazData::State &s) {
  const float num = s.wishspeed - s.a;
  return num >= s.vf ? 0 : std::acos(num / s.vf);
}

// This needs to take into account some edge cases with low speeds on slick:
//
// - player takes fall damage and enters slick state, while holding movement
//   inputs (immediate acceleration after velocity reset)
// - player starts accelerating on slick with really low velocity
//   (e.g. with '+strafe')
//
// In these cases, the contribution from gravity to the total velocity vector
// may be higher than the contribution from player's horizontal velocity,
// which breaks down the math here, as the size of the optimal acceleration
// angle would exceed 180 degrees.
// When this happens, we simply return Pi as the angle, as there's no
// meaningful way to represent the angle on screen.
float CGazV2::updateMaxCosAngle(const CGazData::State &s,
                                const float angleOpt) {
  const float vXYSquared = s.vSquared - s.gSquared;

  // gravity has higher contribution - no meaningful angle
  if (vXYSquared < 0) {
    return M_PIf;
  }

  const float num = std::sqrt(vXYSquared) - s.vf;
  float angleMaxCos = 0.0f;

  if (num < s.a) {
    // gravity has higher contribution - no meaningful angle
    if (num <= -s.a) {
      angleMaxCos = M_PIf;
    } else {
      angleMaxCos = std::acos(num / s.a);
    }
  }

  if (angleMaxCos < angleOpt) {
    assert((s.v * s.vf) - s.vfSquared >= (s.a * s.wishspeed) - s.aSquared);
    angleMaxCos = angleOpt;
  }

  return angleMaxCos;
}

float CGazV2::updateMaxAngle(const CGazData::State &s,
                             const float angleMaxCos) {
#ifndef NDEBUG
  if (s.a == 0) {
    assert(s.vSquared - s.vfSquared == (2 * s.a * s.wishspeed) - s.aSquared);
  } else {
    assert(s.vSquared - s.vfSquared < (2 * s.a * s.wishspeed) - s.aSquared);
  }
#endif

  const float num = s.vSquared - s.vfSquared - s.aSquared - s.gSquared;
  const float den = 2 * s.a * s.vf;

  if (num >= den) {
    return 0;
  }

  if (-num >= den) {
    return M_PIf;
  }

  float angleMax = std::acos(num / den);

  if (angleMax < angleMaxCos) {
    assert(s.a == 0);
    angleMax = angleMaxCos;
  }

  return angleMax;
}

bool CGazV2::beforeRender() {
  const CGazData::State &s = cgazData->getState();

  if (canSkipDraw(s)) {
    return false;
  }

  if (PmoveUtilsV2::skipUpdate(lastUpdateTime, HUDLerpFlags::CGAZ, s.pm)) {
    return true;
  }

  if (etj_drawCGaz.integer & 1) {
    updateCGaz1(s);

    if (etj_CGaz1DrawSnapZone.integer) {
      updateDrawSnap(s);
    }
  }

  if (etj_drawCGaz.integer & 2) {
    updateCGaz2(s);
  }

  return true;
}

void CGazV2::render() const {
  if (etj_drawCGaz.integer & 1) {
    renderCGaz1();
  }

  if (etj_drawCGaz.integer & 2) {
    renderCGaz2();
  }
}

void CGazV2::renderCGaz1() const {
  if (etj_CGaz1DrawSnapZone.integer && cgaz1.drawSnap.has_value()) {
    CG_FillAngleYaw(+cgaz1.minAngle, +cgaz1.drawSnap.value(), cgaz1.yaw,
                    cgaz1.y, cgaz1.h, cgaz1.fov, cgaz1.colors[1]);
    CG_FillAngleYaw(-cgaz1.drawSnap.value(), -cgaz1.minAngle, cgaz1.yaw,
                    cgaz1.y, cgaz1.h, cgaz1.fov, cgaz1.colors[1]);
  } else {
    // no accel zone
    CG_FillAngleYaw(-cgaz1.minAngle, +cgaz1.minAngle, cgaz1.yaw, cgaz1.y,
                    cgaz1.h, cgaz1.fov, cgaz1.colors[0]);

    // partial accel zone
    CG_FillAngleYaw(+cgaz1.minAngle, +cgaz1.optAngle, cgaz1.yaw, cgaz1.y,
                    cgaz1.h, cgaz1.fov, cgaz1.colors[1]);
    CG_FillAngleYaw(-cgaz1.optAngle, -cgaz1.minAngle, cgaz1.yaw, cgaz1.y,
                    cgaz1.h, cgaz1.fov, cgaz1.colors[1]);

    // full accel zone
    CG_FillAngleYaw(+cgaz1.optAngle, +cgaz1.maxCosAngle, cgaz1.yaw, cgaz1.y,
                    cgaz1.h, cgaz1.fov, cgaz1.colors[2]);
    CG_FillAngleYaw(-cgaz1.maxCosAngle, -cgaz1.optAngle, cgaz1.yaw, cgaz1.y,
                    cgaz1.h, cgaz1.fov, cgaz1.colors[2]);

    // max angle
    CG_FillAngleYaw(+cgaz1.maxCosAngle, +cgaz1.maxAngle, cgaz1.yaw, cgaz1.y,
                    cgaz1.h, cgaz1.fov, cgaz1.colors[3]);
    CG_FillAngleYaw(-cgaz1.maxAngle, -cgaz1.maxCosAngle, cgaz1.yaw, cgaz1.y,
                    cgaz1.h, cgaz1.fov, cgaz1.colors[3]);

    if (etj_CGaz1DrawMidLine.integer) {
      CG_FillAngleYaw(+cgaz1.midlineStart, +cgaz1.midLineEnd, cgaz1.yaw,
                      cgaz1.y, cgaz1.h, cgaz1.fov, cgaz1.midlineColor);
      CG_FillAngleYaw(-cgaz1.midlineStart, -cgaz1.midLineEnd, cgaz1.yaw,
                      cgaz1.y, cgaz1.h, cgaz1.fov, cgaz1.midlineColor);
    }
  }
}

void CGazV2::renderCGaz2() const {
  float x = SCREEN_CENTER_X;

  if (etj_stretchCgaz.integer) {
    ETJump_EnableWidthScale(false);
    x -= SCREEN_OFFSET_X;
  }

  if (cgaz2.forwardmove || cgaz2.rightmove) {
    float mult = 1.0f;

    if (etj_CGaz2WishDirFixedSpeed.value > 0) {
      constexpr float wishDirScale = 2.0f * 5.0f * CMDSCALE_DEFAULT;
      mult = etj_CGaz2WishDirFixedSpeed.value / wishDirScale;
    }

    if (etj_CGaz2WishDirUniformLength.integer && cgaz2.rightmove &&
        cgaz2.forwardmove) {
      mult /= M_SQRT2;
    }

    const float fmove = mult * static_cast<float>(cgaz2.forwardmove);
    const float smove = mult * static_cast<float>(cgaz2.rightmove);

    if (cgaz2.highRes) {
      drawLineWu(x, cgaz2.y, x + smove, cgaz2.y - fmove, cgaz2.thickness[1],
                 cgaz2.colors[1]);
    } else {
      drawLineDDA(x, cgaz2.y, x + smove, cgaz2.y - fmove, cgaz2.colors[1]);
    }
  }

  // draw velocity direction if requested
  if (!etj_CGaz2NoVelocityDir.integer ||
      (!cgaz2.drawSides && etj_CGaz2NoVelocityDir.integer == 2)) {
    float dirSize = cgaz2.velSize;

    // prevent comically long velocity direction lines on fixed speeds
    if (!cgaz2.drawSides && etj_CGaz2FixedSpeed.value > 0) {
      dirSize = std::min(static_cast<float>(CMDSCALE_DEFAULT), dirSize);
    }

    const float velDirSin = dirSize * std::sin(cgaz2.velAngle);
    const float velDirCos = dirSize * std::cos(cgaz2.velAngle);

    if (cgaz2.highRes) {
      drawLineWu(x, cgaz2.y, x + velDirSin, cgaz2.y - velDirCos,
                 cgaz2.thickness[0], cgaz2.colors[0]);
    } else {
      drawLineDDA(x, cgaz2.y, x + velDirSin, cgaz2.y - velDirCos,
                  cgaz2.colors[0]);
    }
  }

  if (cgaz2.drawSides) {
    const float velAngleSinL =
        (cgaz2.velSize / 2) * std::sin(cgaz2.velAngle - cgaz2.optAngle);
    const float velAngleCosL =
        (cgaz2.velSize / 2) * std::cos(cgaz2.velAngle - cgaz2.optAngle);

    const float velAngleSinR =
        (cgaz2.velSize / 2) * std::sin(cgaz2.velAngle + cgaz2.optAngle);
    const float velAngleCosR =
        (cgaz2.velSize / 2) * std::cos(cgaz2.velAngle + cgaz2.optAngle);

    if (cgaz2.highRes) {
      drawLineWu(x, cgaz2.y, x + velAngleSinL, cgaz2.y - velAngleCosL,
                 cgaz2.thickness[0], cgaz2.colors[0]);
      drawLineWu(x, cgaz2.y, x + velAngleSinR, cgaz2.y - velAngleCosR,
                 cgaz2.thickness[0], cgaz2.colors[0]);
    } else {
      drawLineDDA(x, cgaz2.y, x + velAngleSinL, cgaz2.y - velAngleCosL,
                  cgaz2.colors[0]);
      drawLineDDA(x, cgaz2.y, x + velAngleSinR, cgaz2.y - velAngleCosR,
                  cgaz2.colors[0]);
    }
  }

  if (etj_stretchCgaz.integer) {
    ETJump_EnableWidthScale(true);
  }
}

bool CGazV2::canSkipDraw(const CGazData::State &s) {
  if (!etj_drawCGaz.integer) {
    return true;
  }

  if (s.result != PmoveUtilsV2::PmoveSingleResult::WALK &&
      s.result != PmoveUtilsV2::PmoveSingleResult::AIR) {
    return true;
  }

  if (VectorLengthSquared2(s.pm.ps->velocity) == 0) {
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
