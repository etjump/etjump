/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
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

#include "etj_demo_compatibility.h"
#include "etj_client_commands_handler.h"
#include "../game/etj_string_utilities.h"

namespace ETJump {
DemoCompatibility::DemoCompatibility() {
  if (!cg.demoPlayback) {
    return;
  }

  parseDemoVersion();
  setupCompatibilityFlags();

  consoleCommandsHandler->subscribe(
      "printDemoCompatInfo",
      [this](const std::vector<std::string> &) { printCompatibilityInfo(); });
}

void DemoCompatibility::parseDemoVersion() {
  const char *serverInfoCS = CG_ConfigString(CS_SERVERINFO);
  std::string versionStr = Info_ValueForKey(serverInfoCS, "mod_version");

  // mod_version is present from '2.0.3 Alpha' onwards
  // if not present, look for sv_referencedPakNames as a fallback
  if (!versionStr.empty()) {
    fillVersionInfo(demoVersion, versionStr, ".");
  } else {
    const char *sysInfoCS = CG_ConfigString(CS_SYSTEMINFO);
    const char *pakNames = Info_ValueForKey(sysInfoCS, "sv_referencedPakNames");

    // sanity check, shouldn't happen
    if (pakNames == nullptr) {
      return;
    }

    const char *pak = std::strchr(pakNames, '/'); // etjump/...

    // should not happen
    if (pak == nullptr) {
      return;
    }

    versionStr = pak;
    size_t idx = std::string::npos;

    for (size_t i = 0; i < versionStr.length(); i++) {
      if (Q_isnumeric(versionStr[i])) {
        idx = i;
        break;
      }
    }

    // parsing failed, bail
    if (idx == std::string::npos) {
      return;
    }

    versionStr.erase(0, idx);

    // pre 2.4.0 used '_' in pk3 name as delimiter
    fillVersionInfo(demoVersion, versionStr, "_");
  }
}

void DemoCompatibility::fillVersionInfo(Version &version,
                                        const std::string &versionStr,
                                        const std::string &delimiter) {
  const auto splits = StringUtil::split(versionStr, delimiter);

  // bail if we have some weird version without at least 3 digits
  if (splits.size() < 3) {
    CG_Printf("^3WARNING: could not parse version compatibility info!\n");
    return;
  }

  version.major = std::stoi(splits[0].substr(0, 1));
  version.minor = std::stoi(splits[1].substr(0, 1));
  version.patch = std::stoi(splits[2].substr(0, 1));

  // special entity type adjustment for ETJump 2.3.0/2.3.0 RC4
  // this is kinda awkward to do in setupCompatibilityFlags (and we don't
  // parse extra versions anyway), so for simplicity's sake parse this here
  if (isExactVersion({2, 3, 0}) ||
      !Q_stricmpn(versionStr.c_str(), "etjump-2_3_0-RC4", 17)) {
    flags.adjustEntityTypes = true;
    compatibilityStrings.emplace_back(
        "Adjusted entity type order for ET_VELOCITY_PUSH_TRIGGER");
  }
}

void DemoCompatibility::setupCompatibilityFlags() {
  if (!isCompatible({2, 0, 6})) {
    flags.adjustEvTokens = true;
    compatibilityStrings.emplace_back(
        "Adjusted event indices for ET_TOKEN_ entities");

    flags.adjustItemlistIndex = true;
    compatibilityStrings.emplace_back(
        "Adjusted item indices for removal of duplicate 'weapon_medic_heal'");
  }

  if (!isCompatible({2, 3, 0})) {
    flags.adjustEvGeneralClientSoundVolume = true;
    compatibilityStrings.emplace_back(
        "Adjusted event indices for EV_GENERAL_CLIENT_SOUND_VOLUME");

    flags.adjustEvVelocityPushTrigger = true;
    compatibilityStrings.emplace_back(
        "Adjusted event indices for ET_VELOCITY_PUSH_TRIGGER");
  }

  if (!isCompatible({2, 5, 0})) {
    flags.predictedJumpSpeeds = true;
    compatibilityStrings.emplace_back(
        "Using predicted speeds for jump speeds display");
  }

  if (!isCompatible({3, 2, 0})) {
    flags.serverSideCoronas = true;
    compatibilityStrings.emplace_back("Using fully server-side coronas");

    flags.svFpsUnavailable = true;
    compatibilityStrings.emplace_back(
        "Unable to determine sv_fps, assuming default");
  } else if (!isCompatible({3, 3, 0})) {
    flags.svFpsInSysteminfo = true;
    compatibilityStrings.emplace_back(
        "Using systeminfo string to determine sv_fps");
  }

  if (!isCompatible({3, 3, 0})) {
    flags.noSavePosTimerunInfo = true;
    compatibilityStrings.emplace_back(
        "Timerun state unavailable for 'savepos'");

    flags.adjustEvFakebrushAndClientTeleporter = true;
    compatibilityStrings.emplace_back("Adjusted event indices for ET_FAKEBRUSH "
                                      "and ET_TELEPORT_TRIGGER_CLIENT");
  }

  if (!isCompatible({3, 4, 0})) {
    flags.serverSideDlights = true;
    compatibilityStrings.emplace_back("Using fully server-side dlights");

    flags.setAttack2FiringFlag = true;
    compatibilityStrings.emplace_back(
        "Faking EF_FIRING state for '+attack2' shooting");
  }

  if (!isCompatible({3, 5, 0})) {
    flags.noSpecCountInVoteCs = true;
    compatibilityStrings.emplace_back(
        "Vote tally not differentiating players from spectators");

    flags.stripLocalizationMarkers = true;
    compatibilityStrings.emplace_back(
        "Stripping localization markers from server commands manually");
  }
}

void DemoCompatibility::printCompatibilityInfo() {
  if (compatibilityStrings.empty()) {
    CG_Printf("No compatibility settings active.\n");
    return;
  }

  CG_Printf("Active compatibility settings:\n");

  for (const auto &str : compatibilityStrings) {
    CG_Printf("- %s\n", str.c_str());
  }
}

int DemoCompatibility::adjustedEventNum(const int event) const {
  int adjust = 0;

  if (flags.adjustEvTokens) {
    adjust += 3;
  }

  if (flags.adjustEvVelocityPushTrigger) {
    adjust += 1;
  }

  if (flags.adjustEvFakebrushAndClientTeleporter) {
    adjust += 2;
  }

  switch (event + adjust) {
    case EV_RAILTRAIL:
    case EV_GLOBAL_CLIENT_SOUND:
    case EV_OBITUARY:
    case EV_GLOBAL_ITEM_PICKUP:
    case EV_GLOBAL_SOUND:
    case EV_SMOKE:
    case EV_MG42BULLET_HIT_WALL:
    case EV_MG42BULLET_HIT_FLESH:
    case EV_MISSILE_HIT:
    case EV_MISSILE_MISS:
    case EV_SHAKE:
    case EV_MORTAR_MISS:
    case EV_MORTAR_IMPACT:
    case EV_BULLET_HIT_FLESH:
    case EV_BULLET_HIT_WALL:
    case EV_MORTAREFX:
    case EV_ITEM_POP:
    case EV_EFFECT:
    case EV_RUBBLE:
    case EV_BUILDDECAYED_SOUND:
    case EV_GUNSPARKS:
    case EV_DUST:
    case EV_OILSLICK:
    case EV_OILSLICKREMOVE:
    case EV_SNOWFLURRY:
    case EV_DEBRIS:
    case EV_ALERT_SPEAKER:
    case EV_GLOBAL_TEAM_SOUND:
    case EV_RUMBLE_EFX:
    case EV_GENERAL_SOUND_VOLUME:
    case EV_GENERAL_CLIENT_SOUND_VOLUME:
    case EV_GENERAL_SOUND:
    case EV_AIRSTRIKEMESSAGE:
    case EV_ARTYMESSAGE:
    case EV_EMITTER:
    case EV_PORTAL_TRAIL:
    case EV_POPUPMESSAGE:
      return event + adjust;
    default:
      return event;
  }
}

void DemoCompatibility::printDemoInformation() const {
  const char *cs = CG_ConfigString(CS_SERVERINFO);
  const char *server = Info_ValueForKey(cs, "sv_hostname");

  if (!server || !*server) {
    server = "Unknown";
  }

  const char *player = cgs.clientinfo[cg.clientNum].name;

  CG_Printf("\n^g========================================\n");
  CG_Printf(" Demo information:\n Mod version: ETJump ^2%i.%i.%i\n ^7Player: "
            "%s\n ^7Map: %s\n ^7Server: %s\n",
            demoVersion.major, demoVersion.minor, demoVersion.patch, player,
            cgs.rawmapname, server);

  CG_Printf("^g========================================\n\n");
}

bool DemoCompatibility::isCompatible(const Version &minimum) const {
  if (demoVersion.major != minimum.major) {
    return demoVersion.major > minimum.major;
  }

  if (demoVersion.minor != minimum.minor) {
    return demoVersion.minor > minimum.minor;
  }

  return demoVersion.patch >= minimum.patch;
}

bool DemoCompatibility::isExactVersion(const Version &version) const {
  return demoVersion.major == version.major &&
         demoVersion.minor == version.minor &&
         demoVersion.patch == version.patch;
}

} // namespace ETJump
