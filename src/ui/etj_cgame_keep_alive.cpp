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

#include "etj_cgame_keep_alive.h"
#include "ui_local.h"

#include "../game/etj_fatal_error_shared.h"

namespace ETJump {
#ifdef _WIN32
namespace {
vmCvar_t cgameKeepAlive;
bool registered = false;
CgameKeepAliveTracker tracker;
} // namespace
#endif

void initCgameKeepAlive() {
#ifdef _WIN32
  trap_Cvar_Register(&cgameKeepAlive, FatalErrorBoundary::CGAME_KEEP_ALIVE_CVAR,
                     "0", 0);
  registered = true;
#endif
}

void releaseCgameAfterError(const int realtime) {
#ifdef _WIN32
  // the screen might be refreshed while ui is still initializing
  if (!registered) {
    return;
  }

  trap_Cvar_Update(&cgameKeepAlive);

  if (!tracker.update(cgameKeepAlive.integer != 0, realtime)) {
    return;
  }

  FatalErrorBoundary::releaseKeptModule("cgame" MODULE_NAME_SUFFIX ".dll");
  trap_Cvar_Set(FatalErrorBoundary::CGAME_KEEP_ALIVE_CVAR, "0");
#else
  static_cast<void>(realtime);
#endif
}
} // namespace ETJump
