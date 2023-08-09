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

#pragma once

#include <string>
#include "etj_drawable.h"
#include "etj_timerun.h"

namespace ETJump {
class TimerunView : public Drawable {
public:
  explicit TimerunView(std::shared_ptr<Timerun> timerun);
  ~TimerunView();
  
  void draw();

  int getTransitionRange(int previousRunTime);

  void pastRecordAnimation(vec4_t *color, const char *text, int timerTime,
                           int record);

  // returns the currently active run if there's any
  // e.g. if player is running => return player's run,
  // else if player is running and we're speccing the player
  // => return that player's run
  const Timerun::PlayerTimerunInformation *currentRun() const;

private:
  std::string getTimerString(const int msec);

  vec4_t inactiveTimerColor;
  std::shared_ptr<Timerun> _timerun;

  bool canSkipDraw() const;
};
} // namespace ETJump
