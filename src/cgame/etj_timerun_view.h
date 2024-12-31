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

#include <string>
#include "etj_drawable.h"
#include "etj_timerun.h"
#include "etj_cvar_parser.h"

namespace ETJump {
class TimerunView : public Drawable {
public:
  explicit TimerunView(std::shared_ptr<Timerun> timerun);
  ~TimerunView() override;

  void draw() override;

private:
  // returns the currently active run if there's any
  // e.g. if player is running => return player's run,
  // else if player is running, and we're speccing the player
  // => return that player's run
  const Timerun::PlayerTimerunInformation *currentRun() const;

  static std::string getTimerString(int msec);

  static int getTransitionRange(int previousTime);

  static void pastRecordAnimation(vec4_t *color, const char *text,
                                  int timerTime, int record);

  static float getTimerAlpha(bool running, bool autoHide, int fadeStart,
                             int duration);

  void setCheckpointSize();
  void setCheckpointPopupSize();

  vec4_t inactiveTimerColor{};
  std::shared_ptr<Timerun> _timerun;

  vec4_t colorDefault = {1.0f, 1.0f, 1.0f, 1.0f};
  vec4_t colorSuccess = {0.627f, 0.941f, 0.349f, 1.0f};
  vec4_t colorFail = {0.976f, 0.262f, 0.262f, 1.0f};
  static const int animationTime = 300;
  static const int fadeHold = 5000; // 5s pause before fade starts
  static const int fadeTime = 2000; // 2s fade

  CvarValue::Size checkpointSize{};
  CvarValue::Size popupSize{};

  static const int popupFadeTime = 100;

  int demoSvFps{};

  static bool canSkipDraw();
};
} // namespace ETJump
