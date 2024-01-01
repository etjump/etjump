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

#include <memory>
#include "etj_progression_tracker_parser.h"

#include "g_local.h"

#include "etj_progression_tracker.h"
#include "etj_printer.h"
#include "etj_string_utilities.h"

#include <iostream>

const char
    *ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET =
        "-1";

ETJump::ProgressionTrackers::ProgressionTrackers() {
  _progressionTrackers.clear();
}

ETJump::ProgressionTrackers::~ProgressionTrackers() {}

void ETJump::ProgressionTrackers::printParserErrors(
    const std::vector<std::string> &errors, const std::string &text) {
  auto buffer = "Tracker parse error on line: " + text + "\n";
  buffer += ETJump::StringUtil::join(errors, "\n");
  G_Error(buffer.c_str());
}

void ETJump::ProgressionTrackers::updateTracker(
    std::vector<ProgressionTrackerParser::IndexValuePair> pairs,
    int tracker[MaxProgressionTrackers]) {
  for (const auto &pair : pairs) {
    if (pair.index >= MaxProgressionTrackers) {
      G_Error("Tracker error: specified index (%d) is "
              "greater than maximum "
              "number of trackers indices (%d)",
              pair.index + 1, MaxProgressionTrackers);
      return;
    }

    tracker[pair.index] = pair.value;
  }
}

std::vector<ETJump::ProgressionTrackerParser::IndexValuePair>
ETJump::ProgressionTrackers::parseKey(const std::string &key) {
  auto parser = ProgressionTrackerParser(key);
  auto errors = parser.getErrors();
  if (errors.size()) {
    printParserErrors(errors, key);
  }
  return parser.getParsedPairs();
}

int ETJump::ProgressionTrackers::registerTracker(ProgressionTrackerKeys keys) {
  auto progressionTracker = ProgressionTracker();

  updateTracker(parseKey(keys.equal), progressionTracker.equal);
  updateTracker(parseKey(keys.notEqual), progressionTracker.notEqual);
  updateTracker(parseKey(keys.greaterThan), progressionTracker.greaterThan);
  updateTracker(parseKey(keys.lessThan), progressionTracker.lessThan);
  updateTracker(parseKey(keys.set), progressionTracker.set);
  updateTracker(parseKey(keys.setIf), progressionTracker.setIf);
  updateTracker(parseKey(keys.increment), progressionTracker.increment);
  updateTracker(parseKey(keys.incrementIf), progressionTracker.incrementIf);

  _progressionTrackers.push_back(progressionTracker);
  return _progressionTrackers.size() - 1;
}

void ETJump::ProgressionTrackers::useTracker(
    gentity_t *ent, gentity_t *activator, const ProgressionTracker &tracker) {
  int values[MaxProgressionTrackers];
  memcpy(values, activator->client->sess.progression, sizeof(values));

  auto idx = 0;
  for (auto &v : tracker.set) {
    if (v >= 0) {
      activator->client->sess.progression[idx] = v;
    }

    ++idx;
  }

  idx = 0;
  for (auto &v : tracker.increment) {
    if (v != 0) {
      activator->client->sess.progression[idx] += v;
    }
    ++idx;
  }

  auto activate = true;
  int clientTracker;

  for (idx = 0; idx < MaxProgressionTrackers; ++idx) {
    clientTracker = activator->client->sess.progression[idx];

    if ((tracker.equal[idx] != ProgressionTrackerValueNotSet &&
         tracker.equal[idx] != clientTracker) ||
        (tracker.lessThan[idx] != ProgressionTrackerValueNotSet &&
         tracker.lessThan[idx] <= clientTracker) ||
        (tracker.greaterThan[idx] != ProgressionTrackerValueNotSet &&
         tracker.greaterThan[idx] >= clientTracker)) {
      activate = false;
      break;
    }
  }

  // notEqual must be checked in a separate loop, otherwise it will work
  // as an OR statement instead of AND
  for (idx = 0; idx < MaxProgressionTrackers; ++idx) {
    clientTracker = activator->client->sess.progression[idx];

    if (tracker.notEqual[idx] != ProgressionTrackerValueNotSet) {
      if (tracker.notEqual[idx] == clientTracker) {
        activate = false;
        continue;
      } else { // value set but not matched - break and fire targets
        activate = true;
        break;
      }
    }
  }

  if (activate) {
    G_UseTargetedEntities(ent, activator);

    for (idx = 0; idx < MaxProgressionTrackers; ++idx) {
      if (tracker.setIf[idx] >= 0) {
        activator->client->sess.progression[idx] = tracker.setIf[idx];
      }

      if (tracker.incrementIf[idx] != 0) {
        activator->client->sess.progression[idx] += tracker.incrementIf[idx];
      }
    }

    auto clientNum = ClientNum(activator);

    if (g_debugTrackers.integer > 0) {
      for (int i = 0; i < MaxProgressionTrackers; i++) {
        if (values[i] != activator->client->sess.progression[i]) {
          std::string trackerChangeMsg = stringFormat(
              "^7Tracker change - "
              "index: ^3%i "
              "^7value: ^2%i "
              "^7from: ^9%i^7\n",
              i + 1, activator->client->sess.progression[i], values[i]);
          Printer::SendPopupMessage(clientNum, trackerChangeMsg);
        }
      }
    }
  }
}

void ETJump::ProgressionTrackers::useTriggerTracker(gentity_t *ent,
                                                    gentity_t *activator) {
  const auto &tracker = _progressionTrackers[ent->key];

  if (!activator || !activator->client) {
    return;
  }

  useTracker(ent, activator, tracker);
}

void ETJump::ProgressionTrackers::useTargetTracker(gentity_t *ent,
                                                   gentity_t *other,
                                                   gentity_t *activator) {
  const auto &tracker = _progressionTrackers[ent->key];

  if (!activator || !activator->client) {
    return;
  }

  useTracker(ent, activator, tracker);
}

void SP_target_tracker(gentity_t *self) {
  ETJump::ProgressionTrackers::ProgressionTrackerKeys keys;

  G_SpawnString(
      "tracker_eq",
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET,
      &keys.equal);
  G_SpawnString(
      "tracker_not_eq",
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET,
      &keys.notEqual);
  G_SpawnString(
      "tracker_gt",
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET,
      &keys.greaterThan);
  G_SpawnString(
      "tracker_lt",
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET,
      &keys.lessThan);
  G_SpawnString(
      "tracker_set",
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET,
      &keys.set);
  G_SpawnString(
      "tracker_set_if",
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET,
      &keys.setIf);
  G_SpawnString("tracker_inc", "0", &keys.increment);
  G_SpawnString("tracker_inc_if", "0", &keys.incrementIf);

  self->key = ETJump::progressionTrackers->registerTracker(keys);
  self->use = [](gentity_t *ent, gentity_t *other, gentity_t *activator) {
    ETJump::progressionTrackers->useTargetTracker(ent, other, activator);
  };
}

void SP_trigger_tracker(gentity_t *self) {
  InitTrigger(self);
  // just make it same type as trigger multiple for now
  self->s.eType = ET_TRIGGER_MULTIPLE;

  ETJump::ProgressionTrackers::ProgressionTrackerKeys keys;

  G_SpawnString(
      "tracker_eq",
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET,
      &keys.equal);
  G_SpawnString(
      "tracker_not_eq",
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET,
      &keys.notEqual);
  G_SpawnString(
      "tracker_gt",
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET,
      &keys.greaterThan);
  G_SpawnString(
      "tracker_lt",
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET,
      &keys.lessThan);
  G_SpawnString(
      "tracker_set",
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET,
      &keys.set);
  G_SpawnString(
      "tracker_set_if",
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET,
      &keys.setIf);
  G_SpawnString("tracker_inc", "0", &keys.increment);
  G_SpawnString("tracker_inc_if", "0", &keys.incrementIf);

  self->key = ETJump::progressionTrackers->registerTracker(keys);
  self->use = [](gentity_t *ent, gentity_t *other, gentity_t *activator) {
    ETJump::progressionTrackers->useTriggerTracker(ent, activator);
  };
  self->touch = [](gentity_t *ent, gentity_t *activator, trace_t *trace) {
    ETJump::progressionTrackers->useTriggerTracker(ent, activator);
  };
}
