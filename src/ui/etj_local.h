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

#include "etj_colorpicker.h"
#include "etj_demo_queue.h"
#include "etj_quick_connect.h"

#include "../cgame/etj_color_parser.h"

#include "../game/etj_syscall_ext_shared.h"

namespace ETJump {
// global UI objects
struct UIContext {
  std::unique_ptr<SyscallExt> syscallExt;
  std::unique_ptr<ColorPicker> colorPicker;
  std::unique_ptr<DemoQueue> demoQueue;
  std::unique_ptr<QuickConnect> quickConnect;
  std::unique_ptr<ColorParser> colorParser;
};

inline UIContext ui;

void init(int32_t legacyClient, int32_t clientVersion);
void shutdown();
// outside of 'init()', as this requires menus to be loaded already
void initQuickConnect();
} // namespace ETJump
