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

#include "etj_irenderable.h"
#include "cg_local.h"
#include "etj_cvar_parser.h"

namespace ETJump {
class UpmoveMeter : public IRenderable {
  typedef enum {
    AIR_NOJUMP,       // player is midair, not holding the jump button
    AIR_JUMP,         // player is midair, holding jump button
    GROUND_JUMP,      // player is on the ground, holding jump button
    GROUND_NOJUMP,    // player is on the ground, not holding jump
                      // button
    AIR_JUMPNORELEASE // player is midair, without releasing the
                      // jump button
  } state_t;

  typedef struct {
    // timestamps for computation
    unsigned int t_jumpPreGround;
    unsigned int t_groundTouch;

    // state machine
    state_t lastState;

    // draw data
    int postDelay;
    int preDelay;
    int fullDelay;

    mutable vec4_t graph_xywh;
    mutable vec2_t text_xh;

    mutable vec4_t graph_rgba;
    mutable vec4_t graph_rgbaPostJump;
    mutable vec4_t graph_rgbaOnGround;
    mutable vec4_t graph_rgbaPreJump;
    mutable vec4_t graph_outline_rgba;
    mutable vec4_t text_rgba;
  } jump_t;

  jump_t jump_{};

  int team_;

  // default absolute hud position
  static constexpr float graphX_ = 8.0f;
  static constexpr float graphY_ = 8.0f;

  CvarValue::Size textSize{};

  int lastUpdateTime{};
  const pmove_t *pm{};
  const playerState_t *ps = &cg.predictedPlayerState;

  void startListeners();
  void parseAllColors();
  void setTextSize();
  void resetUpmoveMeter();
  bool canSkipDraw() const;
  bool canSkipUpdate() const;

public:
  UpmoveMeter();
  bool beforeRender() override;
  void render() const override;
};
} // namespace ETJump
