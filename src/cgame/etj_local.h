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

#include <cstdint>

#include "etj_cgame.h"

namespace ETJump {
inline constexpr int32_t CGAME_INIT_DELAY_FRAMES = 10;

enum extraTraceOptions {
  OB_DETECTOR,
  SLICK_DETECTOR,
  NJD_DETECTOR,
  CHS_10_11,
  CHS_12,
  CHS_13_15,
  CHS_16,
  CHS_53,
};

enum class ChatHighlightFlags {
  HIGHLIGHT_BEEPER = 1,
  HIGHLIGHT_FLASH = 2,
};

enum class HUDLerpFlags {
  DRAWSPEED2 = 1 << 0,
  CGAZ = 1 << 1,
  SNAPHUD = 1 << 2,
  STRAFE_QUALITY = 1 << 3,
};

enum class ExecFileType {
  NONE = 0,
  MAP_AUTOEXEC = 1 << 0,
  TEAM_AUTOEXEC = 1 << 1,
};

enum class HideFlamethrowerFlags {
  HIDE_SELF = 1 << 0,
  HIDE_OTHERS = 1 << 1,
};

enum class ChatMessageType {
  DEFAULT = 0,         // normal message from any team
  REPLAY_MSG = 1 << 0, // chat replay message
  SERVER_MSG = 1 << 1, // server console chat message
};

void init();
void shutdown();
// performs one-time actions slightly delayed from actual cgame init,
// to work around issues that certain actions have when they are performed
// on the same frame as the module is initialized
void delayedInit();

extern CGameContext cgame;
} // namespace ETJump
