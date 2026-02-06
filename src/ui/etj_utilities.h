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

#include "ui_shared.h"

namespace ETJump::Utilities {
// parses maplist for callvote menu
void parseMaplist();

// parses number of custom vote lists
void parseNumCustomvotes();

// parses information for custom vote lists
void parseCustomvote();

// invalidates custom vote data
// called on module restarts, or when server updates custom votes
void resetCustomvotes();

// wacky hack to toggle ETJump settings menu with a keybind
void toggleSettingsMenu();

// handles illegal server redirect attacks to work around
// exploits in 2.60b, when 'Play Online' menu is opened
void handleIllegalRedirect();

// draws levelshot in 'vote -> map -> details' menu
// TODO: this should be somewhere else, after 'ui_shared.cpp' is cleaned up
void drawLevelshotPreview(const rectDef_t &rect);

// draws map name in 'vote -> map -> details' menu
// TODO: this should be somewhere else, after 'ui_shared.cpp' is cleaned up
void drawMapname(rectDef_t &rect, float scale, vec4_t color, float text_x,
                 int textStyle, int align);

// formats the current changelog to a given window width, preserving indentation
// NOTE: this does not handle color codes, there's no reason we ever
// should have color codes in the changelog
std::vector<std::string>
fitChangelogLinesToWidth(std::vector<std::string> &lines, int maxW, float scale,
                         fontInfo_t *font);
} // namespace ETJump::Utilities
