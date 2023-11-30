/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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

#include "etj_cvar_master_drawer.h"
#include "../game/etj_numeric_utilities.h"

ETJump::CvarBasedMasterDrawer::CvarBasedMasterDrawer(const vmCvar_t &cvar)
    : selector(cvar) {
  cvarUpdateHandler->subscribe(&selector, [&](const vmCvar_t *update) {
    updateCurrentIndex(update->integer);
  });
}

void ETJump::CvarBasedMasterDrawer::updateCurrentIndex(int index) {
  currentIndex = Numeric::clamp(index, 1, renderables.size()) - 1;
}

bool ETJump::CvarBasedMasterDrawer::beforeRender() {
  if (shouldRender()) {
    return renderables[currentIndex]->beforeRender();
  }

  return false;
}

void ETJump::CvarBasedMasterDrawer::render() const {
  if (shouldRender()) {
    renderables[currentIndex]->render();
  }
}

bool ETJump::CvarBasedMasterDrawer::shouldRender() const {
  return etj_drawKeys.integer > 0 && renderables.size() > 0;
}

void ETJump::CvarBasedMasterDrawer::push(IRenderable *renderable) {
  renderables.push_back(std::unique_ptr<IRenderable>(renderable));
  updateCurrentIndex(selector.integer);
}
