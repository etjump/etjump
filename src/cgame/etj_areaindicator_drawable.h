/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
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

#include <vector>

#include "etj_irenderable.h"
#include "cg_local.h"

namespace ETJump {
class AreaIndicator : public IRenderable {
  bool canSkipDraw();
  void checkPronePrint(trace_t &trace);

  enum class DrawMode {
    Always = 1,
    Outside = 2,
    Inside = 3,
  };

  struct Indicator {
    Indicator(vmCvar_t *controlCvar, vmCvar_t *controlCvarX,
              vmCvar_t *controlCvarY, int contents, int shared,
              qhandle_t iconShader);

    vmCvar_t *cvar;
    bool draw;
    bool drawForbid;
    vmCvar_t *cvarX;
    vmCvar_t *cvarY;
    float x;
    float y;
    int traceContents;
    int sharedValue;
    qhandle_t shader;
  };

  std::vector<Indicator> indicators;

  playerState_t *ps;
  clientInfo_t *ci;
  qhandle_t *forbidShader;
  int pronePressTime;
  bool printProneMessage;

  // color for the base icons
  const vec4_t color = {0.88f, 0.88f, 0.88f, 1.f};
  static constexpr float size = 20.0f;

public:
  AreaIndicator();
  ~AreaIndicator() override = default;

  bool beforeRender() override;
  void render() const override;
};
} // namespace ETJump
