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

#include "cg_local.h"

namespace ETJump {
class PmoveUtils {
public:
  // returns real userCmd for players and a faked
  // one for spectators/demo playback
  static usercmd_t getUserCmd(const playerState_t &ps, int8_t uCmdScale);

  // returns cg_pmove for players or runs PmoveSingle again
  // for spectators/demo playback to get correct values for pmext
  static pmove_t *getPmove(usercmd_t cmd);

  // returns either sprintSpeedScale or runSpeedScale
  static float PM_SprintScale(const playerState_t *ps);

  // calculates wishspeed projected onto flat ground plane
  static float PM_GetWishspeed(vec3_t wishvel, float scale, usercmd_t cmd,
                               vec3_t forward, vec3_t right, vec3_t up,
                               const playerState_t &ps, const pmove_t *pm);

  // updates XY wishvel based on cmdScale and angle vectors
  // projects velocity down to a flat ground plane
  // Z vector is taken as input for AngleVectors
  static void PM_UpdateWishvel(vec3_t wishvel, usercmd_t cmd, vec3_t forward,
                               vec3_t right, vec3_t up,
                               const playerState_t &ps);

  // returns total acceleration per frame
  static float getFrameAccel(const playerState_t &ps, const pmove_t *pm);

  // if an update should happen, updates lastUpdateTime to current frametime
  // and returns false
  static bool skipUpdate(int &lastUpdateTime, const pmove_t *pm,
                         const playerState_t *ps);
};
} // namespace ETJump
