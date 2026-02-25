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

#pragma once

#include <memory>
#include <vector>

#include "etj_irenderable.h"

#include "../game/q_shared.h"

namespace ETJump {
class CvarUpdateHandler;

class CvarBasedMasterDrawer : public IRenderable {
  std::vector<std::unique_ptr<IRenderable>> renderables;
  const vmCvar_t *selector;
  int currentIndex = 0;

  std::shared_ptr<CvarUpdateHandler> cvarUpdate;

  void updateCurrentIndex(int index);
  [[nodiscard]] bool shouldRender() const;

public:
  CvarBasedMasterDrawer(const vmCvar_t *cvar,
                        const std::shared_ptr<CvarUpdateHandler> &cvarUpdate);
  ~CvarBasedMasterDrawer() override;
  bool beforeRender() override;
  void render() const override;
  void push(std::unique_ptr<IRenderable> renderable);
};
} // namespace ETJump
