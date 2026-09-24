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

#include <array>
#include <cstddef>

#include "etj_fatal_error_shared.h"

namespace ETJump {
// real time to wait between dropping the clients and shutting down,
// the disconnect messages are delivered in the next server frames
inline constexpr int GAME_FAILURE_SHUTDOWN_DELAY = 1000;

// long server commands are discarded by some engines, so the disconnect
// wouldn't arrive, the full message is still printed to the server console,
// and written to the game log (by G_Error, or by the game when it fails)
inline constexpr size_t GAME_FAILURE_MAX_DENIAL_LENGTH = 256;

// the text of unexpected errors (unhandled exceptions) might contain
// internal details, so clients only get a generic message for those
inline constexpr char GAME_FAILURE_GENERIC_DENIAL[] =
    "Server shut down due to an internal error.";

// holds 'killserver' while a shutdown is queued, the queued command only
// expands this, so loading a map in the meantime can cancel it
inline constexpr char GAME_FAILURE_SHUTDOWN_CVAR[] = "etj_failureShutdown";

// A fatal error in qagame doesn't go through trap_Error, which would make the
// engine longjmp over the module's frames, and which doesn't notify remote
// clients either, so they would sit there until they time out. Instead, the
// game goes inert, clients are dropped with the error message as the reason,
// and the server is shut down a moment later with a regular 'killserver',
// once the engine had the chance to deliver the disconnect messages.
//
// The local client of a listen server is the exception, if the failure
// happened while it was loading a map: the engine already shut down its ui,
// cgame and renderer, and without cgame, it never handles its disconnect,
// only an engine error brings them back. So unless it was dropped while it
// had cgame loaded, the error is raised to the engine at the point the
// server would be shut down otherwise, when there's nothing left to unwind.
class GameFailure {
  bool active = false;
  bool dropsPending = false;
  bool shutdownQueued = false;
  bool localClientDropped = false;
  int dropTime = 0;
  std::array<char, GAME_FAILURE_MAX_DENIAL_LENGTH + 1> denialMessage{};
  std::array<char, FatalErrorBoundary::MAX_MESSAGE_LENGTH> message{};

public:
  GameFailure();

  bool isActive() const;

  // 'dropClients' is false when the engine drops the clients itself, which
  // happens during map load, as the game denies their reconnect
  void activate(const char *errorMessage, bool unexpected, bool dropClients,
                int time);
  void reset();

  // reason given to dropped or connecting clients, stays at the same
  // address, so it can be handed to the engine before a failure happens
  const char *getDenialMessage() const;

  // the error message as it was given, raised to the engine for the local
  // client of a listen server
  const char *getMessage() const;

  // returns true once, when the clients should be dropped now
  bool takeClientDrops(int time);

  // the local client of a listen server was dropped, which it handles in
  // its cgame, as only clients that loaded the map are dropped
  void setLocalClientDropped();

  // returns true once, when the server should be shut down now
  bool takeShutdown(int time);

  // true if the server is shut down by raising the error to the engine
  bool shutsDownThroughError(bool dedicated) const;
};
} // namespace ETJump
