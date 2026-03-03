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

#include "etj_irenderable.h"

#include "../ui/ui_shared.h"

namespace ETJump {
class RtvDrawable : public IRenderable {
  void render() const override;
  bool beforeRender() override;
  static bool canSkipDraw();

  static void setupPanelButtons();

  static void drawMenuTitleText(panel_button_t *button);
  static void drawMenuText(panel_button_t *button);

public:
  RtvDrawable();
  ~RtvDrawable() override = default;

  static void keyHandling(int key, bool down);
  // this is qboolean so that we don't need to cast the result to engine
  static qboolean checkExecKey(int key, qboolean doAction);
};
} // namespace ETJump
