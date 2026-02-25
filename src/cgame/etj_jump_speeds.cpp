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

#include "etj_jump_speeds.h"
#include "etj_local.h"
#include "etj_utilities.h"

namespace ETJump {
inline constexpr float BASE_OFFSET_X = 30.0f;
inline constexpr float BASE_OFFSET_Y = 12.0f;
inline constexpr float HOR_FIRSTJUMP_OFFSET = 5.0f;
inline constexpr float DEFAULT_TEXT_SIZE = 0.2f;

JumpSpeeds::JumpSpeeds(
    const std::shared_ptr<EntityEventsHandler> &entityEvents,
    const std::shared_ptr<PlayerEventsHandler> &playerEvents,
    const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
    const std::shared_ptr<ClientCommandsHandler> &serverCommands,
    const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : entityEvents(entityEvents), playerEvents(playerEvents),
      consoleCommands(consoleCommands), serverCommands(serverCommands),
      cvarUpdate(cvarUpdate) {
  startListeners();
  adjustSize(etj_jumpSpeedsTextSize);
}

JumpSpeeds::~JumpSpeeds() {
  entityEvents->unsubscribe(EV_JUMP);
  playerEvents->unsubscribe("respawn");

  consoleCommands->unsubscribe("resetJumpSpeeds");
  serverCommands->unsubscribe("resetJumpSpeeds");

  cvarUpdate->unsubscribe(&etj_jumpSpeedsTextSize);
}

void JumpSpeeds::startListeners() {
  entityEvents->subscribe(EV_JUMP, [this](centity_t *) { updateJumpSpeeds(); });

  playerEvents->subscribe("respawn", [this](const std::vector<std::string> &) {
    queueJumpSpeedsReset();
  });

  consoleCommands->subscribe(
      "resetJumpSpeeds",
      [this](const std::vector<std::string> &) { queueJumpSpeedsReset(); });

  serverCommands->subscribe(
      "resetJumpSpeeds",
      [this](const std::vector<std::string> &) { queueJumpSpeedsReset(); });

  cvarUpdate->subscribe(&etj_jumpSpeedsTextSize,
                        [this](const vmCvar_t *cvar) { adjustSize(*cvar); });
}

bool JumpSpeeds::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }
  return true;
}

// FIXME: this is dumb, a lot of this can be precalculated
// or doesn't need to be recalculated each frame, refactor later
void JumpSpeeds::render() const {
  float x1 = 6 + etj_jumpSpeedsX.value;
  float x2 = 6 + (BASE_OFFSET_X * (size.x / DEFAULT_TEXT_SIZE)) +
             etj_jumpSpeedsX.value;

  // This is a bit dumb, but because the text drawing starts from the bottom
  // left of the glyph rather than top left, adjusting the size will grow
  // the text also slightly upwards. This wasn't an issue before since the
  // size was hardcoded, but now that it can be adjusted, calculate an offset
  // relative to the default text size so that the size adjustment works as
  // expected (grows right/down, shrinks left/up), without having to change
  // the base starting position, in order to not break peoples configs.
  float y1 = SCREEN_CENTER_Y + etj_jumpSpeedsY.value;
  y1 -= static_cast<float>(
      CG_Text_Height_Ext(label, DEFAULT_TEXT_SIZE, 0, &cgs.media.limboFont2) -
      CG_Text_Height_Ext(label, size.y, 0, &cgs.media.limboFont2));
  float y2 = y1;

  auto textStyle = etj_jumpSpeedsShadow.integer ? ITEM_TEXTSTYLE_SHADOWED
                                                : ITEM_TEXTSTYLE_NORMAL;
  vec4_t color;
  bool horizontal = etj_jumpSpeedsStyle.integer &
                    static_cast<int>(jumpSpeedStyle::Horizontal);
  int numJumps = static_cast<int>(jumpSpeeds.size());

  const float offsetX = BASE_OFFSET_X * (size.x / DEFAULT_TEXT_SIZE);
  const float offsetY = BASE_OFFSET_Y * (size.y / DEFAULT_TEXT_SIZE);

  x1 = ETJump_AdjustPosition(x1);
  x2 = ETJump_AdjustPosition(x2);
  if (!(etj_jumpSpeedsStyle.integer &
        static_cast<int>(jumpSpeedStyle::NoLabel))) {
    cgame.utils.colorParser->parseColorString(baseColorStr, color);
    DrawString(x1, y1, size.x, size.y, color, qfalse, label.c_str(), 0,
               textStyle);

    // adjust x or y depending on style chosen
    if (horizontal) {
      x1 += static_cast<float>(
                DrawStringWidth(std::string(label + " ").c_str(), size.x)) +
            (HOR_FIRSTJUMP_OFFSET * (size.x / DEFAULT_TEXT_SIZE));
    } else {
      y1 += offsetY;
      y2 += offsetY;
    }
  }

  if (etj_jumpSpeedsStyle.integer &
      static_cast<int>(jumpSpeedStyle::Reversed)) {
    for (int i = numJumps - 1; i >= 0; i--) {
      auto jumpSpeed = std::to_string(jumpSpeeds[i].first);

      if (horizontal) {
        DrawString(x1, y1, size.x, size.y, jumpSpeedsColors[i], qfalse,
                   jumpSpeed.c_str(), 0, textStyle);
        x1 += offsetX;
      } else {
        // max 5 jumps - first column
        if (numJumps <= 5) {
          DrawString(x1, y1, size.x, size.y, jumpSpeedsColors[i], qfalse,
                     jumpSpeed.c_str(), 0, textStyle);
          y1 += offsetY;
        } else {
          // second column
          if (i < numJumps - 5) {
            DrawString(x2, y2, size.x, size.y, jumpSpeedsColors[i], qfalse,
                       jumpSpeed.c_str(), 0, textStyle);
            y2 += offsetY;
          }
          // first column
          else {
            DrawString(x1, y1, size.x, size.y, jumpSpeedsColors[i], qfalse,
                       jumpSpeed.c_str(), 0, textStyle);
            y1 += offsetY;
          }
        }
      }
    }
  } else {
    for (int i = 0; i < numJumps; i++) {
      auto jumpSpeed = std::to_string(jumpSpeeds[i].first);

      if (horizontal) {
        DrawString(x1, y1, size.x, size.y, jumpSpeedsColors[i], qfalse,
                   jumpSpeed.c_str(), 0, textStyle);
        x1 += offsetX;
      } else {
        // first column
        if (i < 5) {
          DrawString(x1, y1, size.x, size.y, jumpSpeedsColors[i], qfalse,
                     jumpSpeed.c_str(), 0, textStyle);
          y1 += offsetY;
        }
        // second column
        else {
          DrawString(x2, y2, size.x, size.y, jumpSpeedsColors[i], qfalse,
                     jumpSpeed.c_str(), 0, textStyle);
          y2 += offsetY;
        }
      }
    }
  }
}

void JumpSpeeds::updateJumpSpeeds() {
  // events are processed at playerstate transition before interpolation
  // runs, so we can't rely on predictedPlayerState on demos because it
  // still contains previous playerstate at the time EV_JUMP event is
  // processed
  playerState_t *ps =
      (cg.snap->ps.clientNum == cg.clientNum && !cg.demoPlayback)
          ? &cg.predictedPlayerState
          : &cg.snap->ps;

  // queue reset if last update was on different team
  if (team != ps->persistant[PERS_TEAM]) {
    queueJumpSpeedsReset();
  }
  // if reset is queued, do that before we start storing new jump speeds
  if (resetQueued) {
    jumpSpeeds.clear();
    resetQueued = false;
  }

  team = ps->persistant[PERS_TEAM];
  baseColorStr = etj_jumpSpeedsColor.string;

  if (cgame.demo.compatibility->flags.predictedJumpSpeeds) {
    jumpSpeeds.emplace_back(VectorLength2(ps->velocity), baseColorStr);
  } else {
    jumpSpeeds.emplace_back(ps->persistant[PERS_JUMP_SPEED], baseColorStr);
  }

  // we only want to keep last 10 jumps, so remove first value if we go
  // over that
  if (jumpSpeeds.size() > MAX_JUMPS) {
    jumpSpeeds.erase(jumpSpeeds.begin());
  }

  if (etj_jumpSpeedsShowDiff.integer && jumpSpeeds.size() > 1) {
    adjustColors();
  }

  if (etj_jumpSpeedsMinSpeed.integer >
      jumpSpeeds.at(jumpSpeeds.size() - 1).first) {
    jumpSpeeds.at(jumpSpeeds.size() - 1).second =
        etj_jumpSpeedsSlowerColor.string;
  }

  // parse the colors for drawing here, so we don't need to do it every frame
  colorStrToVec();
}

void JumpSpeeds::queueJumpSpeedsReset() { resetQueued = true; }

void JumpSpeeds::adjustColors() {
  auto jumpNum = jumpSpeeds.size() - 1;
  fasterColorStr = etj_jumpSpeedsFasterColor.string;
  slowerColorStr = etj_jumpSpeedsSlowerColor.string;

  if (jumpSpeeds[jumpNum].first == jumpSpeeds[jumpNum - 1].first) {
    return; // color is already set to default, no need to adjust
  }

  jumpSpeeds[jumpNum].second =
      jumpSpeeds[jumpNum].first > jumpSpeeds[jumpNum - 1].first
          ? fasterColorStr
          : slowerColorStr;
}

void JumpSpeeds::colorStrToVec() {
  for (std::size_t i = 0; i < jumpSpeeds.size(); i++) {
    cgame.utils.colorParser->parseColorString(jumpSpeeds[i].second,
                                              jumpSpeedsColors[i]);
  }
}

void JumpSpeeds::adjustSize(const vmCvar_t &cvar) {
  size = CvarValueParser::parse<CvarValue::Size>(cvar, 0.1f, 10.0f);
  size.x *= 0.1f;
  size.y *= 0.1f;
}

bool JumpSpeeds::canSkipDraw() const {
  if (!etj_drawJumpSpeeds.integer) {
    return true;
  }

  // need to get playerState here because we only update team
  // in the update function, which is never called as a spectator
  playerState_s *ps = getValidPlayerState();
  if (ps->persistant[PERS_TEAM] == TEAM_SPECTATOR) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
