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

#pragma once

#include <functional>
#include <map>

#include "etj_local.h"

#include "../game/etj_shared.h"

namespace ETJump {
class CvarUpdateHandler;
class ClientCommandsHandler;

inline constexpr int32_t MAX_CHS_INFO = 8;

// just some constants so we don't need to pass around random numbers
inline constexpr int32_t CHS_HUD_1 = 1;
inline constexpr int32_t CHS_HUD_2 = 2;
inline constexpr int32_t CHS_HUD_3 = 3;

class CHSDataHandler {
public:
  CHSDataHandler(
      const std::shared_ptr<UpmoveMeterData> &upmoveMeterData,
      const std::shared_ptr<CvarUpdateHandler> &cvarUpdateHandler,
      const std::shared_ptr<ClientCommandsHandler> &consoleCommandsHandler);
  ~CHSDataHandler();

  struct CHSCvar {
    const vmCvar_t *cvar{};
    bool valid = false;
  };

  struct CHSObject {
    std::array<CHSCvar, MAX_CHS_INFO> cvars{};
    bool needTrace{};
    bool needExtraTrace{};
  };

  void runFrame();
  static bool check();
  std::string getStat(const vmCvar_t *cvar) const;
  std::string getStatName(const vmCvar_t *cvar) const;
  [[nodiscard]] const std::array<CHSCvar, MAX_CHS_INFO> &
  getCvars(int32_t chs) const;

private:
  // as a general rule, try to keep this in sync with
  // the values used in DeFRaG (where applicable)
  // https://www.q3df.org/wiki?p=136#CHS-list
  enum class Stats {
    SPEED = 1,
    HEALTH = 2,
    AMMO = 4,

    DISTANCE_XY = 10,
    DISTANCE_Z = 11,
    DISTANCE_XYZ = 12,
    DISTANCE_VIEW_XYZ = 13,
    DISTANCE_XY_Z_XYZ = 14,
    DISTANCE_XY_Z_VIEW_XYZ = 15,
    LOOK_XYZ = 16,

    SPEED_X = 20,
    SPEED_Y = 21,
    SPEED_Z = 22,
    SPEED_XY = 23,
    SPEED_XYZ = 24,
    SPEED_FORWARD = 25,
    SPEED_SIDE = 26,
    SPEED_FORWARD_SIDE = 27,
    SPEED_XY_FORWARD_SIDE = 28,

    PITCH = 30,
    YAW = 31,
    ROLL = 32,
    POS_X = 33,
    POS_Y = 34,
    POS_Z = 35,
    VIEW_POS_X = 36,
    VIEW_POS_Y = 37,
    VIEW_POS_Z = 38,

    PITCH_YAW = 40,
    PLAYER_POS_XYZ = 41,
    PLAYER_POS_XYZ_PITCH_YAW = 42,
    VIEW_POS_XYZ_PITCH_YAW = 43,
    POS_XYZ = 44,
    VIEW_POS_XYZ = 45,
    ANGLES_XYZ = 46,
    VELOCITY_XYZ = 47,

    JUMP_XYZ = 50,
    PLANE_ANGLE_Z = 53,
    LAST_JUMP_SPEED = 55,

    UPMOVE_PRE_DELAY = 70,
    UPMOVE_POST_DELAY = 71,
    UPMOVE_FULL_DELAY = 72,
    UPMOVE_PRE_FULL_POST_DELAY = 73,
    UPMOVE_POST_FULL_PRE_DELAY = 74,
  };

  enum class StatOpts {
    TRACE = 1 << 0,
    EXTRA_TRACE = 1 << 1,
  };

  struct StatData {
    std::function<std::string()> fn;
    std::string name;
    std::string description;
    EnumBitset<StatOpts> opts;
  };

  void setupListeners();
  void setupObjects();
  void setupStats();
  void setZOffset(const vmCvar_t *cvar);

  void updateState(CHSObject &chsObject);
  static bool statNeedsExtraTrace(Stats stat);

  void viewTrace(trace_t *tr, int32_t mask);

  void printInfo() const;

  std::map<int32_t, CHSObject> chsObjects;

  const playerState_t *ps{};
  trace_t trace{};
  trace_t extraTrace{}; // trace results with 'etj_extraTrace'
  float ZOffset{};      // std::abs(ps->mins[2])

  enum class SpeedType {
    X = 0,
    Y = 1,
    Z = 2,
    XY = 3,
    XYZ = 4,
    FORWARD = 5,
    SIDE = 6,
    FORWARD_SIDE = 7,
    XY_FORWARD_SIDE = 8,
  };

  enum class DistanceType {
    XY = 0,
    Z = 1,
    XYZ = 2,
    VIEW_XYZ = 3,
    XY_Z_XYZ = 4,
    XY_Z_VIEW_XYZ = 5,
  };

  enum class PositionType {
    X = 0,
    Y = 1,
    Z = 2,
    VIEW_X = 3,
    VIEW_Y = 4,
    VIEW_Z = 5,
  };

  enum class UpmoveType {
    PRE_DELAY = 0,
    POST_DELAY = 1,
    FULL_DELAY = 2,
    PRE_FULL_POST_DELAY = 3,
    POST_FULL_PRE_DELAY = 4,
  };

  [[nodiscard]] std::string speed(SpeedType type) const;
  [[nodiscard]] std::string health() const;
  static std::string ammo();
  std::string distance(DistanceType type);
  std::string lookXYZ();
  [[nodiscard]] std::string angle(int32_t angle) const;
  [[nodiscard]] std::string position(PositionType type) const;
  [[nodiscard]] std::string lastJumpPos() const;
  std::string planeAngleZ();
  [[nodiscard]] std::string lastJumpSpeed() const;
  std::string upmove(UpmoveType type);

  trace_t &getTraceResults(extraTraceOptions opt);

  std::map<Stats, StatData> stats;

  std::shared_ptr<UpmoveMeterData> upMoveMeterData;
  std::shared_ptr<CvarUpdateHandler> cvarUpdateHandler;
  std::shared_ptr<ClientCommandsHandler> consoleCommandsHandler;
};
} // namespace ETJump
