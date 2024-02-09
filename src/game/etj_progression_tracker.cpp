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

template<typename T>
void ETJump::ProgressionTrackers::addDef(const std::string &key,
                                         int defaultValue,
                                         std::vector<TrackerDef<T>> &defs,
                                         const std::string &name, T func) {

  auto parser = ProgressionTrackerParser(key);
  auto errors = parser.getErrors();
  if (errors.size()) {
    printParserErrors(errors, key);
  }
  const auto pairs = parser.getParsedPairs();

  for (const auto &pair : pairs) {
    if (pair.index >= MaxProgressionTrackers) {
      G_Error("Tracker error: specified index (%d) is "
              "greater than maximum "
              "number of trackers indices (%d)",
              pair.index + 1, MaxProgressionTrackers);
      return;
    }

    if (pair.value != defaultValue) {
      defs.push_back(TrackerDef<T>(pair.index, pair.value, name, func));
    }
  }
}

int ETJump::ProgressionTrackers::registerTracker(ProgressionTrackerKeys keys) {
  std::vector<TrackerDef<ChangeFunc>> changes;
  std::vector<TrackerDef<ChangeFunc>> optionalChanges;
  std::vector<TrackerDef<CheckFunc>> checks;

  using PT = ETJump::ProgressionTrackers;

  constexpr int notset = ProgressionTrackerValueNotSet;
  addDef(keys.set, notset, changes, "set", PT::Change::Set);
  addDef(keys.increment, 0, changes, "inc", PT::Change::Increment);
  addDef(keys.bitSet, notset, changes, "bit_set", PT::Change::SetBit);
  addDef(keys.bitReset, notset, changes, "bit_reset", PT::Change::ResetBit);

  addDef(keys.set, notset, optionalChanges, "set_if", PT::Change::Set);
  addDef(keys.incrementIf, 0, optionalChanges, "inc_if", PT::Change::Increment);

  addDef(keys.equal, notset, checks, "eq", PT::Check::Equal);
  addDef(keys.notEqual, notset, checks, "not_eq", PT::Check::NotEqual);
  addDef(keys.greaterThan, notset, checks, "gt", PT::Check::GreaterThan);
  addDef(keys.lessThan, notset, checks, "lt", PT::Check::LessThan);
  addDef(keys.bitIsSet, notset, checks, "bitset", PT::Check::BitSet);
  addDef(keys.bitNotSet, notset, checks, "bitnotset", PT::Check::BitNotSet);

  _progressionTrackers.push_back({changes, checks, optionalChanges});
  return _progressionTrackers.size() - 1;
}

void ETJump::ProgressionTrackers::useTracker(gentity_t *ent,
                                             gentity_t *activator,
                                             const TrackerCollection &tracker) {
  for (const auto &def : tracker.changes) {
    executeChange(activator, def.name, def.index, def.value, def.execute);
  }

  bool activate = true;

  for (const auto &def : tracker.checks) {
    if (!executeCheck(activator, def.name, def.index, def.value, def.execute)) {
      activate = false;
    }

    // short circuit if not debugging trackers
    if (!activate && g_debugTrackers.integer <= 0) {
      break;
    }
  }

  if (activate) {
    G_UseTargetedEntities(ent, activator);

    for (const auto &def : tracker.conditionalChanges) {
      executeChange(activator, def.name, def.index, def.value, def.execute);
    }
  }
}

bool ETJump::ProgressionTrackers::executeCheck(const gentity_t *ent,
                                               const std::string &name,
                                               const int index, const int value,
                                               const CheckFunc &exec) {

  const bool result = exec(ent->client->sess.progression, index, value);

  if (g_debugTrackers.integer > 0) {
    const auto clientNum = ClientNum(ent);

    const auto resultString = result ? "^hPASS" : "^jFAIL";
    const std::string &msg = stringFormat(
        "^7Tracker ^3%i ^7check %s^7: ^2%i ^9%s ^l%i", index + 1, resultString,
        ent->client->sess.progression[index], name, value);
    Printer::SendPopupMessage(clientNum, msg);
  }

  return result;
}

void ETJump::ProgressionTrackers::executeChange(const gentity_t *ent,
                                                const std::string &name,
                                                const int index,
                                                const int value,
                                                const ChangeFunc &exec) {
  const int oldValue = ent->client->sess.progression[index];
  exec(ent->client->sess.progression, index, value);

  if (g_debugTrackers.integer > 0 &&
      oldValue != ent->client->sess.progression[index]) {
    const auto clientNum = ClientNum(ent);

    const std::string &msg =
        stringFormat("^7Tracker ^3%i ^7change: ^l%i ^7-> ^2%i ^9(%s %i)", index + 1,
                     oldValue, ent->client->sess.progression[index], name, value);
    Printer::SendPopupMessage(clientNum, msg);
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
  const auto keys = ETJump::ProgressionTrackers::parseTrackerKeys();

  self->key = ETJump::progressionTrackers->registerTracker(keys);
  self->use = [](gentity_t *ent, gentity_t *other, gentity_t *activator) {
    ETJump::progressionTrackers->useTargetTracker(ent, other, activator);
  };
}

void SP_trigger_tracker(gentity_t *self) {
  InitTrigger(self);
  // just make it same type as trigger multiple for now
  self->s.eType = ET_TRIGGER_MULTIPLE;

  const auto keys = ETJump::ProgressionTrackers::parseTrackerKeys();

  self->key = ETJump::progressionTrackers->registerTracker(keys);
  self->use = [](gentity_t *ent, gentity_t *other, gentity_t *activator) {
    ETJump::progressionTrackers->useTriggerTracker(ent, activator);
  };
  self->touch = [](gentity_t *ent, gentity_t *activator, trace_t *trace) {
    ETJump::progressionTrackers->useTriggerTracker(ent, activator);
  };
}

ETJump::ProgressionTrackers::ProgressionTrackerKeys
ETJump::ProgressionTrackers::parseTrackerKeys() {
  ETJump::ProgressionTrackers::ProgressionTrackerKeys keys{};

  const char *VALUE_NOT_SET =
      ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET;

  G_SpawnString("tracker_eq", VALUE_NOT_SET, &keys.equal);
  G_SpawnString("tracker_not_eq", VALUE_NOT_SET, &keys.notEqual);
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
