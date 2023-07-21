/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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

#include "etj_local.h"
#include "etj_printer.h"
#include "etj_string_utilities.h"
#include "etj_timerun_v2.h"

namespace ETJump {
std::map<std::string, int> ETJump::TimerunEntity::runIndices;

void TimerunEntity::setTimerunIndex(gentity_t *self) {
  char *name = nullptr;
  int i;
  G_SpawnString("name", "default", &name);

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

  auto idx = runIndices[runName];

  Q_strncpyz(level.timerunNames[idx], runName.c_str(),
             sizeof(level.timerunNames[idx]));

  return runIndices[runName];
}

void TargetStartTimer::spawn(gentity_t *self) {
  setTimerunIndex(self);
  level.hasTimerun = true;
  G_SpawnFloat("speed_limit", "700", &self->velocityUpperLimit);

  self->use = [](gentity_t *self, gentity_t *other, gentity_t *activator) {
    use(self, activator);
  };
}

bool TargetStartTimer::canStartTimerun(gentity_t *self, gentity_t *activator,
                                       const int *clientNum,
                                       const float *speed) {
  auto client = activator->client;

  if (!client->pers.enableTimeruns || client->sess.timerunActive) {
    return false;
  }

  // disable some checks if we're on a debugging session
  if (!g_debugTimeruns.integer) {
    if (client->noclip || activator->flags & FL_GODMODE) {
      Printer::SendCenterMessage(
          *clientNum,
          "^3WARNING: ^7Timerun was not started. Invalid playerstate!");
      return false;
    }
    if (*speed > self->velocityUpperLimit) {
      Printer::SendCenterMessage(
          *clientNum, stringFormat("^3WARNING: ^7Timerun was not started. Too "
                                   "high starting speed (%.2f > %.2f)\n",
                                   *speed, self->velocityUpperLimit));
      return false;
    }
    if (client->ps.viewangles[ROLL] != 0) {
      Printer::SendCenterMessage(
          *clientNum, stringFormat("^3WARNING: ^7Timerun was not started. "
                                   "Illegal roll angles (%.2f != 0)",
                                   client->ps.viewangles[ROLL]));
      return false;
    }
  }

  return true;
}

void TargetStartTimer::use(gentity_t *self, gentity_t *activator) {
  if (!canActivate(activator)) {
    return;
  }
  const int clientNum = ClientNum(activator);
  auto client = activator->client;
  const float speed = VectorLength(client->ps.velocity);

  if (!canStartTimerun(self, activator, &clientNum, &speed)) {
    return;
  }

  client->sess.runSpawnflags = self->spawnflags;

  // check for pmove_fixed 0
  if (!client->sess.runSpawnflags ||
      client->sess.runSpawnflags &
      static_cast<int>(TimerunSpawnflags::ResetNoPmove)) {
    if (!client->pers.pmoveFixed) {
      Printer::SendCenterMessage(
          clientNum,
          "^3WARNING: ^7Timerun was not started. ^3pmove_fixed ^7is disabled!");
      return;
    }
  }

  if (!client->sess.timerunCheatsNotified && g_cheats.integer) {
    Printer::SendPopupMessage(clientNum,
                              "^3WARNING: ^7Cheats are enabled! Timerun "
                              "records will not be saved!\n");
    client->sess.timerunCheatsNotified = true;
  }

  StartTimer(level.timerunNames[self->runIndex], activator);

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

void TargetCheckpoint::use(gentity_t *self, gentity_t *activator) {
  if (!canActivate(activator)) {
    return;
  }

  game.timerunV2->checkpoint(level.timerunNames[self->runIndex],
                             self->checkpointIndex, ClientNum(activator),
                             activator->client->ps.commandTime);
}

void TargetCheckpoint::spawn(gentity_t *self) {
  char *name = nullptr;
  G_SpawnString("name", "default", &name);

  Q_strncpyz(self->runName, name, sizeof(self->runName));

  setTimerunIndex(self);

  level.hasCheckpoints = true;

  self->checkpointIndex = level.checkpointsCount[self->runIndex]++;
  self->use = [](gentity_t *self, gentity_t *other, gentity_t *activator) {
    use(self, activator);
  };
}

void TargetStopTimer::use(gentity_t *self, gentity_t *activator) {
  if (!canActivate(activator)) {
    return;
  }

  StopTimer(level.timerunNames[self->runIndex], activator);

  game.timerunV2->stopTimer(level.timerunNames[self->runIndex],
                            ClientNum(activator),
                            activator->client->ps.commandTime);
}
} // namespace ETJump
