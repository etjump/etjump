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

#include "etj_overbounce_watcher.h"
#include "etj_client_commands_handler.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_overbounce_shared.h"
#include "etj_utilities.h"

#include "../game/etj_string_utilities.h"

namespace ETJump {
OverbounceWatcher::OverbounceWatcher(
    const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
    const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : shader(cgs.media.voiceChatShader), consoleCommands(consoleCommands),
      cvarUpdate(cvarUpdate) {

  setSize(&etj_obWatcherSize);
  cgame.utils.colorParser->parseColorString(etj_obWatcherColor.string, color);
  startListeners();
}

OverbounceWatcher::~OverbounceWatcher() {
  consoleCommands->unsubscribe("ob_save");
  consoleCommands->unsubscribe("ob_load");
  consoleCommands->unsubscribe("ob_reset");
  consoleCommands->unsubscribe("ob_list");

  cvarUpdate->unsubscribe(&etj_obWatcherSize);
  cvarUpdate->unsubscribe(&etj_obWatcherColor);
}

void OverbounceWatcher::startListeners() {
  consoleCommands->subscribe(
      "ob_save", [this](const std::vector<std::string> &args) { save(args); });

  consoleCommands->subscribe(
      "ob_load", [this](const std::vector<std::string> &args) { load(args); });

  consoleCommands->subscribe(
      "ob_reset", [this](const std::vector<std::string> &) { reset(); });

  consoleCommands->subscribe(
      "ob_list", [this](const std::vector<std::string> &) { list(); });

  cvarUpdate->subscribe(&etj_obWatcherColor, [&](const vmCvar_t *cvar) {
    cgame.utils.colorParser->parseColorString(cvar->string, color);
  });

  cvarUpdate->subscribe(&etj_obWatcherSize,
                        [&](const vmCvar_t *cvar) { setSize(cvar); });
}

void OverbounceWatcher::setSize(const vmCvar_t *cvar) {
  size = CvarValueParser::parse<CvarValue::Size>(*cvar, 0, 10);
  size.x *= 0.1f;
  size.y *= 0.1f;
}

bool OverbounceWatcher::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  ps = getValidPlayerState();
  pmoveSec = static_cast<float>(cgs.pmove_msec) / 1000.f;
  gravity = ps->gravity;
  zVel = ps->velocity[2];
  startHeight = ps->origin[2] + ps->mins[2];

  // impossible OB - negative z velocity & below saved coordinate
  if (zVel < 0 && startHeight < endHeight) {
    return false;
  }

  overbounce = false;

  VectorSet(snap, 0, 0, gravity * pmoveSec);
  trap_SnapVector(snap);
  zVelSnapped = snap[2];

  endHeight = (*current)[2];

  x = etj_obWatcherX.value;
  ETJump_AdjustPosition(&x);

  // setup & do trace, so we can determine if surface allows OB
  trace_t trace;
  VectorCopy(*current, start);
  start[2] = startHeight;
  VectorCopy(start, end);
  end[2] -= Overbounce::MAX_TRACE_DIST;

  CG_Trace(&trace, start, vec3_origin, vec3_origin, end, ps->clientNum,
           CONTENTS_SOLID);

  // CG_Printf("startHeight: %f endHeight: %f\n", startHeight, endHeight);
  if (Overbounce::isOverbounce(zVel, startHeight, endHeight, zVelSnapped,
                               pmoveSec, gravity) &&
      Overbounce::surfaceAllowsOverbounce(&trace)) {
    overbounce = true;
  }

  return overbounce;
}

void OverbounceWatcher::render() const {
  DrawString(x, etj_obWatcherY.value, size.x, size.y, color, qfalse, "OB", 0,
             ITEM_TEXTSTYLE_SHADOWED);
}

void OverbounceWatcher::save(const std::vector<std::string> &args) {
  ps = getValidPlayerState();
  const std::string name = !args.empty() ? sanitize(args[0], true) : "default";

  VectorCopy(ps->origin, positions[name]);
  // shift z-coordinate to feet level
  positions[name][2] += ps->mins[2];

  current = &positions[name];
  CG_AddPMItem(PM_MESSAGE,
               va("^3OB watcher: ^7saved coordinate as ^3%s ^7%s\n",
                  name.c_str(), vtosf(positions[name])),
               shader);
}

void OverbounceWatcher::reset() {
  current = nullptr;
  CG_AddPMItem(PM_MESSAGE,
               "^3OB watcher: ^7current coordinates have been reset.\n",
               shader);
}

void OverbounceWatcher::load(const std::vector<std::string> &args) {
  const std::string name = !args.empty() ? sanitize(args[0], true) : "default";

  if (positions.find(name) == positions.cend()) {
    CG_AddPMItem(
        PM_MESSAGE,
        va("^3OB watcher: ^7coordinate ^3%s ^7was not found.\n", name.c_str()),
        shader);
    return;
  }

  current = &positions[name];

  CG_AddPMItem(PM_MESSAGE,
               va("^3OB watcher: ^7loaded coordinate ^3%s ^7%s\n", name.c_str(),
                  vtosf(*current)),
               shader);
}

void OverbounceWatcher::list() const {
  if (positions.empty()) {
    CG_Printf("^3OB watcher: ^7no saved positions.\n");
    return;
  }

  CG_Printf("Saved OB watcher coordinates:\n\n");

  for (const auto &[name, pos] : positions) {
    CG_Printf("^2%-15s ^7%s%s\n", name.c_str(), vtosf(pos),
              current && VectorCompare(*current, pos) ? " ^9(current)" : "");
  }
}

bool OverbounceWatcher::canSkipDraw() const {
  if (!etj_drawObWatcher.integer || !current) {
    return true;
  }

  if (ps->groundEntityNum != ENTITYNUM_NONE) {
    return true;
  }

  if (ps->pm_type == PM_NOCLIP) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
