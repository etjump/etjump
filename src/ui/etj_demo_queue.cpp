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

#include "ui_local.h"
#include "etj_demo_queue.h"

#include "../game/etj_filesystem.h"

namespace ETJump {
inline constexpr char startHelp[] =
    "Starts playback from the directory specified by ^3'etj_demoQueueDir'^7.";
inline constexpr char stopHelp[] =
    "Stops demo queue playback.\n\nDue to communication limitations with the "
    "engine, it's necessary to issue this command to prevent the next demo "
    "from playing when hitting ESC while the queue playback is active";
inline constexpr char nextHelp[] = "Skip to next demo in queue.";
inline constexpr char prevHelp[] = "Go back to previous demo in queue.";
inline constexpr char restartHelp[] = "Restarts the queue from beginning.";
inline constexpr char gotoHelp[] =
    "Skips to a specified demo number in the queue.";
inline constexpr char statusHelp[] = "Prints the current status of demo queue.";
inline constexpr char helpHelp[] =
    "Prints general usage information or details about specific command.";

DemoQueue::DemoQueue() {
  initCommands();
  initProtocol();
}

void DemoQueue::initCommands() {
  commands = {
      DemoQueueCommand{"start", startHelp, [this] { start(); }},
      DemoQueueCommand{"stop", stopHelp, [] { stop(); }},
      DemoQueueCommand{"next", nextHelp, [this] { gotoDemo({"next"}); }},
      DemoQueueCommand{"previous", prevHelp,
                       [this] { gotoDemo({"previous"}); }},
      DemoQueueCommand{"restart", restartHelp, [this] { restart(); }},
      DemoQueueCommand{"goto", gotoHelp,
                       [this] {
                         // this is a bit silly but allows for less duplication
                         // and better error messages
                         if (trap_Argc() > 2) {
                           char buf[MAX_TOKEN_CHARS]{};
                           trap_Argv(2, buf, sizeof(buf));
                           gotoDemo({"goto", buf});
                         } else {
                           gotoDemo({"goto"});
                         }
                       }},
      DemoQueueCommand{"status", statusHelp, [this] { printStatus(); }},
      DemoQueueCommand{"help", helpHelp, [this] {
                         if (trap_Argc() < 3) {
                           printHelp("");
                         } else {
                           char buf[MAX_TOKEN_CHARS]{};
                           trap_Argv(2, buf, sizeof(buf));
                           printHelp(isValidCommand(buf) ? buf : "");
                         }
                       }}};
}

void DemoQueue::initProtocol() {
  char buf[MAX_CVAR_VALUE_STRING]{};
  trap_Cvar_VariableStringBuffer("protocol", buf, sizeof(buf));

  if (buf[0] == '\0') {
    uiInfo.uiDC.Print(S_COLOR_YELLOW "WARNING: unable to parse protocol for "
                                     "demo queue handler, assuming '84'\n");
    demoExt = "dm_84";
  } else {
    demoExt = "dm_" + std::string(buf);
  }
}

void DemoQueue::initQueue() {
  queue.clear();
  queue = FileSystem::getFileList(
      "demos/" + std::string(etj_demoQueueDir.string), demoExt, true);
}

void DemoQueue::playDemo() {
  // make sure the cvars are up to date
  trap_Cvar_Update(&etj_demoQueueCurrent);
  trap_Cvar_Update(&etj_demoQueueDir);

  trap_Cmd_ExecuteText(EXEC_APPEND,
                       va("demo \"%s/%s\"\n", etj_demoQueueDir.string,
                          etj_demoQueueCurrent.string));
}

void DemoQueue::commandHandler() {
  const int argc = trap_Argc();

  if (argc == 1) {
    printHelp("");
    return;
  }

  char buf[MAX_TOKEN_CHARS]{};
  trap_Argv(1, buf, sizeof(buf));
  const auto cmd = std::string(buf);

  if (!isValidCommand(cmd)) {
    uiInfo.uiDC.Print("Invalid demo queue command ^3'%s'^7.\n", cmd.c_str());
    printHelp("");
    return;
  }

  // always initialize the queue here, to ensure we have proper queue setup
  // if the user performs 'ui_restart' in the middle of demo playback,
  // or adds/deletes demos to/from the queue directory
  initQueue();

  for (const auto &command : commands) {
    if (command.cmdString == cmd) {
      command.callback();
      break;
    }
  }
}

void DemoQueue::setManualSkip() {
  // default to false if no arguments are given
  // (this can happen if user types in the command manually)
  if (trap_Argc() < 2) {
    manualSkip = false;
  }

  char buf[MAX_TOKEN_CHARS]{};
  trap_Argv(1, buf, sizeof(buf));

  try {
    manualSkip = std::stoi(buf);
  } catch (const std::logic_error &) {
    manualSkip = false;
  }
}

void DemoQueue::start() const {
  if (playbackActive()) {
    uiInfo.uiDC.Print("Already playing back demo queue. Use ^3'demoQueue "
                      "restart' ^7to restart playback from the beginning.\n");
    return;
  }

  if (!canStartPlayback()) {
    return;
  }

  trap_Cvar_Set("etj_demoQueueCurrent", queue.front().c_str());
  playDemo();
}

void DemoQueue::stop() {
  if (!uiInfo.demoPlayback) {
    uiInfo.uiDC.Print("Demo queue playback is not active.\n");
    return;
  }

  trap_Cvar_Set("etj_demoQueueCurrent", "");
  trap_Cvar_Update(&etj_demoQueueCurrent);
  trap_Cmd_ExecuteText(EXEC_APPEND, "disconnect\n");
}

void DemoQueue::restart() const {
  if (!playbackActive()) {
    return;
  }

  if (!canStartPlayback()) {
    return;
  }

  trap_Cvar_Set("etj_demoQueueCurrent", queue.front().c_str());
  playDemo();
}

void DemoQueue::gotoDemo(const std::vector<std::string> &args) {
  if (!playbackActive() || args.empty()) {
    return;
  }

  if (args[0] == "goto" && args.size() == 1) {
    uiInfo.uiDC.Print("^3'demoQueue goto' ^7requires an argument.\n");
    return;
  }

  if (queue.empty()) {
    initQueue();
  }

  int offset;
  const int currentIndex = getCurrentDemoIndex();

  if (args[0] == "goto") {
    try {
      // 1-indexing for user friendliness
      offset = std::stoi(args[1]) - currentIndex - 1;
    } catch (const std::invalid_argument &) {
      uiInfo.uiDC.Print("'%s' is not a number.\n", args[1].c_str());
      disableManualSkip();
      return;
    } catch (const std::out_of_range &) {
      uiInfo.uiDC.Print("'%s' is out of range.\n", args[1].c_str());
      disableManualSkip();
      return;
    }
  } else {
    offset = args[0] == "next" ? 1 : -1;
  }

  const int newIndex = currentIndex + offset;

  if (args[0] == "goto") {
    if (newIndex < 0 || newIndex >= static_cast<int>(queue.size())) {
      uiInfo.uiDC.Print("Specified index it out of range. There are currently "
                        "^3%i ^7demos in queue\n",
                        static_cast<int>(queue.size()));
      disableManualSkip();
      return;
    }
  } else {
    if (newIndex < 0) {
      uiInfo.uiDC.Print("Current demo is the first demo in the queue.\n");
      disableManualSkip();
      return;
    }

    // unfortunately we can't show error message for manually issued 'next'
    // command at the end of the queue, as the command which sets 'manualSkip'
    // to true arrives after 'demoQueue next', so it's never true here
    if (newIndex >= static_cast<int>(queue.size())) {
      trap_Cvar_Set("etj_demoQueueCurrent", "");
      return;
    }
  }

  trap_Cvar_Set("etj_demoQueueCurrent", queue[newIndex].c_str());
  playDemo();
}

void DemoQueue::printStatus() {
  if (!playbackActive()) {
    return;
  }

  const std::string demoDir = etj_demoQueueDir.string[0] == '\0'
                                  ? ""
                                  : "/" + std::string(etj_demoQueueDir.string);
  uiInfo.uiDC.Print("Playing back demo ^3%i ^7of ^3%i ^7from "
                    "^3'demos%s'\n^7Currently queued demos:\n\n",
                    getCurrentDemoIndex() + 1, static_cast<int>(queue.size()),
                    demoDir.c_str());

  const std::string current = etj_demoQueueCurrent.string;
  std::string msg;

  for (int i = 0; i < static_cast<int>(queue.size()); i++) {
    msg +=
        stringFormat("%-4i%s %s\n", i + 1, current == queue[i] ? "^z>^7" : " ",
                     sanitize(queue[i]));
  }

  // this can be quite lengthy with lots of demos in queue, so do splits
  const auto splits = wrapWords(msg, '\n', 998);
  for (const auto &split : splits) {
    uiInfo.uiDC.Print("%s", split.c_str());
  }
}

void DemoQueue::printHelp(const std::string &cmd) const {
  if (cmd.empty()) {
    std::string validCmds;

    for (const auto &command : commands) {
      validCmds += "  " + command.cmdString + "\n";
    }

    uiInfo.uiDC.Print(
        "demoQueue\n\nUsage:\ndemoQueue "
        "[command]\n\nDescription:\nAutomatically play back all demos from a "
        "subdirectory specified by ^3'etj_demoQueueDir'^7.\nLeave the cvar "
        "empty to play directly from ^3'demos' ^7directory.\n\nNote that "
        "performing a ^3'vid_restart' ^7while the playback is active\nwill "
        "break the queue playback due to communication limitations with the "
        "engine.\n\nAvailable commands:\n\n%s\n\nUse ^3'help [command]'^7 to "
        "get more information about a specific command.\n",
        validCmds.c_str());
  } else {
    for (const auto &command : commands) {
      if (command.cmdString == cmd) {
        uiInfo.uiDC.Print("%s\n", command.helpText.c_str());
        break;
      }
    }
  }
}

int DemoQueue::getCurrentDemoIndex() {
  if (queue.empty()) {
    initQueue();
  }

  // this can happen if a cvar_restart is performed while in demo playback
  if (etj_demoQueueCurrent.string[0] == '\0') {
    return 0;
  }

  const auto it =
      std::find(queue.cbegin(), queue.cend(), etj_demoQueueCurrent.string);

  if (it == queue.cend()) {
    return 0;
  }

  return static_cast<int>(std::distance(queue.cbegin(), it));
}

bool DemoQueue::isValidCommand(const std::string &cmd) const {
  return std::any_of(commands.cbegin(), commands.cend(),
                     [&](const DemoQueueCommand &command) {
                       return cmd == command.cmdString;
                     });
}

bool DemoQueue::canStartPlayback() const {
  if (queue.empty()) {
    const std::string demoDir =
        etj_demoQueueDir.string[0] == '\0'
            ? ""
            : "/" + std::string(etj_demoQueueDir.string);
    uiInfo.uiDC.Print("No demos found. Make ^3'etj_demoQueueDir' ^7points to "
                      "the correct directory.\n",
                      demoDir.c_str());
    return false;
  }

  return true;
}

bool DemoQueue::playbackActive() {
  if (etj_demoQueueCurrent.string[0] == '\0') {
    uiInfo.uiDC.Print("Demo queue playback is not active.\n");
    return false;
  }

  return true;
}

/*
 * The reason this function exists at all (and we don't just set
 * 'manualSkip' to false inside the functions) is that we catch some of the
 * demoQueue commands in cgame in order to skip the automatic 'next' command
 * issued in UI shutdown. Because console commands are handled in cgame first,
 * this allows us to differentiate manual 'next' from automatic 'next', and
 * we can inform the UI that the user manually specified a demo to goto next.
 *
 * However, the original demoQueue command will arrive in UI first, before the
 * command that disables automatic playback at shutdown, which means that we
 * cannot just manually set 'manualSkip' to false in some scenarios
 * (e.g. 'goto' with invalid index), because it will be immediately overwritten
 * by the command sent afterward, as we are still executing the original command
 *
 * demoQueue [cmd] -> uiDemoQueueManualSkip 1
 * ^^^^^^^^^^^^^^^
 * if we set 'manualSkip' to false here, it's just going to be overwritten
 *
 * Instead, we need to send another console command, which will override
 * the playback state set by the previous command.
 * This results in following command chain:
 *
 * demoQueue [cmd] -> uiDemoQueueManualSkip 1 -> uiDemoQueueManualSkip 0
 *
 * It's not beautiful by any means, but this is what we have to do.
 */
void DemoQueue::disableManualSkip() {
  trap_Cmd_ExecuteText(EXEC_APPEND, "uiDemoQueueManualSkip 0\n");
}
} // namespace ETJump
