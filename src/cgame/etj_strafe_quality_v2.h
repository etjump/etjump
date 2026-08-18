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

#include "etj_cvar_parser.h"
#include "etj_irenderable.h"
#include "etj_pmove_utils_v2.h"

#include "../game/q_shared.h"

namespace ETJump {
class SnaphudData;
class CvarUpdateHandler;
class ClientCommandsHandler;
class PlayerEventsHandler;

class StrafeQualityV2 : public IRenderable {
public:
  StrafeQualityV2(const std::shared_ptr<SnaphudData> &snaphudData,
                  const std::shared_ptr<CvarUpdateHandler> &cvarUpdate,
                  const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
                  const std::shared_ptr<PlayerEventsHandler> &playerEvents);
  ~StrafeQualityV2() override;

  bool beforeRender() override;
  void render() const override;

private:
  void startListeners();
  void setSize(const vmCvar_t &cvar);
  void resetStrafeQuality();

  static float updateOptAngle(const PmoveUtilsV2::State &s);
  void updateStrafeQuality(const PmoveUtilsV2::State &s);

  static bool canSkipUpdate(const PmoveUtilsV2::State &s);
  [[nodiscard]] bool canSkipDraw() const;

  enum class Style {
    FULL = 0,
    PERCENT = 1,
    NUMBER = 2,
  };

  uint32_t totalFrames{};
  double score{};
  double strafeQuality{};
  float oldSpeed{};

  float optAngle{};

  Style style{};
  int32_t textStyle{};

  float x{};
  float y{};
  CvarValue::Size size{};

  team_t team{};
  vec4_t color{};

  int32_t lastUpdateTime{};

  std::shared_ptr<SnaphudData> snaphudData;
  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
  std::shared_ptr<ClientCommandsHandler> consoleCommands;
  std::shared_ptr<PlayerEventsHandler> playerEvents;
};
} // namespace ETJump
