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

#include "etj_irenderable.h"
#include "etj_pmove_utils_v2.h"
#include "../game/bg_local.h"

namespace ETJump {
class CvarUpdateHandler;

class CGazV2 : public IRenderable {
public:
  explicit CGazV2(const std::shared_ptr<CvarUpdateHandler> &cvarUpdate);
  ~CGazV2() override;

  bool beforeRender() override;
  void render() const override;

private:
  void updateCGazState(float wishspeed, float accel, float slickGravity);
  void updateCGaz1();
  void updateCGaz2();

  [[nodiscard]] float updateMinAngle() const;
  [[nodiscard]] float updateOptAngle() const;
  [[nodiscard]] float updateMaxCosAngle(float angleOpt) const;
  [[nodiscard]] float updateMaxAngle(float angleMaxCos) const;

  void walkMove();
  void airMove();
  void friction() const;
  void accelerate(float wishspeed, float accel, bool slick);

  void startListeners();
  void setThickness(const vmCvar_t *cvar);
  void setDefaultInput();

  [[nodiscard]] bool canSkipDraw() const;

  enum class CGazTrueness { UPMOVE = 1, GROUND = 2 };

  struct State {
    float gSquared;  // gravity squared, 0 when not on slick
    float vSquared;  // velocity squared (before friction)
    float vfSquared; // velocity final squared (after friction)
    float aSquared;  // accel squared

    float v;  // velocity (before friction)
    float vf; // velocity final (after friction)
    float a;  // accel

    vec2_t wishvel;
    float wishspeed;
    float velAngle;
  };

  State s{};

  struct CGaz1 {
    float minAngle;
    float optAngle;
    float maxCosAngle;
    float maxAngle;

    float midlineStart;
    float midLineEnd;

    float yaw;
    float y;
    float h;
    float fov;

    std::array<vec4_t, 4> colors;
    vec4_t midlineColor;
  };

  CGaz1 cgaz1{};

  struct CGaz2 {
    float velAngle;
    float velSize;

    float y;

    bool highRes;
    bool drawSides;

    std::array<vec4_t, 4> colors;
    std::array<float, 2> thickness;
  };

  CGaz2 cgaz2{};

  EnumBitset<PmoveUtilsV2::PmoveDefaultInput> defaultInput;

  playerState_t ps{};
  pmove_t pm{};
  pmoveExt_t pmext{};
  pml_t pml{};
  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
};
} // namespace ETJump
