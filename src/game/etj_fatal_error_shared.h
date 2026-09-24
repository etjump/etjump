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
#include <cstdint>
#include <exception>
#include <string>
#include <utility>

/*
 * Fatal errors (G_Error, CG_Error, Com_Error in ui) end up in trap_Error,
 * which makes the engine shut the module down, unload it and longjmp back
 * to its main loop, jumping over every mod stack frame in between. Calling
 * trap_Error deep inside mod code therefore skips all C++ destructors,
 * runs the module shutdown on top of half executed functions, and on
 * engines whose longjmp unwinds SEH style (MSVC built Windows engines) makes
 * the unwinder walk frames of an already unloaded library.
 *
 * Instead, fatal errors throw a FatalError, which unwinds the mod stack with
 * regular C++ semantics up to the vmMain boundary. The boundary then either
 * hands the error to the module (qagame shuts the server down through the
 * command buffer instead, see GameFailure), or raises it to the engine from
 * vmMain itself, where nothing is left to destruct, and on Windows keeps the
 * module mapped until the engine loads it again.
 *
 * The engine also raises errors of its own inside syscalls, e.g. when the
 * server disconnects a client, which unloads the module the same way while
 * its frames are still on the stack. The module's shutdown command detects
 * this and keeps the module mapped as well.
 */
namespace ETJump {
// Intentionally not derived from std::exception, so handlers for those don't
// catch it. A handler that catches everything (catch (...)) and doesn't
// rethrow still can't hide it: throwFatal records the error before throwing,
// and the vmMain call fails once it ends.
class FatalError {
  std::string message;
  bool unexpected;
  bool reported;

public:
  explicit FatalError(std::string message, bool unexpected = false,
                      bool reported = false);

  const std::string &getMessage() const;

  // set for errors that originate from an unhandled exception,
  // whose text is only meant for the server console and logs
  bool isUnexpected() const;

  // set once the error was printed, while it's passed on from a call nested
  // inside a syscall to the call that made the syscall
  bool isReported() const;
};

namespace FatalErrorBoundary {
// same as the formatting buffer size of G_Error/CG_Error
inline constexpr size_t MAX_MESSAGE_LENGTH = 1024;

// set by cgame once it keeps itself mapped after an error, cgame is only
// loaded again on the next connect, so ui releases it earlier than that
inline constexpr char CGAME_KEEP_ALIVE_CVAR[] = "etj_cgameKeepAlive";

// trap_Error of the module, must not return
using RaiseFunction = void (*)(const char *message);
using PrintFunction = void (*)(const char *message);

// called when an error reaches the outermost vmMain call, returning true
// means the module handles the error itself instead of raising it to the
// engine (e.g. qagame shuts the server down through the command buffer)
using DeferFunction = bool (*)(const char *message, bool unexpected);

// called once the module started keeping itself mapped (Windows only)
using KeepAliveFunction = void (*)();

// when the reference a module keeps on itself after an error is dropped
// (Windows only)
enum class KeepAliveRelease {
  // once the module is loaded again, for modules the engine only loads again
  // after it's done handling the error (qagame, cgame)
  OnLoad,
  // through releaseKeepAlive(), for modules the engine loads again while it's
  // still handling the error, possibly more than once (ui)
  Manual,
};

// must be called from dllEntry, every time the module is loaded
void initialize(RaiseFunction raiseFunc, PrintFunction printFunc,
                KeepAliveFunction keepAliveFunc = nullptr,
                KeepAliveRelease release = KeepAliveRelease::OnLoad);

// records the error and throws a FatalError if called inside vmMain,
// otherwise (static init, other threads, a destructor running during stack
// unwinding inside the same vmMain call) raises directly
[[noreturn]] void throwFatal(const char *message);

// raises the error to the engine immediately, must only be called
// when the calling frames hold no objects that need destructing
[[noreturn]] void raiseError(const char *message);

// must be called while handling the module shutdown command, returns true
// if another vmMain call is active further up the stack, which means the
// engine unloads the module while handling an error raised inside a syscall
// and is about to longjmp over the module's frames, in which case the
// module is kept mapped on Windows
bool protectActiveFrames();

// true while this module keeps itself mapped after an error (Windows only)
bool isKeptAlive();

// drops the reference this module keeps on itself after an error, must only
// be called from a vmMain call, so the engine holds a reference as well, and
// once the engine is done handling that error (Windows only)
void releaseKeepAlive();

// drops the reference the given module keeps on itself after an error,
// must only be called once the engine is done handling that error
// (Windows only, does nothing on other platforms)
void releaseKeptModule(const char *moduleName);

// an exception as the boundary records it
struct CaughtError {
  std::array<char, MAX_MESSAGE_LENGTH> message{};
  // anything but a FatalError is unexpected
  bool unexpected = false;
  bool reported = false;
  // set for a FatalError, which was recorded when it was thrown
  bool recorded = false;
};

// describes the exception a catch handler is currently handling, so the
// text matches what the boundary reports, must only be called from inside
// a catch block (it rethrows the exception to inspect it)
CaughtError describeCaughtException();

namespace detail {
// set while an error that happened in an active vmMain call is recorded,
// until the outermost call handles it
extern bool hasPending;

// the state when a vmMain call started, which is restored when it ends
struct Entry {
  int depth;
  int uncaughtExceptions;
  uintptr_t outermostFrame;
  // an error recorded before the call started belongs to an earlier call
  bool hadPending;
};

void rethrowPending();
Entry enter();
void capture(const CaughtError &error);
// returns true if the call failed, raises the error if it must be raised now
bool leave(const Entry &entry, DeferFunction defer);
} // namespace detail

// called after every syscall: rethrows an error recorded while the engine
// called back into the module during that syscall, so only syscalls that
// re-entered the module and failed there can throw, which a destructor must
// not make (only throwing during stack unwinding is avoided, by failing the
// call once it ends instead)
inline void rethrowPending() {
  // checked inline, as cgame makes thousands of syscalls per frame
  if (detail::hasPending) {
    detail::rethrowPending();
  }
}

// runs a vmMain command, 'dispatch' must return the result for the engine,
// 'failureResult' is returned instead if the command fails without the error
// being raised right away, because it's nested inside a syscall or deferred
template <typename Dispatch>
intptr_t run(Dispatch &&dispatch, const intptr_t failureResult = 0,
             const DeferFunction defer = nullptr) {
  intptr_t result = failureResult;

  const detail::Entry entry = detail::enter();

  try {
    result = dispatch();
  } catch (...) {
    detail::capture(describeCaughtException());
  }

  // nothing that needs destructing is alive past this point, so it's safe
  // for the engine to longjmp over this frame if the error is raised now,
  // and the call can fail although its dispatch returned normally, e.g. when
  // a handler swallowed the error, or a destructor made a syscall during
  // unwinding that failed in a nested call
  return detail::leave(entry, defer) ? failureResult : result;
}
} // namespace FatalErrorBoundary

// Runs every step, even if an earlier one fails, and rethrows the first error
// afterwards. Meant for shutdown sequences, where a failing step must not
// skip the ones after it, e.g. a failed save must not leave worker threads
// running, as destroying them while they run terminates the process.
template <typename... Steps>
void runEachStep(Steps &&...steps) {
  std::exception_ptr firstError;

  const auto runStep = [&firstError](auto &&step) {
    try {
      step();
    } catch (...) {
      if (!firstError) {
        firstError = std::current_exception();
      }
    }
  };

  (runStep(std::forward<Steps>(steps)), ...);

  if (firstError) {
    std::rethrow_exception(firstError);
  }
}
} // namespace ETJump
