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

#include "etj_accel_color.h"
#include "etj_autodemo_recorder.h"
#include "etj_awaited_command_handler.h"
#include "etj_chs_data.h"
#include "etj_client_authentication.h"
#include "etj_client_commands_handler.h"
#include "etj_client_rtv_handler.h"
#include "etj_color_parser.h"
#include "etj_console_shader.h"
#include "etj_custom_command_menu.h"
#include "etj_cvar_unlocker.h"
#include "etj_cvar_update_handler.h"
#include "etj_demo_compatibility.h"
#include "etj_entity_events_handler.h"
#include "etj_event_loop.h"
#include "etj_irenderable.h"
#include "etj_leaves_remapper.h"
#include "etj_operating_system.h"
#include "etj_player_bbox.h"
#include "etj_player_events_handler.h"
#include "etj_pmove_utils.h"
#include "etj_savepos.h"
#include "etj_timerun.h"
#include "etj_timerun_view.h"
#include "etj_trace_utils.h"
#include "etj_trickjump_lines.h"

#include "../game/etj_syscall_ext_shared.h"

namespace ETJump {
inline constexpr int32_t CGAME_INIT_DELAY_FRAMES = 10;

struct Handlers {
  std::shared_ptr<ClientCommandsHandler> serverCommands;
  std::shared_ptr<ClientCommandsHandler> consoleCommands;
  std::shared_ptr<EntityEventsHandler> entityEvents;
  std::shared_ptr<PlayerEventsHandler> playerEvents;
  std::shared_ptr<AwaitedCommandHandler> awaitedCommand;
  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
  std::shared_ptr<ClientRtvHandler> rtv;
  std::unique_ptr<CustomCommandMenu> customCommandMenu;
  std::shared_ptr<Timerun> timerun;
};

struct Platform {
  std::shared_ptr<ClientAuthentication> authentication;
  std::shared_ptr<OperatingSystem> operatingSystem;
  std::unique_ptr<SyscallExt> syscallExt;
};

struct Demo {
  std::unique_ptr<DemoCompatibility> compatibility;
  std::shared_ptr<AutoDemoRecorder> autoDemoRecorder;
};

struct Utils {
  std::shared_ptr<EventLoop> eventLoop;
  std::vector<std::unique_ptr<CvarUnlocker>> cvarUnlocker;
  std::unique_ptr<SavePos> savePos;
  std::unique_ptr<ColorParser> colorParser;
  std::unique_ptr<TraceUtils> trace;
  std::unique_ptr<PmoveUtils> pmove;
};

struct UI {
  std::shared_ptr<ConsoleShader> consoleShader;
  std::vector<std::unique_ptr<IRenderable>> renderables;
};

struct Visuals {
  std::shared_ptr<LeavesRemapper> leavesRemapper;
  std::shared_ptr<PlayerBBox> playerBBox;
  std::shared_ptr<TrickjumpLines> trickjumpLines;
};

struct HUD {
  std::shared_ptr<AccelColor> accelColor;
  std::shared_ptr<CHSDataHandler> chsDataHandler;

  std::vector<std::unique_ptr<IRenderable>> renderables;
  std::shared_ptr<TimerunView> timerunView;
};

struct CGameContext {
  Handlers handlers;
  Platform platform;
  Demo demo;
  Utils utils;
  UI ui;
  Visuals visuals;
  HUD hud;
};

inline CGameContext cgame;

void init();
void shutdown();
// performs one-time actions slightly delayed from actual cgame init,
// to work around issues that certain actions have when they are performed
// on the same frame as the module is initialized
void delayedInit();
} // namespace ETJump
