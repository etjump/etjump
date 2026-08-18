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

#include "etj_cvar_parser.h"
#include "etj_irenderable.h"
#include "etj_pmove_utils_v2.h"
#include "etj_snaphud_data.h"

namespace ETJump {
class SnaphudData;
class CvarUpdateHandler;

class SnaphudV2 : public IRenderable {
public:
  SnaphudV2(const std::shared_ptr<SnaphudData> &snaphudData,
            const std::shared_ptr<CvarUpdateHandler> &cvarUpdate);
  ~SnaphudV2() override;

  bool beforeRender() override;
  void render() const override;

private:
  void parseCropOffset(const vmCvar_t *cvar);
  void updateSnaphud(const SnaphudData::State &s);
  void buildSnapZones(const SnaphudData::State &s);

  void startListeners();

  [[nodiscard]] bool canSkipDraw(const SnaphudData::State &s) const;

  enum class SnapTrueness { UPMOVE = 1, GROUND = 2 };

  enum class SnaphudStyle {
    OFF = 0,
    NORMAL = 1,
    EDGE = 2,
    BORDER = 3,
  };

  enum class CropStyle {
    OFF = 0,
    IF_STRAFING = 1,
    ALWAYS = 2,
  };

  struct Snapzone {
    float start;
    float end;
    bool alt;
    bool active;
  };

  struct Snaphud {
    float yaw;
    float y;
    float h;
    float fov;

    SnaphudStyle style;

    bool isCurrentAlt;
    bool borderOnly;
    float borderThickness;
    float edgeThickness;

    // val1 = center, val2 = edge
    CvarValue::Pair cropOffset;
    std::optional<float> minX;
    std::optional<float> maxX;

    std::vector<Snapzone> zones;
    std::array<vec4_t, 4> colors;
  };

  Snaphud snaphud{};

  int32_t lastUpdateTime{};

  std::shared_ptr<SnaphudData> snaphudData;
  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
};
} // namespace ETJump
