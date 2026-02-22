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

#include "etj_cvar_unlocker.h"
#include "etj_local.h"

using namespace ETJump;

CvarUnlocker::CvarUnlocker(const vmCvar_t *unlocker, std::string target)
    : unlocker(unlocker), target(std::move(target)) {
  forceCvarSet(unlocker);
  cgame.handlers.cvarUpdate->subscribe(
      unlocker, [this](const vmCvar_t *cvar) { this->forceCvarSet(cvar); });
}

CvarUnlocker::~CvarUnlocker() {
  cgame.handlers.cvarUpdate->unsubscribe(unlocker);
}

void CvarUnlocker::forceCvarSet(const vmCvar_t *cvar) const {
  trap_Cvar_Set(target.c_str(), cvar->string);
}

void CvarUnlocker::forceCvarSet() const {
  trap_Cvar_Set(target.c_str(), unlocker->string);
}
