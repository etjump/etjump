#include "etj_spectator_hud_sync.h"

#include <array>
#include <string>
#include <vector>

#include "cg_local.h"
#include "etj_cgaz_v2.h"
#include "etj_snaphud_v2.h"

namespace ETJump {
namespace {
using CgazValues = std::array<std::string,
                              Constants::SpectatorHudSync::CgazCvarNames.size()>;
using SnaphudValues =
    std::array<std::string,
               Constants::SpectatorHudSync::SnaphudCvarNames.size() + 1>;

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
    payload += SpectatorHudSyncCodec::encodeValue(values[i]);
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

  const auto fields = SpectatorHudSyncCodec::parseEntries(payload);

  for (const auto &[index, decoded] : fields) {
    if (index < 0 || static_cast<size_t>(index) >= values.size()) {
      continue;
    }

    if (Q_stricmp(values[index].data(), decoded.c_str()) != 0) {
      Q_strncpyz(values[index].data(), decoded.c_str(), values[index].size());
      changed = true;
    }
  }

  return changed;
}

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

bool getLocalHudValues(CgazValues &cgazValues, SnaphudValues &snaphudValues) {
  if (!cgame.hud.cgazV2 || !cgame.hud.snaphudV2) {
    return false;
  }

  cgazValues = cgame.hud.cgazV2->getHudSyncValues();
  snaphudValues = cgame.hud.snaphudV2->getHudSyncValues();
  return true;
}

const char *getOverrideHudString(const vmCvar_t *cvar) {
  const int clientNum = cgame.spectatorHudSync.overrideClient;
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

void requestFollowedHudSync(const int followedClient, const int32_t knownRevision) {
  if (followedClient < 0) {
    return;
  }

  trap_SendClientCommand(
      va("%s %d %d", Constants::SpectatorHudSync::WatchCommand.data(),
         followedClient, knownRevision));

  cgame.spectatorHudSync.lastWatchRequestTime = cg.time;
}

void sendLocalHudSync() {
  auto &sync = cgame.spectatorHudSync;
  const bool fullUpdate = !sync.localBaselineSent;

  CgazValues currentCgazValues;
  SnaphudValues currentSnaphudValues;
  if (!getLocalHudValues(currentCgazValues, currentSnaphudValues)) {
    return;
  }

  const std::string cgazPayload = buildHudPayload(
      currentCgazValues, sync.lastSentCgazValues, fullUpdate);
  const std::string snaphudPayload =
      buildHudPayload(currentSnaphudValues, sync.lastSentSnaphudValues,
                      fullUpdate);

  if (!fullUpdate && cgazPayload == "-" && snaphudPayload == "-") {
    return;
  }

  sync.localRevision++;

  trap_SendClientCommand(
      va("%s %d %d %s %s", Constants::SpectatorHudSync::UploadCommand.data(),
         fullUpdate ? 1 : 0, sync.localRevision, cgazPayload.c_str(),
         snaphudPayload.c_str()));

  sync.localBaselineSent = true;
  sync.lastSentCgazValues = currentCgazValues;
  sync.lastSentSnaphudValues = currentSnaphudValues;
}
} // namespace

const char *spectatorHudSyncEffectiveHudCvarString(const vmCvar_t *cvar) {
  const char *overrideValue = getOverrideHudString(cvar);
  return overrideValue ? overrideValue : cvar->string;
}

int32_t spectatorHudSyncEffectiveHudCvarInt(const vmCvar_t *cvar) {
  return Q_atoi(spectatorHudSyncEffectiveHudCvarString(cvar));
}

float spectatorHudSyncEffectiveHudCvarFloat(const vmCvar_t *cvar) {
  return Q_atof(spectatorHudSyncEffectiveHudCvarString(cvar));
}

void spectatorHudSyncReset() { cgame.spectatorHudSync = SpectatorHudSync(); }

void spectatorHudSyncRunFrame() {
  if (cg.demoPlayback) {
    return;
  }

  auto &sync = cgame.spectatorHudSync;

  const int followedClient = getFollowedClient();
  const bool wantsRemoteHud = showSpectatedHud() && followedClient >= 0 &&
                              followedClient < MAX_CLIENTS;

  if (!wantsRemoteHud) {
    sync.watchRegistered = false;
    sync.watchedClient = -1;
    sync.watchedRevision = -1;
  } else {
    const auto &clientInfo = cgs.clientinfo[followedClient];

    if (!clientInfo.specHudAllowed) {
      // force a fresh watch request once sharing is re-enabled
      sync.watchRegistered = false;
      sync.watchedRevision = -1;
    }

    if (!sync.watchRegistered || sync.watchedClient != followedClient) {
      sync.watchRegistered = true;
      sync.watchedClient = followedClient;
      sync.watchedRevision = -1;
      requestFollowedHudSync(followedClient, sync.watchedRevision);
    } else if (clientInfo.specHudAllowed &&
               cg.time - sync.lastWatchRequestTime >=
                   Constants::SpectatorHudSync::WatchRetryIntervalMs) {
      // recovery re-watch: pull baseline if missing, otherwise pull by known rev
      const int32_t knownRevision = clientInfo.hasFullHudSync
                                        ? clientInfo.hudSyncRevision
                                        : -1;
      requestFollowedHudSync(followedClient, knownRevision);
    }
  }

  sync.overrideClient = -1;
  if (wantsRemoteHud) {
    const auto &clientInfo = cgs.clientinfo[followedClient];
    if (clientInfo.specHudAllowed && clientInfo.hasFullHudSync) {
      sync.overrideClient = followedClient;
    }
  }

  sendLocalHudSync();
}

void spectatorHudSyncOnServerCommand(int targetClientNum, bool fullUpdate,
                                     int32_t revision,
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
    clientInfo.hudSyncRevision = revision;

    auto &sync = cgame.spectatorHudSync;
    if (sync.watchedClient == targetClientNum) {
      sync.watchedRevision = revision;
    }
  }
}
} // namespace ETJump
