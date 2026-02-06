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

#include "etj_utilities.h"
#include "ui_local.h"

namespace ETJump::Utilities {
void parseMaplist() {
  char arg[MAX_QPATH];

  // start iterating from 1 to skip the command string
  for (int i = 1, len = trap_Argc(); i < len; i++) {
    trap_Argv(i, arg, sizeof(arg));
    uiInfo.serverMaplist.emplace_back(arg);
  }

  ETJump::StringUtil::sortStrings(uiInfo.serverMaplist, true);
}

void parseNumCustomvotes() {
  if (trap_Argc() < 2) {
    Com_Printf(va(S_COLOR_YELLOW
                  "%s: unable to parse customvote count: no arguments given.\n",
                  __func__));
    return;
  }

  char arg[MAX_TOKEN_CHARS];
  trap_Argv(1, arg, sizeof(arg));
  uiInfo.numCustomvotes = Q_atoi(arg);
}

void parseCustomvote() {
  // if a mapsOnServer or otherMaps is empty, we only have 3 args
  static constexpr int minArgs = 3;
  const int numArgs = trap_Argc();

  if (numArgs < minArgs) {
    Com_Printf(va(S_COLOR_YELLOW "%s: unable to parse customvote: malformed "
                                 "command - too few arguments (%i < %i).\n",
                  __func__, numArgs, minArgs));
    return;
  }

  char arg[MAX_TOKEN_CHARS];
  CustomMapVotes::MapType *mapType = nullptr;

  trap_Argv(1, arg, sizeof(arg));

  // grab an existing list if we've already parsed this list before
  for (auto &customVote : uiInfo.customVotes) {
    if (customVote.type == arg) {
      mapType = &customVote;
      break;
    }
  }

  // create a new entry if we didn't find an existing list
  if (mapType == nullptr) {
    uiInfo.customVotes.emplace_back();
    mapType = &uiInfo.customVotes.back();
    mapType->type = arg;
  }

  trap_Argv(2, arg, sizeof(arg));
  const std::string field = arg;

  if (field == CUSTOMVOTE_TYPE) {
    trap_Argv(3, arg, sizeof(arg));
    mapType->type = arg;
  } else if (field == CUSTOMVOTE_CVTEXT) {
    // this can potentially be multiple args
    std::string cvtext;

    for (int i = 3; i < numArgs; i++) {
      trap_Argv(i, arg, sizeof(arg));
      cvtext += std::string(arg) + " ";
    }

    cvtext.pop_back();
    mapType->callvoteText = std::move(cvtext);
  } else if (field == CUSTOMVOTE_SERVERMAPS) {
    for (int i = 3; i < numArgs; i++) {
      trap_Argv(i, arg, sizeof(arg));
      mapType->mapsOnServer.emplace_back(arg);
    }
  } else if (field == CUSTOMVOTE_OTHERMAPS) {
    for (int i = 3; i < numArgs; i++) {
      trap_Argv(i, arg, sizeof(arg));
      mapType->otherMaps.emplace_back(arg);
    }
  }
}

void resetCustomvotes() {
  uiInfo.customVotes.clear();
  uiInfo.numCustomvotes = -1;
  uiInfo.customvoteIndex = 0;
  uiInfo.customvoteMapsOnServerIndex = 0;
  uiInfo.customvoteOtherMapsIndex = 0;

  static constexpr char DETAILS_MENU[] = "ingame_customvote_details";
  static constexpr char VOTE_MENU[] = "ingame_vote_customvote";

  const menuDef_t *detailsMenu = Menus_FindByName(DETAILS_MENU);
  const menuDef_t *voteMenu = Menus_FindByName(VOTE_MENU);

  if (detailsMenu && detailsMenu->window.flags & WINDOW_VISIBLE) {
    Menus_CloseByName(DETAILS_MENU);
  }

  if (voteMenu && voteMenu->window.flags & WINDOW_VISIBLE) {
    Menus_CloseByName(VOTE_MENU);
    Menus_OpenByName("ingame_vote");
  }
}

void toggleSettingsMenu() {
  const menuDef_t *activeMenu = Menu_GetFocused();

  if (activeMenu &&
      StringUtil::startsWith(activeMenu->window.name, "etjump_settings_")) {
    Menus_CloseAll();
  } else {
    trap_Key_SetCatcher(KEYCATCH_UI);
    Menus_CloseAll();
    Menus_OpenByName("etjump_settings_general_gameplay");
  }
}

static void markAllServersVisible() {
  const int32_t count = trap_LAN_GetServerCount(ui_netSource.integer);

  for (int i = 0; i < count; i++) {
    trap_LAN_MarkServerVisible(ui_netSource.integer, i, qtrue);
  }
}

static void keepServerListUpdating() {
  uiInfo.serverStatus.refreshActive = qtrue;
  uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
}

static void openPlayOnlineMenu() {
  Menus_CloseByName("main"); // opened by main_opener after ui reload
  Menus_ActivateByName("playonline", qtrue);

  const char *str = "clearError"; // clears com_errorMessage
  uiInfo.uiDC.runScript(&str);
}

void handleIllegalRedirect() {
  Com_Printf("^1ETJump: illegal redirect was detected, reacting...\n");

  markAllServersVisible();
  keepServerListUpdating();
  openPlayOnlineMenu();
}

void drawLevelshotPreview(const rectDef_t &rect) {
  // unregistered levelshot is -1, not 0
  if (uiInfo.mapList[ui_mapIndex.integer].levelShot == -1) {
    uiInfo.mapList[ui_mapIndex.integer].levelShot = trap_R_RegisterShaderNoMip(
        va("levelshots/%s", uiInfo.mapList[ui_mapIndex.integer].mapLoadName));

    if (!uiInfo.mapList[ui_mapIndex.integer].levelShot) {
      uiInfo.mapList[ui_mapIndex.integer].levelShot =
          trap_R_RegisterShaderNoMip("levelshots/unknownmap");
    }
  }

  uiInfo.uiDC.drawHandlePic(rect.x, rect.y, rect.w, rect.h,
                            uiInfo.mapList[ui_mapIndex.integer].levelShot);
}

void drawMapname(rectDef_t &rect, float scale, vec4_t color, float text_x,
                 int textStyle, int align) {
  rectDef_t textRect = {0, 0, rect.w, rect.h};

  const int map = ui_currentNetMap.integer;
  std::string mapname;

  if (uiInfo.mapList[map].mapLoadName != nullptr) {
    mapname = uiInfo.mapList[map].mapLoadName;
  } else {
    mapname = "unknownmap";
  }

  const auto width =
      static_cast<float>(uiInfo.uiDC.textWidth(mapname.c_str(), scale, 0));

  switch (align) {
    case ITEM_ALIGN_LEFT:
      textRect.x = text_x;
      break;
    case ITEM_ALIGN_RIGHT:
      textRect.x = text_x - width;
      break;
    case ITEM_ALIGN_CENTER:
      textRect.x = text_x - (width * 0.5f);
      break;
    default:
      break;
  }

  textRect.x += rect.x;
  textRect.y += rect.y;
  uiInfo.uiDC.drawText(textRect.x, textRect.y, scale, color, mapname.c_str(), 0,
                       0, textStyle);
}

std::vector<std::string>
fitChangelogLinesToWidth(std::vector<std::string> &lines, const int maxW,
                         const float scale, fontInfo_t *font) {
  std::vector<std::string> fmtLines;

  for (auto &line : lines) {
    int width = 0;
    size_t indent = 0;
    size_t lastWhitespace = 0;

    // do we have to split this line at all?
    if (uiInfo.uiDC.textWidthExt(line.c_str(), scale, 0, font) <= maxW) {
      fmtLines.emplace_back(line);
      continue;
    }

    // find first non-dash, non-whitespace character to get indentation
    auto textStart = std::find_if(line.begin(), line.end(), [](const char c) {
      return c != '-' && !std::isspace(c);
    });

    if (textStart != line.end()) {
      indent = std::distance(line.begin(), textStart);
    }

    std::string tmp;
    tmp.reserve(line.length());

    for (int i = 0; i < static_cast<int>(line.length()); i++) {
      tmp += line[i];

      if (std::isspace(line[i])) {
        lastWhitespace = i;
      }

      width = uiInfo.uiDC.textWidthExt(tmp.c_str(), scale, 0, font);

      if (width > maxW) {
        if (lastWhitespace != 0) {
          // we have a line with single word that is over max width,
          // just split at the line end
          if (lastWhitespace < indent) {
            fmtLines.emplace_back(tmp.substr(0, i));
            line.erase(0, i);
          } else {
            fmtLines.emplace_back(tmp.substr(0, lastWhitespace));
            line.erase(0, lastWhitespace + 1); // consume the whitespace too
          }

          lastWhitespace = 0;
        } else {
          // this should never happen, but it protects against an infinite loop
          fmtLines.emplace_back(tmp);
          line.erase(0, i);
        }

        i = -1; // so we start from 0 again after i++

        tmp.clear();
        line.insert(0, indent, ' ');
      }
    }

    fmtLines.emplace_back(line);
  }

  return fmtLines;
}
} // namespace ETJump::Utilities
