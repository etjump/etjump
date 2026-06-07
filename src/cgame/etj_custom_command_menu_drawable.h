/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
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
class ClientCommandsHandler;
class CvarUpdateHandler;

class CustomCommandMenuDrawable : public IRenderable {
public:
  CustomCommandMenuDrawable(
      const std::shared_ptr<ClientCommandsHandler> &consoleCommands,
      const std::shared_ptr<CvarUpdateHandler> &cvarUpdate);
  ~CustomCommandMenuDrawable() override;

  bool beforeRender() override;
  void render() const override;

  static void commandMenuTitleDraw(panel_button_t *button);
  static void commandMenuTextDraw(panel_button_t *button);

  static void keyHandling(int32_t key, bool down);
  // this is qboolean so that we don't need to cast the result to engine
  static qboolean checkExecKey(int32_t key, qboolean doAction);

private:
  static uint8_t currentPage;
  static size_t maxChars;

  std::shared_ptr<ClientCommandsHandler> consoleCommands;
  std::shared_ptr<CvarUpdateHandler> cvarUpdate;

  void setupListeners();
  static void openMenu(uint8_t page);
  static void setupPanels();
  static void resizePanels(float width);
  static void computeMaxChars();
  static bool canSkipDraw();
};
} // namespace ETJump
