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

#include "ui_local.h"
#include "etj_local.h"

#include "../game/etj_file.h"

#include "../../assets/ui/changelog/version_headers.h"

namespace ETJump {
static void initExtensionSystem() {
  ui.syscallExt = std::make_unique<SyscallExt>();
  ui.syscallExt->setupExtensions();
}

/*
 * TL;DR - every client is shit and pretends to be something it's not.
 *
 * ET: Legacy sends 3 args to UI_INIT, but 2.60b only sends 1.
 * The idea is that ET: Legacy wants to nag 2.60b users to download ET: Legacy
 * client if legacy mod is loaded with the vanilla client.
 * arg1 is a boolean (is this ET: Legacy client?) and arg2 is an integer
 * representation of current version (for various compatibility checks).
 *
 * ETe also sends 3 args, and fakes being an ET: Legacy client (though for UI,
 * I'm not sure what good this does as it doesn't get around the nagging).
 * What this means for us is that we can't rely on checking the arg1/2
 * to detect if a client is ET: Legacy or not, because ETe pretends
 * that it's ET: Legacy too.
 *
 * Solution? Check the 'version' string that each client sends, right?
 * Right...? Yeah lol, no.
 *
 * We CAN differentiate ETe from 2.60b using the version string, vanilla client
 * is either 'ET 2.60b' or 'ET 2.60d', and ETe is 'ET 2.60e'.
 * So this means we can just check for the version string first,
 * try to match that to ETe, and then fallback to arg1/arg2 for ET: Legacy,
 * and if neither match = vanilla client, right? Nope.
 *
 * The VM_Call argument parsing function in vanilla is not ideal.
 * For every VM_Call, it reads memory as if you're sending the max amount of
 * supported varags. This means that it will *very* likely read and send
 * garbage memory as arg1 and arg2 to UI_Init, which means it can accidentally
 * identify as an ET: Legacy client, because arg1 is treated as boolean.
 *
 * Okay, so just read the version string then and forget arg1/2 completely?
 * Yeah nope, ET: Legacy needs to be special.
 *
 * For some reason it sends a faked 'version' string, pretending
 * to be 2.60b client, presumably to maintain compatibility with some mods.
 * Which is a bit odd as ETPro doesn't run anyway, and no other mod has a
 * client version check that prevents you from playing them on custom clients.
 *
 * There is a saving grace though: ET: Legacy also sends a special 'etVersion'
 * string, along with the regular 'version' string. So this means, we can
 * identify the client by doing the following:
 *
 * 1. parse 'etVersion' string
 * 2. if 'etVersion string is empty, we can parse the 'version' string safely
 * to differentiate between vanilla client and ETe
 * 3. if 'etVersion' string is not empty, we can parse arg1/2 to grab
 * ET: Legacy client version.
 */

static void detectClientEngine(int legacyClient, int clientVersion) {
  char etVersionStr[MAX_CVAR_VALUE_STRING]; // ET: Legacy exclusive
  trap_Cvar_VariableStringBuffer("etVersion", etVersionStr,
                                 sizeof(etVersionStr));

  if (etVersionStr[0] == '\0') {
    char versionStr[MAX_CVAR_VALUE_STRING];
    trap_Cvar_VariableStringBuffer("version", versionStr, sizeof(versionStr));

    // we can use this length for every detection
    const auto len = static_cast<int>(strlen("ET 2.60b"));

    if (!Q_stricmpn(versionStr, "ET 2.60b", len) ||
        !Q_stricmpn(versionStr, "ET 2.60d", len)) {
      uiInfo.vetClient = true;
    } else if (!Q_stricmpn(versionStr, "ET 2.60e", len)) {
      uiInfo.eteClient = true;
    }
  } else {
    MOD_CHECK_ETLEGACY(legacyClient, clientVersion, uiInfo.etLegacyClient);
  }
}

static void initColorPicker() {
  ui.colorPicker = std::make_unique<ColorPicker>();

  uiInfo.uiDC.updateSliderState = [p = ui.colorPicker.get()](itemDef_t *item) {
    p->updateSliderState(item);
  };

  uiInfo.uiDC.cvarToColorPickerState =
      [p = ui.colorPicker.get()](const std::string &cvar) {
        p->cvarToColorPickerState(cvar);
      };

  uiInfo.uiDC.resetColorPickerState = [p = ui.colorPicker.get()] {
    p->resetColorPickerState();
  };

  uiInfo.uiDC.colorPickerDragFunc =
      [p = ui.colorPicker.get()](itemDef_t *item, const float cursorX,
                                 const float cursorY, const int key) {
        p->colorPickerDragFunc(item, cursorX, cursorY, key);
      };

  uiInfo.uiDC.toggleRGBSliderValues = [p = ui.colorPicker.get()] {
    p->toggleRGBSliderValues();
  };

  uiInfo.uiDC.RGBSlidersAreNormalized = [p = ui.colorPicker.get()] {
    return p->RGBSlidersAreNormalized();
  };

  uiInfo.uiDC.getColorSliderString = &ColorPicker::getColorSliderString;
  uiInfo.uiDC.setColorSliderType = &ColorPicker::setColorSliderType;
  uiInfo.uiDC.getColorSliderValue = &ColorPicker::getColorSliderValue;
  uiInfo.uiDC.setColorSliderValue = &ColorPicker::setColorSliderValue;
}

static void initDemoQueueHandler() {
  ui.demoQueue = std::make_unique<DemoQueue>();
}

static void parseChangelogs() {
  // the "cvar" names are the filenames, excluding .txt extension
  const std::vector<std::string> files =
      StringUtil::split(CHANGELOG_CVARS, "|");
  const std::string path = "ui/changelog/";

  for (const auto &file : files) {
    try {
      File fIn(path + file + ".txt");
      const auto contents = fIn.read();
      uiInfo.changelogs[file] = std::string(contents.begin(), contents.end());
    } catch (...) {
      Com_Printf(S_COLOR_RED
                 "%s: failed to open changelog '%s.txt' for reading.\n",
                 __func__, file.c_str());
    }
  }
}

// NOTE: this must be called after 'UI_LoadMenus'!
void initQuickConnect() {
  assert(Menu_Count() > 0);

  ui.quickConnect = std::make_unique<QuickConnect>();

  uiInfo.uiDC.quickConnectListIsFull = [p = ui.quickConnect.get()] {
    return p->isFull();
  };
}

void init(const int32_t legacyClient, const int32_t clientVersion) {
  initExtensionSystem();

  detectClientEngine(legacyClient, clientVersion);

  initColorPicker();
  initDemoQueueHandler();

  parseChangelogs();
}

void shutdown() {
  if (etj_demoQueueCurrent.string[0] != '\0' && uiInfo.demoPlayback &&
      !ui.demoQueue->manualSkip) {
    trap_Cmd_ExecuteText(EXEC_APPEND, "demoQueue next\n");
  }

  ui.quickConnect = nullptr;
  ui.demoQueue = nullptr;
  ui.colorPicker = nullptr;
  ui.syscallExt = nullptr;
}
} // namespace ETJump
