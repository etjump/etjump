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

  static void setupPmove(pmove_t &pm);
  static void setupUserCmd(int8_t scale, pmove_t &pm);

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
  static void updateWishvel(vec2_t &wishvel, pmove_t &pm, pml_t &pml);
  static float cmdScale(const pmove_t &pm, const usercmd_t &cmd, bool upmove);

private:
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
};
} // namespace ETJump
