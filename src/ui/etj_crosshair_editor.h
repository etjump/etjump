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

#include <string>
#include <vector>

#include "../cgame/etj_crosshair_renderer.h"
#include "ui_shared.h"

namespace ETJump {
// backs the in-game crosshair editor.
//
// .menu binds widgets to cvars by name at parse time, so there's no way to point
// a slider at one field inside a packed etj_crosshairElementN string. instead we
// keep a working cvar per field (etj_ce*) for the widgets to bind to, and this
// shuffles values between those and the selected slot both ways, so console
// edits and preset loads show up in the widgets as well.
//
// update() runs off the preview ownerdraw, which only paints while an editor
// page is open, which is also the only time any of this needs to happen.
class CrosshairEditor {
public:
  // ownerdraw entry point, syncs first then draws into rect
  void drawPreview(rectDef_t *rect);

  // copies a slot's element into the working cvars, slot is 1 based
  void loadSlot(int slot);

  // presets are plain text under presetDir(), one serialized element per line.
  // names get sanitized, empty ones are rejected.
  bool savePreset(const std::string &name);
  bool loadPreset(const std::string &name);
  bool deletePreset(const std::string &name);

  // save works off the name field so you can type a new one, load and delete
  // work off whatever is picked in the list
  bool saveCurrentPreset();
  bool loadSelectedPreset();
  bool deleteSelectedPreset();

  // backs FEEDER_CROSSHAIR_PRESETS. the list is cached because the feeder gets
  // asked for counts and names every frame and we don't want to hit the disk
  // that often, so it only rescans when the menu opens or a preset changes.
  void refreshPresets();
  int presetCount() const;
  const char *presetName(int index) const;
  void selectPreset(int index);

  static std::string presetDir();

private:
  // reads etj_crosshairElement1..8 into a definition
  static CrosshairDefinition readDefinition();
  static std::string slotCvarName(int slot);

  // working cvars <-> element
  static CrosshairElement elementFromWorkingCvars();
  static void elementToWorkingCvars(const CrosshairElement &element);

  // keeps the working cvars and the slot cvar in agreement, whichever one
  // changed since last frame wins
  void update();

  // rewrites "6.000000" style values into "6" so the value boxes read cleanly
  static void tidyWorkingCvars();

  const CrosshairPainter &painter();

  // 1 based, 0 means nothing loaded yet so the first update() loads
  int activeSlot = 0;
  // last value seen on each side, tells us which one changed
  std::string lastSlotValue;
  std::string lastWidgetValue;

  std::vector<std::string> presets;
  int selectedPreset = 0;
};
} // namespace ETJump
