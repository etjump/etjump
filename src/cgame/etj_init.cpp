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

#include <string>
#include <memory>

#include "cg_local.h"

#include "etj_init.h"

#include "etj_trickjump_lines.h"
#include "etj_timerun.h"
#include "etj_timerun_view.h"
#include "etj_event_loop.h"
#include "etj_awaited_command_handler.h"
#include "etj_player_events_handler.h"
#include "etj_client_authentication.h"
#include "etj_operating_system.h"
#include "etj_draw_leaves_handler.h"
#include "etj_console_alpha.h"
#include "etj_cvar_shadow.h"
#include "etj_keyset_system.h"
#include "etj_irenderable.h"
#include "etj_overbounce_watcher.h"
#include "etj_maxspeed.h"
#include "etj_speed_drawable.h"
#include "etj_strafe_quality_drawable.h"
#include "etj_jump_speeds.h"
#include "etj_quick_follow_drawable.h"
#include "etj_cgaz.h"
#include "etj_snaphud.h"
#include "etj_autodemo_recorder.h"
#include "etj_upper_right_drawable.h"
#include "etj_upmove_meter_drawable.h"
#include "etj_spectatorinfo_drawable.h"
#include "etj_crosshair.h"
#include "etj_overbounce_detector.h"
#include "etj_rtv_drawable.h"
#include "etj_client_rtv_handler.h"
#include "etj_areaindicator_drawable.h"
#include "etj_demo_compatibility.h"
#include "etj_accelmeter_drawable.h"
#include "etj_accel_color.h"
#include "etj_utilities.h"
#include "etj_player_bbox.h"
#include "etj_pmove_utils.h"
#include "etj_savepos.h"
#include "etj_servercommands.h"
#include "etj_consolecommands.h"

#include "../game/etj_syscall_ext_shared.h"

namespace ETJump {
std::shared_ptr<ClientCommandsHandler> serverCommandsHandler;
std::shared_ptr<ClientCommandsHandler> consoleCommandsHandler;
std::shared_ptr<EntityEventsHandler> entityEventsHandler;
std::shared_ptr<ClientAuthentication> authentication;
std::shared_ptr<OperatingSystem> operatingSystem;
std::vector<std::shared_ptr<IRenderable>> renderables;
std::shared_ptr<CvarUpdateHandler> cvarUpdateHandler;
std::vector<std::shared_ptr<CvarShadow>> cvarShadows;
std::shared_ptr<ConsoleAlphaHandler> consoleAlphaHandler;
std::shared_ptr<DrawLeavesHandler> drawLeavesHandler;
std::shared_ptr<AwaitedCommandHandler> awaitedCommandHandler;
std::shared_ptr<AutoDemoRecorder> autoDemoRecorder;
std::shared_ptr<EventLoop> eventLoop;
std::shared_ptr<PlayerEventsHandler> playerEventsHandler;
std::shared_ptr<Timerun> timerun;
std::shared_ptr<TimerunView> timerunView;
std::shared_ptr<TrickjumpLines> trickjumpLines;
std::shared_ptr<ClientRtvHandler> rtvHandler;
std::shared_ptr<AreaIndicator> areaIndicator;
std::unique_ptr<DemoCompatibility> demoCompatibility;
std::shared_ptr<AccelColor> accelColor;
std::array<bool, MAX_CLIENTS> tempTraceIgnoredClients;
std::shared_ptr<PlayerBBox> playerBBox;
std::unique_ptr<SavePos> savePos;
std::unique_ptr<SyscallExt> syscallExt;
std::unique_ptr<PmoveUtils> pmoveUtils;

void delayedInit() {
  // force original cvars to match the shadow values, as ETe and ETL
  // reset cheat cvars to original values after the 'CG_INIT' VMCall
  if (!cg.shadowCvarsSet) {
    for (const auto &cvarShadow : cvarShadows) {
      cvarShadow->forceCvarSet();
    }

    cg.shadowCvarsSet = true;
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

static void initMainHandlers() {
  serverCommandsHandler = std::make_shared<ClientCommandsHandler>(nullptr);
  consoleCommandsHandler =
      std::make_shared<ClientCommandsHandler>(trap_AddCommand);
  entityEventsHandler = std::make_shared<EntityEventsHandler>();
  playerEventsHandler = std::make_shared<PlayerEventsHandler>();

  awaitedCommandHandler = std::make_shared<AwaitedCommandHandler>(
      consoleCommandsHandler, trap_SendConsoleCommand,
      [](const char *text) { Com_Printf(text); });
}

static void initOperatingSystem() {
  operatingSystem = std::make_shared<OperatingSystem>();

  auto minimize = [](const std::vector<std::string> &args) {
    OperatingSystem::minimize();
  };

  consoleCommandsHandler->subscribe("min", minimize);
  consoleCommandsHandler->subscribe("minimize", minimize);
}

static void initAuthentication() {
  authentication = std::make_shared<ClientAuthentication>(
      [](const std::string &command) {
        trap_SendClientCommand(command.c_str());
      },
      [](const std::string &message) { CG_Printf(message.c_str()); },
      [] { return OperatingSystem::getHwid(); }, serverCommandsHandler);
}

static void initSyscallExtensions() {
  syscallExt = std::make_unique<SyscallExt>();
  syscallExt->setupExtensions();
  SyscallExt::trap_CmdBackup_Ext();
}

static void initDrawKeys(const std::shared_ptr<KeySetSystem> &keySetSystem) {
  // key set themes
  const char *keySetNames[]{
      "keyset",  // Keyset 1 (original)
      "keyset2", // Aciz: Keyset 2 (DeFRaG style keys)
      "keyset3",
      "keyset4",
      // + add more
  };
  for (const auto &keySetName : keySetNames) {
    keySetSystem->addSet(keySetName);
  }
  keySetSystem->addKeyBindSet("keyset5");
}

static void initRenderables() {
  assert(pmoveUtils != nullptr && accelColor != nullptr);

  renderables.emplace_back(
      std::make_shared<OverbounceWatcher>(consoleCommandsHandler.get()));
  renderables.emplace_back(std::make_shared<OverbounceDetector>());
  renderables.emplace_back(
      std::make_shared<DisplayMaxSpeed>(entityEventsHandler.get()));
  renderables.emplace_back(std::make_shared<DrawSpeed>());
  renderables.emplace_back(std::make_shared<AccelMeter>());
  renderables.emplace_back(std::make_shared<StrafeQuality>());
  renderables.emplace_back(
      std::make_shared<JumpSpeeds>(entityEventsHandler.get()));
  renderables.emplace_back(std::make_shared<QuickFollowDrawer>());
  renderables.emplace_back(std::make_shared<SpectatorInfo>());
  renderables.emplace_back(std::make_shared<AreaIndicator>());

  if (etj_CGazOnTop.integer) {
    renderables.emplace_back(std::make_shared<Snaphud>());
    renderables.emplace_back(std::make_shared<CGaz>());
  } else {
    renderables.emplace_back(std::make_shared<CGaz>());
    renderables.emplace_back(std::make_shared<Snaphud>());
  }

  renderables.emplace_back(std::make_shared<UpperRight>());
  renderables.emplace_back(std::make_shared<UpmoveMeter>());
  renderables.emplace_back(std::make_shared<RtvDrawable>());

  // FIXME: this is dumb, the init should be handled in the constructor
  const auto keySetSystem = std::make_shared<KeySetSystem>(etj_drawKeys);
  renderables.push_back(keySetSystem);
  initDrawKeys(keySetSystem);

  renderables.emplace_back(std::make_shared<Crosshair>());
}

static void initCvarUnlockers() {
  // TODO: move to the class
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
    cvarShadows.emplace_back(std::make_shared<CvarShadow>(shadow, target));
  }
}

void initTimeruns() {
  if (timerun) {
    timerunView->removeFromDrawables();
    timerun = nullptr;
    timerunView = nullptr;
  }

  timerun =
      std::make_shared<Timerun>(playerEventsHandler, serverCommandsHandler);
  timerunView = std::make_shared<TimerunView>(timerun);

  // restores timerun after vid_restart (if required)
  trap_SendClientCommand("timerun_status");
}

static void initTrickjumpLines() {
  trickjumpLines = std::make_shared<TrickjumpLines>(serverCommandsHandler,
                                                    consoleCommandsHandler);

  // Check if load TJL on connection is enable
  if (etj_tjlAlwaysLoadTJL.integer == 1) {
    // CG_Printf("All mapper Trickjump lines will be loaded due
    // to your cvar : etj_tjlAlwaysLoadTJL. \n");
    trickjumpLines->loadRoutes(nullptr);
  }

  if (etj_tjlEnableLine.integer == 1) {
    trickjumpLines->toggleRoutes(true);
  } else {
    trickjumpLines->toggleRoutes(false);
  }

  if (etj_tjlEnableMarker.integer == 1) {
    trickjumpLines->toggleMarker(true);
  } else {
    trickjumpLines->toggleMarker(false);
  }
}

void init() {
  CG_Printf(S_COLOR_LTGREY GAME_HEADER);
  CG_Printf(S_COLOR_LTGREY "____________________________\n");

  CG_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION
                                     " " S_COLOR_LTGREY GAME_BINARY_NAME
                                     " init...\n");

  // NOTE: client and server commands handlers must be created before other
  // modules as other modules use them to subscribe to commands.
  // Generally all modules should get these as constructor params but
  // they're still being used in the C code
  // => make sure they're created first
  initMainHandlers();

  initOperatingSystem();
  initAuthentication();
  initSyscallExtensions();
  initCvarUnlockers();

  eventLoop = std::make_shared<EventLoop>();

  demoCompatibility = std::make_unique<DemoCompatibility>();

  rtvHandler = std::make_shared<ClientRtvHandler>(serverCommandsHandler);
  rtvHandler->initialize();

  playerBBox = std::make_shared<PlayerBBox>();

  consoleAlphaHandler = std::make_shared<ConsoleAlphaHandler>();
  drawLeavesHandler = std::make_shared<DrawLeavesHandler>();
  autoDemoRecorder = std::make_shared<AutoDemoRecorder>();

  // must be initialized before accelColor & renderables!
  pmoveUtils = std::make_unique<PmoveUtils>();
  accelColor = std::make_shared<AccelColor>();
  initRenderables();

  // FIXME: remove this 'Drawable' class and move this to 'IRenderable'
  ETJump_ClearDrawables();
  initTimeruns();

  initTrickjumpLines();

  assert(timerun != nullptr);
  savePos = std::make_unique<SavePos>(timerun);

  std::fill_n(tempTraceIgnoredClients.begin(), MAX_CLIENTS, false);

  ServerCommands::registerCommands();
  ConsoleCommands::registerCommands();

  CG_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION
                                     " " S_COLOR_LTGREY GAME_BINARY_NAME
                                     " init... " S_COLOR_GREEN "DONE\n");
}

void shutdown() {
  CG_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION_DATED
                                     " " S_COLOR_LTGREY GAME_BINARY_NAME
                                     " shutdown...\n");

  if (consoleCommandsHandler) {
    consoleCommandsHandler->unsubscribe("min");
    consoleCommandsHandler->unsubscribe("minimize");
  }

  operatingSystem = nullptr;
  authentication = nullptr;
  renderables.clear();
  cvarShadows.clear();
  cvarUpdateHandler = nullptr;

  // clear dynamic shaders in reverse order
  drawLeavesHandler = nullptr;
  consoleAlphaHandler = nullptr;

  if (eventLoop) {
    eventLoop->shutdown();
    eventLoop = nullptr;
  }

  consoleCommandsHandler = nullptr;
  serverCommandsHandler = nullptr;
  playerEventsHandler = nullptr;
  entityEventsHandler = nullptr;

  savePos = nullptr;

  syscallExt = nullptr;

  trickjumpLines = nullptr;

  CG_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION
                                     " " S_COLOR_LTGREY GAME_BINARY_NAME
                                     " shutdown... " S_COLOR_GREEN "DONE\n");
}
} // namespace ETJump
