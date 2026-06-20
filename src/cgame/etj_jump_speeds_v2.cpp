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

#include "etj_jump_speeds_v2.h"
#include "cg_local.h"
#include "etj_client_commands_handler.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_demo_compatibility.h"
#include "etj_entity_events_handler.h"
#include "etj_player_events_handler.h"
#include "etj_utilities.h"

inline constexpr size_t MAX_JUMPS = 10;

inline constexpr float POS_X = 6.0f;
inline constexpr float TEXT_MIN_SIZE = 0.1f;
inline constexpr float TEXT_MAX_SIZE = 10.0f;
inline constexpr float DEFAULT_TEXT_SIZE = 0.2f;

// base gap between speeds on horizontal layout
inline constexpr float BASE_OFFSET_X = 30.0f;
// base row height on vertical layout
inline constexpr float BASE_OFFSET_Y = 12.0f;
// offset for the first jump speed after label on horizontal layout
inline constexpr float HOR_FIRSTJUMP_OFFSET = 5.0f;

namespace ETJump {
JumpSpeedsV2::JumpSpeedsV2(
    const std::shared_ptr<EntityEventsHandler> &entityEvents,
    const std::shared_ptr<PlayerEventsHandler> &playerEvents,
    const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
    const std::shared_ptr<ClientCommandsHandler> &serverCommands,
    const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : entityEvents(entityEvents), playerEvents(playerEvents),
      consoleCommands(consoleCommands), serverCommands(serverCommands),
      cvarUpdate(cvarUpdate) {
  parseColor(etj_jumpSpeedsColor.string, colorBase);
  parseColor(etj_jumpSpeedsFasterColor.string, colorFaster);
  parseColor(etj_jumpSpeedsSlowerColor.string, colorSlower);

  adjustTextSize(etj_jumpSpeedsTextSize);
  startListeners();
}

JumpSpeedsV2::~JumpSpeedsV2() {
  entityEvents->unsubscribe(EV_JUMP);
  playerEvents->unsubscribe("respawn");

  consoleCommands->unsubscribe("resetJumpSpeeds");
  serverCommands->unsubscribe("resetJumpSpeeds");

  cvarUpdate->unsubscribe(&etj_jumpSpeedsColor);
  cvarUpdate->unsubscribe(&etj_jumpSpeedsFasterColor);
  cvarUpdate->unsubscribe(&etj_jumpSpeedsSlowerColor);
  cvarUpdate->unsubscribe(&etj_jumpSpeedsTextSize);
}

void JumpSpeedsV2::startListeners() {
  entityEvents->subscribe(EV_JUMP, [this](centity_t *) { updateJumpSpeeds(); });

  playerEvents->subscribe("respawn", [this](const std::vector<std::string> &) {
    resetQueued = true;
  });

  consoleCommands->subscribe(
      "resetJumpSpeeds",
      [this](const std::vector<std::string> &) { resetQueued = true; });

  serverCommands->subscribe(
      "resetJumpSpeeds",
      [this](const std::vector<std::string> &) { resetQueued = true; });

  cvarUpdate->subscribe(&etj_jumpSpeedsColor, [this](const vmCvar_t *cvar) {
    parseColor(cvar->string, colorBase);
  });

  cvarUpdate->subscribe(
      &etj_jumpSpeedsFasterColor,
      [this](const vmCvar_t *cvar) { parseColor(cvar->string, colorFaster); });

  cvarUpdate->subscribe(
      &etj_jumpSpeedsSlowerColor,
      [this](const vmCvar_t *cvar) { parseColor(cvar->string, colorSlower); });

  cvarUpdate->subscribe(&etj_jumpSpeedsTextSize, [this](const vmCvar_t *cvar) {
    adjustTextSize(*cvar);
  });
}

void JumpSpeedsV2::parseColor(const std::string &colorStr, vec4_t &color) {
  cgame.utils.colorParser->parseColorString(colorStr, color);
}

void JumpSpeedsV2::adjustTextSize(const vmCvar_t &cvar) {
  size = CvarValueParser::parse<CvarValue::Size>(cvar, TEXT_MIN_SIZE,
                                                 TEXT_MAX_SIZE);
  size.x *= 0.1f;
  size.y *= 0.1f;

  computeTextOffsets();
}

// Text drawing origin is the bottom left of the text, but we want text
// size adjustment to grow the text down/right, so calculate an offset based
// on the current size, so adjusting the text size works as expected.
// We could just shift the drawing down by the text height unconditionally
// and call it a day, but because jump speed text size was not always
// adjustable, it would cause slight mismatch on the position
// that players have set prior to it being possible.
void JumpSpeedsV2::computeTextOffsets() {
  const int32_t defaultTextHeight = CG_Text_Height_Ext(
      LABEL_TEXT, DEFAULT_TEXT_SIZE, 0, &cgs.media.limboFont2);
  const int32_t currentTextHeight =
      CG_Text_Height_Ext(LABEL_TEXT, size.y, 0, &cgs.media.limboFont2);

  textYAdjust = static_cast<float>(defaultTextHeight - currentTextHeight);
  textOffsetX = BASE_OFFSET_X * (size.x / DEFAULT_TEXT_SIZE);
  rowHeight = BASE_OFFSET_Y * (size.y / DEFAULT_TEXT_SIZE);

  firstJumpHorOffsetX = static_cast<float>(CG_Text_Width_Ext(
                            LABEL_TEXT, size.x, 0, &cgs.media.limboFont2)) +
                        (HOR_FIRSTJUMP_OFFSET * (size.x / DEFAULT_TEXT_SIZE));
}

void JumpSpeedsV2::updateJumpSpeeds() {
  // events are processed at playerstate transition before interpolation
  // runs, so we can't rely on 'cg.predictedPlayerState' on demo playback,
  // because it still contains previous playerstate at the time
  // 'EV_JUMP' event is processed
  ps = getValidPlayerState();

  // team switch resets the display
  if (team != ps->persistant[PERS_TEAM]) {
    resetQueued = true;
  }

  if (resetQueued) {
    jumpSpeeds.clear();
    resetQueued = false;
  }

  team = static_cast<team_t>(ps->persistant[PERS_TEAM]);

  if (cgame.demo.compatibility->flags.predictedJumpSpeeds) {
    jumpSpeeds.emplace_back(VectorLength2(ps->velocity), colorBase);
  } else {
    jumpSpeeds.emplace_back(ps->persistant[PERS_JUMP_SPEED], colorBase);
  }

  if (jumpSpeeds.size() > MAX_JUMPS) {
    jumpSpeeds.pop_front();
  }

  if (etj_jumpSpeedsShowDiff.integer && jumpSpeeds.size() > 1) {
    setDiffColor();
  }

  auto &current = jumpSpeeds[jumpSpeeds.size() - 1];

  if (etj_jumpSpeedsMinSpeed.integer > current.speed) {
    Vector4Copy(colorSlower, current.color);
  }
}

void JumpSpeedsV2::setDiffColor() {
  assert(jumpSpeeds.size() > 1);
  const size_t currentJump = jumpSpeeds.size() - 1;

  auto &current = jumpSpeeds[currentJump];
  const auto &prev = jumpSpeeds[currentJump - 1];

  // speeds are equal, no adjustment needed (base color is default)
  if (current.speed == prev.speed) {
    return;
  }

  if (current.speed > prev.speed) {
    Vector4Copy(colorFaster, current.color);
  } else {
    Vector4Copy(colorSlower, current.color);
  }
}

bool JumpSpeedsV2::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  textStyle = etj_jumpSpeedsShadow.integer ? ITEM_TEXTSTYLE_SHADOWED
                                           : ITEM_TEXTSTYLE_NORMAL;

  style.reset();
  style.set(static_cast<Style>(etj_jumpSpeedsStyle.integer));

  return true;
}

void JumpSpeedsV2::render() const {
  float x1 = std::clamp(POS_X + etj_jumpSpeedsX.value, 0.0f, 640.0f);
  float x2 = x1 + textOffsetX;
  ETJump_AdjustPosition(&x1);
  ETJump_AdjustPosition(&x2);

  float y1 = std::clamp(SCREEN_CENTER_Y + etj_jumpSpeedsY.value, 0.0f,
                        static_cast<float>(SCREEN_HEIGHT));
  y1 -= textYAdjust;
  float y2 = y1;

  if (!(style & Style::NO_LABEL)) {
    CG_Text_Paint_Ext(x1, y1, size.x, size.y, colorBase, LABEL_TEXT, 0, 0,
                      textStyle, &cgs.media.limboFont2);

    if (style & Style::HORIZONTAL) {
      x1 += firstJumpHorOffsetX;
    } else {
      y1 += rowHeight;
      y2 = y1;
    }
  }

  // 'pos' is the current position index we're drawing at on the list,
  // 'i' is the actual jump index that we're drawing at 'pos'
  for (int32_t pos = 0; pos < static_cast<int32_t>(jumpSpeeds.size()); pos++) {
    const int32_t i = style & Style::REVERSED
                          ? static_cast<int32_t>(jumpSpeeds.size()) - 1 - pos
                          : pos;

    if (style & Style::HORIZONTAL) {
      CG_Text_Paint_Ext(x1, y1, size.x, size.y, jumpSpeeds[i].color,
                        jumpSpeeds[i].speedStr, 0, 0, textStyle,
                        &cgs.media.limboFont2);

      x1 += textOffsetX;
    } else {
      // first column
      if (pos < 5) {
        CG_Text_Paint_Ext(x1, y1, size.x, size.y, jumpSpeeds[i].color,
                          jumpSpeeds[i].speedStr, 0, 0, textStyle,
                          &cgs.media.limboFont2);

        y1 += rowHeight;
      } else {
        CG_Text_Paint_Ext(x2, y2, size.x, size.y, jumpSpeeds[i].color,
                          jumpSpeeds[i].speedStr, 0, 0, textStyle,
                          &cgs.media.limboFont2);

        y2 += rowHeight;
      }
    }
  }
}

bool JumpSpeedsV2::canSkipDraw() {
  if (!etj_drawJumpSpeeds.integer) {
    return true;
  }

  if (getValidPlayerState()->persistant[PERS_TEAM] == TEAM_SPECTATOR) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
