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

#include <optional>

#include "cg_local.h"

namespace ETJump {
class PmoveUtils {
public:
  PmoveUtils();
  ~PmoveUtils() = default;
  bool check() const;
  void runPmove();

  // returns either sprintSpeedScale or runSpeedScale
  float getSprintScale() const;

  // calculates wishspeed projected onto flat ground plane
  float getWishspeed(vec3_t wishvel, float scale, vec3_t forward, vec3_t right,
                     vec3_t up) const;

  // updates XY wishvel based on cmdScale and angle vectors
  // projects velocity down to a flat ground plane
  // Z vector is taken as input for AngleVectors
  // this takes in usercmd_t instead of using the class member variable,
  // so cgaz & snaphud can send in a "fake" usercmd for drawing
  // if there's no user input
  void updateWishvel(vec3_t wishvel, vec3_t forward, vec3_t right, vec3_t up,
                     const usercmd_t &ucmd) const;

  // returns total acceleration per frame
  float getFrameAccel(bool upmoveTrueness);

  // true if current time is < lastUpdateTime + pmove_msec,
  // except if lerping is requested for the HUD element
  // updates lastUpdateTime if the frame is not skipped
  bool skipUpdate(int &lastUpdateTime, std::optional<HUDLerpFlags> flag) const;

  [[nodiscard]] const pmove_t *getPmove() const;
  [[nodiscard]] const usercmd_t *getUserCmd() const;
  [[nodiscard]] int8_t getUserCmdScale() const;

private:
  void initCvars();
  void setupCallbacks();
  void setPmoveStatus();
  void setupUserCmd();
  void setupPmove();

  pmove_t pm{};
  pmoveExt_t pmext{};
  usercmd_t cmd{};
  playerState_t ps{};

  // a lot of drawables need this so make it accessible here since
  // we need to calculate it here to set up usercmd anyway
  int8_t uCmdScale{};

  bool doPmove{};
  std::vector<const vmCvar_t *> cvars{};
};
} // namespace ETJump
