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
#include "etj_demo_compatibility.h"
#include "etj_local.h"
#include "cg_local.h"
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

static void printMapCustomizationInfo() {
  if (cgame.demo.compatibility->flags.noMapCustomizationHashes) {
    CG_Printf("Map customization data is unavailable in demos recorded prior "
              "to ETJump 3.5.0.\n");
    return;
  }

  const std::string cs = CG_ConfigString(CS_ETJUMP_MAPINFO);
  const std::string mapscriptHash = Info_ValueForKey(cs.c_str(), "msh");
  const std::string entityFileHash = Info_ValueForKey(cs.c_str(), "efh");

  if (mapscriptHash.empty() || entityFileHash.empty()) {
    CG_Printf("No map customization data found in configstrings. This is a "
              "bug, please report this to the developers.\n");
    return;
  }

  if (mapscriptHash == "-" && entityFileHash == "-") {
    CG_Printf(
        "No custom mapscript or entity file loaded for the current map.\n");
    return;
  }

  CG_Printf("%-25s %s\n%-25s %s\n\nPlease note that the hashes are computed "
            "with line endings normalized to ^3LF^7.\nIf you're computing the "
            "hashes using external tools for comparison, ensure the files are "
            "saved with ^3LF ^7line endings.\n",
            "Custom mapscript hash:", mapscriptHash.c_str(),
            "Entity file hash:", entityFileHash.c_str());
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

std::optional<CommandParser::Command>
getOptCommand(const std::string &commandPrefix,
              const CommandParser::CommandDefinition &def,
              const std::vector<std::string> &args) {
  auto cmd = CommandParser(def, args).parse();

  if (cmd.helpRequested) {
    CG_AddToTeamChat(
        stringFormat("^3%s: ^7check console for help.", commandPrefix).c_str(),
        TEAM_SPECTATOR);
    const auto splits = wrapWords(def.help(), '\n', MAX_STRING_CHARS - 1);

    for (const auto &s : splits) {
      CG_Printf("%s", s.c_str());
    }

    return std::nullopt;
  }

  if (!cmd.errors.empty()) {
    CG_AddToTeamChat(
        stringFormat(
            "^3%s: ^7operation failed. Check console for more information.",
            commandPrefix)
            .c_str(),
        TEAM_SPECTATOR);
    CG_Printf("%s\n", cmd.getErrorMessage().c_str());
    return std::nullopt;
  }

  return cmd;
}

void registerCommands() {
  cgame.handlers.consoleCommands->subscribe(
      "ftSaveLimitSet", [](const auto &) { ftSaveLimitSet(); }, false);

  cgame.handlers.consoleCommands->subscribe(
      "forceMaplistRefresh", [](const auto &) { forceMaplistRefresh(); },
      false);

  cgame.handlers.consoleCommands->subscribe(
      "forceCustomvoteRefresh", [](const auto &) { forceCustomvoteRefresh(); },
      false);

  cgame.handlers.consoleCommands->subscribe(
      "uiRequestCustomvotes", [](const auto &) { uiRequestCustomvotes(); },
      false);

  cgame.handlers.consoleCommands->subscribe(
      "uiChatMenuOpen", [](const auto &args) { uiChatMenuOpen(args); }, false);

  cgame.handlers.consoleCommands->subscribe(
      "printMapCustomizationInfo",
      [](const auto &) { printMapCustomizationInfo(); });
}
} // namespace ETJump::ConsoleCommands
