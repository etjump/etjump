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
#include <vector>

#include "etj_irenderable.h"
#include "cg_local.h"

namespace ETJump {
class CvarUpdateHandler;

class Snaphud : public IRenderable {
public:
  static constexpr float INVALID_SNAP_DIR = -9999;
  struct CurrentSnap {
    float snap; // next snap in strafe dir, INVALID_SNAP_DIR if not available
    float yaw;  // view yaw
    float opt;  // optimal angle
    bool rightStrafe; // whether turning right
  };

  void render() const override;
  bool beforeRender() override;
  static CurrentSnap getCurrentSnap(const playerState_t &ps, const pmove_t *pm,
                                    bool upmoveTrueness);
  static bool inMainAccelZone(const playerState_t &ps, const pmove_t *pm);

  explicit Snaphud(const std::shared_ptr<CvarUpdateHandler> &cvarUpdate);
  ~Snaphud() override;

private:
  bool canSkipDraw() const;
  void InitSnaphud(vec3_t wishvel, int8_t uCmdScale);
  void UpdateMaxSnapZones();
  void UpdateSnapState();
  void PrepareDrawables();
  void startListeners();

  enum class SnapTrueness { SNAP_JUMPCROUCH = 1, SNAP_GROUND = 2 };

  enum class HudType {
    SNAP_OFF = 0,
    SNAP_NORMAL = 1,
    SNAP_EDGE = 2,
    SNAP_BORDER = 3,
  };

  struct snaphud_t {
    float a = 0.0f;
    unsigned char maxAccel;
    std::vector<unsigned short> zones;
    std::vector<unsigned char> xAccel;
    std::vector<unsigned char> yAccel;
    std::vector<float> absAccel;
    float minAbsAccel;
    float maxAbsAccel;
  };

  snaphud_t snap;

  int yaw{};
  vec4_t snaphudColors[4]{};
  int edgeThickness{};
  float borderThickness{};
  HudType hudType{};
  bool borderOnly{};
  int lastUpdateTime{};

  const playerState_t *ps = &cg.predictedPlayerState;
  const pmove_t *pm{};

  struct DrawableSnap {
    int bSnap;
    int eSnap;
    bool alt;
    bool active;
  };

  std::vector<DrawableSnap> drawableSnaps;
  bool isCurrentAlt{};

  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
};
} // namespace ETJump
