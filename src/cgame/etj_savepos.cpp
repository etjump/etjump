/*
 * MIT License
 *
 * Copyright (c) 2024 ETJump team <zero@etjump.com>
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

#include "cg_local.h"
#include "etj_demo_compatibility.h"

#include "../game/etj_string_utilities.h"
#include "../game/etj_json_utilities.h"
#include "../game/etj_filesystem.h"

#include "etj_savepos.h"

namespace ETJump {

SavePos::SavePos(const std::shared_ptr<Timerun> &p) {
  const bool timerunCompatible =
      cg.demoPlayback ? demoCompatibility->isCompatible({3, 0, 0}) : true;

  if (!p || !timerunCompatible) {
    CG_Printf("^3WARNING: ^7unable to initialize timerun information for "
              "^3'savepos'^7. Timerun state will not be saved in positions!\n");

    if (!timerunCompatible) {
      CG_Printf("Timerun state cannot be saved from demos recorded prior to "
                "^3ETJump 3.0.0\n");
    } else {
      CG_Printf("Try reconnecting or performing a 'vid_restart' to resolve the "
                "issue.\n");
    }
  } else {
    timerun = p;
  }

  parseExistingPositions();
}

void SavePos::parseExistingPositions() {
  const std::vector<std::string> files =
      FileSystem::getFileList("savepos", ".dat", false);

  for (const auto &file : files) {
    parseSavepos(file);
  }

  if (!savePositions.empty()) {
    CG_Printf("Parsed data for ^3%i ^7savepos files.\n", savePositions.size());
  }
}

void SavePos::createSaveposData(const std::string &file, const int flags) {
  SavePosData data;
  const playerState_t *ps = &cg.snap->ps;

  if (!ps) {
    CG_Printf("Unable to create a ^3savepos ^7- invalid playerstate!\n");
    return;
  }

  data.name = file.empty() ? defaultName : file;
  data.mapname = cgs.rawmapname;

  VectorCopy(ps->origin, data.pos.origin);
  VectorCopy(ps->viewangles, data.pos.angles);

  if (flags & static_cast<int>(PosFlags::NoPitch)) {
    data.pos.angles[0] = 0;
  }

  if (!(flags & static_cast<int>(PosFlags::NoVelocity))) {
    VectorCopy(ps->velocity, data.pos.velocity);
  }

  data.pos.stance = getStance(ps);

  if (timerun && cgs.clientinfo[ps->clientNum].timerunActive) {
    const auto currentRun = timerun->getTimerunInformationFor(ps->clientNum);
    const int currentTime = (ps->clientNum == cg.clientNum || cgs.sv_fps == 125)
                                ? ps->commandTime
                                : cg.time;

    data.timerunInfo.runName = currentRun->runName;
    data.timerunInfo.currentRunTimer = currentTime - currentRun->startTime;
    data.timerunInfo.previousRecord = currentRun->previousRecord;

    std::copy(currentRun->checkpoints.cbegin(), currentRun->checkpoints.cend(),
              data.timerunInfo.checkpoints.begin());
    std::copy(currentRun->previousRecordCheckpoints.cbegin(),
              currentRun->previousRecordCheckpoints.cend(),
              data.timerunInfo.previousRecordCheckpoints.begin());
  }

  writeSaveposFile(data);
  storePosition(data);
}

void SavePos::writeSaveposFile(SavePosData &data) {
  Json::Value root;
  Json::Value position;
  Json::Value timerunInfo;

  // this should always be filled, but set default name just in case
  if (data.name.empty()) {
    data.name = defaultName;
  }

  root["name"] = data.name;
  root["mapname"] = data.mapname;

  position["origin"] = Json::arrayValue;
  position["angles"] = Json::arrayValue;
  position["velocity"] = Json::arrayValue;

  for (int i = 0; i < 3; i++) {
    position["origin"].append(data.pos.origin[i]).asFloat();
    position["angles"].append(data.pos.angles[i]).asFloat();
    position["velocity"].append(data.pos.velocity[i]).asFloat();
  }

  position["stance"] = static_cast<int>(data.pos.stance);

  root["position"] = position;

  timerunInfo["runName"] = data.timerunInfo.runName;
  timerunInfo["currentRunTimer"] = data.timerunInfo.currentRunTimer;
  timerunInfo["previousRecord"] = data.timerunInfo.previousRecord;

  timerunInfo["checkpoints"] = Json::arrayValue;
  timerunInfo["previousRecordCheckpoints"] = Json::arrayValue;

  for (const auto &checkpoint : data.timerunInfo.checkpoints) {
    timerunInfo["checkpoints"].append(checkpoint).asInt();
  }

  for (const auto &checkpoint : data.timerunInfo.previousRecordCheckpoints) {
    timerunInfo["previousRecordCheckpoints"].append(checkpoint).asInt();
  }

  root["timerunInfo"] = timerunInfo;

  const std::string filename = "savepos/" + data.name + ".dat";

  if (!JsonUtils::writeFile(filename, root)) {
    CG_Printf("Unable to write savepos file ^3'%s'\n", filename.c_str());
    return;
  }

  CG_Printf("Saved new position to ^3'%s'\n", filename.c_str());
}

void SavePos::parseSavepos(const std::string &file) {
  SavePosData data;
  Json::Value root;

  const std::string filename =
      "savepos/" + (file.empty() ? defaultName + ".dat" : file);

  if (!JsonUtils::readFile(filename, root)) {
    CG_Printf("Couldn't parse savepos file ^3'%s'\n", filename.c_str());
    return;
  }

  data.name = root["name"].asString();
  data.mapname = root["mapname"].asString();

  const Json::Value position = root["position"];
  const Json::Value timerunInfo = root["timerunInfo"];

  const Json::Value &origin = position["origin"];
  const Json::Value &angles = position["angles"];
  const Json::Value &velocity = position["velocity"];

  for (int i = 0; i < 3; i++) {
    data.pos.origin[i] = origin[i].asFloat();
    data.pos.angles[i] = angles[i].asFloat();
    data.pos.velocity[i] = velocity[i].asFloat();
  }

  data.pos.stance = static_cast<PlayerStance>(position["stance"].asInt());

  data.timerunInfo.runName = timerunInfo["runName"].asString();
  data.timerunInfo.currentRunTimer = timerunInfo["currentRunTimer"].asInt();
  data.timerunInfo.previousRecord = timerunInfo["previousRecord"].asInt();

  const Json::Value &checkpoints = timerunInfo["checkpoints"];
  const Json::Value &previousRecordCheckpoints =
      timerunInfo["previousRecordCheckpoints"];

  for (int i = 0; i < MAX_TIMERUN_CHECKPOINTS; i++) {
    data.timerunInfo.checkpoints[i] = checkpoints[i].asInt();
    data.timerunInfo.previousRecordCheckpoints[i] =
        previousRecordCheckpoints[i].asInt();
  }

  storePosition(data);
}

void SavePos::storePosition(ETJump::SavePosData &data) {
  savePositions[data.name] = data;
}

bool SavePos::saveposExists(const std::string &name) {
  return savePositions.find(name.empty() ? defaultName : name) !=
         savePositions.end();
}

const SavePosData &SavePos::getSaveposData(const std::string &name) {
  return savePositions[name.empty() ? defaultName : name];
}

std::string SavePos::getDefaultSaveposName() const { return defaultName; }

std::vector<std::string> SavePos::getSaveposNames() {
  std::vector<std::string> saveposNames;

  for (const auto &pos : savePositions) {
    saveposNames.emplace_back(pos.second.name);
  }

  return saveposNames;
}

PlayerStance SavePos::getStance(const playerState_t *ps) {
  if (ps->pm_flags & PMF_DUCKED) {
    return PlayerStance::Crouch;
  } else if (ps->eFlags & (EF_PRONE | EF_PRONE_MOVING)) {
    return PlayerStance::Prone;
  } else {
    return PlayerStance::Stand;
  }
}

std::string SavePos::serialize(const SavePosData &data) {
  const std::string &origin = StringUtil::join(data.pos.origin, " ");
  const std::string &angles = StringUtil::join(data.pos.angles, " ");
  const std::string &velocity = StringUtil::join(data.pos.velocity, " ");

  // omit timerun data if this position wasn't saved during a timerun
  if (data.timerunInfo.runName.empty()) {
    return stringFormat("loadpos %s %s %s %i", origin, angles, velocity,
                        static_cast<int>(data.pos.stance));
  }

  const std::string &checkpoints =
      StringUtil::join(data.timerunInfo.checkpoints, ",");
  const std::string &previousRecordCheckpoints =
      StringUtil::join(data.timerunInfo.previousRecordCheckpoints, ",");

  return stringFormat(
      "loadpos %s %s %s %i \"%s\" %i %i %s %s", origin, angles, velocity,
      static_cast<int>(data.pos.stance), data.timerunInfo.runName,
      data.timerunInfo.currentRunTimer, data.timerunInfo.previousRecord,
      checkpoints, previousRecordCheckpoints);
}
} // namespace ETJump
