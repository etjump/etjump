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

#pragma once

#include <vector>
#include <array>
#include <unordered_map>

#include "g_local.h"
#include "etj_progression_tracker_parser.h"

namespace ETJump {
class ProgressionTrackers {
public:
  static const char *ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET;
  static const int ProgressionTrackerValueNotSet = -1;
  struct ProgressionTrackerKeys {
    char *equal;
    char *notEqualAny;
    char *notEqualAll;
    char *greaterThan;
    char *lessThan;
    char *set;
    char *setIf;
    char *increment;
    char *incrementIf;
    char *bitIsSet;
    char *bitNotSet;
    char *bitSet;
    char *bitReset;
  };

  static void targetTrackerSpawn(gentity_t *self);
  static void triggerTrackerSpawn(gentity_t *self);

  static ProgressionTrackerKeys ParseTrackerKeys();
  static void printTrackerChanges(
      const gentity_t *activator,
      const std::array<int32_t, MAX_PROGRESSION_TRACKERS> &oldValues);

  struct ProgressionTracker {
    ProgressionTracker() {
      std::fill_n(equal, MAX_PROGRESSION_TRACKERS, -1);
      std::fill_n(notEqualAny, MAX_PROGRESSION_TRACKERS, -1);
      std::fill_n(notEqualAll, MAX_PROGRESSION_TRACKERS, -1);
      std::fill_n(greaterThan, MAX_PROGRESSION_TRACKERS, -1);
      std::fill_n(lessThan, MAX_PROGRESSION_TRACKERS, -1);
      std::fill_n(set, MAX_PROGRESSION_TRACKERS, -1);
      std::fill_n(setIf, MAX_PROGRESSION_TRACKERS, -1);
      std::fill_n(bitIsSet, MAX_PROGRESSION_TRACKERS, -1);
      std::fill_n(bitNotSet, MAX_PROGRESSION_TRACKERS, -1);
      std::fill_n(bitSet, MAX_PROGRESSION_TRACKERS, -1);
      std::fill_n(bitReset, MAX_PROGRESSION_TRACKERS, -1);

      // increments default to 0
      std::fill_n(increment, MAX_PROGRESSION_TRACKERS, 0);
      std::fill_n(incrementIf, MAX_PROGRESSION_TRACKERS, 0);
    }
    int equal[MAX_PROGRESSION_TRACKERS]{};
    int notEqualAny[MAX_PROGRESSION_TRACKERS]{};
    int notEqualAll[MAX_PROGRESSION_TRACKERS]{};
    int greaterThan[MAX_PROGRESSION_TRACKERS]{};
    int lessThan[MAX_PROGRESSION_TRACKERS]{};
    int set[MAX_PROGRESSION_TRACKERS]{};
    int setIf[MAX_PROGRESSION_TRACKERS]{};
    int increment[MAX_PROGRESSION_TRACKERS]{};
    int incrementIf[MAX_PROGRESSION_TRACKERS]{};
    int bitIsSet[MAX_PROGRESSION_TRACKERS]{};
    int bitNotSet[MAX_PROGRESSION_TRACKERS]{};
    int bitSet[MAX_PROGRESSION_TRACKERS]{};
    int bitReset[MAX_PROGRESSION_TRACKERS]{};
  };

  ProgressionTrackers();
  ~ProgressionTrackers();
  void printParserErrors(const std::vector<std::string> &errors,
                         const std::string &text);
  std::vector<ProgressionTrackerParser::IndexValuePair>
  parseKey(const std::string &key);
  void
  updateTracker(std::vector<ProgressionTrackerParser::IndexValuePair> pairs,
                int tracker[MAX_PROGRESSION_TRACKERS]);
  int registerTracker(ProgressionTrackerKeys keys);
  void useTargetTracker(gentity_t *ent, gentity_t *other, gentity_t *activator);
  void useTriggerTracker(gentity_t *ent, gentity_t *activator);

  void saveClientProgression(const gentity_t *ent);
  void restoreClientProgression(const gentity_t *ent);

private:
  void useTracker(gentity_t *ent, gentity_t *activator,
                  const ProgressionTracker &tracker);

  std::vector<ProgressionTracker> _progressionTrackers;
  std::unordered_map<std::string, std::array<int32_t, MAX_PROGRESSION_TRACKERS>>
      savedProgression;
};
} // namespace ETJump
