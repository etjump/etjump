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

#include <string>
#include <memory>

#include "cg_local.h"

#include "etj_trickjump_lines.h"
#include "etj_init.h"
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
std::shared_ptr<DemoCompatibility> demoCompatibility;
std::shared_ptr<AccelColor> accelColor;
std::array<bool, MAX_CLIENTS> tempTraceIgnoredClients;
std::shared_ptr<PlayerBBox> playerBBox;
} // namespace ETJump

static bool isInitialized{false};
static int nextNearest = 0;

namespace ETJump {
void addLoopingSound(const vec3_t origin, const vec3_t velocity,
                     sfxHandle_t sfx, int volume, int soundTime) {
  if (etj_loopedSounds.integer > 0) {
    trap_S_AddLoopingSound(origin, velocity, sfx, volume, soundTime);
  }
}

void addRealLoopingSound(const vec3_t origin, const vec3_t velocity,
                         sfxHandle_t sfx, int range, int volume,
                         int soundTime) {
  if (etj_loopedSounds.integer > 0) {
    trap_S_AddRealLoopingSound(origin, velocity, sfx, range, volume, soundTime);
  }
}

// General purpose etj_hideMe check for cgame events
bool hideMeCheck(int entityNum) {
  if (entityNum < MAX_CLIENTS) {
    // entity is a player
    if (cgs.clientinfo[entityNum].hideMe && entityNum != cg.clientNum) {
      // player is hidden and it is not ourselves
      return true;
    }
  }
  return false;
}

// Get correct trace contents depending on etj_extraTrace value
int checkExtraTrace(int value) {
  if (etj_extraTrace.integer & 1 << value) {
    return CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
  }

  return CONTENTS_SOLID;
}

int weapnumForClient() {
  if (cg.snap->ps.pm_flags & PMF_FOLLOW || cg.demoPlayback) {
    return cg.snap->ps.weapon;
  }

  return cg.weaponSelect;
}

void initTimer() {
  if (timerun) {
    timerunView->removeFromDrawables();
    timerun = nullptr;
    timerunView = nullptr;
  }
  timerun = std::make_shared<Timerun>(cg.clientNum, playerEventsHandler);
  timerunView = std::make_shared<TimerunView>(timerun);
}

void execCmdOnRunStart() {
  if (etj_onRunStart.string[0]) {
    trap_SendConsoleCommand(va("%s\n", etj_onRunStart.string));
  }
}

void execCmdOnRunEnd() {
  if (etj_onRunEnd.string[0]) {
    trap_SendConsoleCommand(va("%s\n", etj_onRunEnd.string));
  }
}

void onPlayerRespawn(qboolean revived) {
  playerEventsHandler->check("respawn", {revived ? "1" : "0"});
}

void initDrawKeys(KeySetSystem *keySetSystem) {
  // key set themes
  const char *keySetNames[]{
      "keyset",  // Keyset 1 (original)
      "keyset2", // Aciz: Keyset 2 (DeFRaG style keys)
      "keyset3", "keyset4",
      // + add more
  };
  for (const auto &keySetName : keySetNames) {
    keySetSystem->addSet(keySetName);
  }
  keySetSystem->addKeyBindSet("keyset5");
}

bool showingScores() {
  return (cg.showScores || cg.scoreFadeTime + FADE_TIME > cg.time);
}

void init() {

  CG_Printf(S_COLOR_LTGREY GAME_HEADER);
  CG_Printf(S_COLOR_LTGREY "____________________________\n");

  CG_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION
                                     " " S_COLOR_LTGREY GAME_BINARY_NAME
                                     " init...\n");

  isInitialized = false;

  // NOTE: client server commands handlers must be created before other
  // modules as other modules use them to subscribe to commands.
  // Generally all modules should get these as constructor params but
  // they're still being used in the C code
  // => make sure they're created first
  serverCommandsHandler = std::make_shared<ClientCommandsHandler>(nullptr);
  consoleCommandsHandler =
      std::make_shared<ClientCommandsHandler>(trap_AddCommand);
  entityEventsHandler = std::make_shared<EntityEventsHandler>();
  operatingSystem = std::make_shared<OperatingSystem>();
  authentication = std::make_shared<ClientAuthentication>(
      [](const std::string &command) {
        trap_SendClientCommand(command.c_str());
      },
      [](const std::string &message) { CG_Printf(message.c_str()); },
      bind(&ETJump::OperatingSystem::getHwid, ETJump::operatingSystem),
      ETJump::serverCommandsHandler);

  playerEventsHandler = std::make_shared<PlayerEventsHandler>();
  awaitedCommandHandler = std::make_shared<AwaitedCommandHandler>(
      consoleCommandsHandler, trap_SendConsoleCommand,
      [](const char *text) { Com_Printf(text); });
  eventLoop = std::make_shared<ETJump::EventLoop>();

  ////////////////////////////////////////////////////////////////
  // TODO: move these to own client commands handler
  ////////////////////////////////////////////////////////////////
  auto minimize = [](const std::vector<std::string> &args) {
    operatingSystem->minimize();
  };
  consoleCommandsHandler->subscribe("min", minimize);
  consoleCommandsHandler->subscribe("minimize", minimize);
  ////////////////////////////////////////////////////////////////

  rtvHandler = std::make_shared<ClientRtvHandler>();
  rtvHandler->initialize();

  demoCompatibility = std::make_shared<DemoCompatibility>();
  accelColor = std::make_shared<AccelColor>();

  playerBBox = std::make_shared<PlayerBBox>();

  // initialize renderables
  // Overbounce watcher
  ETJump::renderables.push_back(
      std::make_shared<OverbounceWatcher>(consoleCommandsHandler.get()));
  ETJump::renderables.push_back(std::make_shared<OverbounceDetector>());
  // Display max speed from previous load session
  ETJump::renderables.push_back(
      std::make_shared<DisplayMaxSpeed>(ETJump::entityEventsHandler.get()));
  ETJump::renderables.push_back(std::make_shared<DrawSpeed>());
  ETJump::renderables.push_back(std::make_shared<AccelMeter>());
  ETJump::renderables.push_back(std::make_shared<StrafeQuality>());
  ETJump::renderables.push_back(
      std::make_shared<JumpSpeeds>(ETJump::entityEventsHandler.get()));
  ETJump::renderables.push_back(std::make_shared<QuickFollowDrawer>());
  ETJump::renderables.push_back(std::make_shared<SpectatorInfo>());
  ETJump::renderables.push_back(std::make_shared<AreaIndicator>());

  if (etj_CGazOnTop.integer) {
    ETJump::renderables.push_back(std::make_shared<Snaphud>());
    ETJump::renderables.push_back(std::make_shared<CGaz>());
  } else {
    ETJump::renderables.push_back(std::make_shared<CGaz>());
    ETJump::renderables.push_back(std::make_shared<Snaphud>());
  }

  ETJump::renderables.push_back(std::make_shared<UpperRight>());
  ETJump::renderables.push_back(std::make_shared<UpmoveMeter>());

  ETJump::renderables.push_back(std::make_shared<RtvDrawable>());

  ETJump::consoleAlphaHandler = std::make_shared<ETJump::ConsoleAlphaHandler>();
  ETJump::drawLeavesHandler = std::make_shared<ETJump::DrawLeavesHandler>();
  auto keySetSystem = new ETJump::KeySetSystem(etj_drawKeys);
  ETJump::renderables.push_back(
      std::shared_ptr<ETJump::IRenderable>(keySetSystem));
  ETJump::initDrawKeys(keySetSystem);
  ETJump::autoDemoRecorder = std::make_shared<ETJump::AutoDemoRecorder>();

  ETJump::renderables.push_back(std::make_shared<Crosshair>());

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

  for (auto &shadow : cvars) {
    ETJump::cvarShadows.push_back(
        std::make_shared<CvarShadow>(shadow.first, shadow.second));
  }

  playerEventsHandler->subscribe("timerun:start", [](auto a) {
    auto clientNum = std::stoi(a[0]);
    if (clientNum == cg.clientNum) {
      execCmdOnRunStart();
    }
  });
  playerEventsHandler->subscribe("timerun:stop", [](auto a) {
    auto clientNum = std::stoi(a[0]);
    if (clientNum == cg.clientNum) {
      execCmdOnRunEnd();
    }
  });
  playerEventsHandler->subscribe("timerun:interrupt", [](auto a) {
    auto clientNum = std::stoi(a[0]);
    if (clientNum == cg.clientNum) {
      execCmdOnRunEnd();
    }
  });

  ETJump_ClearDrawables();
  initTimer();
  // restores timerun after vid_restart (if required)
  trap_SendClientCommand("timerun_status");

  std::fill_n(tempTraceIgnoredClients.begin(), MAX_CLIENTS, false);

  trickjumpLines = std::make_shared<TrickjumpLines>();

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

  CG_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION
                                     " " S_COLOR_LTGREY GAME_BINARY_NAME
                                     " init... " S_COLOR_GREEN "DONE\n");

  isInitialized = true;
}

void shutdown() {
  CG_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION_DATED
                                     " " S_COLOR_LTGREY GAME_BINARY_NAME
                                     " shutdown...\n");

  if (ETJump::consoleCommandsHandler) {
    ETJump::consoleCommandsHandler->unsubscribe("min");
    ETJump::consoleCommandsHandler->unsubscribe("minimize");
  }

  ETJump::operatingSystem = nullptr;
  ETJump::authentication = nullptr;
  ETJump::renderables.clear();
  ETJump::cvarShadows.clear();
  ETJump::cvarUpdateHandler = nullptr;

  // clear dynamic shaders in reverse order
  ETJump::drawLeavesHandler = nullptr;
  ETJump::consoleAlphaHandler = nullptr;
  if (ETJump::eventLoop) {
    ETJump::eventLoop->shutdown();
    ETJump::eventLoop = nullptr;
  }
  ETJump::consoleCommandsHandler = nullptr;
  ETJump::serverCommandsHandler = nullptr;
  ETJump::playerEventsHandler = nullptr;
  ETJump::entityEventsHandler = nullptr;

  isInitialized = false;

  CG_Printf(S_COLOR_LTGREY GAME_NAME " " S_COLOR_GREEN GAME_VERSION
                                     " " S_COLOR_LTGREY GAME_BINARY_NAME
                                     " shutdown... " S_COLOR_GREEN "DONE\n");
}
} // namespace ETJump

/**
 * Extended CG_ServerCommand function. Checks whether server
 * sent command matches to any defined here. If no match is found
 * returns false
 * @return qboolean Whether a match was found or not
 */
qboolean CG_ServerCommandExt(const char *cmd) {
  const std::string command = cmd != nullptr ? cmd : "";

  if (command == "timerun") {
    char arg[MAX_TOKEN_CHARS]{};
    std::vector<std::string> args;

    for (int i = 0, len = trap_Argc(); i < len; ++i) {
      trap_Argv(i, arg, sizeof(arg));

      args.push_back(arg);
    }

    ETJump::timerun->parseServerCommand(args);
    return qtrue;
  }

  if (command == "tjl_displaybyname") {
    return CG_displaybyname();
  }

  if (command == "tjl_displaybynumber") {
    return CG_displaybynumber();
  }

  return qfalse;
}

/**
 * Checks if the command exists and calls the handler
 * @param cmd The command to be matched
 * @returns qboolean Whether a match was found
 */
qboolean CG_ConsoleCommandExt(const char *cmd) {
  const std::string command = cmd ? cmd : "";

  if (command == "tjl_displaybyname") {
    return CG_displaybyname();
  }

  if (command == "tjl_displaybynumber") {
    return CG_displaybynumber();
  }

  if (command == "tjl_clearrender") {
    ETJump::trickjumpLines->setCurrentRouteToRender(-1);
    return qtrue;
  }

  // TODO: could just make an array out of this and go thru it
  if (command == "tjl_record") {
    const auto argc = trap_Argc();
    if (argc == 1) {
      ETJump::trickjumpLines->record(nullptr);
    } else {
      auto name = CG_Argv(1);
      ETJump::trickjumpLines->record(name);
    }
    return qtrue;
  }

  if (command == "tjl_stoprecord") {
    ETJump::trickjumpLines->stopRecord();
    return qtrue;
  }

  if (command == "tjl_listroute") {
    ETJump::trickjumpLines->listRoutes();
    return qtrue;
  }

  if (command == "tjl_displaynearestroute") {
    ETJump::trickjumpLines->displayNearestRoutes();
    return qtrue;
  }

  if (command == "tjl_renameroute") {
    const auto argc = trap_Argc();

    if (argc > 2) {
      const std::string name = CG_Argv(1);
      const std::string name2 = CG_Argv(2);
      ETJump::trickjumpLines->renameRoute(name.c_str(), name2.c_str());
    } else {
      ETJump::trickjumpLines->renameRoute(nullptr, nullptr);
    }
    return qtrue;
  }

  if (command == "tjl_saveroute") {
    const auto argc = trap_Argc();
    if (argc > 1) {
      const auto name = CG_Argv(1);
      ETJump::trickjumpLines->saveRoutes(name);
      return qtrue;
    } else {
      CG_Printf("Please provide a name to save your "
                "TJL. (without .tjl "
                "extension). \n");
      return qfalse;
    }
  }

  if (command == "tjl_loadroute") {
    const auto argc = trap_Argc();
    if (argc > 1) {
      const auto name = CG_Argv(1);
      ETJump::trickjumpLines->loadRoutes(name);
    } else {
      ETJump::trickjumpLines->loadRoutes(nullptr);
    }
    return qtrue;
  }

  if (command == "tjl_deleteroute") {
    const auto argc = trap_Argc();
    if (argc > 1) {
      const auto name = CG_Argv(1);
      ETJump::trickjumpLines->deleteRoute(name);
    } else {
      ETJump::trickjumpLines->deleteRoute(nullptr);
    }
    return qtrue;
  }

  if (command == "tjl_overwriterecording") {
    const auto argc = trap_Argc();
    if (argc == 1) {
      ETJump::trickjumpLines->overwriteRecording(nullptr);
    } else {
      const auto name = CG_Argv(1);
      ETJump::trickjumpLines->overwriteRecording(name);
    }
    return qtrue;
  }

  if (command == "tjl_enableline") {
    const auto argc = trap_Argc();
    if (argc == 1) {
      CG_Printf("Please add 0 or 1 as argument to "
                "enable or disable line.\n");
      return qfalse;
    } else {
      const std::string state = CG_Argv(1);
      if (state == "0") {
        ETJump::trickjumpLines->toggleRoutes(false);
      } else {
        ETJump::trickjumpLines->toggleRoutes(true);
      }
      return qtrue;
    }
  }

  if (command == "tjl_enablejumpmarker") {
    const auto argc = trap_Argc();
    if (argc == 1) {
      CG_Printf("Please add 0 or 1 as argument to "
                "enable or disable marker.\n");
      return qfalse;
    } else {
      std::string state = CG_Argv(1);
      CG_Printf("Enable marker arg : %s \n", state.c_str());
      if (state == "0") {
        ETJump::trickjumpLines->toggleMarker(false);
      } else {
        ETJump::trickjumpLines->toggleMarker(true);
      }
      return qtrue;
    }
  }

  if (command == "ftSaveLimitSet") {
    char buffer[MAX_CVAR_VALUE_STRING];
    int limit;

    trap_Cvar_VariableStringBuffer("etj_ftSaveLimit", buffer, sizeof(buffer));
    limit = Q_atoi(buffer);

    trap_SendConsoleCommand(va("fireteam rules savelimit %i\n", limit));
    return qtrue;
  }
  return qfalse;
}

// TODO : (Zero) And this prolly should be elsewhere (e.g. cg_view_ext.cpp) but
// I'll just go with this one for now.. :P
void CG_DrawActiveFrameExt() {
  // Check if recording
  if (ETJump::trickjumpLines->isRecording()) {
    // TODO : (xis) player origin doesn't change if crouch or
    // prone, stay on feet. //cg.refdef.vieworg ,
    // //cg.predictedPlayerState.origin
    ETJump::trickjumpLines->addPosition(cg.predictedPlayerState.origin);
  } else {
    // Check if nearest mode is activate
    if (etj_tjlNearestInterval.integer > 0) {
      // Check if nearest mode timer is due to check for
      // nearest.
      if (nextNearest < cg.time) {
        if (ETJump::trickjumpLines->isDebug()) {
          CG_Printf("Check for nearest "
                    "line!. \n");
        }
        ETJump::trickjumpLines->displayNearestRoutes();
        nextNearest = cg.time + 1000 * etj_tjlNearestInterval.integer;
      }
    }

    // Check if line or jumper marker are enable.
    if (ETJump::trickjumpLines->isEnableLine() ||
        ETJump::trickjumpLines->isEnableMarker()) {
      // Check if record or if there line in the list
      if (ETJump::trickjumpLines->countRoute() > 0 &&
          !ETJump::trickjumpLines->isRecording()) {
        // Check if display has not been
        // cleared.
        if (ETJump::trickjumpLines->getCurrentRouteToRender() != -1) {
          // Display current route with
          // the #
          ETJump::trickjumpLines->displayCurrentRoute(
              ETJump::trickjumpLines->getCurrentRouteToRender());
        }
      }
    }
  }
}

namespace ETJump {
void runFrameEnd() {
  awaitedCommandHandler->runFrame();
  eventLoop->run();

  // force original cvars to match the shadow values
  // we need to delay this a bit from initial cgame load because ETe and ETL
  // reset cheat cvars to original values after cgame VMCall is done
  if (cg.clientFrame >= 10 && !cg.shadowCvarsSet) {
    for (auto &cvarShadow : cvarShadows) {
      cvarShadow->forceCvarSet();
    }
    cg.shadowCvarsSet = true;
  }

  if (!cg.demoPlayback && cg.clientFrame >= 10 && !cg.chatReplayReceived) {
    if (etj_chatReplay.integer) {
      trap_SendConsoleCommand("getchatreplay");
    }

    // keep this separate from the cvar check, so client doesn't immediately
    // receive chat replay in the middle of a map when toggling this,
    // as the replay would just be whatever is currently in chat
    cg.chatReplayReceived = true;
  }

  if (etj_autoPortalBinds.integer) {
    // confusingly cg.weaponSelect gets set to followed clients weapon,
    // so we need to do this only if we're not in spec
    if (isPlaying(cg.clientNum) && cg.weaponSelect == WP_PORTAL_GUN &&
        !cg.portalgunBindingsAdjusted) {
      cgDC.getKeysForBinding("weapalt", &cg.weapAltB1, &cg.weapAltB2);

      if (cg.weapAltB1 != -1) {
        trap_Key_SetBinding(cg.weapAltB1, "+attack2");
        cg.portalgunBindingsAdjusted = true;
      }

      if (cg.weapAltB2 != -1) {
        trap_Key_SetBinding(cg.weapAltB2, "+attack2");
        cg.portalgunBindingsAdjusted = true;
      }
      // since you never spawn with a portalgun,
      // binds should reset at the very beginning of a level too
    } else if (((cg.weaponSelect != WP_PORTAL_GUN ||
                 !isPlaying(cg.clientNum)) &&
                cg.portalgunBindingsAdjusted) ||
               cg.clientFrame < 10) {
      cgDC.getKeysForBinding("+attack2", &cg.weapAltB1, &cg.weapAltB2);

      if (cg.weapAltB1 != -1) {
        trap_Key_SetBinding(cg.weapAltB1, "weapalt");
        cg.portalgunBindingsAdjusted = false;
      }

      if (cg.weapAltB2 != -1) {
        trap_Key_SetBinding(cg.weapAltB2, "weapalt");
        cg.portalgunBindingsAdjusted = false;
      }
    }
  }
}

playerState_t *getValidPlayerState() {
  return (cg.snap->ps.clientNum != cg.clientNum || cg.demoPlayback)
             // spectating/demo playback
             ? &cg.snap->ps
             // playing
             : &cg.predictedPlayerState;
}
} // namespace ETJump

qboolean CG_displaybyname() {
  const auto argc = trap_Argc();
  if (argc > 1) {
    const auto name = CG_Argv(1);
    ETJump::trickjumpLines->displayByName(name);
  } else {
    ETJump::trickjumpLines->displayByName(nullptr);
  }
  return qtrue;
}

qboolean CG_displaybynumber() {
  const auto argc = trap_Argc();
  if (argc > 1) {
    const auto number = Q_atoi(CG_Argv(1));
    const auto total = ETJump::trickjumpLines->countRoute();
    if (number > -1 && number < total) {
      ETJump::trickjumpLines->setCurrentRouteToRender(number);
      return qtrue;
    }
    return qfalse;
  } else {
    CG_Printf("You need to pass the route number by argument. "
              "Use command "
              "/tjl_listroute to get number. \n");
    return qfalse;
  }
}

void CG_ResetTransitionEffects() {
  cg.damageTime = cg.duckTime = cg.landTime = cg.stepTime = 0;
}
