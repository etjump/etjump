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

#include "etj_servercommands.h"
#include "etj_local.h"
#include "etj_utilities.h"

#include "../game/etj_syscall_ext_shared.h"
#include "../game/etj_string_utilities.h"

namespace ETJump::ServerCommands {
using Arguments = std::vector<std::string>;
// TODO: The client will always request map list after cgame is initialized,
// thus making '!listmaps' command sort of redundant. The command itself
// should probably stay (the output formatting is nice and all),
// but now that we're always receiving a full map list from server,
// we could use a local cache for map list instead of requesting it again.
static void maplist(const Arguments &args) {
  // we need to forward this command to UI to parse the list there,
  // so we can populate the map vote list
  trap_SendConsoleCommand(
      va("uiParseMaplist %s\n", StringUtil::join(args, " ").c_str()));
}

static void forceCustomvoteRefresh() { resetCustomvoteInfo(); }

static void numCustomvotes(const Arguments &args) {
  if (args.empty()) {
    return;
  }

  cg.numCustomvotes = Q_atoi(args[0]);
  // forward count to UI
  trap_SendConsoleCommand(va("uiNumCustomvotes %i\n", cg.numCustomvotes));
}

static void customvoteList(const Arguments &args) {
  // forward to UI
  trap_SendConsoleCommand(
      va("uiParseCustomvote %s\n", StringUtil::join(args, " ").c_str()));
}

static void pmFlashWindow() {
  if (etj_highlight.integer &
      static_cast<int>(ChatHighlightFlags::HIGHLIGHT_FLASH)) {
    SyscallExt::trap_SysFlashWindowETLegacy(
        SyscallExt::FlashWindowState::SDL_FLASH_UNTIL_FOCUSED);
  }
}

static void extShaderIndex(const Arguments &args) {
  for (const auto &arg : args) {
    const auto shaderKvp = StringUtil::split(arg, "|");

    // sanity check, shouldn't happen,
    // but just skip if the command is somehow malformed
    if (shaderKvp.size() != 2) {
      continue;
    }

    registerGameShader(Q_atoi(shaderKvp[0]), shaderKvp[1].c_str());
  }

  // request state once we've gotten the shaders
  trap_SendClientCommand("getExtShaderState");
}

static void extShaderState(const Arguments &args) {
  // this *should* be a single argument, but 'timeOffset' can have padding,
  // introducing whitespace to the command, which breaks it into multiple args
  // we don't need to preserve it, the parser will ignore it anyway
  CG_ShaderStateChanged(StringUtil::join(args, ""));
}

static void resetStrafeQuality() {
  // noop, just silence when this gets sent on 'save' command
}

static void savePrint(const Arguments &args) {
  std::string saveMsg = etj_saveMsg.string;

  // this happens on forced save (e.g. 'target_save')
  if (args.empty()) {
    CG_CenterPrint(saveMsg.c_str(), SCREEN_HEIGHT - SCREEN_HEIGHT * 0.2,
                   SMALLCHAR_WIDTH, false);
    return;
  }

  const int32_t pos = Q_atoi(args[0]);

  if (pos) {
    saveMsg += ' ' + std::to_string(pos);
  }

  if (args.size() >= 2) {
    const int32_t remainingSaves = Q_atoi(args[1]);
    const std::string remainingSavesStr =
        stringFormat("^7(^3%d ^7remaining)\n", remainingSaves);
    saveMsg += '\n' + remainingSavesStr;
  }

  CG_CenterPrint(saveMsg.c_str(), SCREEN_HEIGHT - SCREEN_HEIGHT * 0.2,
                 SMALLCHAR_WIDTH, false);
}

// for !rtv admin command
static void callvote(const Arguments &args) {
  if (args.empty()) {
    return;
  }

  std::string command = "callvote " + args[0];

  // !rtv [customvotelist]
  if (args.size() > 1) {
    command += " " + args[1];
  }

  trap_SendConsoleCommand(command.c_str());
}

void registerCommands() {
  cgame.handlers.serverCommands->subscribe(
      "maplist", [](const auto &args) { maplist(args); }, false);

  cgame.handlers.serverCommands->subscribe(
      "forceCustomvoteRefresh", [](const auto &) { forceCustomvoteRefresh(); },
      false);

  cgame.handlers.serverCommands->subscribe(
      "numcustomvotes", [](const auto &args) { numCustomvotes(args); }, false);

  cgame.handlers.serverCommands->subscribe(
      "customvotelist", [](const auto &args) { customvoteList(args); }, false);

  cgame.handlers.serverCommands->subscribe(
      "pmFlashWindow", [](const auto &) { pmFlashWindow(); }, false);

  cgame.handlers.serverCommands->subscribe(
      "extShaderIndex", [](const auto &args) { extShaderIndex(args); }, false);

  cgame.handlers.serverCommands->subscribe(
      "extShaderState", [](const auto &args) { extShaderState(args); }, false);

  cgame.handlers.serverCommands->subscribe(
      "resetStrafeQuality", [](const auto &) { resetStrafeQuality(); }, false);

  cgame.handlers.serverCommands->subscribe(
      "savePrint", [](const auto &args) { savePrint(args); }, false);

  cgame.handlers.serverCommands->subscribe(
      "callvote", [](const auto &args) { callvote(args); }, false);
}
} // namespace ETJump::ServerCommands
