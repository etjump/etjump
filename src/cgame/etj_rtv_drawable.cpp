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

#include <array>

#include "etj_rtv_drawable.h"
#include "cg_local.h"
#include "etj_client_rtv_handler.h"
#include "etj_utilities.h"

#include "../game/etj_string_utilities.h"

namespace ETJump {
std::vector<panel_button_t> rtvButtonsLayout;
std::array<const char *, 10> rtvMenuStrings = {
    "", "", "", "", "", "", "", "", "", "Keep current map"};

RtvDrawable::RtvDrawable() { setupPanelButtons(); }

void RtvDrawable::setupPanelButtons() {
  static panel_button_text_t rtvTitleFont = {
      0.19f, 0.19f, {0.6f, 0.6f, 0.6f, 1.f}, 0, 0, &cgs.media.limboFont1_lo,
  };

  static panel_button_text_t rtvFont = {
      0.2f,
      0.2f,
      {0.6f, 0.6f, 0.6f, 1.f},
      ITEM_TEXTSTYLE_SHADOWED,
      0,
      &cgs.media.limboFont2,
  };

  static panel_button_t rtvTopBorder = {
      nullptr,
      "",
      {10, 129, 204, 136},
      {1, static_cast<int>(255 * .5f), static_cast<int>(255 * .5f),
       static_cast<int>(255 * .5f), static_cast<int>(255 * .5f), 1, 0, 0},
      nullptr, // font
      nullptr, // keyDown
      nullptr, // keyUp
      BG_PanelButtonsRender_Img,
      nullptr,
  };

  static panel_button_t rtvTopBorderBack = {
      "white",
      "",
      {11, 130, 202, 134},
      {1, 0, 0, 0, static_cast<int>(255 * 0.75f), 0, 0, 0},
      nullptr, // font
      nullptr, // keyDown
      nullptr, // keyUp
      BG_PanelButtonsRender_Img,
      nullptr,
  };

  static panel_button_t rtvTopBorderInner = {
      "white",
      "",
      {12, 131, 200, 12},
      {1, 41, 51, 43, 204, 0, 0, 0},
      nullptr, // font
      nullptr, // keyDown
      nullptr, // keyUp
      BG_PanelButtonsRender_Img,
      nullptr,
  };

  static panel_button_t rtvTopBorderInnerText = {
      nullptr,           "",      {15, 141, 200, 12}, {0, 0, 0, 0, 0, 0, 0, 0},
      &rtvTitleFont, // font
      nullptr,       // keyDown
      nullptr,       // keyUp
      drawMenuTitleText, nullptr,
  };

  static panel_button_t rtvMenuItemText = {
      nullptr,      "",      {16, 153, 128, 12}, {0, 0, 0, 0, 0, 0, 0, 0},
      &rtvFont, // font
      nullptr,  // keyDown
      nullptr,  // keyUp
      drawMenuText, nullptr,
  };

  static panel_button_t *rtvButtons[] = {
      &rtvTopBorderBack,      &rtvTopBorder,    &rtvTopBorderInner,
      &rtvTopBorderInnerText, &rtvMenuItemText, nullptr};

  rtvButtonsLayout.clear();

  for (auto buttonPtr : rtvButtons) {
    if (buttonPtr) {
      rtvButtonsLayout.push_back(*buttonPtr);
    }
  }

  BG_PanelButtonsSetup(rtvButtonsLayout);
}

void RtvDrawable::drawMenuTitleText(panel_button_t *button) {
  int sec = (VOTE_TIME - (cg.time - cgs.voteTime)) / 1000;
  if (sec < 0) {
    sec = 0;
  }

  std::string str = stringFormat("VOTE FOR A MAP (%i)", sec);

  CG_Text_Paint_Ext(
      button->rect.x, button->rect.y + static_cast<float>(button->data[0]),
      button->font->scalex, button->font->scaley, button->font->colour, str, 0,
      0, button->font->style, button->font->font);
}

void RtvDrawable::drawMenuText(panel_button_t *button) {
  float y = button->rect.y;
  const auto arrSize = static_cast<int>(rtvMenuStrings.size());
  auto rtvMaps = cgame.handlers.rtv->getRtvMaps();

  for (int i = 0; i < arrSize; i++) {
    // skip empty entries
    if (rtvMenuStrings[i][0] == '\0') {
      y += button->rect.h;
      continue;
    }

    std::string str = stringFormat("%i. %s", (i + 1) % 10, rtvMenuStrings[i]);

    // last entry is current map, so we need 'No' vote count for that
    if (i == arrSize - 1) {
      str += stringFormat(": %i(%i)", cgs.voteNo, cgs.voteNoSpectators);
    } else {
      str +=
          stringFormat(": %i(%i)", cgame.handlers.rtv->getTotalVotesForMap(i),
                       (*rtvMaps)[i].voteCountInfo.spectatorCount);
    }

    CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                      button->font->scaley, button->font->colour, str, 0, 0,
                      button->font->style, button->font->font);

    y += button->rect.h;
  }
}

void RtvDrawable::keyHandling(int key, bool down) {
  if (!down) {
    return;
  }

  checkExecKey(key, qtrue);
}

qboolean RtvDrawable::checkExecKey(int key, qboolean doAction) {
  if (key == K_ESCAPE) {
    return qtrue;
  }

  if (key >= '0' && key <= '9') {
    // this corresponds to the actual menu item number,
    // so 0 = 'Keep current map'
    int selection = key - '0';

    if (selection >
            static_cast<int>(cgame.handlers.rtv->getRtvMaps()->size()) &&
        selection != 0) {
      return qfalse;
    }

    // we don't want to set event handling back to CGAME_EVENT_NONE here
    // so that the menu stays open, as user is likely interested to see
    // how the vote is going
    if (doAction) {
      if (selection == 0) {
        trap_SendClientCommand("vote no");
      } else {
        trap_SendClientCommand(va("rtvVote %i", selection));
      }
    }

    return qtrue;
  }

  return qfalse;
}

bool RtvDrawable::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  auto rtvMaps = cgame.handlers.rtv->getRtvMaps();
  std::fill_n(rtvMenuStrings.begin(), rtvMenuStrings.size() - 1, "");

  for (size_t i = 0; i < rtvMaps->size(); i++) {
    rtvMenuStrings[i] = (*rtvMaps)[i].mapName.c_str();
  }

  return true;
}

void RtvDrawable::render() const { BG_PanelButtonsRender(rtvButtonsLayout); }

bool RtvDrawable::canSkipDraw() {
  if (cgs.eventHandling != CGAME_EVENT_RTV) {
    return true;
  }

  if (CG_ConfigString(CS_VOTE_YES) == nullptr) {
    return true;
  }

  if (!cgame.handlers.rtv->rtvVoteActive()) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
