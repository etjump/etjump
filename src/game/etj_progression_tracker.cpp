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

#include <memory>
#include "etj_progression_tracker_parser.h"

#include "g_local.h"

#include "etj_progression_tracker.h"
#include "etj_printer.h"
#include "etj_string_utilities.h"

#include <iostream>
#include <bitset>

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
  updateTracker(parseKey(keys.notEqualAny), progressionTracker.notEqualAny);
  updateTracker(parseKey(keys.notEqualAll), progressionTracker.notEqualAll);
  updateTracker(parseKey(keys.greaterThan), progressionTracker.greaterThan);
  updateTracker(parseKey(keys.lessThan), progressionTracker.lessThan);
  updateTracker(parseKey(keys.set), progressionTracker.set);
  updateTracker(parseKey(keys.setIf), progressionTracker.setIf);
  updateTracker(parseKey(keys.increment), progressionTracker.increment);
  updateTracker(parseKey(keys.incrementIf), progressionTracker.incrementIf);
  updateTracker(parseKey(keys.bitIsSet), progressionTracker.bitIsSet);
  updateTracker(parseKey(keys.bitNotSet), progressionTracker.bitNotSet);
  updateTracker(parseKey(keys.bitSet), progressionTracker.bitSet);
  updateTracker(parseKey(keys.bitReset), progressionTracker.bitReset);

  _progressionTrackers.push_back(progressionTracker);
  return _progressionTrackers.size() - 1;
}

void ETJump::ProgressionTrackers::useTracker(
    gentity_t *ent, gentity_t *activator, const ProgressionTracker &tracker) {
  int oldValues[MaxProgressionTrackers];

  if (g_debugTrackers.integer > 0) {
    memcpy(oldValues, activator->client->sess.progression, sizeof(oldValues));
  }

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

  idx = 0;
  for (auto &v : tracker.bitSet) {
    if (v >= 0) {
      activator->client->sess.progression[idx] |= (1 << v);
    }
    ++idx;
  }

  idx = 0;
  for (auto &v : tracker.bitReset) {
    if (v >= 0) {
      activator->client->sess.progression[idx] &= ~(1 << v);
    }
    ++idx;
  }

  auto activate = true;
  uint8_t numNotEqualAllKeys = 0;
  uint8_t numNotEqualAllMatches = 0;

  // FIXME: I really, really do not like this loop. This is very error prone
  //  for mappers, because they can specify keys which cancel out each others
  //  functionality with seemingly no logic, and no error messages whatsoever.
  //  Logically you should never specify multiple of these keys, and that's what
  //  this assumes, but there's nothing stopping mappers from doing that for
  //  any other keys besides 'tracker_not_eq_any' and 'tracker_not_eq_all'.
  for (idx = 0; idx < MaxProgressionTrackers; ++idx) {
    const int clientTracker = activator->client->sess.progression[idx];

    if ((tracker.equal[idx] != ProgressionTrackerValueNotSet &&
         tracker.equal[idx] != clientTracker) ||
        (tracker.lessThan[idx] != ProgressionTrackerValueNotSet &&
         tracker.lessThan[idx] <= clientTracker) ||
        (tracker.greaterThan[idx] != ProgressionTrackerValueNotSet &&
         tracker.greaterThan[idx] >= clientTracker) ||
        (tracker.notEqualAny[idx] != ProgressionTrackerValueNotSet &&
         tracker.notEqualAny[idx] == clientTracker)) {
      activate = false;
      break;
    }

    if (tracker.notEqualAll[idx] != ProgressionTrackerValueNotSet) {
      numNotEqualAllKeys++;

      if (tracker.notEqualAll[idx] == clientTracker) {
        numNotEqualAllMatches++;
      }
    }
  }

  if (numNotEqualAllKeys > 0 && numNotEqualAllKeys == numNotEqualAllMatches) {
    activate = false;
  }

  if (activate) {
    for (idx = 0; idx < MaxProgressionTrackers; ++idx) {
      const auto clientBits =
          std::bitset<32>(activator->client->sess.progression[idx]);

      if (tracker.bitIsSet[idx] != ProgressionTrackerValueNotSet &&
          !clientBits.test(tracker.bitIsSet[idx])) {
        activate = false;
        break;
      }

      if (tracker.bitNotSet[idx] != ProgressionTrackerValueNotSet &&
          clientBits.test(tracker.bitNotSet[idx])) {
        activate = false;
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

    printTrackerChanges(activator, oldValues);
  }
}

void ETJump::ProgressionTrackers::printTrackerChanges(gentity_t *activator,
                                                      int *oldValues) {
  if (g_debugTrackers.integer <= 0) {
    return;
  }
  const auto clientNum = ClientNum(activator);

  for (int i = 0; i < MaxProgressionTrackers; i++) {
    if (oldValues[i] != activator->client->sess.progression[i]) {
      const std::string &trackerChangeMsg = stringFormat(
          "^7Tracker change - index: ^3%i ^7value: ^2%i ^7from: ^9%i^7\n",
          i + 1, activator->client->sess.progression[i], oldValues[i]);
      Printer::popup(clientNum, trackerChangeMsg);
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
  const auto keys = ETJump::ProgressionTrackers::ParseTrackerKeys();
  const char *defaultValue =
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET;

  if (Q_stricmp(keys.notEqualAny, defaultValue) &&
      Q_stricmp(keys.notEqualAll, defaultValue)) {
    G_Error("%s: 'tracker_not_eq_any' and 'tracker_not_eq_all' cannot be used "
            "in the same tracker entity.",
            __func__);
  }

  self->key = ETJump::progressionTrackers->registerTracker(keys);
  self->use = [](gentity_t *ent, gentity_t *other, gentity_t *activator) {
    ETJump::progressionTrackers->useTargetTracker(ent, other, activator);
  };
}

void SP_trigger_tracker(gentity_t *self) {
  InitTrigger(self);
  // just make it same type as trigger multiple for now
  self->s.eType = ET_TRIGGER_MULTIPLE;

  const auto keys = ETJump::ProgressionTrackers::ParseTrackerKeys();
  const char *defaultValue =
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET;

  if (Q_stricmp(keys.notEqualAny, defaultValue) &&
      Q_stricmp(keys.notEqualAll, defaultValue)) {
    G_Error("%s: 'tracker_not_eq_any' and 'tracker_not_eq_all' cannot be used "
            "in the same tracker entity.",
            __func__);
  }

  self->key = ETJump::progressionTrackers->registerTracker(keys);
  self->use = [](gentity_t *ent, gentity_t *other, gentity_t *activator) {
    ETJump::progressionTrackers->useTriggerTracker(ent, activator);
  };
  self->touch = [](gentity_t *ent, gentity_t *activator, trace_t *trace) {
    ETJump::progressionTrackers->useTriggerTracker(ent, activator);
  };
}

ETJump::ProgressionTrackers::ProgressionTrackerKeys
ETJump::ProgressionTrackers::ParseTrackerKeys() {
  ETJump::ProgressionTrackers::ProgressionTrackerKeys keys{};

  const char *VALUE_NOT_SET =
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET;

  G_SpawnString("tracker_eq", VALUE_NOT_SET, &keys.equal);

  // legacy 'tracker_not_eq', superseded by 'tracker_not_eq_any'
  // kept here for backwards compatibility with old maps
  if (!G_SpawnString("tracker_not_eq", VALUE_NOT_SET, &keys.notEqualAny)) {
    G_SpawnString("tracker_not_eq_any", VALUE_NOT_SET, &keys.notEqualAny);
  }

  G_SpawnString("tracker_not_eq_all", VALUE_NOT_SET, &keys.notEqualAll);
  G_SpawnString("tracker_gt", VALUE_NOT_SET, &keys.greaterThan);
  G_SpawnString("tracker_lt", VALUE_NOT_SET, &keys.lessThan);
  G_SpawnString("tracker_set", VALUE_NOT_SET, &keys.set);
  G_SpawnString("tracker_set_if", VALUE_NOT_SET, &keys.setIf);
  G_SpawnString("tracker_inc", "0", &keys.increment);
  G_SpawnString("tracker_inc_if", "0", &keys.incrementIf);
  G_SpawnString("tracker_bit_is_set", VALUE_NOT_SET, &keys.bitIsSet);
  G_SpawnString("tracker_bit_not_set", VALUE_NOT_SET, &keys.bitNotSet);
  G_SpawnString("tracker_bit_set", VALUE_NOT_SET, &keys.bitSet);
  G_SpawnString("tracker_bit_reset", VALUE_NOT_SET, &keys.bitReset);

  return keys;
}
