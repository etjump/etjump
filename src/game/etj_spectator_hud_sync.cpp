#include "etj_spectator_hud_sync.h"

#include "etj_local.h"
#include "g_local.h"

namespace ETJump::SpectatorHudSync {
namespace {
template <size_t N>
void clearHudValues(std::array<std::array<char, MAX_CVAR_VALUE_STRING>, N> &out) {
  for (auto &entry : out) {
    entry[0] = '\0';
  }
}

template <size_t N>
bool parseHudPayload(const std::string &payload,
                     std::array<std::array<char, MAX_CVAR_VALUE_STRING>, N>
                         &out,
                     const bool fullUpdate) {
  bool changed = false;

  if (fullUpdate) {
    clearHudValues(out);
  }

  if (payload.empty() || payload == "-") {
    return false;
  }

  const auto entries = SpectatorHudSyncCodec::parseEntries(payload);

  for (const auto &[index, decoded] : entries) {
    if (index < 0 || static_cast<size_t>(index) >= out.size()) {
      continue;
    }

    if (Q_stricmp(out[index].data(), decoded.c_str()) != 0) {
      Q_strncpyz(out[index].data(), decoded.c_str(), out[index].size());
      changed = true;
    }
  }

  return changed;
}

template <size_t N>
std::string buildFullHudPayload(
    const std::array<std::array<char, MAX_CVAR_VALUE_STRING>, N> &values) {
  std::string payload;

  for (size_t i = 0; i < values.size(); ++i) {
    if (values[i][0] == '\0') {
      continue;
    }

    if (!payload.empty()) {
      payload += ETJump::Constants::SpectatorHudSync::FieldSeparator;
    }

    payload += std::to_string(i);
    payload += ETJump::Constants::SpectatorHudSync::KvSeparator;
    payload += SpectatorHudSyncCodec::encodeValue(values[i].data());
  }

  return payload;
}

inline bool isValidClientNum(const int clientNum) {
  return clientNum >= 0 && clientNum < level.maxclients;
}

inline bool isConnectedClient(const int clientNum) {
  if (!isValidClientNum(clientNum)) {
    return false;
  }

  const auto &client = level.clients[clientNum];
  return client.pers.connected == CON_CONNECTED ||
         client.pers.connected == CON_CONNECTING;
}

inline bool allowsSpectatorHudSharing(const clientPersistant_t &pers) {
  return (pers.clientFlags & CGF_SPEC_HUD_ALLOW) != 0;
}

inline void sendHudSyncToClient(const int receiverClientNum,
                                const int sourceClientNum,
                                const bool fullUpdate,
                                const int32_t revision,
                                const std::string &cgazPayload,
                                const std::string &snaphudPayload) {
  trap_SendServerCommand(
      receiverClientNum,
      va("%s %d %d %d %s %s",
         ETJump::Constants::SpectatorHudSync::ServerCommand.data(),
         sourceClientNum, fullUpdate ? 1 : 0, revision, cgazPayload.c_str(),
         snaphudPayload.c_str()));
}

void clearWatchState(clientPersistant_t &pers) {
  pers.hudSyncWatchedTarget = -1;
  pers.hudSyncWatchedRevision = -1;
}

void notifyWatchers(int sourceClientNum) {
  const auto &sourcePers = level.clients[sourceClientNum].pers;
  if (!sourcePers.hasFullHudSync || !allowsSpectatorHudSharing(sourcePers)) {
    return;
  }

  const auto cgazPayload = buildFullHudPayload(sourcePers.cgazHudValues);
  const auto snaphudPayload = buildFullHudPayload(sourcePers.snaphudHudValues);

  for (int i = 0; i < level.numConnectedClients; ++i) {
    const int receiverClientNum = level.sortedClients[i];
    if (!isValidClientNum(receiverClientNum) || receiverClientNum == sourceClientNum) {
      continue;
    }

    auto &receiverPers = level.clients[receiverClientNum].pers;
    if (receiverPers.hudSyncWatchedTarget != sourceClientNum) {
      continue;
    }

    if (receiverPers.hudSyncWatchedRevision == sourcePers.hudSyncRevision) {
      continue;
    }

    sendHudSyncToClient(receiverClientNum, sourceClientNum,
                        receiverPers.hudSyncWatchedRevision < 0,
                        sourcePers.hudSyncRevision, cgazPayload, snaphudPayload);
    receiverPers.hudSyncWatchedRevision = sourcePers.hudSyncRevision;
  }
}
} // namespace

bool upload(gentity_t *ent, Arguments argv) {
  if (!ent || !ent->client || argv->size() < 5) {
    return false;
  }

  auto &pers = ent->client->pers;

  const bool fullUpdate = Q_atoi(argv->at(1).c_str()) > 0;
  const int32_t revision = Q_atoi(argv->at(2).c_str());

  bool changed = false;
  changed |= parseHudPayload(argv->at(3), pers.cgazHudValues, fullUpdate);
  changed |= parseHudPayload(argv->at(4), pers.snaphudHudValues, fullUpdate);

  if (fullUpdate) {
    pers.hasFullHudSync = true;
  }

  if (!pers.hasFullHudSync) {
    return true;
  }

  if (!changed && revision <= pers.hudSyncRevision) {
    return true;
  }

  pers.hudSyncRevision = revision;

  notifyWatchers(ClientNum(ent));

  return true;
}

bool watch(gentity_t *ent, Arguments argv) {
  if (!ent || !ent->client || argv->size() < 3) {
    return false;
  }

  auto &pers = ent->client->pers;

  const int targetClientNum = Q_atoi(argv->at(1).c_str());
  const int32_t knownRevision = Q_atoi(argv->at(2).c_str());

  if (!isConnectedClient(targetClientNum)) {
    clearWatchState(pers);
    return true;
  }

  const auto &targetPers = level.clients[targetClientNum].pers;
  if (!allowsSpectatorHudSharing(targetPers)) {
    clearWatchState(pers);
    return true;
  }

  pers.hudSyncWatchedTarget = targetClientNum;
  pers.hudSyncWatchedRevision = knownRevision;

  if (!targetPers.hasFullHudSync) {
    return true;
  }

  if (knownRevision == targetPers.hudSyncRevision) {
    return true;
  }

  sendHudSyncToClient(ClientNum(ent), targetClientNum, knownRevision < 0,
                      targetPers.hudSyncRevision,
                      buildFullHudPayload(targetPers.cgazHudValues),
                      buildFullHudPayload(targetPers.snaphudHudValues));

  pers.hudSyncWatchedRevision = targetPers.hudSyncRevision;

  return true;
}

void clearWatchForTarget(int clientNum) {
  for (int i = 0; i < level.numConnectedClients; i++) {
    gentity_t *flag = g_entities + level.sortedClients[i];
    if (flag->client->pers.hudSyncWatchedTarget == clientNum) {
      clearWatchState(flag->client->pers);
    }
  }
}

void resetWatchState(gentity_t *ent) {
  if (!ent || !ent->client) {
    return;
  }

  clearWatchState(ent->client->pers);
}
} // namespace ETJump::SpectatorHudSync
