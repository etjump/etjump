/*
 * MIT License
 *
 * Copyright (c) 2024 ETJump team <zero@etjump.com>
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
#include "etj_utilities.h"
#include "etj_client_commands_handler.h"
#include "etj_player_events_handler.h"

namespace ETJump {
JumpSpeeds::JumpSpeeds(EntityEventsHandler *entityEventsHandler)
    : _entityEventsHandler{entityEventsHandler} {
  serverCommandsHandler->subscribe(
      "resetJumpSpeeds",
      [&](const std::vector<std::string> &args) { queueJumpSpeedsReset(); });
  consoleCommandsHandler->subscribe(
      "resetJumpSpeeds",
      [&](const std::vector<std::string> &args) { queueJumpSpeedsReset(); });
  entityEventsHandler->subscribe(EV_JUMP,
                                 [&](centity_t *cent) { updateJumpSpeeds(); });
  playerEventsHandler->subscribe(
      "respawn",
      [&](const std::vector<std::string> &args) { queueJumpSpeedsReset(); });
}

JumpSpeeds::~JumpSpeeds() {
  consoleCommandsHandler->unsubcribe("resetJumpSpeeds");
  serverCommandsHandler->unsubcribe("resetJumpSpeeds");
  _entityEventsHandler->unsubscribe(EV_JUMP);
}

bool JumpSpeeds::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }
  return true;
}

void JumpSpeeds::render() const {
  float x1 = 6 + etj_jumpSpeedsX.value;
  float x2 = 6 + 30 + etj_jumpSpeedsX.value;
  float y1 = 240 + etj_jumpSpeedsY.value;
  float y2 = y1;
  auto textStyle = etj_jumpSpeedsShadow.integer ? ITEM_TEXTSTYLE_SHADOWED
                                                : ITEM_TEXTSTYLE_NORMAL;
  vec4_t color;
  bool horizontal = etj_jumpSpeedsStyle.integer &
                    static_cast<int>(jumpSpeedStyle::Horizontal);
  int numJumps = static_cast<int>(jumpSpeeds.size());

  x1 = ETJump_AdjustPosition(x1);
  x2 = ETJump_AdjustPosition(x2);
  if (!(etj_jumpSpeedsStyle.integer &
        static_cast<int>(jumpSpeedStyle::NoLabel))) {
    parseColorString(baseColorStr, color);
    DrawString(x1, y1, 0.2f, 0.2f, color, qfalse, "Jump Speeds:", 0, textStyle);

    // adjust x or y depending on style chosen
    if (horizontal) {
      x1 += static_cast<float>(DrawStringWidth("Jump Speeds: ", 0.2f)) + 5;
    } else {
      y1 += 12;
      y2 += 12;
    }
  }

  if (etj_jumpSpeedsStyle.integer &
      static_cast<int>(jumpSpeedStyle::Reversed)) {
    for (int i = numJumps - 1; i >= 0; i--) {
      auto jumpSpeed = std::to_string(jumpSpeeds[i].first);

      if (horizontal) {
        DrawString(x1, y1, 0.2f, 0.2f, jumpSpeedsColors[i], qfalse,
                   jumpSpeed.c_str(), 0, textStyle);
        x1 += 30;
      } else {
        // max 5 jumps - first column
        if (numJumps <= 5) {
          DrawString(x1, y1, 0.2f, 0.2f, jumpSpeedsColors[i], qfalse,
                     jumpSpeed.c_str(), 0, textStyle);
          y1 += 12;
        } else {
          // second column
          if (i < numJumps - 5) {
            DrawString(x2, y2, 0.2f, 0.2f, jumpSpeedsColors[i], qfalse,
                       jumpSpeed.c_str(), 0, textStyle);
            y2 += 12;
          }
          // first column
          else {
            DrawString(x1, y1, 0.2f, 0.2f, jumpSpeedsColors[i], qfalse,
                       jumpSpeed.c_str(), 0, textStyle);
            y1 += 12;
          }
        }
      }
    }
  } else {
    for (int i = 0; i < numJumps; i++) {
      auto jumpSpeed = std::to_string(jumpSpeeds[i].first);

      if (horizontal) {
        DrawString(x1, y1, 0.2f, 0.2f, jumpSpeedsColors[i], qfalse,
                   jumpSpeed.c_str(), 0, textStyle);
        x1 += 30;
      } else {
        // first column
        if (i < 5) {
          DrawString(x1, y1, 0.2f, 0.2f, jumpSpeedsColors[i], qfalse,
                     jumpSpeed.c_str(), 0, textStyle);
          y1 += 12;
        }
        // second column
        else {
          DrawString(x2, y2, 0.2f, 0.2f, jumpSpeedsColors[i], qfalse,
                     jumpSpeed.c_str(), 0, textStyle);
          y2 += 12;
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
  jumpSpeeds.emplace_back(ps->persistant[PERS_JUMP_SPEED], baseColorStr);

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
    parseColorString(jumpSpeeds[i].second, jumpSpeedsColors[i]);
  }
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
