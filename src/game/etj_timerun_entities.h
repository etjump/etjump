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
#include <array>
#include <map>
#include <set>

#include "etj_log.h"

namespace ETJump {
class TimerunEntity {
private:
  static Log logger;
  static std::map<std::string, int> runIndices;
  static std::set<std::string> cleanNames;
  static std::set<std::string> names;

protected:
  static void setTimerunIndex(gentity_t *self);
  static bool canActivate(gentity_t *activator);
  static int getOrSetTimerunIndex(const std::string &runName);
  static bool canStartTimerun(gentity_t *self, gentity_t *activator,
                              int clientNum, float speed);

public:
  static void validateTimerunEntities();
};

/**
 * TargetStartTimer
 */
class TargetStartTimer : virtual public TimerunEntity {
protected:
  static void use(gentity_t *self, gentity_t *activator);

public:
  static void spawn(gentity_t *self);
};

/**
 * TriggerStartTimer
 */
class TriggerStartTimer : virtual public TimerunEntity,
                          virtual public TargetStartTimer {
public:
  static void spawn(gentity_t *self);
};

/**
 * TriggerStartTimerExt
 */
class TriggerStartTimerExt : virtual public TimerunEntity,
                             virtual public TargetStartTimer {
public:
  static void spawn(gentity_t *self);
};

/**
 * TargetStopTimer
 */
class TargetStopTimer : virtual public TimerunEntity {
protected:
  static void use(gentity_t *self, gentity_t *activator);

public:
  static void spawn(gentity_t *self);
};

/**
 * TriggerStopTimer
 */
class TriggerStopTimer : virtual public TimerunEntity,
                         virtual public TargetStopTimer {
public:
  static void spawn(gentity_t *self);
};

/**
 * TriggerStopTimerExt
 */
class TriggerStopTimerExt : virtual public TimerunEntity,
                            virtual public TargetStopTimer {
public:
  static void spawn(gentity_t *self);
};

/**
 * TargetCheckpoint
 */
class TargetCheckpoint : virtual public TimerunEntity {
protected:
  static void use(gentity_t *self, gentity_t *activator);

public:
  static void spawn(gentity_t *self);
};

/**
 * TriggerCheckpoint
 */
class TriggerCheckpoint : virtual public TimerunEntity,
                          virtual public TargetCheckpoint {
public:
  static void spawn(gentity_t *self);
};

/**
 * TriggerCheckpointExt
 */
class TriggerCheckpointExt : virtual public TimerunEntity,
                             virtual public TargetCheckpoint {
public:
  static void spawn(gentity_t *self);
};
} // namespace ETJump
