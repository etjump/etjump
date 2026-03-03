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

#include "cg_public.h"
#include "etj_cgame.h"

namespace ETJump {
inline constexpr int32_t CGAME_INIT_DELAY_FRAMES = 10;

// we need to reserve the extended value for this, it doesn't matter
// if the client doesn't actually support CMD_BACKUP_EXT
inline constexpr int MAX_BACKUP_STATES = CMD_BACKUP_EXT + 2;

enum class FTMenuOptions {
  FT_DISBAND_PROPOSE = 0,
  FT_CREATE_LEAVE = 1,
  FT_INVITE = 2,
  FT_KICK = 3,
  FT_WARN = 4,
  FT_RULES = 5,
  FT_TJMODE = 6,
  FT_COUNTDOWN_START = 7,
  FT_MAX_OPTIONS = 8,
};

enum class FTMenuMode {
  FT_VSAY = 0,
  FT_MANAGE = 1, // create, leave, disband
  FT_APPLY = 2,
  FT_PROPOSE = 3,
  FT_ADMIN = 4
};

// sub-pages of fireteam menus
enum class FTMenuPos {
  FT_MENUPOS_NONE = -1,
  FT_MENUPOS_INVITE = 2,
  FT_MENUPOS_KICK = 3,
  FT_MENUPOS_WARN = 4,
  FT_MENUPOS_RULES = 5,
};

enum class FTMenuRulesPos {
  FT_RULES_RESET = 0,
  FT_RULES_SAVELIMIT = 1,
  FT_RULES_NOGHOST = 2
};

enum AutoSwitchFlags {
  Disabled = 0 << 0,
  Enabled = 1 << 0,
  IfReplacingPrimary = 1 << 1,
  IgnorePortalGun = 1 << 2,
};

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
