/*
 * MIT License
 *
 * Copyright (c) 2022 ETJump team <zero@etjump.com>
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

#include "etj_cvar_shadow.h"
#include "etj_cvar_update_handler.h"

using namespace ETJump;

CvarShadow::CvarShadow(const vmCvar_t *shadow, const std::string &target)
    : _shadow(shadow), _target(target) {
  forceCvarSet(shadow);
  cvarUpdateHandler->subscribe(
      _shadow, [this](const vmCvar_t *cvar) { this->forceCvarSet(cvar); });
}

CvarShadow::~CvarShadow() { cvarUpdateHandler->unsubscribe(_shadow); }

void CvarShadow::forceCvarSet(const vmCvar_t *cvar) const {
  trap_Cvar_Set(_target.c_str(), cvar->string);
}

void CvarShadow::forceCvarSet() const {
  trap_Cvar_Set(_target.c_str(), _shadow->string);
}
