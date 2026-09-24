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

#include "etj_keep_alive.h"
#include "ui_local.h"

#include "../game/etj_fatal_error_shared.h"

namespace ETJump {
#ifdef _WIN32
namespace {
vmCvar_t cgameKeepAlive;
bool registered = false;
KeepAliveTracker cgameTracker;
KeepAliveTracker uiTracker;

void releaseCgame() {
  FatalErrorBoundary::releaseKeptModule("cgame" MODULE_NAME_SUFFIX ".dll");
  trap_Cvar_Set(FatalErrorBoundary::CGAME_KEEP_ALIVE_CVAR, "0");
}
} // namespace
#endif

void resetKeepAlive() {
#ifdef _WIN32
  // otherwise, a refresh before initKeepAlive() would use the cvar handle of
  // the previous load, and a tracker could release a reference while the
  // error is still handled, based on a refresh from before the error
  cgameKeepAlive = {};
  registered = false;
  cgameTracker = {};
  uiTracker = {};
#endif
}

void initKeepAlive() {
#ifdef _WIN32
  trap_Cvar_Register(&cgameKeepAlive, FatalErrorBoundary::CGAME_KEEP_ALIVE_CVAR,
                     "0", 0);
  registered = true;

  // While handling an error, the engine only loads ui after it disconnected
  // (ETe, 2.60b and ET: Legacy all set the client state to disconnected in
  // CL_Disconnect before loading ui again there, and Com_Error disconnects
  // before it reloads everything else), so if ui is loaded while connected,
  // the error a module was kept mapped for has been handled already. The
  // references must be dropped right away then, as joining a server that
  // switches fs_game loads ui and then cgame in the same frame, and the cvar
  // cgame sets is gone if the error restored the previous fs_game.
  uiClientState_t state{};
  trap_GetClientState(&state);

  if (state.connState > CA_DISCONNECTED) {
    releaseCgame();
    FatalErrorBoundary::releaseKeepAlive();
  }
#endif
}

void releaseKeptModules(const int realtime) {
#ifdef _WIN32
  // the screen might be refreshed while ui is still initializing
  if (!registered) {
    return;
  }

  trap_Cvar_Update(&cgameKeepAlive);

  if (cgameTracker.update(cgameKeepAlive.integer != 0, realtime)) {
    releaseCgame();
  }

  // the engine loads ui again while it's still handling the error
  if (uiTracker.update(FatalErrorBoundary::isKeptAlive(), realtime)) {
    FatalErrorBoundary::releaseKeepAlive();
  }
#else
  static_cast<void>(realtime);
#endif
}
} // namespace ETJump
