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

#include <algorithm>

#include "etj_cvar_master_drawer.h"
#include "cg_local.h"
#include "etj_cvar_update_handler.h"

namespace ETJump {
CvarBasedMasterDrawer::CvarBasedMasterDrawer(
    const vmCvar_t *cvar, const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : selector(cvar), cvarUpdate(cvarUpdate) {
  this->cvarUpdate->subscribe(selector, [this](const vmCvar_t *cvar) {
    updateCurrentIndex(cvar->integer);
  });
}

CvarBasedMasterDrawer::~CvarBasedMasterDrawer() {
  cvarUpdate->unsubscribe(selector);
}

void CvarBasedMasterDrawer::updateCurrentIndex(int index) {
  currentIndex = std::clamp(index, 1, static_cast<int>(renderables.size())) - 1;
}

bool CvarBasedMasterDrawer::beforeRender() {
  if (shouldRender()) {
    return renderables[currentIndex]->beforeRender();
  }

  return false;
}

void CvarBasedMasterDrawer::render() const {
  if (shouldRender()) {
    renderables[currentIndex]->render();
  }
}

bool CvarBasedMasterDrawer::shouldRender() const {
  return etj_drawKeys.integer > 0 && renderables.size() > 0;
}

void CvarBasedMasterDrawer::push(std::unique_ptr<IRenderable> renderable) {
  renderables.push_back(std::move(renderable));
  updateCurrentIndex(selector->integer);
}
} // namespace ETJump
