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

#include "etj_accelmeter_drawable.h"
#include "etj_areaindicator_drawable.h"
#include "etj_cgaz.h"
#include "etj_chs_drawable.h"
#include "etj_consolecommands.h"
#include "etj_crosshair.h"
#include "etj_custom_command_menu_drawable.h"
#include "etj_jump_speeds.h"
#include "etj_keyset_system.h"
#include "etj_local.h"
#include "etj_maxspeed.h"
#include "etj_overbounce_detector.h"
#include "etj_overbounce_watcher.h"
#include "etj_quick_follow_drawable.h"
#include "etj_rtv_drawable.h"
#include "etj_servercommands.h"
#include "etj_snaphud.h"
#include "etj_spectatorinfo_drawable.h"
#include "etj_speed_drawable.h"
#include "etj_strafe_quality_drawable.h"
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

static void initHandlers() {
  cgame.handlers.serverCommands =
      std::make_shared<ClientCommandsHandler>(nullptr);
  cgame.handlers.consoleCommands =
      std::make_shared<ClientCommandsHandler>(trap_AddCommand);
  cgame.handlers.entityEvents = std::make_shared<EntityEventsHandler>();
  cgame.handlers.playerEvents = std::make_shared<PlayerEventsHandler>();

  cgame.handlers.awaitedCommand = std::make_shared<AwaitedCommandHandler>(
      cgame.handlers.consoleCommands, trap_SendConsoleCommand,
      [](const char *text) { Com_Printf(text); });

  cgame.handlers.rtv =
      std::make_shared<ClientRtvHandler>(cgame.handlers.serverCommands);
  cgame.handlers.rtv->initialize();

  cgame.handlers.customCommandMenu = std::make_unique<CustomCommandMenu>(
      cgame.handlers.cvarUpdate, cgame.handlers.consoleCommands);

  cgame.handlers.timerun = std::make_shared<Timerun>(
      cgame.handlers.playerEvents, cgame.handlers.serverCommands);
}

static void initPlatform() {
  cgame.platform.authentication = std::make_shared<ClientAuthentication>(
      [](const std::string &command) {
        trap_SendClientCommand(command.c_str());
      },
      [](const std::string &message) { CG_Printf(message.c_str()); },
      [] { return OperatingSystem::getHwid(); }, cgame.handlers.serverCommands);

  cgame.platform.operatingSystem = std::make_shared<OperatingSystem>();

  const auto minimize = [](const std::vector<std::string> &) {
    OperatingSystem::minimize();
  };

  cgame.handlers.consoleCommands->subscribe("min", minimize);
  cgame.handlers.consoleCommands->subscribe("minimize", minimize);

  cgame.platform.syscallExt = std::make_unique<SyscallExt>();
  cgame.platform.syscallExt->setupExtensions();
  SyscallExt::trap_CmdBackup_Ext();
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
    cgame.utils.cvarUnlocker.emplace_back(
        std::make_shared<CvarUnlocker>(shadow, target));
  }
}

static void initUtils() {
  assert(cgame.handlers.timerun != nullptr);

  cgame.utils.eventLoop = std::make_shared<EventLoop>();
  initCvarUnlockers();
  cgame.utils.savePos = std::make_unique<SavePos>(cgame.handlers.timerun);
  cgame.utils.colorParser = std::make_unique<ColorParser>();
  cgame.utils.trace = std::make_unique<TraceUtils>();
  cgame.utils.pmove = std::make_unique<PmoveUtils>();
}

static void initDemo() {
  cgame.demo.compatibility = std::make_unique<DemoCompatibility>();
  cgame.demo.autoDemoRecorder = std::make_shared<AutoDemoRecorder>();
}

static void initUserInterface() {
  cgame.ui.consoleShader = std::make_shared<ConsoleShader>();
}

static void initTrickjumpLines() {
  cgame.visuals.trickjumpLines = std::make_shared<TrickjumpLines>(
      cgame.handlers.serverCommands, cgame.handlers.consoleCommands);

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
  cgame.visuals.leavesRemapper = std::make_shared<LeavesRemapper>();
  cgame.visuals.playerBBox = std::make_shared<PlayerBBox>();
  initTrickjumpLines();
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

static void initHUD() {
  assert(cgame.utils.pmove != nullptr);

  cgame.hud.accelColor = std::make_shared<AccelColor>();
  cgame.hud.chsDataHandler = std::make_shared<CHSDataHandler>(
      cgame.handlers.cvarUpdate, cgame.handlers.consoleCommands);

  cgame.hud.renderables.emplace_back(std::make_shared<CHS>(
      cgame.handlers.cvarUpdate, cgame.hud.chsDataHandler));
  cgame.hud.renderables.emplace_back(std::make_shared<OverbounceWatcher>(
      cgame.handlers.consoleCommands.get()));
  cgame.hud.renderables.emplace_back(std::make_shared<OverbounceDetector>());
  cgame.hud.renderables.emplace_back(
      std::make_shared<DisplayMaxSpeed>(cgame.handlers.entityEvents.get()));
  cgame.hud.renderables.emplace_back(std::make_shared<DrawSpeed>());
  cgame.hud.renderables.emplace_back(std::make_shared<AccelMeter>());
  cgame.hud.renderables.emplace_back(std::make_shared<StrafeQuality>());
  cgame.hud.renderables.emplace_back(
      std::make_shared<JumpSpeeds>(cgame.handlers.entityEvents.get()));
  cgame.hud.renderables.emplace_back(std::make_shared<QuickFollowDrawer>());
  cgame.hud.renderables.emplace_back(std::make_shared<SpectatorInfo>());
  cgame.hud.renderables.emplace_back(std::make_shared<AreaIndicator>());

  if (etj_CGazOnTop.integer) {
    cgame.hud.renderables.emplace_back(std::make_shared<Snaphud>());
    cgame.hud.renderables.emplace_back(std::make_shared<CGaz>());
  } else {
    cgame.hud.renderables.emplace_back(std::make_shared<CGaz>());
    cgame.hud.renderables.emplace_back(std::make_shared<Snaphud>());
  }

  cgame.hud.renderables.emplace_back(std::make_shared<UpperRight>());
  cgame.hud.renderables.emplace_back(std::make_shared<UpmoveMeter>());
  cgame.hud.renderables.emplace_back(std::make_shared<RtvDrawable>());
  cgame.hud.renderables.emplace_back(
      std::make_shared<CustomCommandMenuDrawable>());

  // FIXME: this is dumb, the init should be handled in the constructor
  const auto keySetSystem = std::make_shared<KeySetSystem>(etj_drawKeys);
  cgame.hud.renderables.emplace_back(keySetSystem);
  initDrawKeys(keySetSystem);

  // FIXME: move to renderables
  ETJump_ClearDrawables();
  cgame.hud.timerunView = std::make_shared<TimerunView>(cgame.handlers.timerun);

  // restores timerun after vid_restart (if required)
  trap_SendClientCommand("timerun_status");

  cgame.hud.renderables.emplace_back(std::make_shared<Crosshair>());
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
  initHandlers();

  initPlatform();
  initDemo();
  initUtils();
  initUserInterface();
  initVisuals();
  initHUD();

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

  if (cgame.handlers.consoleCommands) {
    cgame.handlers.consoleCommands->unsubscribe("min");
    cgame.handlers.consoleCommands->unsubscribe("minimize");
  }

  cgame.platform.operatingSystem = nullptr;
  cgame.platform.authentication = nullptr;
  cgame.hud.renderables.clear();
  cgame.utils.cvarUnlocker.clear();
  cgame.handlers.cvarUpdate = nullptr;

  // clear dynamic shaders in reverse order
  cgame.visuals.leavesRemapper = nullptr;
  cgame.ui.consoleShader = nullptr;

  if (cgame.utils.eventLoop) {
    cgame.utils.eventLoop->shutdown();
    cgame.utils.eventLoop = nullptr;
  }

  cgame.utils.trace = nullptr;

  cgame.handlers.consoleCommands = nullptr;
  cgame.handlers.serverCommands = nullptr;
  cgame.handlers.playerEvents = nullptr;
  cgame.handlers.entityEvents = nullptr;

  cgame.utils.savePos = nullptr;

  cgame.platform.syscallExt = nullptr;

  cgame.visuals.trickjumpLines = nullptr;

  cgame.hud.chsDataHandler = nullptr;

  cgame.utils.colorParser = nullptr;

  CG_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION
                                     " " S_COLOR_LTGREY GAME_BINARY_NAME
                                     " shutdown... " S_COLOR_GREEN "DONE\n");
}
} // namespace ETJump
