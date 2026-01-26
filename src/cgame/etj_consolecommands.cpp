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

#include "etj_consolecommands.h"
#include "etj_client_commands_handler.h"
#include "etj_utilities.h"

#include "../game/etj_string_utilities.h"

namespace ETJump::ConsoleCommands {
static void ftSaveLimitSet() {
  char buffer[MAX_CVAR_VALUE_STRING]{};
  trap_Cvar_VariableStringBuffer("etj_ftSaveLimit", buffer, sizeof(buffer));
  trap_SendConsoleCommand(va("fireteam rules savelimit %i\n", Q_atoi(buffer)));
}

static void forceMaplistRefresh() { cg.maplistRequested = false; }

static void forceCustomvoteRefresh() { resetCustomvoteInfo(); }

static void uiRequestCustomvotes() { cg.customvoteInfoRequested = true; }

static void uiChatMenuOpen(const Arguments &args) {
  if (args.empty()) {
    return;
  }

  cg.chatMenuOpen = Q_atoi(args[0]);
}

/*
 * cgame handles console commands before UI, so we catch some of the demo
 * queue commands here in order to inform UI that the command was
 * manually executed by the user. This avoids sending automatic
 * 'next' command on UI shutdown, as we can inform the UI that the command
 * was executed manually. On normal queue behavior, UI sends 'next' command
 * on shutdown, but because cgame isn't loaded at that point, it's ignored.
 */
static void demoQueue(const Arguments &args) {
  if (args.empty()) {
    return;
  }

  if (StringUtil::iEqual(args[0], "next") ||
      StringUtil::iEqual(args[0], "previous") ||
      StringUtil::iEqual(args[0], "restart") ||
      StringUtil::iEqual(args[0], "goto")) {
    trap_SendConsoleCommand("uiDemoQueueManualSkip 1\n");
  }
}

static bool fireteam(const Arguments &args) {
  if (args.empty()) {
    return true;
  }

  // 'fireteam' commands are normally handled by the server,
  // but if we're using 'fireteam countdown', catch it here and make sure
  // the duration is sent with the command if it's not manually specified
  if (!StringUtil::iEqual(args[0], "countdown")) {
    return true;
  }

  if (args.size() == 1) {
    const int32_t sec = etj_fireteamCountdownLength.integer > 0
                            ? etj_fireteamCountdownLength.integer
                            : 3;
    trap_SendClientCommand(va("fireteam countdown %i", sec));
  } else {
    trap_SendClientCommand(va("fireteam countdown %i", Q_atoi(args[1])));
  }

  return false;
}

/*
 * This handles console commands that are in some way handled on cgame,
 * but we also want to potentially forward to either UI or qagame. We can't use
 * the regular 'consoleCommandsHandler' for this at the moment, because
 * 'consoleCommandsHandler->check()' always returns true if the command exists,
 * since we need some commands to "fall through" cgame console command handling,
 * so that they are handled by UI or qagame.
 * Returns true if the command should be forwarded.
 */
bool forwardedConsoleCommand(const std::string_view cmd,
                             const Arguments &args) {
  if (StringUtil::iEqual(cmd, "demoQueue")) {
    // this should always return true, regardless if we actually modify
    // anything, as we always want UI to also handle this command
    demoQueue(args);
    return true;
  }

  if (StringUtil::iEqual(cmd, "fireteam")) {
    return fireteam(args);
  }

  return true;
}

void registerCommands() {
  consoleCommandsHandler->subscribe(
      "ftSaveLimitSet", [](const auto &) { ftSaveLimitSet(); }, false);

  consoleCommandsHandler->subscribe(
      "forceMaplistRefresh", [](const auto &) { forceMaplistRefresh(); },
      false);

  consoleCommandsHandler->subscribe(
      "forceCustomvoteRefresh", [](const auto &) { forceCustomvoteRefresh(); },
      false);

  consoleCommandsHandler->subscribe(
      "uiRequestCustomvotes", [](const auto &) { uiRequestCustomvotes(); },
      false);

  consoleCommandsHandler->subscribe(
      "uiChatMenuOpen", [](const auto &args) { uiChatMenuOpen(args); }, false);
}
} // namespace ETJump::ConsoleCommands
