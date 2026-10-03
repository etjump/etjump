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

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <thread>
#include <utility>

#ifdef _MSC_VER
  #include <intrin.h>
#endif

#ifdef _WIN32
  // MSVC builds already define this on the command line
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  // windows.h defines min/max macros that break std::numeric_limits<>::max()
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
#endif

#include "etj_fatal_error_shared.h"

namespace ETJump {
FatalError::FatalError(std::string message, const bool unexpected,
                       const bool reported)
    : message(std::move(message)), unexpected(unexpected), reported(reported) {}

const std::string &FatalError::getMessage() const { return message; }

bool FatalError::isUnexpected() const { return unexpected; }

bool FatalError::isReported() const { return reported; }

namespace FatalErrorBoundary {
namespace detail {
bool hasPending = false;
} // namespace detail

namespace {
RaiseFunction raiseFunction = nullptr;
PrintFunction printFunction = nullptr;
KeepAliveFunction keepAliveFunction = nullptr;

// number of active vmMain calls
int depth = 0;

// std::uncaught_exceptions() when the innermost active vmMain call started,
// a higher count means a destructor runs during stack unwinding inside that
// call, exceptions already in flight when it started (e.g. an outer call's
// destructor made the syscall) don't matter, as the call catches its own
int entryUncaughtExceptions = 0;

// only the thread that runs vmMain may throw, others raise directly,
// not thread_local, as that keeps macOS from ever unloading the module
std::thread::id ownerThread;

// the first error that happened in an active vmMain call, recorded when a
// FatalError is thrown, or when a call catches any other exception, along
// with the depth of the call it currently belongs to
CaughtError pending;
int pendingDepth = 0;

// set when the call the error was passed on to couldn't rethrow it after
// the syscall returned, as a destructor made the syscall during unwinding,
// the call then fails once it ends, and its later syscalls don't throw it
bool pendingDeferred = false;

// set once the error has been handed to the engine, the engine calls back
// into the module (shutdown) before it longjmps out of trap_Error, from
// further down the stack than the stack position the error was raised at
bool raising = false;
uintptr_t raiseFrame = 0;
std::array<char, MAX_MESSAGE_LENGTH> raiseMessage{};

// stack position of the outermost active vmMain call
uintptr_t outermostFrame = 0;

// the stack position of the function it's inlined into
#ifdef _MSC_VER
__forceinline uintptr_t framePosition() {
  return reinterpret_cast<uintptr_t>(_AddressOfReturnAddress());
}
#else
__attribute__((always_inline)) inline uintptr_t framePosition() {
  return reinterpret_cast<uintptr_t>(__builtin_frame_address(0));
}
#endif

void copyMessage(std::array<char, MAX_MESSAGE_LENGTH> &dest,
                 const char *message, const char *details) {
  std::snprintf(dest.data(), dest.size(), "%s%s", message ? message : "",
                details ? details : "");
}

void resetState() {
  depth = 0;
  detail::hasPending = false;
  pending = {};
  pendingDepth = 0;
  pendingDeferred = false;
  raising = false;
  raiseFrame = 0;
}

bool isUnwinding() {
  return std::uncaught_exceptions() > entryUncaughtExceptions;
}

void printError(const char *prefix, const char *message) {
  if (!printFunction) {
    return;
  }

  std::array<char, MAX_MESSAGE_LENGTH + 64> buffer{};
  std::snprintf(buffer.data(), buffer.size(), "%s%s\n", prefix, message);

  // the print is a syscall itself, which mustn't rethrow the pending error
  const bool hadPending = detail::hasPending;
  detail::hasPending = false;
  printFunction(buffer.data());
  detail::hasPending = hadPending;
}

// Keeps the first error that's recorded, anything after it is most likely a
// consequence, and is only printed, in which case this returns false. That's
// not necessarily the error that happened first: other exceptions are only
// recorded once a call catches them, so a fatal error in a call nested inside
// a syscall made by a destructor while one unwinds the stack comes first.
bool record(const CaughtError &error) {
  if (detail::hasPending) {
    // a FatalError is either the recorded error itself, or was printed
    // here when throwFatal couldn't record it
    if (!error.recorded) {
      printError("Additional fatal error, another one is handled instead: ",
                 error.message.data());
    }

    return false;
  }

  pending = error;
  pendingDepth = depth;
  pendingDeferred = false;
  detail::hasPending = true;
  return true;
}

#ifdef _WIN32
// Engines built with MSVC longjmp by unwinding SEH style, which needs the
// unwind data of every frame it passes, including the vmMain frame that
// raised the error. The engine unloads the module before it longjmps,
// so an extra reference keeps the module mapped until the engine is done
// handling the error. The reference is dropped when the engine loads the
// module again, or later by the module itself if the engine does that while
// it's still handling the error (see KeepAliveRelease), or by another module
// earlier than the next load (ui does that for cgame).
//
// This has costs, which is why qagame avoids trap_Error for its own errors
// (except for the local client of a listen server, see GameFailure):
// * A load while the module is still mapped reuses the same image, so its
//   globals and statics keep their state instead of being initialized again
//   (e.g. a static reentrancy guard that was set when the error hit).
//   Windows builds never reload like that otherwise.
// * The file stays locked until then, so it can't be replaced or updated
//   by the engine extracting a different version from a pk3.
// * For errors the engine raises inside a syscall, the longjmp also runs the
//   destructors of C++ objects in the frames it unwinds (for MSVC built
//   modules, MinGW builds depend on the exception model), after the
//   module's shutdown already ran.
//
// Other platforms don't need this for errors, as their longjmp doesn't
// unwind. Whether a module is really unloaded there depends on the build:
// GCC debug builds export STB_GNU_UNIQUE symbols, which keeps glibc from
// ever unloading them, while GCC release builds are unloaded normally.
// An engine that unloads and loads a module again inside a syscall and then
// returns into the old frames (instead of longjmp'ing over them) makes them
// run unloaded code there, which isn't handled.
HMODULE keepAliveHandle = nullptr;

void acquireModule() {
  if (keepAliveHandle) {
    return;
  }

  HMODULE handle = nullptr;

  // any address inside this module identifies it,
  // and this increments the reference count of the module
  if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                         reinterpret_cast<LPCWSTR>(&keepAliveHandle),
                         &handle)) {
    keepAliveHandle = handle;

    if (keepAliveFunction) {
      keepAliveFunction();
    }
  }
}

void releaseModule() {
  if (keepAliveHandle) {
    FreeLibrary(keepAliveHandle);
    keepAliveHandle = nullptr;
  }
}
#endif

// 'frame' is the stack position of the outermost vmMain call the error is
// raised from, or of the raising code itself outside of vmMain
[[noreturn]] void raiseAt(const char *message, const uintptr_t frame) {
  copyMessage(raiseMessage, message, nullptr);

  detail::hasPending = false;
  pendingDeferred = false;
  raising = true;
  raiseFrame = frame;
  depth = 0;

#ifdef _WIN32
  acquireModule();
#endif

  if (raiseFunction) {
    raiseFunction(raiseMessage.data());
  }

  // trap_Error never returns
  std::abort();
}
} // namespace

#ifdef _WIN32
// lets another module take over the reference this module keeps on itself,
// the caller can't be this module, as releasing it might unmap its code
extern "C" __declspec(dllexport) void *etj_takeKeepAliveReference() {
  HMODULE handle = keepAliveHandle;
  keepAliveHandle = nullptr;
  return handle;
}
#endif

void initialize(const RaiseFunction raiseFunc, const PrintFunction printFunc,
                const KeepAliveFunction keepAliveFunc,
                const KeepAliveRelease release) {
  raiseFunction = raiseFunc;
  printFunction = printFunc;
  keepAliveFunction = keepAliveFunc;
  ownerThread = std::this_thread::get_id();

  // the engine might have longjmp'd over active vmMain calls,
  // either through trap_Error or due to an error of its own
  resetState();

#ifdef _WIN32
  if (release == KeepAliveRelease::OnLoad) {
    releaseModule();
  }
#else
  static_cast<void>(release);
#endif
}

void throwFatal(const char *message) {
  if (depth > 0 && std::this_thread::get_id() == ownerThread &&
      !isUnwinding()) {
    // before throwing, so a handler that swallows it can't hide it
    CaughtError error;
    copyMessage(error.message, message, nullptr);
    record(error);

    throw FatalError(message ? message : "");
  }

  // outside of vmMain, or called by a destructor during stack unwinding,
  // throwing isn't possible, so this is no worse than before
  raiseError(message);
}

// Never inlined, so outside of vmMain, the stack position it sees only depends
// on its caller. That position is always above the calls the engine makes
// while handling the error, but whether a later call through the same path
// is at or above it depends on the frames in between, the position is only
// exact for errors raised inside a vmMain call (see enter()).
#ifdef _MSC_VER
__declspec(noinline)
#else
__attribute__((noinline))
#endif
void raiseError(const char *message) {
  // a stack position on another thread can't be compared with the calls the
  // engine makes into the module, so only loading the module again ends it
  if (std::this_thread::get_id() != ownerThread) {
    raiseAt(message, std::numeric_limits<uintptr_t>::max());
  }

  raiseAt(message, depth > 0 ? outermostFrame : framePosition());
}

bool protectActiveFrames() {
  // the shutdown command itself is one of the active calls
  if (depth <= 1) {
    return false;
  }

#ifdef _WIN32
  acquireModule();
#endif

  return true;
}

bool isKeptAlive() {
#ifdef _WIN32
  return keepAliveHandle != nullptr;
#else
  return false;
#endif
}

void releaseKeepAlive() {
#ifdef _WIN32
  releaseModule();
#endif
}

void releaseKeptModule(const char *moduleName) {
#ifdef _WIN32
  const HMODULE module = GetModuleHandleA(moduleName);

  if (!module) {
    return;
  }

  using TakeFunction = void *(*)();
  // casting through void (*)() avoids -Wcast-function-type
  const auto take = reinterpret_cast<TakeFunction>(reinterpret_cast<void (*)()>(
      GetProcAddress(module, "etj_takeKeepAliveReference")));

  if (!take) {
    return;
  }

  if (auto *const handle = static_cast<HMODULE>(take())) {
    FreeLibrary(handle);
  }
#else
  static_cast<void>(moduleName);
#endif
}

CaughtError describeCaughtException() {
  CaughtError error;

  try {
    throw;
  } catch (const FatalError &e) {
    copyMessage(error.message, e.getMessage().c_str(), nullptr);
    error.unexpected = e.isUnexpected();
    error.reported = e.isReported();
    error.recorded = true;
  } catch (const std::exception &e) {
    copyMessage(error.message, "Unhandled exception: ", e.what());
    error.unexpected = true;
  } catch (...) {
    copyMessage(error.message, "Unhandled unknown exception", nullptr);
    error.unexpected = true;
  }

  return error;
}

namespace detail {
void rethrowPending() {
  // only the call whose syscall contained the failed call rethrows it,
  // other calls the engine makes into the module meanwhile (e.g. its own
  // shutdown command when it errors out) must run normally
  if (depth == 0 || depth != pendingDepth - 1 || pendingDeferred ||
      std::this_thread::get_id() != ownerThread) {
    return;
  }

  // A destructor made the syscall during stack unwinding, where throwing
  // would terminate the process. The call fails once it ends instead, and
  // later syscalls don't throw the error either, as they didn't fail (e.g.
  // one made by a destructor on a regular scope exit).
  if (isUnwinding()) {
    pendingDeferred = true;
    return;
  }

  // the error belongs to this call now, and stays recorded while it unwinds
  pendingDepth = depth;
  throw FatalError(pending.message.data(), pending.unexpected,
                   pending.reported);
}

// never inlined, so the stack position it sees only depends on its caller,
// even if link time optimization inlines it at some call sites otherwise
#ifdef _MSC_VER
__declspec(noinline)
#else
__attribute__((noinline))
#endif
Entry enter() {
  const uintptr_t frame = framePosition();

  // A nested call always comes through the engine (a syscall that calls
  // vmMain again), so it's strictly deeper in the stack than the outermost
  // active call. A call at the same position or above means the engine
  // jumped over the active calls without loading the module again (which
  // resets the state), e.g. when it calls the module through the same path
  // again. Otherwise the depth would stay too high, and errors would never
  // be raised. The same goes for the calls the engine makes while handling
  // a raised error, which are deeper in the stack than where it was raised,
  // otherwise every later error would be ignored. This can't detect a next
  // call that comes through a deeper engine path, it's then taken as nested,
  // but engines load the modules again after every error anyway (ETe, 2.60b
  // and ET: Legacy all unload them while handling it, which is what this is
  // a safety net for).
  if ((depth > 0 && frame >= outermostFrame) ||
      (raising && frame >= raiseFrame)) {
    resetState();

    if (printFunction) {
      printFunction("Resetting fatal error state, the engine returned "
                    "without loading the module again\n");
    }
  }

  // an error can only belong to an earlier call that is still active, one
  // that is left over at the outermost level is handled once this call ends
  const Entry entry{depth, entryUncaughtExceptions, outermostFrame,
                    hasPending && depth > 0};

  if (depth == 0) {
    outermostFrame = frame;
  }

  ++depth;
  entryUncaughtExceptions = std::uncaught_exceptions();
  return entry;
}

void capture(const CaughtError &error) { record(error); }

bool leave(const Entry &entry, const DeferFunction defer) {
  const uintptr_t frame = outermostFrame;

  // restored rather than decremented, the module might have been loaded
  // again while this call was active (e.g. an engine command executed
  // inside a syscall that restarts the module), which resets the depth,
  // and the calls after that loading overwrite the outermost frame
  depth = entry.depth;
  outermostFrame = entry.outermostFrame;
  entryUncaughtExceptions = entry.uncaughtExceptions;

  if (!hasPending || entry.hadPending) {
    return false;
  }

  if (depth > 0) {
    // printed right away, as the engine might never return from the syscall,
    // e.g. when it shuts the module down while handling an error of its own,
    // and not every module prints its errors when they're raised
    if (!pending.reported) {
      printError("Fatal error in a call nested inside a syscall: ",
                 pending.message.data());
    }

    pending.reported = true;

    // passed on to the call that made the syscall,
    // which rethrows it once the syscall returns
    pendingDepth = depth + 1;
    pendingDeferred = false;
    return true;
  }

  // the error is handled from here on, so it's no longer pending, syscalls
  // made while handling it don't see it, and an error raised meanwhile
  // (e.g. by a call into the module during a syscall) is handled on its own,
  // which is also why the message is copied
  const std::array<char, MAX_MESSAGE_LENGTH> message = pending.message;
  const bool unexpected = pending.unexpected;
  hasPending = false;

  if (raising) {
    // the engine is already handling an error and called back into the
    // module, raising another one would turn into a fatal recursive error
    printError("Ignoring fatal error during error handling: ", message.data());
    return true;
  }

  if (defer && defer(message.data(), unexpected)) {
    return true;
  }

  // this frame and run()'s hold nothing that needs destructing
  raiseAt(message.data(), frame);
}
} // namespace detail
} // namespace FatalErrorBoundary
} // namespace ETJump
