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

#include "etj_quick_follow_drawable.h"
#include "../game/etj_string_utilities.h"

char *BindingFromName(const char *cvar);

ETJump::QuickFollowDrawer::QuickFollowDrawer() {}

bool ETJump::QuickFollowDrawer::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  return true;
}

void ETJump::QuickFollowDrawer::render() const {
  float *color = CG_FadeColor(cg.crosshairClientTime, 1000);
  if (!color) {
    return;
  }

  auto binding = BindingFromName("+activate");
  auto hintText = stringFormat("^9[%s] to follow", binding);
  float w = DrawStringWidth(cgs.clientinfo[cg.crosshairClientNum].name, 0.23f);
  auto offx = SCREEN_CENTER_X - w / 2;
  DrawString(offx, 182 + 10, 0.16f, 0.16f, color, qfalse, hintText.c_str(), 0,
             ITEM_TEXTSTYLE_SHADOWED);
}

bool ETJump::QuickFollowDrawer::canSkipDraw() const {
  if (etj_quickFollow.integer < 2 || cg.crosshairClientNum >= MAX_CLIENTS) {
    return true;
  }
  if (ETJump::showingScores()) {
    return true;
  }
  if (cgs.clientinfo[cg.crosshairClientNum].hideMe) {
    return true;
  }
  return false;
}
