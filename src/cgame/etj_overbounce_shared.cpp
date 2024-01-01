/*
 * MIT License
 *
 * Copyright (c) 2024 ETJump team <zero@etjump.com>
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

#include "etj_overbounce_shared.h"

namespace ETJump {
bool Overbounce::isOverbounce(float zVel, float startHeight, float endHeight,
                              float zVelSnapped, float pmoveSec, int gravity) {
  float a, b, c, discriminant;
  float root1;
  float reachedHeight;
  int timeSteps;

  a = -pmoveSec * zVelSnapped / 2;
  b = pmoveSec *
      (zVel - static_cast<float>(gravity) * pmoveSec / 2 + zVelSnapped / 2);
  c = startHeight - endHeight;

  if (a == 0) {
    // no quadratic equation
    return false;
  }

  discriminant = b * b - 4 * a * c;
  if (discriminant < 0) {
    // no real roots
    return false;
  }

  root1 = (-b - std::sqrt(discriminant)) / (2 * a);
  // float root2 = (-b + std::sqrt(discriminant)) / (2 * a);

  timeSteps = static_cast<int>(std::floor(root1));
  reachedHeight =
      startHeight + pmoveSec * static_cast<float>(timeSteps) *
                        (zVel - static_cast<float>(gravity) * pmoveSec / 2 -
                         static_cast<float>(timeSteps - 1) * zVelSnapped / 2);

  // CG_Printf("zVel=%f, startHeight=%f, endHeight=%f, timeSteps=%d,
  // reachedHeight: %f\n", zVel, startHeight, endHeight, timeSteps,
  // reachedHeight);

  if (timeSteps && reachedHeight < endHeight + stickyOffset &&
      reachedHeight > endHeight) {
    return true;
  }
  return false;
}

bool Overbounce::surfaceAllowsOverbounce(trace_t *trace) {
  if (cg_pmove.shared & BG_LEVEL_NO_OVERBOUNCE) {
    return ((trace->surfaceFlags & SURF_OVERBOUNCE) != 0);
  } else {
    return (trace->surfaceFlags & SURF_OVERBOUNCE) == 0;
  }
}
} // namespace ETJump
