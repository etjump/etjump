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

#include "etj_banner_system.h"
#include "etj_common.h"
#include "etj_printer.h"
#include "etj_string_utilities.h"

static const char *LocationText[] = {"Center", "Top", "Chat", "Left"};

ETJump::BannerSystem::BannerSystem(Options options) : _bannerIdx(0) {
  _options = options;
  subcribeToRunFrame([=](int levelTime) { check(levelTime); });
  Printer::LogPrintln(stringFormat("Initialized banner system\n"
                                   "- %d banners\n"
                                   "- %ds interval\n"
                                   "- %s location",
                                   _options.messages.size(),
                                   (_options.interval / 1000),
                                   LocationText[_options.location]));
}

void ETJump::BannerSystem::check(int levelTime) {
  if (_nextBannerTime > levelTime) {
    return;
  }

  if (_options.messages.size() == 0) {
    return;
  }

  auto message = _options.messages[_bannerIdx];

  switch (_options.location) {
    case Center:
      Printer::BroadcastCenterMessage(message);
      break;
    case Top:
      Printer::BroadcastTopBannerMessage(message);
      break;
    case Chat:
      Printer::BroadcastChatMessage(message);
      break;
    case Left:
      Printer::BroadcastLeftBannerMessage(message);
      break;
    default:
      Printer::BroadcastTopBannerMessage(message);
      break;
  }

  _bannerIdx = (_bannerIdx + 1) % _options.messages.size();
  _nextBannerTime = levelTime + _options.interval;
}

ETJump::BannerSystem::~BannerSystem() {}
