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

/*
 * Crosshair stats HUD inspired by DeFRaG one.
 */

#include <algorithm>

#include "etj_chs_drawable.h"
#include "cg_local.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_utilities.h"

// TODO: make font size configurable via cvars
inline constexpr float CHSCHAR_SIZEX = 0.2f;
inline constexpr float CHSCHAR_SIZEY = 0.2f;

namespace ETJump {
CHS::CHS(const std::shared_ptr<CvarUpdateHandler> &cvarUpdateHandler,
         const std::shared_ptr<CHSDataHandler> &dataHandler)
    : cvarUpdateHandler(cvarUpdateHandler), data(dataHandler) {
  setupListeners();

  setColor(&etj_CHSColor);
  setAlpha(&etj_CHSAlpha);

  // chs 1 does not currently have any positional adjustments,
  // so we can just set it up here
  chs1.x = SCREEN_CENTER_X - 1;
  chs1.y = SCREEN_CENTER_Y - 1 +
           (static_cast<float>(CG_Text_Height_Ext("0", CHSCHAR_SIZEY, 0,
                                                  &cgs.media.limboFont1)) /
            2);
}

CHS::~CHS() {
  cvarUpdateHandler->unsubscribe(&etj_CHSColor);
  cvarUpdateHandler->unsubscribe(&etj_CHSAlpha);
}

void CHS::setupListeners() {
  cvarUpdateHandler->subscribe(
      &etj_CHSColor, [this](const vmCvar_t *cvar) { setColor(cvar); });

  // TODO: remove this cvar and just use 'etj_CHSColor' for alpha as well
  cvarUpdateHandler->subscribe(
      &etj_CHSAlpha, [this](const vmCvar_t *cvar) { setAlpha(cvar); });
}

void CHS::setColor(const vmCvar_t *cvar) {
  cgame.utils.colorParser->parseColorString(cvar->string, color);
}

void CHS::setAlpha(const vmCvar_t *cvar) {
  color[3] = std::clamp(cvar->value, 0.0f, 1.0f);
}

void CHS::setupListCHS(CHSHUD &chs, float x, float y, const bool rightAlign,
                       const bool hideLabels) {
  const float offsetX = ETJump_AdjustPosition(x);

  chs.y = SCREEN_CENTER_Y + 40 + y;

  // just reusing item alignment constants, the values don't really matter
  if (rightAlign) {
    chs.align = ITEM_ALIGN_RIGHT;
    chs.x = SCREEN_WIDTH - 30 + offsetX;
  } else {
    chs.align = ITEM_ALIGN_LEFT;
    chs.x = 30 + offsetX;
  }

  chs.hideLabels = hideLabels;
}

bool CHS::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  if (etj_drawCHS2.integer) {
    setupListCHS(chs2, etj_CHS2PosX.value, etj_CHS2PosY.value,
                 etj_drawCHS2.integer == 2, etj_CHS2HideLabels.integer);
  }

  if (etj_drawCHS3.integer) {
    setupListCHS(chs3, etj_CHS3PosX.value, etj_CHS3PosY.value,
                 etj_drawCHS3.integer == 2, etj_CHS3HideLabels.integer);
  }

  textStyle =
      etj_CHSShadow.integer ? ITEM_TEXTSTYLE_SHADOWED : ITEM_TEXTSTYLE_NORMAL;

  return true;
}

void CHS::render() const {
  if (etj_drawCHS1.integer) {
    drawCHSCrosshair(chs1, data->getCvars(CHS_HUD_1));
  }

  if (etj_drawCHS2.integer) {
    drawCHSList(chs2, data->getCvars(CHS_HUD_2));
  }

  if (etj_drawCHS3.integer) {
    drawCHSList(chs3, data->getCvars(CHS_HUD_3));
  }
}

void CHS::drawCHSCrosshair(
    const CHSHUD &hud,
    const std::array<CHSDataHandler::CHSCvar, MAX_CHS_INFO> &cvars) const {
  for (size_t i = 0; i < cvars.size(); i++) {
    if (!cvars[i].valid) {
      continue;
    }

    const auto &pos = CHS1Positions[i];
    const std::string text = data->getStat(cvars[i].cvar);
    const float distScale =
        std::clamp(etj_CHS1DistanceScale.value, 0.25f, 10.0f);

    switch (pos.align) {
      case ITEM_ALIGN_LEFT:
        CG_Text_Paint_Ext(hud.x + (pos.offsetX * distScale),
                          hud.y + (pos.offsetY * distScale), CHSCHAR_SIZEX,
                          CHSCHAR_SIZEY, color, text, 0, 0, textStyle,
                          &cgs.media.limboFont1);
        break;
      case ITEM_ALIGN_RIGHT:
        CG_Text_Paint_RightAligned_Ext(
            hud.x + (pos.offsetX * distScale),
            hud.y + (pos.offsetY * distScale), CHSCHAR_SIZEX, CHSCHAR_SIZEY,
            color, text, 0, 0, textStyle, &cgs.media.limboFont1);
        break;
      default:
        CG_Text_Paint_Centred_Ext(hud.x + (pos.offsetX * distScale),
                                  hud.y + (pos.offsetY * distScale),
                                  CHSCHAR_SIZEX, CHSCHAR_SIZEY, color, text, 0,
                                  0, textStyle, &cgs.media.limboFont1);
    }
  }
}

void CHS::drawCHSList(
    const CHSHUD &hud,
    const std::array<CHSDataHandler::CHSCvar, MAX_CHS_INFO> &cvars) const {
  for (size_t i = 0; i < cvars.size(); i++) {
    if (!cvars[i].valid) {
      continue;
    }

    std::string text;

    if (!hud.hideLabels) {
      text = data->getStatName(cvars[i].cvar) + ": ";
    }

    text += data->getStat(cvars[i].cvar);

    if (hud.align == ITEM_ALIGN_RIGHT) {
      CG_Text_Paint_RightAligned_Ext(
          hud.x, hud.y + (static_cast<float>(i) * 10), CHSCHAR_SIZEX,
          CHSCHAR_SIZEY, color, text, 0, 0, textStyle, &cgs.media.limboFont1);
    } else {
      CG_Text_Paint_Ext(hud.x, hud.y + (static_cast<float>(i) * 10),
                        CHSCHAR_SIZEX, CHSCHAR_SIZEY, color, text, 0, 0,
                        textStyle, &cgs.media.limboFont1);
    }
  }
}

bool CHS::canSkipDraw() {
  if (!etj_drawCHS1.integer && !etj_drawCHS2.integer && !etj_drawCHS3.integer) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
