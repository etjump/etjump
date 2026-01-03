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

#include "g_local.h"
#include "etj_timerun_entities.h"

#include <stdexcept>

#include "etj_container_utilities.h"
#include "etj_local.h"
#include "etj_printer.h"
#include "etj_string_utilities.h"
#include "etj_timerun_v2.h"

namespace ETJump {
std::map<std::string, int> TimerunEntity::runIndices;
std::set<std::string> TimerunEntity::cleanNames;
std::set<std::string> TimerunEntity::names;

void TimerunEntity::setTimerunIndex(gentity_t *self) {
  char *name = nullptr;
  G_SpawnString("name", "default", &name);

  Q_strncpyz(self->runName, name, sizeof(self->runName));

  if (level.timerunNamesCount > MAX_TIMERUNS) {
    G_Error("setTimerunIndex: Too many timeruns in the map (%d > %d)\n",
            level.timerunNamesCount, MAX_TIMERUNS);
    return;
  }

  self->runIndex = getOrSetTimerunIndex(name);
}

bool TimerunEntity::canActivate(gentity_t *activator) {
  if (!activator || !activator->client) {
    return false;
  }
  if (activator->client->sess.sessionTeam == TEAM_SPECTATOR) {
    return false;
  }

  return true;
}

int TimerunEntity::getOrSetTimerunIndex(const std::string &runName) {
  if (runIndices.count(runName) == 0) {
    runIndices[runName] = level.timerunNamesCount++;
  }

  cleanNames.insert(sanitize(runName, true));
  names.insert(sanitize(runName, true));

  if (cleanNames.size() != names.size()) {
    G_Error(
        "Timeruns should have the same names with different color codes. E.g. "
        "^1Run and ^2Run are not valid names. Please rename the runs. Possible "
        "run name to rename: %s\n",
        runName.c_str());
    return 0;
  }

  auto idx = runIndices[runName];

  Q_strncpyz(level.timerunNames[idx], runName.c_str(),
             sizeof(level.timerunNames[idx]));

  return runIndices[runName];
}

struct TimerunEntityValidationResult {
  bool hasStartTimer;
  bool hasStopTimer;
  bool hasCheckpoints;
};

std::vector<std::string> timerunEntities{
    "target_startTimer", "trigger_startTimer", "target_stopTimer",
    "trigger_stopTimer", "target_checkpoint",  "trigger_checkpoint"};

void TimerunEntity::validateTimerunEntities() {
  std::map<std::string, TimerunEntityValidationResult> validationResults;

  for (int i = MAX_CLIENTS + BODY_QUEUE_SIZE; i < level.num_entities; ++i) {
    const gentity_t *ent = &g_entities[i];

    if (!std::any_of(timerunEntities.cbegin(), timerunEntities.cend(),
                     [&ent](const std::string &entity) {
                       return StringUtil::iEqual(ent->classname, entity);
                     })) {
      continue;
    }

    if (validationResults.count(ent->runName) == 0) {
      validationResults[ent->runName] = TimerunEntityValidationResult{};
    }

    if (StringUtil::iEqual(ent->classname, "target_startTimer") ||
        StringUtil::iEqual(ent->classname, "trigger_startTimer")) {
      validationResults[ent->runName].hasStartTimer = true;
    } else if (StringUtil::iEqual(ent->classname, "target_stopTimer") ||
               StringUtil::iEqual(ent->classname, "trigger_stopTimer")) {
      validationResults[ent->runName].hasStopTimer = true;
    } else if (StringUtil::iEqual(ent->classname, "target_checkpoint") ||
               StringUtil::iEqual(ent->classname, "trigger_checkpoint")) {
      validationResults[ent->runName].hasCheckpoints = true;
    }
  }

  for (const auto &r : validationResults) {
    if (!r.second.hasStartTimer && !r.second.hasStopTimer) {
      G_Printf("^3WARNING: checkpoint found for timerun '%s^3' without start "
               "and stop triggers\n",
               r.first.c_str());
    } else if (!r.second.hasStartTimer) {
      G_Printf("^3WARNING: timerun '%s^3' has no start trigger\n",
               r.first.c_str());
    } else if (!r.second.hasStopTimer) {
      G_Printf("^3WARNING: timerun '%s^3' has no stop trigger\n",
               r.first.c_str());
    }
  }
}

bool TimerunEntity::canStartTimerun(const gentity_t *self,
                                    const gentity_t *activator,
                                    const int clientNum, const float speed) {
  const auto client = activator->client;

  if (!client->pers.enableTimeruns || client->sess.timerunActive) {
    return false;
  }

  // disable checks if cheats are enabled
  if (g_cheats.integer) {
    return true;
  }

  if (client->noclip || activator->flags & FL_GODMODE) {
    Printer::center(
        clientNum,
        "^3WARNING: ^7Timerun was not started. Invalid playerstate!");
    return false;
  }

  if (client->noclipThisLife) {
    Printer::center(clientNum, "^3WARNING: ^7Timerun was not started. ^3noclip "
                               "^7activated this life, ^3/kill ^7required!");
    return false;
  }

  if (client->setoffsetThisLife) {
    Printer::center(clientNum,
                    "^3WARNING: ^7Timerun was not started. ^3setoffset "
                    "^7activated this life, ^3/kill ^7required!");
    return false;
  }

  if (client->ftNoGhostThisLife &&
      !(self->spawnflags &
        static_cast<int>(TimerunSpawnflags::AllowFTNoGhost))) {
    Printer::center(
        clientNum,
        "^3WARNING: ^7Timerun was not started. ^3fireteam noghost ^7enabled "
        "this life & run does not allow noghost, ^3/kill ^7required!");
    return false;
  }

  if (client->pmoveOffThisLife &&
      (!self->spawnflags ||
       self->spawnflags & static_cast<int>(TimerunSpawnflags::ResetNoPmove))) {
    Printer::center(clientNum,
                    "^3WARNING: ^7Timerun was not started. ^3pmove_fixed 0 "
                    "^7set this life, ^3/kill ^7required!");
    return false;
  }

  if (speed > self->velocityUpperLimit) {
    Printer::center(clientNum,
                    stringFormat("^3WARNING: ^7Timerun was not started. Too "
                                 "high starting speed (%.2f > %.2f)\n",
                                 speed, self->velocityUpperLimit));
    return false;
  }

  if (client->ps.viewangles[ROLL] != 0) {
    Printer::center(clientNum,
                    stringFormat("^3WARNING: ^7Timerun was not started. "
                                 "Illegal roll angles (%.2f != 0)",
                                 client->ps.viewangles[ROLL]));
    return false;
  }

  return true;
}

void TargetStartTimer::spawn(gentity_t *self) {
  setTimerunIndex(self);
  level.hasTimerun = true;
  G_SpawnFloat("speed_limit", "700", &self->velocityUpperLimit);

  self->use = [](gentity_t *self, gentity_t *other, gentity_t *activator) {
    use(self, activator);
  };
}

void TriggerStartTimer::spawn(gentity_t *self) {
  setTimerunIndex(self);
  level.hasTimerun = true;
  G_SpawnFloat("speed_limit", "700", &self->velocityUpperLimit);

  self->use = [](gentity_t *self, gentity_t *other, gentity_t *activator) {
    use(self, activator);
  };
  self->touch = [](gentity_t *self, gentity_t *other, trace_t *t) {
    use(self, other);
  };

  self->s.eType = ET_TRIGGER_MULTIPLE;
  InitTrigger(self);
}

void TriggerStartTimerExt::spawn(gentity_t *self) {
  setTimerunIndex(self);
  level.hasTimerun = true;
  G_SpawnFloat("speed_limit", "700", &self->velocityUpperLimit);

  if (!G_SpawnVector("mins", "0 0 0", self->r.mins)) {
    G_Error("'trigger_starttimer_ext' does not have mins\n");
  }
  if (!G_SpawnVector("maxs", "0 0 0", self->r.maxs)) {
    G_Error("'trigger_starttimer_ext' does not have maxs\n");
  }

  self->use = [](gentity_t *self, gentity_t *other, gentity_t *activator) {
    use(self, activator);
  };
  self->touch = [](gentity_t *self, gentity_t *other, trace_t *t) {
    use(self, other);
  };

  self->s.eType = ET_TRIGGER_MULTIPLE;
  InitTrigger(self);
  trap_LinkEntity(self);
}

void TargetStartTimer::use(gentity_t *self, gentity_t *activator) {
  if (!canActivate(activator)) {
    return;
  }
  const int clientNum = ClientNum(activator);
  auto client = activator->client;
  const float speed = VectorLength(client->ps.velocity);

  if (!canStartTimerun(self, activator, clientNum, speed)) {
    return;
  }

  client->sess.runSpawnflags = self->spawnflags;

  // check for pmove_fixed 0
  if (!client->sess.runSpawnflags ||
      client->sess.runSpawnflags &
          static_cast<int>(TimerunSpawnflags::ResetNoPmove)) {
    if (!client->pers.pmoveFixed) {
      Printer::center(
          clientNum,
          "^3WARNING: ^7Timerun was not started. ^3pmove_fixed ^7is disabled!");
      return;
    }
  }

  if (!client->sess.timerunCheatsNotified && g_cheats.integer) {
    Printer::popup(clientNum, "^3WARNING: ^7Cheats are enabled! Timerun "
                              "records will not be saved!\n");
    client->sess.timerunCheatsNotified = true;
  }

  game.timerunV2->startTimer(level.timerunNames[self->runIndex], clientNum,
                             activator->client->pers.netname,
                             activator->client->ps.commandTime);
}

void TargetStopTimer::spawn(gentity_t *self) {
  setTimerunIndex(self);

  self->use = [](gentity_t *self, gentity_t *other, gentity_t *activator) {
    use(self, activator);
  };
}

void TriggerStopTimer::spawn(gentity_t *self) {
  setTimerunIndex(self);

  self->use = [](gentity_t *self, gentity_t *other, gentity_t *activator) {
    use(self, activator);
  };
  self->touch = [](gentity_t *self, gentity_t *other, trace_t *t) {
    use(self, other);
  };

  self->s.eType = ET_TRIGGER_MULTIPLE;
  InitTrigger(self);
}

void TriggerStopTimerExt::spawn(gentity_t *self) {
  setTimerunIndex(self);

  if (!G_SpawnVector("mins", "0 0 0", self->r.mins)) {
    G_Error("'trigger_stoptimer_ext' does not have mins\n");
  }
  if (!G_SpawnVector("maxs", "0 0 0", self->r.maxs)) {
    G_Error("'trigger_stoptimer_ext' does not have maxs\n");
  }

  self->use = [](gentity_t *self, gentity_t *other, gentity_t *activator) {
    use(self, activator);
  };
  self->touch = [](gentity_t *self, gentity_t *other, trace_t *t) {
    use(self, other);
  };

  self->s.eType = ET_TRIGGER_MULTIPLE;
  InitTrigger(self);
  trap_LinkEntity(self);
}

void TargetStopTimer::use(gentity_t *self, gentity_t *activator) {
  if (!canActivate(activator)) {
    return;
  }

  game.timerunV2->stopTimer(level.timerunNames[self->runIndex],
                            ClientNum(activator),
                            activator->client->ps.commandTime);
}

void TargetCheckpoint::spawn(gentity_t *self) {
  char *name = nullptr;
  G_SpawnString("name", "default", &name);

  Q_strncpyz(self->runName, name, sizeof(self->runName));

  setTimerunIndex(self);

  self->checkpointIndex = level.checkpointsCount[self->runIndex]++;
  self->use = [](gentity_t *self, gentity_t *other, gentity_t *activator) {
    use(self, activator);
  };
}

void TriggerCheckpoint::spawn(gentity_t *self) {
  char *name = nullptr;
  G_SpawnString("name", "default", &name);

  Q_strncpyz(self->runName, name, sizeof(self->runName));

  setTimerunIndex(self);

  self->checkpointIndex = level.checkpointsCount[self->runIndex]++;
  self->use = [](gentity_t *self, gentity_t *other, gentity_t *activator) {
    use(self, activator);
  };
  self->touch = [](gentity_t *self, gentity_t *other, trace_t *t) {
    use(self, other);
  };
  self->s.eType = ET_TRIGGER_MULTIPLE;
  InitTrigger(self);
}

void TriggerCheckpointExt::spawn(gentity_t *self) {
  char *name = nullptr;
  G_SpawnString("name", "default", &name);

  Q_strncpyz(self->runName, name, sizeof(self->runName));

  setTimerunIndex(self);

  if (!G_SpawnVector("mins", "0 0 0", self->r.mins)) {
    G_Error("'trigger_checkpoint_ext' does not have mins\n");
  }
  if (!G_SpawnVector("maxs", "0 0 0", self->r.maxs)) {
    G_Error("'trigger_checkpoint_ext' does not have maxs\n");
  }

  self->checkpointIndex = level.checkpointsCount[self->runIndex]++;
  self->use = [](gentity_t *self, gentity_t *other, gentity_t *activator) {
    use(self, activator);
  };
  self->touch = [](gentity_t *self, gentity_t *other, trace_t *t) {
    use(self, other);
  };
  self->s.eType = ET_TRIGGER_MULTIPLE;
  InitTrigger(self);
  trap_LinkEntity(self);
}

void TargetCheckpoint::use(gentity_t *self, gentity_t *activator) {
  if (!canActivate(activator)) {
    return;
  }

  game.timerunV2->checkpoint(level.timerunNames[self->runIndex],
                             ClientNum(activator), self->checkpointIndex,
                             activator->client->ps.commandTime);
}
} // namespace ETJump
