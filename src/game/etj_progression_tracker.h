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

#pragma once

#include <vector>
#include <array>

#include "etj_progression_tracker_parser.h"

namespace ETJump {

class ProgressionTrackers {
public:
  static const char *ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET;
  static const int ProgressionTrackerValueNotSet = -1;
  struct ProgressionTrackerKeys {
    char *equal;
    char *notEqual;
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
  static const int MaxProgressionTrackers = 50;
  static ETJump::ProgressionTrackers::ProgressionTrackerKeys parseTrackerKeys();

  typedef void (*ChangeFunc)(int* progression, int index, int value);
  typedef bool (*CheckFunc)(int* progression, int index, int value);

  template<typename T>
  struct TrackerDef {
    const int index;
    const int value;
    const std::string name;
    const T execute;

    TrackerDef(const int index, const int value, const std::string &name,
      T execute)
      : index(index), value(value), name(name), execute(execute) {}
  };

  static bool executeCheck(
    const gentity_t *ent,
    const std::string &name,
    const int index,
    const int value,
    const CheckFunc &exec);
  static void executeChange(
    const gentity_t *ent,
    const std::string &name,
    const int index,
    const int value,
    const ChangeFunc &exec);

  class Check {
  public:
    static bool Equal(int *p, int i, int v) { return v == p[i]; };
    static bool NotEqual(int *p, int i, int v) { return v != p[i]; };
    static bool GreaterThan(int *p, int i, int v) { return v > p[i]; };
    static bool LessThan(int *p, int i, int v) { return v < p[i]; };
    static bool BitSet(int *p, int i, int v) {
      return static_cast<bool>((1 << v) & p[i]);
    };
    static bool BitNotSet(int *p, int i, int v) {
      return !static_cast<bool>((1 << v) & p[i]);
    };
  };

  class Change {
  public:
    static void Set(int *p, int i, int v) { p[i] = v; };
    static void Increment(int *p, int i, int v) { p[i] += v; };
    static void SetBit(int *p, int i, int v) { p[i] |= (1 << v); };
    static void ResetBit(int *p, int i, int v) { p[i] &= ~(1 << v); }
  };

  ProgressionTrackers();
  ~ProgressionTrackers();
  void printParserErrors(const std::vector<std::string> &errors,
                         const std::string &text);
  int registerTracker(ProgressionTrackerKeys keys);
  void useTargetTracker(gentity_t *ent, gentity_t *other, gentity_t *activator);
  void useTriggerTracker(gentity_t *ent, gentity_t *activator);

private:
  struct TrackerCollection {
    const std::vector<TrackerDef<ChangeFunc>> changes;
    const std::vector<TrackerDef<CheckFunc>> checks;
    const std::vector<TrackerDef<ChangeFunc>> conditionalChanges;
  };

  template<typename T>
  void addDef(const std::string &key, int defaultValue,
              std::vector<TrackerDef<T>> &defs, const std::string &name,
              T func);

  void useTracker(gentity_t *ent, gentity_t *activator,
                  const TrackerCollection &tracker);

  std::vector<TrackerCollection> _progressionTrackers;
};
} // namespace ETJump
