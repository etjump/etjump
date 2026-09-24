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

namespace ETJump {
// Decides when the reference cgame keeps on itself after an error can be
// dropped. The engine only advances realtime between frames, so once it
// changed after the flag was first seen, the error handling that unloaded
// cgame (and the longjmp over its frames) is guaranteed to be over.
class CgameKeepAliveTracker {
  // realtime of the first refresh that saw the flag set
  int flaggedAt = -1;

public:
  // returns true when the reference should be dropped now
  bool update(const bool flagged, const int realtime) {
    if (!flagged) {
      flaggedAt = -1;
      return false;
    }

    if (flaggedAt < 0) {
      flaggedAt = realtime;
      return false;
    }

    if (realtime == flaggedAt) {
      return false;
    }

    flaggedAt = -1;
    return true;
  }
};

// registers the cvar cgame sets when it keeps itself mapped (Windows only)
void initCgameKeepAlive();

// drops the reference cgame keeps on itself after an error (Windows only),
// once the engine has finished handling it, so the next connect doesn't
// reuse the old cgame image instead of the version from the server's pk3
void releaseCgameAfterError(int realtime);
} // namespace ETJump
