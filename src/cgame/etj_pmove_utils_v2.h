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

#include <memory>
#include <optional>
#include <vector>

#include "etj_local.h"

#include "../game/bg_public.h"
#include "../game/bg_local.h"
#include "../game/etj_shared.h"

namespace ETJump {
class CvarUpdateHandler;

class PmoveUtilsV2 {
public:
  static constexpr float PM_FRAMETIME = 0.008f;

  enum class PmoveSingleResult {
    NONE = 0,
    LADDER = 1,
    WATER = 2,
    MOUNTED = 3,
    WALK = 4,
    AIR = 5,
  };

  enum class PmoveDefaultInput {
    NONE = 0,
    FORWARD = 1 << 0,
    SIDE = 1 << 2,
    UP = 1 << 3,
    SPRINT = 1 << 4,
  };

  struct State {
    float vfSquared; // velocity final squared (after friction)
    float vf;        // velocity final (after friction)
    float a;         // accel

    vec2_t wishvel;
    float wishspeed;
    float velAngle;
    float optAngle;

    PmoveSingleResult result;

    playerState_t ps;
    pmove_t pm;
    pmoveExt_t pmext;
    pml_t pml;
  };

  explicit PmoveUtilsV2(const std::shared_ptr<CvarUpdateHandler> &cvarUpdate);
  ~PmoveUtilsV2();

  static void setupPmove(pmove_t &pm);
  static void setupUserCmd(int8_t scale, pmove_t &pm);
  [[nodiscard]] bool check() const;
  void runFrame();

  [[nodiscard]] const State &getState() const;

  static PmoveSingleResult
  pmoveSingle(pmove_t &pm, pml_t &pml,
              const EnumBitset<PmoveDefaultInput> &defaultInput);

  static void setWaterLevel(pmove_t &pm);
  static bool checkProne(pmove_t &pm);
  static void checkDuck(pmove_t &pm);
  static void groundTrace(pmove_t &pm, pml_t &pml);
  static void checkLadderMove(pmove_t &pm, pml_t &pml);
  static bool checkJump(pmove_t &pm, pml_t &pml, bool isLerpFrame);
  static void sprint(pmove_t &pm);
  static void updateWishvel(vec2_t wishvel, const pmove_t &pm,
                            const pml_t &pml);
  static float cmdScale(const pmove_t &pm, const usercmd_t &cmd, bool upmove);

  // "forwards" means player viewangles naturally match keys pressed,
  // i.e. looking ahead with +forward and looking backwards with +back,
  // except for nobeat: looking to the left with +forward only and
  // looking to the right with +back only
  static bool strafingForwards(const pmove_t &pm, float wishspeed,
                               const vec2_t wishvel);

  static bool rightStrafe(bool forwards, const usercmd_t &cmd);

  static bool skipUpdate(int32_t &lastUpdateTime,
                         std::optional<HUDLerpFlags> flag, const pmove_t &pm);

  // 'cg.predictedPlayerState' is considered accurate if:
  // - we are not following anyone
  // - we are in demo playback (not following), and the snapshot intervals
  //   match 'pmove_msec', meaning demo was recorded at 'sv_fps/snaps 125'
  //
  // In any other scenario, we cannot fully trust it, because it has some form
  // of interpolation applied to it - most notably, velocity isn't snapped,
  // so we cannot properly measure for example acceleration.
  // TODO: all code that uses this needs to be revised once
  // 'g_synchronousClients' is implemented, because we no longer use
  // predicted playerstate with that, but rather 'cg.snap->ps'
  static bool ppsIsAccurate(const pmove_t &pm);

private:
  void initCvars();
  void setupCallbacks();
  void setPmoveStatus();

  void updateState(float wishspeed, float accel);

  void walkMove();
  void airMove();
  void accelerate(float wishspeed, float accel);

  static bool canProne(const pmove_t &pm);
  static void groundTraceMissed(pmove_t &pm, pml_t &pml);
  static void traceAll(trace_t &trace, vec3_t start, vec3_t end,
                       const pmove_t &pm);
  static void traceAllLegs(trace_t &trace, float *legsOffset, vec3_t start,
                           vec3_t end, const pmove_t &pm);
  static bool correctAllSolid(trace_t &trace, pmove_t &pm, pml_t &pml);
  static void
  setDefaultInput(pmove_t &pm, int8_t scale,
                  const EnumBitset<PmoveDefaultInput> &defaultInput);

  State s{};

  bool doPmove{};
  std::vector<const vmCvar_t *> cvars;

  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
};
} // namespace ETJump
