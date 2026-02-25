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

#include <string>

#include "etj_leaves_remapper.h"
#include "etj_cvar_update_handler.h"
#include "etj_utilities.h"

namespace ETJump {
LeavesRemapper::LeavesRemapper(
    const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : cvarUpdate(cvarUpdate) {
  auto shader = composeShader(
      shaderName,
      {{"map *white", "blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA",
        "alphaGen const 0.0"}});
  trap_R_LoadDynamicShader(shaderName, shader.c_str());
  trap_R_RegisterShader(shaderName);

  if (!etj_drawLeaves.integer) {
    turnOffLeaves();
  }

  this->cvarUpdate->subscribe(&etj_drawLeaves, [&](const vmCvar_t *cvar) {
    cvar->integer ? turnOnLeaves() : turnOffLeaves();
  });
}

void LeavesRemapper::turnOnLeaves() {
  for (auto &leavesShader : leavesShaders) {
    trap_R_RemapShader(leavesShader, leavesShader, "0");
  }
}

void LeavesRemapper::turnOffLeaves() {
  for (auto &leavesShader : leavesShaders) {
    trap_R_RemapShader(leavesShader, shaderName, "0");
  }
}

LeavesRemapper::~LeavesRemapper() {
  turnOnLeaves();

  cvarUpdate->unsubscribe(&etj_drawLeaves);
}
} // namespace ETJump
