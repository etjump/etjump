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

#include "cg_local.h"
#include "etj_irenderable.h"
#include "etj_entity_events_handler.h"

namespace ETJump {
class JumpSpeeds : public IRenderable {
public:
  explicit JumpSpeeds(EntityEventsHandler *entityEventsHandler);
  ~JumpSpeeds();

  void render() const override;
  bool beforeRender() override;

private:
  EntityEventsHandler *_entityEventsHandler;
  static const int MAX_JUMPS = 10;
  std::vector<std::pair<int, std::string>>
      jumpSpeeds;              // jumpspeeds + color string
  vec4_t jumpSpeedsColors[10]; // jumpspeed colors parsed to vec4_t for
                               // drawing
  std::string baseColorStr = etj_jumpSpeedsColor.string;
  std::string fasterColorStr = etj_jumpSpeedsFasterColor.string;
  std::string slowerColorStr = etj_jumpSpeedsSlowerColor.string;
  bool resetQueued = false;
  int team = 0;

  enum class jumpSpeedStyle { Horizontal = 1, NoLabel = 2, Reversed = 4 };

  void updateJumpSpeeds();
  void queueJumpSpeedsReset();
  void adjustColors();
  void colorStrToVec();
  bool canSkipDraw() const;
};
} // namespace ETJump
