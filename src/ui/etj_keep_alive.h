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
// Decides when a reference a module keeps on itself after an error can be
// dropped. The engine only advances realtime between frames, so once it
// changed after the flag was first seen, the error handling that unloaded
// the module (and the longjmp over its frames) is guaranteed to be over.
class KeepAliveTracker {
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

// must be called from dllEntry, as ui is loaded again into the same image
// while an error is handled, which keeps the state from before (Windows only)
void resetKeepAlive();

// registers the cvar cgame sets when it keeps itself mapped, and drops the
// references ui and cgame keep on themselves if the error they were kept for
// has been handled already (Windows only)
void initKeepAlive();

// drops the references ui and cgame keep on themselves after an error, once
// the engine has finished handling it (Windows only), so the next load
// doesn't reuse the old image, e.g. instead of cgame from the server's pk3
// on the next connect
void releaseKeptModules(int realtime);
} // namespace ETJump
