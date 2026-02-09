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
#include "etj_cvar_update_handler.h"

#include "etj_chs_data.h"

namespace ETJump {
class CHS : public IRenderable {
public:
  CHS(const std::shared_ptr<CvarUpdateHandler> &cvarUpdateHandler,
      const std::shared_ptr<CHSDataHandler> &dataHandler);
  ~CHS() override;

  bool beforeRender() override;
  void render() const override;

private:
  void setupListeners();
  static bool canSkipDraw();

  void setColor(const vmCvar_t *cvar);
  void setAlpha(const vmCvar_t *cvar);
  bool infoUsesExtraTrace(const vmCvar_t *cvar);

  void drawCHS1() const;
  void drawCHS2() const;

  float x1{}; // CHS 1
  float y1{}; // CHS 1
  float x2{}; // CHS 2
  float y2{}; // CHS 2
  int32_t CHS2Align{};
  int32_t textStyle{};
  vec4_t color{};

  struct CHS1PosInfo {
    float offsetX;
    float offsetY;
    int32_t align;
  };

  // lookup table for drawing CHS1 infos at correct positions
  //      1
  //   8     2
  // 7    x    3
  //   6     4
  //      5
  static constexpr std::array<CHS1PosInfo, MAX_CHS_INFO> CHS1Positions = {{
      {0, -20, ITEM_ALIGN_CENTER},
      {10, -10, ITEM_ALIGN_LEFT},
      {20, 0, ITEM_ALIGN_LEFT},
      {10, 10, ITEM_ALIGN_LEFT},
      {0, 20, ITEM_ALIGN_CENTER},
      {-10, 10, ITEM_ALIGN_RIGHT},
      {-20, 0, ITEM_ALIGN_RIGHT},
      {-10, -10, ITEM_ALIGN_RIGHT},
  }};

  std::shared_ptr<CvarUpdateHandler> cvarUpdateHandler;
  std::shared_ptr<CHSDataHandler> data;
};
} // namespace ETJump
