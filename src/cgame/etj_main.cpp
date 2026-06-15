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

#include "cg_local.h"

#include "etj_accel_color_v2.h"
#include "etj_accelmeter_drawable.h"
#include "etj_accelmeter_v2.h"
#include "etj_areaindicator_drawable.h"
#include "etj_autodemo_recorder.h"
#include "etj_awaited_command_handler.h"
#include "etj_cgame.h"
#include "etj_cgaz.h"
#include "etj_cgaz_data.h"
#include "etj_cgaz_v2.h"
#include "etj_chs_drawable.h"
#include "etj_client_authentication.h"
#include "etj_client_commands_handler.h"
#include "etj_client_rtv_handler.h"
#include "etj_color_parser.h"
#include "etj_console_shader.h"
#include "etj_consolecommands.h"
#include "etj_crosshair.h"
#include "etj_custom_command_menu.h"
#include "etj_custom_command_menu_drawable.h"
#include "etj_cvar_unlocker.h"
#include "etj_demo_compatibility.h"
#include "etj_entity_events_handler.h"
#include "etj_event_loop.h"
#include "etj_jump_speeds.h"
#include "etj_keyset_system.h"
#include "etj_leaves_remapper.h"
#include "etj_local.h"
#include "etj_maxspeed.h"
#include "etj_operating_system.h"
#include "etj_overbounce_detector.h"
#include "etj_overbounce_watcher.h"
#include "etj_player_bbox.h"
#include "etj_player_events_handler.h"
#include "etj_pmove_utils.h"
#include "etj_quick_follow_drawable.h"
#include "etj_rtv_drawable.h"
#include "etj_savepos.h"
#include "etj_servercommands.h"
#include "etj_snaphud.h"
#include "etj_snaphud_data.h"
#include "etj_snaphud_v2.h"
#include "etj_spectatorinfo_drawable.h"
#include "etj_speed_drawable.h"
#include "etj_strafe_quality_drawable.h"
#include "etj_strafe_quality_v2.h"
#include "etj_timerun.h"
#include "etj_timerun_view.h"
#include "etj_trace_utils.h"
#include "etj_trickjump_lines.h"
#include "etj_upmove_meter_drawable.h"
#include "etj_upper_right_drawable.h"

#include "../game/etj_syscall_ext_shared.h"

namespace ETJump {
void delayedInit() {
  // force original cvars to match the shadow values, as ETe and ETL
  // reset cheat cvars to original values after the 'CG_INIT' VMCall
  if (!cg.cvarUnlocksForced) {
    for (const auto &cvarUnlocker : cgame.utils.cvarUnlocker) {
      cvarUnlocker->forceCvarSet();
    }

    cg.cvarUnlocksForced = true;
  }

  // NOTE: anything below this will not execute during demo playback!
  if (cg.demoPlayback) {
    return;
  }

  if (!cg.chatReplayReceived) {
    if (etj_chatReplay.integer) {
      trap_SendConsoleCommand("getchatreplay");
    }

    // keep this separate from the cvar check, so client doesn't immediately
    // receive chat replay in the middle of a map when toggling this,
    // as the replay would just be whatever is currently in chat
    cg.chatReplayReceived = true;
  }

  // populate map vote menu
  if (!cg.maplistRequested) {
    trap_SendClientCommand("requestmaplist");
    cg.maplistRequested = true;
  }

  if (!cg.numCustomvotesRequested) {
    cg.numCustomvotes = -1;
    cg.numCustomvoteInfosRequested = 0;

    trap_SendClientCommand("requestnumcustomvotes");
    cg.numCustomvotesRequested = true;
  }

  // space out customvote info requests a bit, otherwise we get a
  // big lag spike if the server has lots of data to send
  if (cg.customvoteInfoRequested && cg.numCustomvotes > 0 &&
      cg.clientFrame >=
          CGAME_INIT_DELAY_FRAMES + (cg.numCustomvoteInfosRequested * 25) &&
      cg.numCustomvoteInfosRequested < cg.numCustomvotes) {
    trap_SendClientCommand(
        va("requestcustomvoteinfo %i", cg.numCustomvoteInfosRequested));
    cg.numCustomvoteInfosRequested++;
  }
}

void initCore() {
  cgame.core.serverCommands = std::make_shared<ClientCommandsHandler>(nullptr);
  cgame.core.consoleCommands =
      std::make_shared<ClientCommandsHandler>(trap_AddCommand);
  cgame.core.entityEvents = std::make_shared<EntityEventsHandler>();
  cgame.core.playerEvents = std::make_shared<PlayerEventsHandler>();

  cgame.core.awaitedCommand = std::make_unique<AwaitedCommandHandler>(
      cgame.core.consoleCommands, cgame.core.playerEvents);

  cgame.core.cvarUpdate = std::make_shared<CvarUpdateHandler>();
}

static void initPlatform() {
  cgame.platform.authentication = std::make_unique<ClientAuthentication>(
      [](const std::string &command) {
        trap_SendClientCommand(command.c_str());
      },
      [](const std::string &message) { CG_Printf(message.c_str()); },
      [] { return OperatingSystem::getHwid(); }, cgame.core.serverCommands);

  cgame.platform.operatingSystem = std::make_unique<OperatingSystem>();

  const auto minimize = [](const std::vector<std::string> &) {
    OperatingSystem::minimize();
  };

  cgame.core.consoleCommands->subscribe("min", minimize);
  cgame.core.consoleCommands->subscribe("minimize", minimize);

  cgame.platform.syscallExt = std::make_unique<SyscallExt>();
  cgame.platform.syscallExt->setupExtensions();
  SyscallExt::trap_CmdBackup_Ext();
}

static void initSystems() {
  cgame.systems.rtv =
      std::make_unique<ClientRtvHandler>(cgame.core.serverCommands);

  cgame.systems.customCommandMenu = std::make_unique<CustomCommandMenu>(
      cgame.core.cvarUpdate, cgame.core.consoleCommands);

  cgame.systems.timerun = std::make_shared<Timerun>(cgame.core.playerEvents,
                                                    cgame.core.serverCommands);
}

void initDemo() {
  cgame.demo.compatibility =
      std::make_unique<DemoCompatibility>(cgame.core.consoleCommands);
  cgame.demo.autoDemoRecorder = std::make_unique<AutoDemoRecorder>(
      cgame.core.playerEvents, cgame.core.consoleCommands);
}

static void initCvarUnlockers() {
  const std::vector<std::pair<const vmCvar_t *, const std::string>> cvars{
      {&etj_drawFoliage, "r_drawfoliage"},
      {&etj_showTris, "r_showtris"},
      {&etj_wolfFog, "r_wolffog"},
      {&etj_zFar, "r_zfar"},
      {&etj_viewlog, "viewlog"},
      {&etj_offsetFactor, "r_offsetFactor"},
      {&etj_offsetUnits, "r_offsetUnits"},
      {&etj_speeds, "r_speeds"},
      {&etj_lightmap, "r_lightmap"},
      {&etj_drawNotify, "con_drawNotify"},
      {&etj_drawClips, "r_drawClips"},
      {&etj_drawTriggers, "r_drawTriggers"},
      {&etj_drawSlicks, "r_drawSlicks"},
      {&etj_clear, "r_clear"},
      {&etj_flareSize, "r_flareSize"},
  };

  for (const auto &[shadow, target] : cvars) {
    cgame.utils.cvarUnlocker.emplace_back(
        std::make_unique<CvarUnlocker>(cgame.core.cvarUpdate, shadow, target));
  }
}

static void initUtils() {
  assert(cgame.systems.timerun != nullptr);

  cgame.utils.eventLoop = std::make_unique<EventLoop>();
  initCvarUnlockers();
  cgame.utils.savePos = std::make_unique<SavePos>(cgame.systems.timerun);
  cgame.utils.colorParser = std::make_unique<ColorParser>();
  cgame.utils.trace = std::make_unique<TraceUtils>();
  cgame.utils.pmove = std::make_unique<PmoveUtils>(cgame.core.cvarUpdate);
  cgame.utils.pmoveV2 = std::make_unique<PmoveUtilsV2>(cgame.core.cvarUpdate);
}

static void initUserInterface() {
  cgame.ui.consoleShader = std::make_unique<ConsoleShader>();

  cgame.ui.renderables.emplace_back(std::make_unique<RtvDrawable>());
  cgame.ui.renderables.emplace_back(std::make_unique<CustomCommandMenuDrawable>(
      cgame.core.consoleCommands, cgame.core.cvarUpdate));
}

static void initTrickjumpLines() {
  cgame.visuals.trickjumpLines = std::make_unique<TrickjumpLines>(
      cgame.core.serverCommands, cgame.core.consoleCommands);

  // Check if load TJL on connection is enable
  if (etj_tjlAlwaysLoadTJL.integer == 1) {
    // CG_Printf("All mapper Trickjump lines will be loaded due
    // to your cvar : etj_tjlAlwaysLoadTJL. \n");
    cgame.visuals.trickjumpLines->loadRoutes(nullptr);
  }

  if (etj_tjlEnableLine.integer == 1) {
    cgame.visuals.trickjumpLines->toggleRoutes(true);
  } else {
    cgame.visuals.trickjumpLines->toggleRoutes(false);
  }

  if (etj_tjlEnableMarker.integer == 1) {
    cgame.visuals.trickjumpLines->toggleMarker(true);
  } else {
    cgame.visuals.trickjumpLines->toggleMarker(false);
  }
}

static void initVisuals() {
  cgame.visuals.leavesRemapper =
      std::make_unique<LeavesRemapper>(cgame.core.cvarUpdate);
  cgame.visuals.playerBBox =
      std::make_unique<PlayerBBox>(cgame.core.cvarUpdate);
  initTrickjumpLines();
}

static void initHUD() {
  assert(cgame.utils.pmove != nullptr);
  assert(cgame.utils.pmoveV2 != nullptr);

  cgame.hud.accelColor = std::make_unique<AccelColor>();
  cgame.hud.chsDataHandler = std::make_unique<CHSDataHandler>(
      cgame.core.cvarUpdate, cgame.core.consoleCommands);
  cgame.hud.cgazDataHandler = std::make_unique<CGazData>();
  cgame.hud.snaphudDataHandler = std::make_unique<SnaphudData>();

  cgame.hud.renderables.emplace_back(
      std::make_unique<CHS>(cgame.core.cvarUpdate, cgame.hud.chsDataHandler));
  cgame.hud.renderables.emplace_back(std::make_unique<OverbounceWatcher>(
      cgame.core.consoleCommands, cgame.core.cvarUpdate));
  cgame.hud.renderables.emplace_back(std::make_unique<OverbounceDetector>());
  cgame.hud.renderables.emplace_back(std::make_unique<DisplayMaxSpeed>(
      cgame.core.entityEvents, cgame.core.cvarUpdate));
  cgame.hud.renderables.emplace_back(std::make_unique<DrawSpeed>(
      cgame.core.cvarUpdate, cgame.core.consoleCommands));
  cgame.hud.renderables.emplace_back(
      std::make_unique<AccelMeter>(cgame.core.cvarUpdate));
  cgame.hud.renderables.emplace_back(
      std::make_unique<AccelMeterV2>(cgame.core.cvarUpdate));
  cgame.hud.renderables.emplace_back(std::make_unique<StrafeQuality>(
      cgame.core.cvarUpdate, cgame.core.consoleCommands,
      cgame.core.playerEvents));
  cgame.hud.renderables.emplace_back(std::make_unique<StrafeQualityV2>(
      cgame.hud.snaphudDataHandler, cgame.core.cvarUpdate,
      cgame.core.consoleCommands, cgame.core.playerEvents));
  cgame.hud.renderables.emplace_back(std::make_unique<JumpSpeeds>(
      cgame.core.entityEvents, cgame.core.playerEvents,
      cgame.core.consoleCommands, cgame.core.serverCommands,
      cgame.core.cvarUpdate));
  cgame.hud.renderables.emplace_back(std::make_unique<QuickFollowDrawer>());
  cgame.hud.renderables.emplace_back(
      std::make_unique<SpectatorInfo>(cgame.core.cvarUpdate));
  cgame.hud.renderables.emplace_back(std::make_unique<AreaIndicator>());

  if (etj_CGazOnTop.integer) {
    cgame.hud.renderables.emplace_back(
        std::make_unique<Snaphud>(cgame.core.cvarUpdate));
    cgame.hud.renderables.emplace_back(std::make_unique<SnaphudV2>(
        cgame.hud.snaphudDataHandler, cgame.core.cvarUpdate));
    cgame.hud.renderables.emplace_back(
        std::make_unique<CGaz>(cgame.core.cvarUpdate));
    cgame.hud.renderables.emplace_back(std::make_unique<CGazV2>(
        cgame.hud.cgazDataHandler, cgame.hud.snaphudDataHandler,
        cgame.core.cvarUpdate));
  } else {
    cgame.hud.renderables.emplace_back(
        std::make_unique<CGaz>(cgame.core.cvarUpdate));
    cgame.hud.renderables.emplace_back(std::make_unique<CGazV2>(
        cgame.hud.cgazDataHandler, cgame.hud.snaphudDataHandler,
        cgame.core.cvarUpdate));
    cgame.hud.renderables.emplace_back(
        std::make_unique<Snaphud>(cgame.core.cvarUpdate));
    cgame.hud.renderables.emplace_back(std::make_unique<SnaphudV2>(
        cgame.hud.snaphudDataHandler, cgame.core.cvarUpdate));
  }

  cgame.hud.renderables.emplace_back(std::make_unique<UpperRight>());
  cgame.hud.renderables.emplace_back(std::make_unique<UpmoveMeter>(
      cgame.core.cvarUpdate, cgame.core.consoleCommands,
      cgame.core.playerEvents));

  cgame.hud.renderables.emplace_back(
      std::make_unique<KeySetSystem>(&etj_drawKeys, cgame.core.cvarUpdate));

  // FIXME: move to renderables
  ETJump_ClearDrawables();
  cgame.hud.timerunView = std::make_unique<TimerunView>(cgame.systems.timerun,
                                                        cgame.core.cvarUpdate);

  cgame.hud.renderables.emplace_back(
      std::make_unique<Crosshair>(cgame.core.cvarUpdate));
}

void init() {
  CG_Printf(S_COLOR_LTGREY GAME_HEADER);
  CG_Printf(S_COLOR_LTGREY "____________________________\n");

  CG_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION
                                     " " S_COLOR_LTGREY GAME_BINARY_NAME
                                     " init...\n");

  initPlatform();
  initSystems();
  initUtils();
  initUserInterface();
  initVisuals();
  initHUD();

  ServerCommands::registerCommands();
  ConsoleCommands::registerCommands();

  // get the timerun status from the server so we know if there's a timerun
  trap_SendClientCommand("timerun_status");

  CG_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION
                                     " " S_COLOR_LTGREY GAME_BINARY_NAME
                                     " init... " S_COLOR_GREEN "DONE\n");
}

void shutdown() {
  CG_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION_DATED
                                     " " S_COLOR_LTGREY GAME_BINARY_NAME
                                     " shutdown...\n");

  cgame = {};

  CG_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION
                                     " " S_COLOR_LTGREY GAME_BINARY_NAME
                                     " shutdown... " S_COLOR_GREEN "DONE\n");
}

CGameContext cgame;
} // namespace ETJump
