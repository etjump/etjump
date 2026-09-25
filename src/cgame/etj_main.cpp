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

#include "etj_accelmeter_v2.h"
#include "etj_areaindicator_drawable.h"
#include "etj_autodemo_recorder.h"
#include "etj_awaited_command_handler.h"
#include "etj_cgame.h"
#include "etj_cgaz_data.h"
#include "etj_cgaz_v2.h"
#include "etj_chs_drawable.h"
#include "etj_client_authentication.h"
#include "etj_client_commands_handler.h"
#include "etj_client_rtv_handler.h"
#include "etj_color_parser.h"
#include "etj_command_complete_ext.h"
#include "etj_console_shader.h"
#include "etj_consolecommands.h"
#include "etj_crosshair.h"
#include "etj_custom_command_menu.h"
#include "etj_custom_command_menu_drawable.h"
#include "etj_cvar_unlocker.h"
#include "etj_demo_compatibility.h"
#include "etj_draw_yaw.h"
#include "etj_drawspeed2_v2.h"
#include "etj_entity_events_handler.h"
#include "etj_event_loop.h"
#include "etj_falldamage_detector.h"
#include "etj_jump_speeds_v2.h"
#include "etj_keyset_system.h"
#include "etj_leaves_remapper.h"
#include "etj_local.h"
#include "etj_maxspeed.h"
#include "etj_operating_system.h"
#include "etj_overbounce_detector.h"
#include "etj_overbounce_watcher.h"
#include "etj_player_bbox.h"
#include "etj_player_events_handler.h"
#include "etj_quick_follow_drawable.h"
#include "etj_rtv_drawable.h"
#include "etj_savepos.h"
#include "etj_servercommands.h"
#include "etj_snaphud_data.h"
#include "etj_snaphud_v2.h"
#include "etj_spectatorinfo_data.h"
#include "etj_spectatorinfo_drawable.h"
#include "etj_strafe_quality_v2.h"
#include "etj_timerun.h"
#include "etj_timerun_view.h"
#include "etj_trace_utils.h"
#include "etj_trickjump_lines.h"
#include "etj_upmove_meter_data.h"
#include "etj_upmove_meter_v2.h"
#include "etj_upper_right_drawable.h"

#include "../game/etj_syscall_ext_shared.h"
#include "../game/etj_worldspawn_shared.h"

namespace ETJump {
namespace {
using CgazValues = std::array<std::string,
                              Constants::SpectatorHudSync::CgazCvarNames.size()>;
using SnaphudValues =
    std::array<std::string,
               Constants::SpectatorHudSync::SnaphudCvarNames.size() + 1>;

struct SpectatorHudSyncState {
  bool fullSent = false;
  int32_t lastUploadTime = 0;
  int32_t lastRequestTime = 0;
  int32_t watchedClient = -1;
  int32_t overrideClient = -1;
  CgazValues lastSentCgazValues;
  SnaphudValues lastSentSnaphudValues;
};

SpectatorHudSyncState hudSyncState;

const auto &cgazCvars() {
  static const std::array<vmCvar_t *, Constants::SpectatorHudSync::CgazCvarNames
                                           .size()>
      values = {
          &etj_drawCGaz,
          &etj_CGazY,
          &etj_CGaz2Y,
          &etj_CGazHeight,
          &etj_CGaz2Color1,
          &etj_CGaz2Color2,
          &etj_CGaz1Color1,
          &etj_CGaz1Color2,
          &etj_CGaz1Color3,
          &etj_CGaz1Color4,
          &etj_CGazFov,
          &etj_CGazTrueness,
          &etj_CGazOnTop,
          &etj_CGaz2FixedSpeed,
          &etj_CGaz2NoVelocityDir,
          &etj_CGaz1DrawSnapZone,
          &etj_CGaz2WishDirFixedSpeed,
          &etj_CGaz2WishDirUniformLength,
          &etj_CGaz1DrawMidLine,
          &etj_CGaz1MidlineColor,
          &etj_CGaz2HighRes,
          &etj_CGaz2Thickness1,
          &etj_CGaz2Thickness2,
          &etj_stretchCgaz,
      };

  return values;
}

const auto &snaphudCvars() {
  static const std::array<vmCvar_t *,
                          Constants::SpectatorHudSync::SnaphudCvarNames.size() +
                              1>
      values = {
          &etj_drawSnapHUD,
          &etj_snapHUDOffsetY,
          &etj_snapHUDHeight,
          &etj_snapHUDColor1,
          &etj_snapHUDColor2,
          &etj_snapHUDHLColor1,
          &etj_snapHUDHLColor2,
          &etj_snapHUDFov,
          &etj_snapHUDHLActive,
          &etj_snapHUDTrueness,
          &etj_snapHUDEdgeThickness,
          &etj_snapHUDBorderThickness,
          &etj_snapHUDActiveIsPrimary,
          &etj_snapHUDCrop,
          &etj_snapHUDCropOffsets,
      };

  return values;
}

template <size_t N>
std::array<std::string, N>
collectHudValues(const std::array<vmCvar_t *, N> &cvars) {
  std::array<std::string, N> values;
  for (size_t i = 0; i < cvars.size(); ++i) {
    values[i] = cvars[i]->string;
  }

  return values;
}

inline std::string encodeHudValue(const std::string &value) {
  std::string encoded = value;
  for (auto &ch : encoded) {
    if (ch == ' ') {
      ch = '~';
    }
  }
  return encoded;
}

inline std::string decodeHudValue(const std::string &value) {
  std::string decoded = value;
  for (auto &ch : decoded) {
    if (ch == '~') {
      ch = ' ';
    }
  }
  return decoded;
}

std::vector<std::string> splitHudPayload(const std::string &payload,
                                         const char separator) {
  std::vector<std::string> parts;
  size_t start = 0;

  while (start <= payload.size()) {
    const auto pos = payload.find(separator, start);

    if (pos == std::string::npos) {
      parts.push_back(payload.substr(start));
      break;
    }

    parts.push_back(payload.substr(start, pos - start));
    start = pos + 1;
  }

  return parts;
}

template <size_t N>
std::string buildHudPayload(const std::array<std::string, N> &values,
                            const std::array<std::string, N> &reference,
                            bool fullUpdate) {
  std::string payload;

  for (size_t i = 0; i < values.size(); ++i) {
    if (!fullUpdate && values[i] == reference[i]) {
      continue;
    }

    if (!payload.empty()) {
      payload += Constants::SpectatorHudSync::FieldSeparator;
    }

    payload += std::to_string(i);
    payload += Constants::SpectatorHudSync::KvSeparator;
    payload += encodeHudValue(values[i]);
  }

  return payload.empty() ? "-" : payload;
}

template <size_t N>
bool parseHudPayloadToClientInfo(
    const std::string &payload,
    std::array<std::array<char, MAX_CVAR_VALUE_STRING>, N> &values,
    bool fullUpdate) {
  bool changed = false;

  if (fullUpdate) {
    for (auto &entry : values) {
      entry[0] = '\0';
    }
  }

  if (payload.empty() || payload == "-") {
    return false;
  }

  const auto fields =
      splitHudPayload(payload, Constants::SpectatorHudSync::FieldSeparator);

  for (const auto &field : fields) {
    if (field.empty()) {
      continue;
    }

    const auto pos = field.find(Constants::SpectatorHudSync::KvSeparator);
    if (pos == std::string::npos) {
      continue;
    }

    const int index = Q_atoi(field.substr(0, pos).c_str());
    if (index < 0 || static_cast<size_t>(index) >= values.size()) {
      continue;
    }

    const std::string decoded = decodeHudValue(field.substr(pos + 1));
    if (Q_stricmp(values[index].data(), decoded.c_str()) != 0) {
      Q_strncpyz(values[index].data(), decoded.c_str(), values[index].size());
      changed = true;
    }
  }

  return changed;
}

bool hasValidCropOffsetValue(const char *value);

int getFollowedClient() {
  if (cg.mvTotalClients > 0 && cg.mvCurrentActive) {
    const int mvClient = (cg.mvCurrentActive->mvInfo & MV_PID);
    if (mvClient >= 0 && mvClient < MAX_CLIENTS && mvClient != cg.clientNum) {
      return mvClient;
    }
  }

  if (!cg.snap || !(cg.snap->ps.pm_flags & PMF_FOLLOW) ||
      cg.snap->ps.clientNum == cg.clientNum) {
    return -1;
  }

  return cg.snap->ps.clientNum;
}

bool showSpectatedHud() { return etj_specHudShow.integer != 0; }

bool allowSpectatingMyHud() { return etj_specHudAllow.integer != 0; }

bool hasValidCropOffsetValue(const char *value) {
  if (!value || value[0] == '\0') {
    return false;
  }

  const std::string_view str(value);
  if (str.find_first_not_of(" \t\r\n") == std::string_view::npos) {
    return false;
  }

  return str.find_first_of(" \t") != std::string_view::npos;
}

const char *getOverrideHudString(const vmCvar_t *cvar) {
  const int clientNum = hudSyncState.overrideClient;
  if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
    return nullptr;
  }

  const auto &clientInfo = cgs.clientinfo[clientNum];
  if (!clientInfo.hasFullHudSync) {
    return nullptr;
  }

  for (size_t i = 0; i < cgazCvars().size(); ++i) {
    if (cvar == cgazCvars()[i] && clientInfo.cgazHudValues[i][0] != '\0') {
      return clientInfo.cgazHudValues[i].data();
    }
  }

  for (size_t i = 0; i < Constants::SpectatorHudSync::SnaphudCvarNames.size();
       ++i) {
    if (cvar == snaphudCvars()[i] && clientInfo.snaphudHudValues[i][0] != '\0') {
      return clientInfo.snaphudHudValues[i].data();
    }
  }

  if (cvar == snaphudCvars().back() &&
      hasValidCropOffsetValue(clientInfo.snaphudHudValues.back().data())) {
    return clientInfo.snaphudHudValues.back().data();
  }

  return nullptr;
}

void requestFollowedHudSync(const int followedClient) {
  if (followedClient < 0 ||
      cg.time - hudSyncState.lastRequestTime <
          Constants::SpectatorHudSync::RequestMinIntervalMs) {
    return;
  }

  hudSyncState.lastRequestTime = cg.time;
  trap_SendClientCommand(
      va("%s %d", Constants::SpectatorHudSync::RequestCommand.data(),
         followedClient));
}

void sendLocalHudSync() {
  const bool fullUpdate = !hudSyncState.fullSent;
  if (!fullUpdate &&
      cg.time - hudSyncState.lastUploadTime <
          Constants::SpectatorHudSync::UploadMinIntervalMs) {
    return;
  }

  const auto currentCgazValues = collectHudValues(cgazCvars());
  const auto currentSnaphudValues = collectHudValues(snaphudCvars());

  const std::string cgazPayload = buildHudPayload(
      currentCgazValues, hudSyncState.lastSentCgazValues, fullUpdate);
  const std::string snaphudPayload =
      buildHudPayload(currentSnaphudValues, hudSyncState.lastSentSnaphudValues,
                      fullUpdate);

  if (!fullUpdate && cgazPayload == "-" && snaphudPayload == "-") {
    return;
  }

  trap_SendClientCommand(
      va("%s %d %s %s", Constants::SpectatorHudSync::UploadCommand.data(),
         fullUpdate ? 1 : 0, cgazPayload.c_str(), snaphudPayload.c_str()));

  hudSyncState.fullSent = true;
  hudSyncState.lastUploadTime = cg.time;
  hudSyncState.lastSentCgazValues = currentCgazValues;
  hudSyncState.lastSentSnaphudValues = currentSnaphudValues;
}
} // namespace

const char *effectiveHudCvarString(const vmCvar_t *cvar) {
  const char *overrideValue = getOverrideHudString(cvar);
  return overrideValue ? overrideValue : cvar->string;
}

int32_t effectiveHudCvarInt(const vmCvar_t *cvar) {
  return Q_atoi(effectiveHudCvarString(cvar));
}

float effectiveHudCvarFloat(const vmCvar_t *cvar) {
  return Q_atof(effectiveHudCvarString(cvar));
}

void runSpectatorHudSyncFrame() {
  if (cg.demoPlayback) {
    return;
  }

  const int followedClient = getFollowedClient();
  if (followedClient != hudSyncState.watchedClient) {
    hudSyncState.watchedClient = followedClient;
    hudSyncState.lastRequestTime = 0;
  }

  hudSyncState.overrideClient = -1;

  if (followedClient >= 0 && followedClient < MAX_CLIENTS && showSpectatedHud()) {
    requestFollowedHudSync(followedClient);

    const auto &clientInfo = cgs.clientinfo[followedClient];
    if (clientInfo.specHudAllowed && clientInfo.hasFullHudSync) {
      hudSyncState.overrideClient = followedClient;
    }
  }

  sendLocalHudSync();
}

void onSpectatorHudSyncCommand(int targetClientNum, bool fullUpdate,
                               const std::string &cgazPayload,
                               const std::string &snaphudPayload) {
  if (targetClientNum < 0 || targetClientNum >= MAX_CLIENTS) {
    return;
  }

  auto &clientInfo = cgs.clientinfo[targetClientNum];

  bool changed = false;
  changed |= parseHudPayloadToClientInfo(cgazPayload, clientInfo.cgazHudValues,
                                         fullUpdate);
  changed |= parseHudPayloadToClientInfo(snaphudPayload,
                                         clientInfo.snaphudHudValues,
                                         fullUpdate);

  if (fullUpdate) {
    clientInfo.hasFullHudSync = true;
    changed = true;
  }

  if (clientInfo.hasFullHudSync && changed) {
    clientInfo.hudSyncRevision++;
  }
}

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

  runSpectatorHudSyncFrame();
}

void parseWorldspawnKeys() {
  const char *s = CG_ConfigString(CS_ETJUMP_WS_KEYS);

  // this won't be present in old demos
  if (s[0] == '\0') {
    return;
  }

  cgame.sharedWSKeysGlobal.noOverbounce =
      Q_atoi(Info_ValueForKey(s, WorldspawnShared::NO_OVERBOUNCE_CS));
  cgame.sharedWSKeysGlobal.noJumpDelay =
      Q_atoi(Info_ValueForKey(s, WorldspawnShared::NO_JUMP_DELAY_CS));
  cgame.sharedWSKeysGlobal.noDrop =
      Q_atoi(Info_ValueForKey(s, WorldspawnShared::NO_DROP_CS));
  cgame.sharedWSKeysGlobal.noWallbug =
      Q_atoi(Info_ValueForKey(s, WorldspawnShared::NO_WALLBUG_CS));
  cgame.sharedWSKeysGlobal.portalPredict =
      Q_atoi(Info_ValueForKey(s, WorldspawnShared::PORTAL_PREDICT_CS));

  cgame.sharedWSKeysGlobal.noSave =
      std::clamp(static_cast<AreaOpts>(
                     Q_atoi(Info_ValueForKey(s, WorldspawnShared::NO_SAVE_CS))),
                 AreaOpts::FORBID_INSIDE, AreaOpts::FORBID_EVERYWHERE);
  cgame.sharedWSKeysGlobal.noProne =
      std::clamp(static_cast<AreaOpts>(Q_atoi(
                     Info_ValueForKey(s, WorldspawnShared::NO_PRONE_CS))),
                 AreaOpts::FORBID_INSIDE, AreaOpts::FORBID_EVERYWHERE);
  cgame.sharedWSKeysGlobal.noNoclip =
      std::clamp(static_cast<AreaOpts>(Q_atoi(
                     Info_ValueForKey(s, WorldspawnShared::NO_NOCLIP_CS))),
                 AreaOpts::FORBID_INSIDE, AreaOpts::FORBID_EVERYWHERE);

  cgame.sharedWSKeysGlobal.noFallDamage =
      std::clamp(static_cast<NoFallDamageOpts>(Q_atoi(
                     Info_ValueForKey(s, WorldspawnShared::NO_FALL_DAMAGE_CS))),
                 NoFallDamageOpts::OFF, NoFallDamageOpts::FORCE_ON);
  cgame.sharedWSKeysGlobal.overbouncePlayers = std::clamp(
      static_cast<OverbouncePlayersOpts>(
          Q_atoi(Info_ValueForKey(s, WorldspawnShared::OVERBOUNCE_PLAYERS_CS))),
      OverbouncePlayersOpts::INHERIT, OverbouncePlayersOpts::FORCE_OFF);
}

void parseWorldspawnKeyOverrides() {
  cgame.wsKeyOverrides.clear();
  const char *s = CG_ConfigString(CS_ETJUMP_WS_KEY_OVERRIDES);

  // no overrides for the current map
  if (s[0] == '\0') {
    return;
  }

  auto splits = StringUtils::split(s, "\\");

  for (const auto &split : splits) {
    // first entry should be empty since the string starts with '\'
    if (split.empty()) {
      continue;
    }

    const auto values = StringUtils::split(split, ":");

    if (values.size() != 3) {
      CG_Printf(S_COLOR_RED "%s: malformed value in configstring: %s\n",
                __func__, StringUtils::join(values, ":").c_str());
      continue;
    }

    std::optional<WorldspawnShared::Keys> key;

    for (const auto &desc : WorldspawnShared::keyDescriptors) {
      if (!desc.csName || !desc.allowOverride) {
        continue;
      }

      if (StringUtils::iEqual(desc.csName, values[0])) {
        key = desc.key;
        break;
      }
    }

    if (!key.has_value()) {
      CG_Printf(S_COLOR_RED
                "%s: failed to find key for configstring value '%s'\n",
                __func__, values[0].c_str());
      continue;
    }

    WorldspawnShared::KeyOverride keyOverride{};
    keyOverride.key = key.value();
    keyOverride.value = Q_atoi(values[1]);
    keyOverride.flags =
        EnumBitset<WorldspawnShared::KeyOverrideFlags>(Q_atoi(values[2]));

    cgame.wsKeyOverrides.emplace_back(keyOverride);
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

  cgame.systems.commandCompletions = std::make_unique<CommandCompletions>();
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

static void initHUDData() {
  cgame.hudData.cgaz = std::make_shared<CGazData>();
  cgame.hudData.snaphud = std::make_shared<SnaphudData>();
  cgame.hudData.upmove = std::make_shared<UpmoveMeterData>(
      cgame.core.consoleCommands, cgame.core.playerEvents);
  cgame.hudData.chs = std::make_shared<CHSData>(
      cgame.hudData.upmove, cgame.core.cvarUpdate, cgame.core.consoleCommands);
  cgame.hudData.spectatorInfo = std::make_shared<SpectatorInfoData>();

  cgame.hudData.pmoveV2 = std::make_unique<PmoveUtilsV2>(cgame.core.cvarUpdate);
}

static void initHUD() {
  assert(cgame.hudData.cgaz && cgame.hudData.snaphud && cgame.hudData.upmove &&
         cgame.hudData.chs && cgame.hudData.spectatorInfo &&
         cgame.hudData.pmoveV2);

  cgame.hud.renderables.emplace_back(
      std::make_unique<CHS>(cgame.core.cvarUpdate, cgame.hudData.chs));
  cgame.hud.renderables.emplace_back(std::make_unique<OverbounceWatcher>(
      cgame.core.consoleCommands, cgame.core.cvarUpdate));
  cgame.hud.renderables.emplace_back(std::make_unique<OverbounceDetector>());
  cgame.hud.renderables.emplace_back(std::make_unique<DisplayMaxSpeed>(
      cgame.core.entityEvents, cgame.core.cvarUpdate));
  cgame.hud.renderables.emplace_back(std::make_unique<DrawSpeed2>(
      cgame.core.cvarUpdate, cgame.core.consoleCommands));
  cgame.hud.renderables.emplace_back(
      std::make_unique<AccelMeterV2>(cgame.core.cvarUpdate));
  cgame.hud.renderables.emplace_back(std::make_unique<StrafeQualityV2>(
      cgame.hudData.snaphud, cgame.core.cvarUpdate, cgame.core.consoleCommands,
      cgame.core.playerEvents));
  cgame.hud.renderables.emplace_back(std::make_unique<JumpSpeedsV2>(
      cgame.core.entityEvents, cgame.core.playerEvents,
      cgame.core.consoleCommands, cgame.core.serverCommands,
      cgame.core.cvarUpdate));
  cgame.hud.renderables.emplace_back(std::make_unique<QuickFollowDrawer>());
  cgame.hud.renderables.emplace_back(std::make_unique<SpectatorInfo>(
      cgame.core.cvarUpdate, cgame.hudData.spectatorInfo));
  cgame.hud.renderables.emplace_back(std::make_unique<AreaIndicator>());

  if (etj_CGazOnTop.integer) {
    cgame.hud.renderables.emplace_back(std::make_unique<SnaphudV2>(
        cgame.hudData.snaphud, cgame.core.cvarUpdate));
    cgame.hud.renderables.emplace_back(std::make_unique<CGazV2>(
        cgame.hudData.cgaz, cgame.hudData.snaphud, cgame.core.cvarUpdate));
  } else {
    cgame.hud.renderables.emplace_back(std::make_unique<CGazV2>(
        cgame.hudData.cgaz, cgame.hudData.snaphud, cgame.core.cvarUpdate));
    cgame.hud.renderables.emplace_back(std::make_unique<SnaphudV2>(
        cgame.hudData.snaphud, cgame.core.cvarUpdate));
  }

  cgame.hud.renderables.emplace_back(std::make_unique<UpperRight>());
  cgame.hud.renderables.emplace_back(std::make_unique<UpmoveMeterV2>(
      cgame.hudData.upmove, cgame.core.cvarUpdate));

  cgame.hud.renderables.emplace_back(
      std::make_unique<KeySetSystem>(&etj_drawKeys, cgame.core.cvarUpdate));
  cgame.hud.renderables.emplace_back(
      std::make_unique<DrawYaw>(cgame.core.cvarUpdate));
  cgame.hud.renderables.emplace_back(std::make_unique<FalldamageDetector>());

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
  initHUDData();
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
