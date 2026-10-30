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
#include "../game/q_shared.h"

namespace ETJump {
class FalldamageDetector : public IRenderable {
public:
  FalldamageDetector() = default;
  ~FalldamageDetector() override = default;

  bool beforeRender() override;
  void render() const override;

private:
  [[nodiscard]] bool canSkipDraw() const;
  static int32_t computeFallDamage(float zVel, float startHeight,
                                   float endHeight, float gravity,
                                   int32_t waterlevel);
  [[nodiscard]] std::string buildDamageString(int32_t damage) const;
  static bool wouldTakeFallDamage(const trace_t &trace);

  int32_t damageBelow{};
  int32_t damageFall{};
  int32_t damageJump{};
  const playerState_t *ps{};
};
} // namespace ETJump
