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

#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>

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
 * regular C++ semantics up to the vmMain boundary. The boundary raises the
 * error to the engine from vmMain itself, where nothing is left to destruct,
 * and on Windows keeps the module mapped until the engine loads it again.
 *
 * The engine also raises errors of its own inside syscalls, e.g. when the
 * server disconnects a client, which unloads the module the same way while
 * its frames are still on the stack. The module's shutdown command detects
 * this and keeps the module mapped as well.
 */
namespace ETJump {
// Intentionally not derived from std::exception,
// so generic exception handlers can't swallow it.
class FatalError {
  std::string message;
  bool unexpected;

public:
  explicit FatalError(std::string message, bool unexpected = false);

  const std::string &getMessage() const;

  // set for errors that originate from an unhandled exception,
  // whose text is only meant for the server console and logs
  bool isUnexpected() const;
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

// called when an error reaches the outermost vmMain call,
// returning true means the caller takes care of raising the error later
using DeferFunction = bool (*)(const char *message, bool unexpected);

// called once the module started keeping itself mapped (Windows only)
using KeepAliveFunction = void (*)();

// must be called from dllEntry, every time the module is loaded
void initialize(RaiseFunction raiseFunc, PrintFunction printFunc,
                KeepAliveFunction keepAliveFunc = nullptr);

// throws a FatalError if called inside vmMain, otherwise
// (static init, other threads, during stack unwinding) raises directly
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

// drops the reference the given module keeps on itself after an error,
// must only be called once the engine is done handling that error
// (Windows only, does nothing on other platforms)
void releaseKeptModule(const char *moduleName);

namespace detail {
// set while an error recorded in a nested vmMain call waits to be rethrown
extern bool hasPending;

void rethrowPending();
int enter();
void capture(const char *message, const char *details, bool unexpected);
bool leave(int entryDepth, DeferFunction defer);
[[noreturn]] void raisePending();
} // namespace detail

// called after every syscall: rethrows an error recorded while the engine
// called back into the module during that syscall, so only syscalls that
// re-entered the module and failed there can throw
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

  const int entryDepth = detail::enter();

  try {
    result = dispatch();
  } catch (const FatalError &e) {
    detail::capture(e.getMessage().c_str(), nullptr, e.isUnexpected());
  } catch (const std::exception &e) {
    detail::capture("Unhandled exception: ", e.what(), true);
  } catch (...) {
    detail::capture("Unhandled unknown exception", nullptr, true);
  }

  // nothing that needs destructing is alive past this point,
  // so it's safe for the engine to longjmp over this frame
  if (detail::leave(entryDepth, defer)) {
    detail::raisePending();
  }

  return result;
}
} // namespace FatalErrorBoundary
} // namespace ETJump
