/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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

#include "etj_irenderable.h"
#include "cg_local.h"

namespace ETJump {
class CGaz : public IRenderable {
public:
  static bool strafingForwards(const playerState_t &ps, pmove_t *pm);
  static float getOptAngle(const playerState_t &ps, pmove_t *pm);

  CGaz();
  ~CGaz() override = default;

  void render() const override;
  void beforeRender() override;

private:
  enum class CGazTrueness { CGAZ_JUMPCROUCH = 1, CGAZ_GROUND = 2 };

  struct state_t {
    float gSquared;  // 0 when not on slick
    float vSquared;  // previous_velocity squared
    float vfSquared; // velocity squared
    float aSquared;  // accel squared

    float v;  // previous_velocity
    float vf; // velocity
    float a;  // accel

    float wishspeed;
  };

  state_t state{};

  float drawMin{};
  float drawOpt{};
  float drawMaxCos{};
  float drawMax{};
  float drawVel{};
  float yaw{};
  vec4_t CGaz1Colors[4]{};
  vec4_t CGaz2Colors[2]{};

  bool canSkipDraw() const;
  void UpdateCGaz1(vec3_t wishvel, int8_t uCmdScale, usercmd_t cmd);
  void UpdateCGaz2();
  float GetSlickGravity();
  void UpdateDraw(float wishspeed, float accel);
  static float UpdateDrawMin(state_t const *state);
  static float UpdateDrawOpt(state_t const *state);
  static float UpdateDrawMaxCos(state_t const *state, float d_opt);
  static float UpdateDrawMax(state_t const *state, float d_max_cos);
  void startListeners();

  playerState_t *ps = &cg.predictedPlayerState;
  pmove_t *pm{};
};
} // namespace ETJump
