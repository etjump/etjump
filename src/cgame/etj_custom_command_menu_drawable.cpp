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

#include "etj_custom_command_menu_drawable.h"
#include "etj_custom_command_menu.h"
#include "etj_client_commands_handler.h"

namespace ETJump {
inline constexpr int32_t NUM_MENU_ITEMS = 10;
inline constexpr int32_t MENU_NEXT = 9;
inline constexpr int32_t MENU_NEXT_KEY = 0;
inline constexpr int32_t MENU_PREV = 8;
inline constexpr int32_t MENU_PREV_KEY = 9;

static panel_button_text_t commandMenuTitleFont = {
    0.19f, 0.19f, {0.6f, 0.6f, 0.6f, 1.f}, 0, 0, &cgs.media.limboFont1_lo,
};

static panel_button_text_t commandMenuFont = {
    0.2f,
    0.2f,
    {0.6f, 0.6f, 0.6f, 1.f},
    ITEM_TEXTSTYLE_SHADOWED,
    0,
    &cgs.media.limboFont2,
};

static panel_button_t commandMenuTopBorder = {
    nullptr,
    "",
    {10, 129, 204, 136},
    // set color, r, g, b, a, draw rect
    {1, static_cast<int32_t>(255 * 0.5f), static_cast<int32_t>(255 * 0.5f),
     static_cast<int32_t>(255 * 0.5f), static_cast<int32_t>(255 * 0.5f), 1, 0,
     0},
    nullptr,
    nullptr,
    nullptr,
    BG_PanelButtonsRender_Img,
    nullptr,
};

static panel_button_t commandMenuTopBorderBack = {
    "white",
    "",
    {11, 130, 202, 134},
    // set color, r, g, b, a
    {1, 0, 0, 0, static_cast<int32_t>(255 * 0.75f), 0, 0, 0},
    nullptr,
    nullptr,
    nullptr,
    BG_PanelButtonsRender_Img,
    nullptr,
};

static panel_button_t commandMenuTopBorderInner = {
    "white",
    "",
    {12, 131, 200, 12},
    // set color, r, g, b, a
    {1, 41, 51, 43, 204, 0, 0, 0},
    nullptr,
    nullptr,
    nullptr,
    BG_PanelButtonsRender_Img,
    nullptr,
};

static panel_button_t commandMenuTopBorderInnerText = {
    nullptr,
    "",
    {15, 141, 200, 12},
    {0, 0, 0, 0, 0, 0, 0, 0},
    &commandMenuTitleFont,
    nullptr,
    nullptr,
    CustomCommandMenuDrawable::commandMenuTitleDraw,
    nullptr,
};

static panel_button_t commandMenuItemText = {
    nullptr,
    "",
    {16, 153, 128, 12},
    {0, 0, 0, 0, 0, 0, 0, 0},
    &commandMenuFont,
    nullptr,
    nullptr,
    CustomCommandMenuDrawable::commandMenuTextDraw,
    nullptr,
};

static std::vector<panel_button_t> commandMenuPanels;
int8_t CustomCommandMenuDrawable::currentPage = 0;

CustomCommandMenuDrawable::CustomCommandMenuDrawable() {
  setupListeners();
  setupPanels();
}

void CustomCommandMenuDrawable::setupListeners() {
  consoleCommandsHandler->subscribe(
      "openCustomCommandMenu",
      [](const std::vector<std::string> &) { openMenu(); });
}

void CustomCommandMenuDrawable::openMenu() {
  if (cg.demoPlayback) {
    return;
  }

  trap_UI_Popup(UIMENU_NONE);

  if (cg.showCustomCommandMenu) {
    CG_EventHandling(CGAME_EVENT_NONE, qfalse);
  } else {
    CG_EventHandling(CGAME_EVENT_CUSTOM_COMMAND, qfalse);

    if (!etj_ccMenu_rememberPage.integer) {
      currentPage = 0;
    }
  }
}

void CustomCommandMenuDrawable::setupPanels() {
  commandMenuPanels.push_back(commandMenuTopBorderBack);
  commandMenuPanels.push_back(commandMenuTopBorder);
  commandMenuPanels.push_back(commandMenuTopBorderInner);
  commandMenuPanels.push_back(commandMenuTopBorderInnerText);
  commandMenuPanels.push_back(commandMenuItemText);

  BG_PanelButtonsSetup(commandMenuPanels);
}

void CustomCommandMenuDrawable::commandMenuTitleDraw(panel_button_t *button) {
  const auto &commands = customCommandMenu->getCustomCommands();
  std::string title = "CUSTOM COMMANDS";

  if (!commands.empty()) {
    title += stringFormat(" (PAGE %i OF %i)", currentPage + 1,
                          static_cast<int32_t>(commands.size()));
  }

  CG_Text_Paint_Ext(button->rect.x, button->rect.y, button->font->scalex,
                    button->font->scaley, button->font->colour, title, 0, 0,
                    button->font->style, button->font->font);
}

void CustomCommandMenuDrawable::commandMenuTextDraw(panel_button_t *button) {
  float y = button->rect.y;
  const auto &commands = customCommandMenu->getCustomCommands();

  if (commands.empty()) {
    CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                      button->font->scaley, button->font->colour,
                      "No commands saved!", 0, 0, button->font->style,
                      button->font->font);

    y += button->rect.h;
    CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                      button->font->scaley, button->font->colour,
                      "Use 'addCustomCommand'", 0, 0, button->font->style,
                      button->font->font);

    y += button->rect.h;
    CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                      button->font->scaley, button->font->colour,
                      "to add a new command.", 0, 0, button->font->style,
                      button->font->font);
    return;
  }

  // this can happen if all commands are deleted from the current page
  if (commands.find(currentPage) == commands.end()) {
    currentPage = static_cast<int8_t>(commands.size() - 1);
  }

  for (int32_t i = 0; i < NUM_MENU_ITEMS; i++) {
    if (i >= static_cast<int32_t>(commands.at(currentPage).size()) &&
        i < MENU_PREV) {
      y += button->rect.h;
      continue;
    }

    std::string s = stringFormat("%i. ", (i + 1) % NUM_MENU_ITEMS);

    switch (i) {
      case MENU_PREV:
        s += "Previous page";
        break;
      case MENU_NEXT:
        s += "Next page";
        break;
      default:
        s += commands.at(currentPage)[i].name;
        break;
    }

    CG_Text_Paint_Ext(button->rect.x, y, button->font->scalex,
                      button->font->scaley, button->font->colour, s, 0, 0,
                      button->font->style, button->font->font);

    y += button->rect.h;
  }
}

void CustomCommandMenuDrawable::keyHandling(const int32_t key,
                                            const bool down) {
  if (!down) {
    return;
  }

  checkExecKey(key, qtrue);
}

qboolean CustomCommandMenuDrawable::checkExecKey(const int32_t key,
                                                 const qboolean doAction) {
  if (key == K_ESCAPE) {
    return qtrue;
  }

  if (key < '0' || key > '9') {
    return qfalse;
  }

  // this corresponds to the actual menu item number, not 0-indexed selection
  int32_t realKey = key - '0';
  const auto &commands = customCommandMenu->getCustomCommands();

  if (commands.empty()) {
    return qfalse;
  }

  // we don't need to care about going out of bounds here as we already
  // checked it before entering here, just check if this is a valid selection
  if (realKey > static_cast<int32_t>(commands.at(currentPage).size()) &&
      realKey < MENU_PREV_KEY) {
    return qfalse;
  }

  if (!doAction) {
    return qtrue;
  }

  switch (realKey) {
    case MENU_PREV_KEY:
      if (currentPage == 0) {
        currentPage = static_cast<int8_t>(commands.size() - 1);
      } else {
        currentPage--;
      }
      break;
    case MENU_NEXT_KEY:
      if (currentPage == static_cast<int8_t>(commands.size() - 1)) {
        currentPage = 0;
      } else {
        currentPage++;
      }
      break;
    default:
      trap_SendConsoleCommand(
          commands.at(currentPage)[realKey - 1].command.c_str());
      CG_EventHandling(CGAME_EVENT_NONE, qfalse);
      break;
  }

  return qtrue;
}

bool CustomCommandMenuDrawable::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  return true;
}

void CustomCommandMenuDrawable::render() const {
  BG_PanelButtonsRender(commandMenuPanels);
}

bool CustomCommandMenuDrawable::canSkipDraw() {
  if (cgs.eventHandling != CGAME_EVENT_CUSTOM_COMMAND) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
