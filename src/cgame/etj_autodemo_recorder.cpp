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

#include "etj_autodemo_recorder.h"
#include "cg_local.h"
#include "etj_client_commands_handler.h"
#include "etj_demo_recorder.h"
#include "etj_player_events_handler.h"
#include "etj_utilities.h"

#include "../game/etj_filesystem.h"
#include "../game/etj_string_utilities.h"
#include "../game/etj_time_utilities.h"

namespace ETJump {
// constants
inline constexpr int DEMO_SAVE_DELAY = 500;
inline constexpr int DEMO_MAX_SAVE_DELAY = 10000;
inline constexpr int DEMO_START_TIMEOUT = 500;
inline constexpr int MAX_TEMP = 20;
inline constexpr char TEMP_PATH[] = "temp";

inline constexpr int32_t AUTODEMO_TIMERUN_ONLY = 1;
inline constexpr int32_t AUTODEMO_ALWAYS = 2;

AutoDemoRecorder::AutoDemoRecorder(
    const std::shared_ptr<PlayerEventsHandler> &playerEvents,
    const std::shared_ptr<ClientCommandsHandler> &consoleCommands)
    : playerEvents(playerEvents), consoleCommands(consoleCommands) {
  if (cg.demoPlayback) {
    return;
  }

  startListeners();
}

AutoDemoRecorder::~AutoDemoRecorder() {
  if (cg.demoPlayback) {
    return;
  }

  playerEvents->unsubscribe("load");
  playerEvents->unsubscribe("respawn");
  playerEvents->unsubscribe("timerun:completion");
  playerEvents->unsubscribe("timerun:record");

  consoleCommands->unsubscribe("ad_save");
}

void AutoDemoRecorder::startListeners() {
  playerEvents->subscribe("load", [this](const std::vector<std::string> &) {
    if (etj_autoDemo.integer) {
      tryRestart();
    }
  });

  playerEvents->subscribe(
      "respawn",
      [this](const std::vector<std::string> &args) { onRespawn(args); });

  playerEvents->subscribe("timerun:completion",
                          [this](const std::vector<std::string> &args) {
                            onTimerunEnd(args, false);
                          });

  playerEvents->subscribe("timerun:record",
                          [this](const std::vector<std::string> &args) {
                            onTimerunEnd(args, true);
                          });

  consoleCommands->subscribe(
      "ad_save",
      [this](const std::vector<std::string> &args) { onManualSave(args); });
}

void AutoDemoRecorder::onRespawn(const std::vector<std::string> &args) {
  if (etj_ad_stopInSpec.integer && DemoRecorder::recordingAutoDemo() &&
      cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR && !delayedTimerId) {
    demo.stop();
    return;
  }

  if (args.empty()) {
    return;
  }

  // don't restart if we're getting revived ('respawn 1')
  if (Q_atoi(args[0])) {
    return;
  }

  if (etj_autoDemo.integer) {
    tryRestart();
  }
}

void AutoDemoRecorder::onTimerunEnd(const std::vector<std::string> &args,
                                    const bool record) {
  if (!etj_autoDemo.integer || args.size() < 3) {
    return;
  }

  if (Q_atoi(args[0]) != cg.clientNum) {
    return;
  }

  if (!record && etj_ad_savePBOnly.integer) {
    return;
  }

  trySaveTimerunDemo(sanitize(args[1]), args[2]);
}

void AutoDemoRecorder::onManualSave(const std::vector<std::string> &args) {
  if (!etj_autoDemo.integer) {
    return;
  }

  if (!cl_demorecording.integer) {
    CG_AddPMItem(PM_MESSAGE, "^7Not recording a demo.\n",
                 cgs.media.voiceChatShader);
    return;
  }

  if (!DemoRecorder::recordingAutoDemo()) {
    CG_AddPMItem(PM_MESSAGE, "Not recording an autodemo.\n",
                 cgs.media.voiceChatShader);
    return;
  }

  const std::string src = createDemoTempPath(demoNames.current());
  const std::string dst = createDemoPath(args.empty() ? "demo" : args[0]);
  saveDemoWithRestart(src, dst);
}

std::string AutoDemoRecorder::TempNameGenerator::pop() {
  if (!names.size()) {
    return "temp";
  }

  auto name = names.front();
  names.pop();
  return name;
}

std::string AutoDemoRecorder::TempNameGenerator::next() {
  const std::string name =
      stringFormat("%s/temp_%02d", TEMP_PATH, (nameCounter++ % MAX_TEMP) + 1);
  names.push(name);
  return name;
}

std::string AutoDemoRecorder::TempNameGenerator::current() {
  return names.size() > 0 ? names.back() : "temp";
}

size_t AutoDemoRecorder::TempNameGenerator::size() { return names.size(); }

void AutoDemoRecorder::tryRestart() {
  // no autodemo for specs
  if (cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR) {
    return;
  }

  // start autodemo for timerun maps only
  if (etj_autoDemo.integer == AUTODEMO_TIMERUN_ONLY && !cg.hasTimerun) {
    return;
  }

  // don't start autodemo if timeruns are disabled,
  // unless autodemo is enabled for all maps
  if (etj_autoDemo.integer != AUTODEMO_ALWAYS && !etj_enableTimeruns.integer) {
    return;
  }

  // timeout
  if (demo.getStartTime() + DEMO_START_TIMEOUT >= cg.time) {
    return;
  }

  // dont attempt to restart if in timerun mode
  if (cgs.clientinfo[cg.clientNum].timerunActive || delayedTimerId) {
    return;
  }

  restart();
}

void AutoDemoRecorder::restart() {
  demo.restart(demoNames.next());

  if (demoNames.size() > MAX_TEMP) {
    demoNames.pop();
  }
}

void AutoDemoRecorder::trySaveTimerunDemo(const std::string &runName,
                                          const std::string &runTime) {
  if (delayedTimerId || !DemoRecorder::recordingAutoDemo()) {
    return;
  }

  const std::string src = createDemoTempPath(demoNames.current());
  const std::string dst = createTimerunDemoPath(runName, runTime);

  CG_AddPMItem(PM_MESSAGE, "^7Stopping demo...\n", cgs.media.voiceChatShader);
  saveTimerunDemo(src, dst);
}

void AutoDemoRecorder::saveTimerunDemo(const std::string &src,
                                       const std::string &dst) {
  const int delay =
      std::clamp(etj_ad_stopDelay.integer, 0, DEMO_MAX_SAVE_DELAY);

  delayedTimerId =
      setTimeout([this, src, dst] { saveDemoWithRestart(src, dst); }, delay);
}

void AutoDemoRecorder::saveDemo(const std::string &src,
                                const std::string &dst) {
  maybeCancelDelayedSave();
  setTimeout([src, dst] { FileSystem::safeCopy(src, dst); }, DEMO_SAVE_DELAY);
}

void AutoDemoRecorder::saveDemoWithRestart(const std::string &src,
                                           const std::string &dst) {
  saveDemo(src, dst);
  CG_AddPMItem(PM_MESSAGE, "^7Demo saved!\n", cgs.media.voiceChatShader);
  CG_Printf("^7Demo saved to %s\n", dst.c_str());
  restart();
}

std::string AutoDemoRecorder::createDemoPath(const std::string &name) {
  return stringFormat(
      "demos/%s/%s_%s_%s[%s].dm_84",
      FileSystem::Path::sanitizeFolder(etj_ad_targetPath.string),
      FileSystem::Path::sanitize(cgs.clientinfo[cg.clientNum].cleanname),
      cgs.rawmapname, FileSystem::Path::sanitize(name), createTimeString());
}

std::string
AutoDemoRecorder::createTimerunDemoPath(const std::string &runName,
                                        const std::string &runTime) {
  return stringFormat(
      "demos/%s/%s_%s_%s_%s[%s].dm_84",
      FileSystem::Path::sanitizeFolder(etj_ad_targetPath.string),
      FileSystem::Path::sanitize(cgs.clientinfo[cg.clientNum].cleanname),
      cgs.rawmapname, FileSystem::Path::sanitize(runName),
      formatRunTime(Q_atoi(runTime.c_str())), createTimeString());
}

std::string AutoDemoRecorder::createDemoTempPath(const std::string &name) {
  return stringFormat("demos/%s.dm_84", name);
}

std::string AutoDemoRecorder::createTimeString() {
  Time time = getCurrentTime();
  return stringFormat("%02d-%02d-%d-%02d%02d%02d", time.date.day, time.date.mon,
                      time.date.year, time.clock.hours, time.clock.min,
                      time.clock.sec);
}

std::string AutoDemoRecorder::formatRunTime(int millis) {
  Clock clock = toClock(millis, false);
  return stringFormat("%02d.%02d.%03d", clock.min, clock.sec, clock.ms);
}

void AutoDemoRecorder::maybeCancelDelayedSave() {
  if (delayedTimerId) {
    clearTimeout(delayedTimerId);
    delayedTimerId = 0;
  }
}
} // namespace ETJump
