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

#include "etj_cgaz_data.h"
#include "etj_irenderable.h"

namespace ETJump {
class CvarUpdateHandler;

class CGazV2 : public IRenderable {
public:
  CGazV2(const std::shared_ptr<CGazData> &cgazData,
         const std::shared_ptr<CvarUpdateHandler> &cvarUpdate);
  ~CGazV2() override;

  bool beforeRender() override;
  void render() const override;

private:
  void updateCGaz1(const CGazData::State &s);
  void updateCGaz2(const CGazData::State &s);
  void updateDrawSnap(const CGazData::State &s);

  static float updateMinAngle(const CGazData::State &s);
  static float updateOptAngle(const CGazData::State &s);
  static float updateMaxCosAngle(const CGazData::State &s, float angleOpt);
  static float updateMaxAngle(const CGazData::State &s, float angleMaxCos);

  void startListeners();
  void setThickness(const vmCvar_t *cvar);

  [[nodiscard]] bool canSkipDraw(const CGazData::State &s) const;

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

    int8_t forwardmove;
    int8_t rightmove;

    bool highRes;
    bool drawSides;

    std::array<vec4_t, 4> colors;
    std::array<float, 2> thickness;
  };

  CGaz2 cgaz2{};

  std::shared_ptr<CGazData> cgazData;
  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
};
} // namespace ETJump
