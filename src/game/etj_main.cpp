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

#include "etj_local.h"

#include <functional>
#include <memory>
#include "etj_banner_system.h"
#include "etj_database_v2.h"
#include "etj_printer.h"

namespace {
// each of these will be called on every frame
std::vector<std::function<void(int levelTime)>> runFrameCallbacks;

// suppress clang-tidy - this might be unused if g_banners is set to 0
[[maybe_unused]] std::unique_ptr<ETJump::BannerSystem> bannerSystem = nullptr;

void InitBannerSystem() {
  ETJump::BannerSystem::Options options;
  options.interval = g_bannerTime.integer;
  options.location =
      static_cast<ETJump::BannerSystem::Location>(g_bannerLocation.integer);
  if (strlen(g_banner1.string) > 0)
    options.messages.push_back(g_banner1.string);
  if (strlen(g_banner2.string) > 0)
    options.messages.push_back(g_banner2.string);
  if (strlen(g_banner3.string) > 0)
    options.messages.push_back(g_banner3.string);
  if (strlen(g_banner4.string) > 0)
    options.messages.push_back(g_banner4.string);
  if (strlen(g_banner5.string) > 0)
    options.messages.push_back(g_banner5.string);

  bannerSystem = std::make_unique<ETJump::BannerSystem>(std::move(options));
}

void ShutdownBannerSystem() {
  Printer::LogPrintln("Banner system shut down");
  if (bannerSystem) {
    bannerSystem = nullptr;
  }
}
} // namespace

namespace ETJump {
int subcribeToRunFrame(std::function<void(int)> callback) {
  runFrameCallbacks.push_back(callback);
  return runFrameCallbacks.size() - 1;
}

void unsubcribeToRunFrame(int id) {
  runFrameCallbacks.erase(begin(runFrameCallbacks) + id);
}
} // namespace ETJump

// Initializes the ETJump subsystems
void ETJump_InitGame(int levelTime, int randomSeed, int restart) {
  Printer::LogPrint("----------------------------------------------------------"
                    "----------------------\n"
                    "Initializing ETJump subsystems\n"
                    "----------------------------------------------------------"
                    "----------------------\n");
  runFrameCallbacks.clear();

  // each subsystem will subscribe to necessary events
  if (g_banners.integer) {
    InitBannerSystem();
  }

  Printer::LogPrint("----------------------------------------------------------"
                    "----------------------\n"
                    "ETJump subsystems initialized\n"
                    "----------------------------------------------------------"
                    "----------------------\n");
}

// Shuts down the ETJump subsystems
void ETJump_ShutdownGame(int restart) {
  Printer::LogPrint("----------------------------------------------------------"
                    "----------------------\n"
                    "Shutting down the ETJump subsystems\n"
                    "----------------------------------------------------------"
                    "----------------------\n");

  ShutdownBannerSystem();

  Printer::LogPrint("----------------------------------------------------------"
                    "----------------------\n"
                    "ETJump subsystems shut down\n"
                    "----------------------------------------------------------"
                    "----------------------\n");
}

void ETJump_RunFrame(int levelTime) {
  for (auto &callback : runFrameCallbacks) {
    callback(levelTime);
  }
}
