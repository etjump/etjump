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

#include "cg_local.h"

namespace ETJump {
class CvarUpdateHandler;

class PlayerBBox {
  struct BBox {
    vec3_t mins{};
    vec3_t maxs{};
    polyVert_t verts[4]{};
    vec4_t color{};
  };

  enum class DrawFlags {
    Self = 1,
    Others = 2,
    Fireteam = 4,
  };

  enum class PlayerType {
    Self = 1,
    Other = 2,
    Fireteam = 3,
  };

  qhandle_t shader;
  vec4_t colorSelf{};
  vec4_t colorOther{};
  vec4_t colorFireteam{};

  std::shared_ptr<CvarUpdateHandler> cvarUpdate;

  // there's no real easy way to determine these for other players,
  // both crouch and prone use ps.crouchMaxZ for maxs[2] but it's calculated
  // differently based off stance, and since the result is not stored
  // in entitystate, it's simpler to just use hardcoded values
  // these are always correct anyway, there's no variation in any scenario

  static constexpr int CROUCH_MAXS_OFFSET_Z = 24;
  static constexpr int PRONE_MAXS_OFFSET_Z = 32;

  void setupListeners();
  static void setupBBoxExtents(const centity_t *cent, BBox &box);
  static bool bottomOnly(const int &pType);
  static void setBBoxAlpha(const centity_t *cent, BBox &box);
  bool canSkipDraw(const centity_t *cent, const clientInfo_t *ci) const;

public:
  void drawBBox(const clientInfo_t *ci, const centity_t *cent) const;

  explicit PlayerBBox(const std::shared_ptr<CvarUpdateHandler> &cvarUpdate);
  ~PlayerBBox();
};
} // namespace ETJump
