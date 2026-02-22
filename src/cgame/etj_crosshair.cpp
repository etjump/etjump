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

#include "etj_crosshair.h"
#include "etj_crosshair_drawer.h"
#include "etj_local.h"
#include "etj_utilities.h"
#include "etj_cvar_parser.h"

namespace ETJump {
Crosshair::Crosshair() {
  startListeners();
  parseColors();
  adjustPosition();
}

void Crosshair::startListeners() {
  // colors
  cgame.handlers.cvarUpdate->subscribe(
      &cg_crosshairColor, [&](const vmCvar_t *cvar) { parseColors(); });
  cgame.handlers.cvarUpdate->subscribe(
      &cg_crosshairColorAlt, [&](const vmCvar_t *cvar) { parseColors(); });

  // position
  cgame.handlers.cvarUpdate->subscribe(
      &cg_crosshairX, [&](const vmCvar_t *cvar) { adjustPosition(); });
  cgame.handlers.cvarUpdate->subscribe(
      &cg_crosshairY, [&](const vmCvar_t *cvar) { adjustPosition(); });
}

void Crosshair::parseColors() {
  cgame.utils.colorParser->parseColorString(cg_crosshairColor.string,
                                            crosshair.color);
  cgame.utils.colorParser->parseColorString(cg_crosshairColorAlt.string,
                                            crosshair.colorAlt);
}

void Crosshair::adjustSize() {
  const auto size =
      CvarValueParser::parse<CvarValue::Size>(cg_crosshairSize, -256, 256);
  crosshair.w = size.x;
  crosshair.h = size.y;
  crosshair.f = 0.0f;
  crosshair.t = std::clamp(etj_crosshairThickness.value, 0.0f, 5.0f);

  if (crosshair.current < 10) {
    // using abs makes sure negative scale will flip correctly
    crosshair.w = std::abs(crosshair.w);
    crosshair.h = std::abs(crosshair.h);
  } else {
    // most other crosshairs are "half-height" crosshairs, so we halve the size
    // to make them work better with cg_crosshairSize
    crosshair.w *= 0.5f;
    crosshair.h *= 0.5f;
  }

  if (cg_crosshairPulse.integer) {
    crosshair.f = static_cast<float>(cg.snap->ps.aimSpreadScale) / 255.0f;
    crosshair.w *= 1 + crosshair.f * 2.0f;
    crosshair.h = crosshair.w;
  }
}

void Crosshair::adjustPosition() {
  crosshair.x = cg_crosshairX.value + (SCREEN_WIDTH * 0.5f);
  crosshair.y = cg_crosshairY.value + (SCREEN_HEIGHT * 0.5f);
}

bool Crosshair::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  crosshair.current = cg_drawCrosshair.integer % NUM_CROSSHAIRS;
  adjustSize();

  // crosshair health must be checked here instead of parseColors
  // to make sure we have a valid snapshot
  if (cg_crosshairHealth.integer) {
    CG_ColorForHealth(crosshair.color);
  }

  // alpha adjustment here to make it work with crosshair health too
  crosshair.color[3] = std::clamp(cg_crosshairAlpha.value, 0.0f, 1.0f);
  crosshair.colorAlt[3] = std::clamp(cg_crosshairAlphaAlt.value, 0.0f, 1.0f);

  return true;
}

void Crosshair::render() const {
  const qhandle_t shader = cgs.media.crosshairShader[crosshair.current];
  const qhandle_t shaderAlt = cg.crosshairShaderAlt[crosshair.current];

  // standard crosshairs (0-9)
  if (crosshair.current < 10) {
    CrosshairDrawer::drawShader(crosshair, shader);

    if (cg.crosshairShaderAlt[crosshair.current]) {
      CrosshairDrawer::drawShader(crosshair, shaderAlt);
    }
  } else {
    // because engine always wants to draw up->down/left->right even when
    // width/height is negative, we need to do some manual x/y coordinate
    // shifting for some crosshairs to ensure negative x/y scale will flip
    // the crosshair correctly instead of shifting its position
    const bool flipX = crosshair.w < 0;
    const bool flipY = crosshair.h < 0;
    const bool outline = etj_crosshairOutline.integer;
    const bool fill = cg_crosshairAlphaAlt.value != 0;
    auto crosshairOutline = crosshair;

    if (outline) {
      crosshairOutline.t += 1;
      Vector4Copy(colorBlack, crosshairOutline.color);
      Vector4Copy(colorBlack, crosshairOutline.colorAlt);
    }

    switch (static_cast<ETJumpCrosshairs>(crosshair.current)) {
      case ETJumpCrosshairs::VerticalLine:
        if (outline) {
          CrosshairDrawer::drawLineOutline(crosshairOutline, shader, flipY);
        }
        CrosshairDrawer::drawLine(crosshair, shader, flipY);
        break;
      case ETJumpCrosshairs::Cross:
        if (outline) {
          CrosshairDrawer::drawCrossOutline(crosshairOutline, shader);
        }
        CrosshairDrawer::drawCross(crosshair, shader);
        break;
      case ETJumpCrosshairs::DiagonalCross:
        if (outline) {
          CrosshairDrawer::drawDiagCross(crosshairOutline);
        }
        CrosshairDrawer::drawDiagCross(crosshair);
        break;
      case ETJumpCrosshairs::V:
        if (outline) {
          CrosshairDrawer::drawV(crosshairOutline);
        }
        CrosshairDrawer::drawV(crosshair);
        break;
      case ETJumpCrosshairs::Triangle:
        if (outline) {
          CrosshairDrawer::drawTriangle(crosshairOutline, false);
        }
        CrosshairDrawer::drawTriangle(crosshair, fill);
        break;
      case ETJumpCrosshairs::T:
        if (outline) {
          CrosshairDrawer::drawTOutline(crosshairOutline, shader, flipX, flipY);
        }
        CrosshairDrawer::drawT(crosshair, shader, flipX, flipY);
        break;
      case ETJumpCrosshairs::TwoVerticalLines:
        if (outline) {
          CrosshairDrawer::drawTwoLinesOutline(crosshairOutline, shader);
        }
        CrosshairDrawer::drawTwoLines(crosshair, shader);
        break;
    }
  }
}

bool Crosshair::canSkipDraw() {
  if (cg_drawCrosshair.integer < 0) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  if (cg.snap->ps.leanf != 0) {
    return true;
  }

  if (cg.renderingThirdPerson) {
    return true;
  }

  if (!cg.editingSpeakers) {
    if (cg.zoomedBinoc && cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR) {
      return true;
    }

    if (BG_IsScopedWeapon(weapnumForClient())) {
      return true;
    }
  }

  if (cg.predictedPlayerState.weapon == WP_MORTAR_SET &&
      cg.predictedPlayerState.weaponstate != WEAPON_RAISING) {
    return true;
  }

  // pretty sure these hints are bogus but keeping it here just in case
  if (cg.snap->ps.serverCursorHint >= HINT_EXIT &&
      cg.snap->ps.serverCursorHint <= HINT_NOEXIT) {
    return true;
  }

  return false;
}
} // namespace ETJump
