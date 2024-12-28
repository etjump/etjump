/*
 * MIT License
 *
 * Copyright (c) 2024 ETJump team <zero@etjump.com>
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

#include "etj_spectatorinfo_drawable.h"
#include "etj_cvar_update_handler.h"

namespace ETJump {
SpectatorInfo::SpectatorInfo() {
  startListeners();
  setScale();
}

void SpectatorInfo::startListeners() {
  cvarUpdateHandler->subscribe(&etj_spectatorInfoScale,
                               [&](const vmCvar_t *) { setScale(); });
}

void SpectatorInfo::setScale() {
  scale =
      CvarValueParser::parse<CvarValue::Scale>(etj_spectatorInfoScale, 0, 5);
}

bool SpectatorInfo::beforeRender() {
  // poll for updates every 3 seconds to refresh list
  // do this before checking if we should render, just to keep it up to date
  if (cg.time > cg.lastScoreTime + 3000) {
    trap_SendClientCommand("score");
    cg.lastScoreTime = cg.time;
  }

  if (canSkipDraw()) {
    return false;
  }

  return true;
}

void SpectatorInfo::render() const {
  float x = etj_spectatorInfoX.value;
  float y = etj_spectatorInfoY.value;
  const float sizeX = 0.23f * scale.x;
  const float sizeY = 0.23f * scale.y;

  // for consistent line spacing, use pre-defined string
  // for height calculation instead of current spectators name
  const auto rowHeight = static_cast<float>(CG_Text_Height_Ext(
                             "Yy", sizeY, 0, &cgs.media.limboFont2)) *
                         1.75f;
  float w;
  const int textStyle = etj_spectatorInfoShadow.integer
                            ? ITEM_TEXTSTYLE_SHADOWED
                            : ITEM_TEXTSTYLE_NORMAL;

  constexpr vec4_t inactiveColor = {1.0f, 1.0f, 1.0f, 0.33f};

  ETJump_AdjustPosition(&x);

  for (auto i = 0; i < cg.numScores; i++) {
    if (cg.snap->ps.clientNum == cg.scores[i].client) {
      // ignore self
      continue;
    }

    if (cgs.clientinfo[cg.scores[i].client].team == TEAM_SPECTATOR) {
      if (cg.scores[i].followedClient == cg.snap->ps.clientNum) {
        const char *spectator = cgs.clientinfo[cg.scores[i].client].name;
        const bool inactive =
            cgs.clientinfo[cg.scores[i].client].clientIsInactive;

        switch (etj_drawSpectatorInfo.integer) {
          case 2: // center align
            w = static_cast<float>(CG_Text_Width_Ext(spectator, sizeX, 0,
                                                     &cgs.media.limboFont2)) *
                0.5f;
            break;
          case 3: // right align
            w = static_cast<float>(
                CG_Text_Width_Ext(spectator, sizeX, 0, &cgs.media.limboFont2));
            break;
          default: // left align
            w = 0.0f;
            break;
        }

        y += rowHeight;

        DrawString(x - w, y, sizeX, sizeY,
                   inactive ? inactiveColor : colorWhite, qfalse, spectator, 0,
                   textStyle);
      }
    }
  }
}
bool SpectatorInfo::canSkipDraw() {
  if (!etj_drawSpectatorInfo.integer) {
    return true;
  }

  if (cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
