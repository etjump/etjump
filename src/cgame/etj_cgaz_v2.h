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

#include <array>
#include <memory>
#include <optional>
#include <string>

#include "etj_cgaz_data.h"
#include "etj_irenderable.h"
#include "../game/etj_shared.h"

namespace ETJump {
class CvarUpdateHandler;
class SnaphudData;

class CGazV2 : public IRenderable {
public:
  CGazV2(const std::shared_ptr<CGazData> &cgazData,
         const std::shared_ptr<SnaphudData> &snaphudData,
         const std::shared_ptr<CvarUpdateHandler> &cvarUpdate);
  ~CGazV2() override;

  bool beforeRender() override;
  void render() const override;

  [[nodiscard]] std::array<std::string,
                           Constants::SpectatorHudSync::CgazCvarNames.size()>
  getHudSyncValues() const;

private:
  void updateCGaz1(const CGazData::State &s);
  void updateCGaz2(const CGazData::State &s);
  void updateDrawSnap(const CGazData::State &s);

  static float updateMinAngle(const CGazData::State &s);
  static float updateOptAngle(const CGazData::State &s);
  static float updateMaxCosAngle(const CGazData::State &s, float angleOpt);
  static float updateMaxAngle(const CGazData::State &s, float angleMaxCos);

  void startListeners();
  void updateColors();
  void setThickness(const vmCvar_t *cvar);

  void renderCGaz1() const;
  void renderCGaz2() const;

  bool canSkipDraw(const CGazData::State &s) const;
  void refreshRenderSettings();

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

    std::optional<float> drawSnap; // etj_CGaz1DrawSnapZone

    std::array<vec4_t, 4> colors;
    vec4_t midlineColor;
  };

  CGaz1 cgaz1{};
  std::array<std::string, 4> cgaz1ColorStrings{};
  std::string cgaz1MidlineColorString;

  struct CGaz2 {
    float velAngle;
    float optAngle;
    float velSize;

    float y;

    int8_t forwardmove;
    int8_t rightmove;

    bool highRes;
    bool drawSides;

    std::array<vec4_t, 4> colors;
    std::array<float, 2> thickness;
  };

  CGaz2 cgaz2{};
  std::array<std::string, 2> cgaz2ColorStrings{};

  struct RenderSettings {
    int drawCgaz{};
    bool drawSnapZone{};
    bool drawMidLine{};
    float cgazY{};
    float cgazHeight{};
    float cgazFov{};
    float cgaz2Y{};
    float cgaz2FixedSpeed{};
    bool cgaz2HighRes{};
    float cgaz2Thickness1{};
    float cgaz2Thickness2{};
    bool stretchCgaz{};
    float cgaz2WishDirFixedSpeed{};
    bool cgaz2WishDirUniformLength{};
    int cgaz2NoVelocityDir{};
  } settings;

  int32_t lastUpdateTime{};

  std::shared_ptr<CGazData> cgazData;
  std::shared_ptr<SnaphudData> snaphudData;
  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
};
} // namespace ETJump
