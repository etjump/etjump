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
#include "etj_local.h"
#include "etj_utilities.h"

// TODO: make font size configurable via cvars
inline constexpr float CHSCHAR_SIZEX = 0.2f;
inline constexpr float CHSCHAR_SIZEY = 0.2f;

namespace ETJump {
CHS::CHS(const std::shared_ptr<CvarUpdateHandler> &cvarUpdateHandler,
         const std::shared_ptr<CHSDataHandler> &dataHandler)
    : x1(SCREEN_CENTER_X - 1),
      y1(SCREEN_CENTER_Y - 1 +
         (static_cast<float>(CG_Text_Height_Ext("0", CHSCHAR_SIZEY, 0,
                                                &cgs.media.limboFont1)) /
          2)),
      cvarUpdateHandler(cvarUpdateHandler), data(dataHandler) {
  setupListeners();

  setColor(&etj_CHSColor);
  setAlpha(&etj_CHSAlpha);
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

bool CHS::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  if (etj_drawCHS2.integer) {
    const float offsetX = ETJump_AdjustPosition(etj_CHS2PosX.value);

    y2 = SCREEN_CENTER_Y + 40 + etj_CHS2PosY.value;

    // just reusing item alignment constants, the values don't really matter
    if (etj_drawCHS2.integer == 2) {
      CHS2Align = ITEM_ALIGN_RIGHT;
      x2 = SCREEN_WIDTH - 30 + offsetX;
    } else {
      CHS2Align = ITEM_ALIGN_LEFT;
      x2 = 30 + offsetX;
    }
  }

  textStyle =
      etj_CHSShadow.integer ? ITEM_TEXTSTYLE_SHADOWED : ITEM_TEXTSTYLE_NORMAL;

  return true;
}

void CHS::render() const {
  if (etj_drawCHS1.integer) {
    drawCHS1();
  }

  if (etj_drawCHS2.integer) {
    drawCHS2();
  }
}

void CHS::drawCHS1() const {
  const auto cvars = data->getCHS1Cvars();

  for (size_t i = 0; i < cvars.size(); i++) {
    if (!cvars[i].valid) {
      continue;
    }

    const auto &pos = CHS1Positions[i];
    const std::string text = data->getStat(cvars[i].cvar);

    switch (pos.align) {
      case ITEM_ALIGN_LEFT:
        CG_Text_Paint_Ext(x1 + pos.offsetX, y1 + pos.offsetY, CHSCHAR_SIZEX,
                          CHSCHAR_SIZEY, color, text, 0, 0, textStyle,
                          &cgs.media.limboFont1);
        break;
      case ITEM_ALIGN_RIGHT:
        CG_Text_Paint_RightAligned_Ext(
            x1 + pos.offsetX, y1 + pos.offsetY, CHSCHAR_SIZEX, CHSCHAR_SIZEY,
            color, text, 0, 0, textStyle, &cgs.media.limboFont1);
        break;
      default:
        CG_Text_Paint_Centred_Ext(x1 + pos.offsetX, y1 + pos.offsetY,
                                  CHSCHAR_SIZEX, CHSCHAR_SIZEY, color, text, 0,
                                  0, textStyle, &cgs.media.limboFont1);
    }
  }
}

void CHS::drawCHS2() const {
  const auto cvars = data->getCHS2Cvars();

  for (size_t i = 0; i < cvars.size(); i++) {
    if (!cvars[i].valid) {
      continue;
    }

    const std::string text =
        data->getStatName(cvars[i].cvar) + ": " + data->getStat(cvars[i].cvar);

    if (CHS2Align == ITEM_ALIGN_RIGHT) {
      CG_Text_Paint_RightAligned_Ext(x2, y2 + (static_cast<float>(i) * 10),
                                     CHSCHAR_SIZEX, CHSCHAR_SIZEY, color, text,
                                     0, 0, textStyle, &cgs.media.limboFont1);
    } else {
      CG_Text_Paint_Ext(x2, y2 + (static_cast<float>(i) * 10), CHSCHAR_SIZEX,
                        CHSCHAR_SIZEY, color, text, 0, 0, textStyle,
                        &cgs.media.limboFont1);
    }
  }
}

bool CHS::canSkipDraw() {
  if (!etj_drawCHS1.integer && !etj_drawCHS2.integer) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
