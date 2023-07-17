/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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

namespace ETJump {
void SpectatorInfo::beforeRender() {
  // poll for updates every 3 seconds to refresh list
  if (cg.time > cg.lastScoreTime + 3000) {
    trap_SendClientCommand("score");
    cg.lastScoreTime = cg.time;
  }
}

void SpectatorInfo::render() const {
  if (canSkipDraw()) {
    return;
  }

  float x = etj_spectatorInfoX.value;
  float y = etj_spectatorInfoY.value;
  const float size = 0.1f * etj_spectatorInfoSize.value;
  float w;
  const char *spectator;
  const int textStyle = etj_spectatorInfoShadow.integer
                            ? ITEM_TEXTSTYLE_SHADOWED
                            : ITEM_TEXTSTYLE_NORMAL;

  ETJump_AdjustPosition(&x);

  for (auto i = 0; i < cg.numScores; i++) {
    if (cg.snap->ps.clientNum == cg.scores[i].client) {
      // ignore self
      continue;
    }

    if (cgs.clientinfo[cg.scores[i].client].team == TEAM_SPECTATOR) {
      if (cg.scores[i].followedClient == cg.snap->ps.clientNum) {
        spectator = cgs.clientinfo[cg.scores[i].client].name;

        switch (etj_drawSpectatorInfo.integer) {
          case 2: // center align
            w = static_cast<float>(CG_Text_Width_Ext(spectator, size, 0,
                                                     &cgs.media.limboFont2)) *
                0.5f;
            break;
          case 3: // right align
            w = static_cast<float>(
                CG_Text_Width_Ext(spectator, size, 0, &cgs.media.limboFont2));
            break;
          default: // left align
            w = 0.0f;
            break;
        }

        // for consistent line height, use pre-defined string
        // for height calculation instead of current spectators name
        y += static_cast<float>(
                 CG_Text_Height_Ext("Yy", size, 0, &cgs.media.limboFont2)) *
             1.75f;

        DrawString(x - w, y, size, size, colorWhite, qfalse, spectator, 0,
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

  return false;
}
} // namespace ETJump
