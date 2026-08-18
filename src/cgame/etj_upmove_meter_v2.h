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
#include "etj_cvar_parser.h"
#include "etj_pmove_utils_v2.h"

#include "../ui/ui_shared.h"

namespace ETJump {
class UpmoveMeterData;
class CvarUpdateHandler;

class UpmoveMeterV2 : public IRenderable {
public:
  UpmoveMeterV2(const std::shared_ptr<UpmoveMeterData> &upmoveMeterData,
                const std::shared_ptr<CvarUpdateHandler> &cvarUpdate);
  ~UpmoveMeterV2() override;

  bool beforeRender() override;
  void render() const override;

private:
  void startListeners();
  void setTextSize(const vmCvar_t &cvar);

  static bool canSkipDraw();

  struct Graph {
    int32_t preDelay;
    int32_t postDelay;
    int32_t fullDelay;
    int32_t absMaxDelay;

    rectDef_t rect;
    float upHeight;
    float downHeight;

    vec4_t colorBg;
    vec4_t colorPreJump;
    vec4_t colorPostJump;
    vec4_t colorOnGround;
    vec4_t colorOutline;
  };

  Graph graph{};

  float textX{};
  float textH{};
  float textHeightOffset{};
  CvarValue::Size textSize{};
  int32_t textStyle{};
  vec4_t colorText{};

  std::shared_ptr<UpmoveMeterData> upmoveMeterData;
  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
};
} // namespace ETJump
