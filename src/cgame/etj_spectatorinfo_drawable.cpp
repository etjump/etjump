/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
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
  setTextSize(etj_spectatorInfoScale);
  setRowHeight();
  setTextStyle(etj_spectatorInfoShadow);
}

void SpectatorInfo::startListeners() {
  cvarUpdateHandler->subscribe(&etj_spectatorInfoScale,
                               [this](const vmCvar_t *cvar) {
                                 setTextSize(*cvar);
                                 setRowHeight();
                               });

  cvarUpdateHandler->subscribe(
      &etj_spectatorInfoShadow,
      [this](const vmCvar_t *cvar) { setTextStyle(*cvar); });
}

void SpectatorInfo::setTextSize(const vmCvar_t &cvar) {
  scale = CvarValueParser::parse<CvarValue::Scale>(cvar, 0, 5);
  sizeX = 0.23f * scale.x;
  sizeY = 0.23f * scale.y;
}

void SpectatorInfo::setRowHeight() {
  // for consistent line spacing, use pre-defined string for height calculation
  rowHeight = static_cast<float>(
                  CG_Text_Height_Ext("Yy", sizeY, 0, &cgs.media.limboFont2)) *
              1.75f;
}

void SpectatorInfo::setTextStyle(const vmCvar_t &cvar) {
  textStyle = cvar.integer ? ITEM_TEXTSTYLE_SHADOWED : ITEM_TEXTSTYLE_NORMAL;
}

float SpectatorInfo::getTextOffset(const char *name, const float fontWidth) {
  switch (etj_drawSpectatorInfo.integer) {
    case 2: // center align
      return static_cast<float>(
                 CG_Text_Width_Ext(name, fontWidth, 0, &cgs.media.limboFont2)) *
             0.5f;
    case 3: // right align
      return static_cast<float>(
          CG_Text_Width_Ext(name, fontWidth, 0, &cgs.media.limboFont2));
    default: // left align
      return 0;
  }
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

  spectators.clear();
  spectators.reserve(cg.numScores);

  for (int32_t i = 0; i < cg.numScores; i++) {
    const int32_t clientNum = cg.scores[i].client;

    if (cg.snap->ps.clientNum == clientNum) {
      continue;
    }

    if (cgs.clientinfo[clientNum].team != TEAM_SPECTATOR) {
      continue;
    }

    if (cg.scores[i].followedClient == cg.snap->ps.clientNum) {
      spectators.emplace_back(clientNum,
                              cgs.clientinfo[clientNum].clientIsInactive);
    }
  }

  // sort so that active clients are at the beginning, so we can easily
  // prioritize them while drawing the list
  std::sort(spectators.begin(), spectators.end(),
            [](const auto &a, const auto &b) { return a.second < b.second; });

  return true;
}

void SpectatorInfo::render() const {
  float x = etj_spectatorInfoX.value;
  float y = etj_spectatorInfoY.value;
  ETJump_AdjustPosition(&x);

  const auto drawRow = [this, x, &y](const char *name, const vec4_t color) {
    const float offset = getTextOffset(name, sizeX);
    y += rowHeight;

    DrawString(x - offset, y, sizeX, sizeY, color, qfalse, name, 0, textStyle);
  };

  const size_t max =
      etj_spectatorInfoMaxClients.integer < 0
          ? spectators.size()
          : std::min(static_cast<size_t>(etj_spectatorInfoMaxClients.integer),
                     spectators.size());

  // simple case, draw all spectators
  if (spectators.size() <= max) {
    for (const auto &client : spectators) {
      drawRow(cgs.clientinfo[client.first].name,
              client.second ? inactiveColor : colorWhite);
    }
  } else {
    for (size_t i = 0; i < max; i++) {
      drawRow(cgs.clientinfo[spectators[i].first].name,
              spectators[i].second ? inactiveColor : colorWhite);
    }

    const auto remaining = static_cast<int>(spectators.size() - max);
    drawRow(
        va("(%s hidden)", getPluralizedString(remaining, "spectator").c_str()),
        colorMdGrey);
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
