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

#include <regex>
#include <unordered_map>
#include <string>
#include <sstream>
#include <memory>
#include <array>

#include "etj_utilities.h"
#include "etj_event_loop.h"
#include "cg_local.h"
#include "etj_demo_compatibility.h"
#include "etj_player_events_handler.h"
#include "etj_cvar_shadow.h"
#include "etj_pmove_utils.h"
#include "../game/etj_string_utilities.h"
#include "../game/etj_portalgun_shared.h"

namespace ETJump {
std::string composeShader(const char *name, ShaderStage general,
                          ShaderStages stages) {
  std::string shader;
  // shader name
  shader = shader + name + " {\n";
  // General Directives
  for (auto &dir : general) {
    shader = shader + dir + "\n";
  }
  // Stage Directives
  for (auto &stage : stages) {
    shader += "{\n";
    for (auto &dir : stage) {
      shader = shader + dir + "\n";
    }
    shader += "}\n";
  }
  shader += "}\n";

  return shader;
}

std::string composeShader(const char *name, ShaderStages stages) {
  return composeShader(name, {""}, stages);
}

static std::unordered_map<std::string, const vec4_t *> validColorNames = {
    {"white", &colorWhite},       {"red", &colorRed},
    {"green", &colorGreen},       {"blue", &colorBlue},
    {"yellow", &colorYellow},     {"magenta", &colorMagenta},
    {"cyan", &colorCyan},         {"orange", &colorOrange},
    {"mdred", &colorMdRed},       {"mdgreen", &colorMdGreen},
    {"dkgreen", &colorDkGreen},   {"mdcyan", &colorMdCyan},
    {"mdyellow", &colorMdYellow}, {"mdorange", &colorMdOrange},
    {"mdblue", &colorMdBlue},     {"gray", &colorMdGrey},
    {"grey", &colorMdGrey},       {"ltgrey", &colorLtGrey},
    {"mdgrey", &colorMdGrey},     {"dkgrey", &colorDkGrey},
    {"black", &colorBlack},
};

static void parseNamedColorString(const std::string &token, vec4_t &color) {
  auto validColor = validColorNames[token];
  if (validColor) {
    color[0] = (*validColor)[0];
    color[1] = (*validColor)[1];
    color[2] = (*validColor)[2];
  }
}

static void parseRGBAValuedColorString(const std::string &colorString,
                                       vec4_t &color) {
  std::istringstream tokenStream{colorString};
  std::string token;
  auto colorChannel = 0;
  while (tokenStream >> token) {
    if (colorChannel > 3) {
      break;
    }
    auto value = std::min(std::max(std::stof(token), 0.f), 255.f);
    color[colorChannel] = value;
    colorChannel++;
  }
}

static void parseHexValuedColorString(const std::string &token, vec4_t &color) {
  auto colorValue = std::stoll(token, nullptr, 16);
  auto channelCount = ((token.size() - 1) >> 1) + 1;
  auto maxShift = 8 * channelCount;
  for (unsigned long i = 0; i < channelCount; i++) {
    color[i] = (colorValue >> (maxShift - 8 * (i + 1))) & 0xff;
  }
}

static void normalizeColorIfRequired(vec4_t &v) {
  float max = 0.f;
  for (auto i = 0; i < 3; i++) {
    max = std::max(v[i], max);
  }

  // non-normalized color
  if (max > 1.0f) {
    for (auto i = 0; i < 3; i++) {
      v[i] /= 255.f;
    }
  }

  // handle alpha separately
  if (v[3] > 1.0) {
    v[3] /= 255.f;
  }
}

const std::string alphaRegexStr{"^[a-z]+"}; // white, black, etc
const std::string digitRegexStr{
    "^([-+]?[0-9]*\\.?[0-9]+\\s*)+"};              // 255 0 0, 1.0 0 0
const std::string hexedRegexStr{"^0[x][a-f0-9]+"}; // 0xff0000
const std::string hashdRegexStr{"^#[a-f0-9]+"};    // #ff0000

const std::basic_regex<char> alphaRegex = std::regex(alphaRegexStr);
const std::basic_regex<char> digitRegex = std::regex(digitRegexStr);
const std::basic_regex<char> hexedRegex = std::regex(hexedRegexStr);
const std::basic_regex<char> hashdRegex = std::regex(hashdRegexStr);

void parseColorString(const std::string &colorString, vec4_t &color) {
  Vector4Set(color, 0.0f, 0.0f, 0.0f, 1.0); // set defaults

  std::string token{StringUtil::toLowerCase(trim(colorString))};

  if (std::regex_match(token, alphaRegex)) {
    parseNamedColorString(token, color);
    return;
  } else if (std::regex_match(token, digitRegex)) {
    parseRGBAValuedColorString(token, color);
  } else if (std::regex_match(token, hexedRegex)) {
    parseHexValuedColorString(token.substr(2, 8), color);
  } else if (std::regex_match(token, hashdRegex)) {
    parseHexValuedColorString(token.substr(1, 8), color);
  }

  normalizeColorIfRequired(color);
}

#ifdef CGAMEDLL

int setTimeout(std::function<void()> fun, int delay) {
  return eventLoop->schedule(fun, delay);
}

bool clearTimeout(int handle) { return eventLoop->unschedule(handle); }

int setInterval(std::function<void()> fun, int delay) {
  return eventLoop->schedulePersistent(fun, delay);
}

bool clearInterval(int handle) { return eventLoop->unschedule(handle); }

int setImmediate(std::function<void()> fun) {
  return eventLoop->schedule(fun, 0, TaskPriorities::Immediate);
}

bool clearImmediate(int handle) { return eventLoop->unschedule(handle); }

void executeTimeout(int handle) { eventLoop->execute(handle); }

bool configFileExists(const std::string &filename) {
  bool fileExists = true;
  int len;
  fileHandle_t f;
  std::string str = filename + ".cfg";

  len = trap_FS_FOpenFile(str.c_str(), &f, FS_READ);
  if (!f || len < 0) {
    // file not found
    fileExists = false;
  }

  trap_FS_FCloseFile(f);
  return fileExists;
}

void execFile(const std::string &filename, ExecFileType type) {
  const bool quiet = static_cast<int>(type) & etj_useExecQuiet.integer;
  const std::string cmd = quiet ? stringFormat("execq \"%s.cfg\"\n", filename)
                                : stringFormat("exec \"%s.cfg\"\n", filename);
  trap_SendConsoleCommand(cmd.c_str());
}

bool isValidClientNum(const int clientNum) {
  return clientNum >= 0 && clientNum < MAX_CLIENTS;
}

bool isPlaying(const int clientNum) {
  return (cgs.clientinfo[clientNum].team == TEAM_ALLIES ||
          cgs.clientinfo[clientNum].team == TEAM_AXIS);
}

int getSvFps() {
  if (!cg.demoPlayback) {
    return cgs.sv_fps;
  }

  int fps;

  if (demoCompatibility->flags.svFpsUnavailable) {
    // no way to know for sure, assume default
    fps = 1000 / DEFAULT_SV_FRAMETIME;
  } else if (demoCompatibility->flags.svFpsInSysteminfo) {
    const char *cs = CG_ConfigString(CS_SYSTEMINFO);
    fps = Q_atoi(Info_ValueForKey(cs, "sv_fps"));
  } else {
    fps = cgs.sv_fps;
  }

  return fps;
}

bool playerIsSolid(const int self, const int other) {
  if (cg_ghostPlayers.integer == 1) {
    if (self == other) {
      return false;
    }

    if (!isPlaying(self) || !isPlaying(other)) {
      return false;
    }

    if (!CG_IsOnSameFireteam(self, other) ||
        (CG_IsOnSameFireteam(self, other) &&
         !cgs.clientinfo[self].fireteamData->noGhost)) {
      return false;
    }
  }

  return true;
}

bool playerIsNoclipping(const int clientNum) {
  return cg_entities[clientNum].currentState.density &
         static_cast<int>(PlayerDensityFlags::Noclip);
}

void tempTraceIgnoreClient(int clientNum) {
  if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
    return;
  }

  tempTraceIgnoredClients[clientNum] = true;
}

void resetTempTraceIgnoredClients() {
  std::fill_n(tempTraceIgnoredClients.begin(), MAX_CLIENTS, false);
}

bool skipPortalDraw(const int selfNum, const int otherNum) {
  if (selfNum == otherNum) {
    return false;
  }

  if (etj_portalTeam.integer == PORTAL_TEAM_ALL ||
      (etj_viewPlayerPortals.integer && !cgs.clientinfo[otherNum].hideMe)) {
    return false;
  }

  if (etj_portalTeam.integer == PORTAL_TEAM_FT &&
      CG_IsOnSameFireteam(selfNum, otherNum)) {
    return false;
  }

  return true;
}

void registerGameShader(const int32_t index, const char *shader) {
  cgs.gameShaders[index] = shader[0] == '*'
                               ? trap_R_RegisterShader(shader + 1)
                               : trap_R_RegisterShaderNoMip(shader);
  Q_strncpyz(cgs.gameShaderNames[index], shader[0] == '*' ? shader + 1 : shader,
             MAX_QPATH);
}

void centerCursor() {
  cgDC.cursor.virtX = SCREEN_CENTER_X;
  cgDC.cursor.virtY = SCREEN_CENTER_Y;
  cgDC.cursor.realX = cgDC.glconfig.vidWidth / 2;
  cgDC.cursor.realY = cgDC.glconfig.vidHeight / 2;

  cgs.cursorX = cgDC.cursor.virtX;
  cgs.cursorY = cgDC.cursor.virtY;
  cgs.realCursorX = cgDC.cursor.realX;
  cgs.realCursorY = cgDC.cursor.realY;
}

void addLoopingSound(const vec3_t origin, const vec3_t velocity,
                     const sfxHandle_t sfx, const int32_t volume,
                     const int32_t soundTime) {
  if (etj_loopedSounds.integer > 0) {
    trap_S_AddLoopingSound(origin, velocity, sfx, volume, soundTime);
  }
}

void addRealLoopingSound(const vec3_t origin, const vec3_t velocity,
                         const sfxHandle_t sfx, const int32_t range,
                         const int32_t volume, const int32_t soundTime) {
  if (etj_loopedSounds.integer > 0) {
    trap_S_AddRealLoopingSound(origin, velocity, sfx, range, volume, soundTime);
  }
}

bool hideMeCheck(const int32_t entityNum) {
  if (entityNum >= MAX_CLIENTS || entityNum == cg.clientNum) {
    return false;
  }

  return cgs.clientinfo[entityNum].hideMe;
}

int32_t getExtraTraceContents(const int32_t value) {
  if (etj_extraTrace.integer & 1 << value) {
    return CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
  }

  return CONTENTS_SOLID;
}

int32_t weapnumForClient() {
  if (cg.snap->ps.pm_flags & PMF_FOLLOW || cg.demoPlayback) {
    return cg.snap->ps.weapon;
  }

  return cg.weaponSelect;
}

bool showingScores() {
  return (cg.showScores || cg.scoreFadeTime + FADE_TIME > cg.time);
}

void onPlayerRespawn(bool revived) {
  playerEventsHandler->check("respawn", {revived ? "1" : "0"});
}

playerState_t *getValidPlayerState() {
  return (cg.snap->ps.clientNum != cg.clientNum || cg.demoPlayback)
             // spectating/demo playback
             ? &cg.snap->ps
             // playing
             : &cg.predictedPlayerState;
}

void resetTransitionEffects() {
  cg.damageTime = cg.duckTime = cg.landTime = cg.stepTime = 0;
}

void resetCustomvoteInfo() {
  cg.numCustomvotesRequested = false;
  cg.customvoteInfoRequested = false;
  cg.numCustomvotes = -1;
  cg.numCustomvoteInfosRequested = 0;

  trap_SendConsoleCommand("uiResetCustomvotes\n");
}
#endif
} // namespace ETJump
