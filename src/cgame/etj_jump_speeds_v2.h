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

#include <deque>
#include <memory>

#include "etj_cvar_parser.h"
#include "etj_irenderable.h"

#include "../game/etj_shared.h"
#include "../game/bg_public.h"

namespace ETJump {
class EntityEventsHandler;
class PlayerEventsHandler;
class ClientCommandsHandler;
class CvarUpdateHandler;

class JumpSpeedsV2 : public IRenderable {
public:
  JumpSpeedsV2(const std::shared_ptr<EntityEventsHandler> &entityEvents,
               const std::shared_ptr<PlayerEventsHandler> &playerEvents,
               const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
               const std::shared_ptr<ClientCommandsHandler> &serverCommands,
               const std::shared_ptr<CvarUpdateHandler> &cvarUpdate);
  ~JumpSpeedsV2() override;

  bool beforeRender() override;
  void render() const override;

private:
  enum class SpeedRelation : uint8_t {
    NEUTRAL = 0,
    FASTER = 1,
    SLOWER = 2,
  };

  struct Jump {
    explicit Jump(int32_t speed) : speed(speed) {
      this->speedStr = std::to_string(speed);
    }

    int32_t speed;
    // store a string representation as well so we don't need to perform
    // int -> string conversions every frame for all jumps when rendering
    std::string speedStr;
    SpeedRelation relation{};
  };

  void startListeners();
  static void parseColor(const std::string &colorStr, vec4_t &color);
  void adjustTextSize(const vmCvar_t &cvar);
  void computeTextOffsets();

  void updateJumpSpeeds();
  void setSpeedRelation();
  void setJumpColor(const Jump &jump, vec4_t color) const;

  [[nodiscard]] bool canSkipDraw() const;

  static constexpr char LABEL_TEXT[] = "Jump Speeds: ";

  enum class Style {
    HORIZONTAL = 1,
    NO_LABEL = 2,
    REVERSED = 4,
  };

  std::deque<Jump> jumpSpeeds;
  int32_t maxJumps{};
  int32_t jumpsPerColumn{};
  int32_t jumpsPerRow{};
  EnumBitset<Style> style;

  int32_t textStyle{};
  float textYAdjust{};         // y adjustment for non-default text size
  float rowHeight{};           // row height on vertical layout
  float textOffsetX{};         // gap between columns on vertical layout,
                               // gap between jump speeds on horizontal layout
  float firstJumpHorOffsetX{}; // x offset for first jump on horizontal layout
  CvarValue::Size size{};

  bool resetQueued{};

  vec4_t colorBase{};
  vec4_t colorFaster{};
  vec4_t colorSlower{};

  team_t team{};
  int32_t clientNum{};
  const playerState_t *ps{};

  std::shared_ptr<EntityEventsHandler> entityEvents;
  std::shared_ptr<PlayerEventsHandler> playerEvents;
  std::shared_ptr<ClientCommandsHandler> consoleCommands;
  std::shared_ptr<ClientCommandsHandler> serverCommands;
  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
};
} // namespace ETJump
