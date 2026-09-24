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

#include <cstdio>
#include <cstdlib>
#include <utility>

#ifdef _WIN32
  // MSVC builds already define this on the command line
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
#endif

#include "etj_fatal_error_shared.h"

namespace ETJump {
FatalError::FatalError(std::string message, const bool unexpected)
    : message(std::move(message)), unexpected(unexpected) {}

const std::string &FatalError::getMessage() const { return message; }

bool FatalError::isUnexpected() const { return unexpected; }

namespace FatalErrorBoundary {
namespace detail {
bool hasPending = false;
} // namespace detail

namespace {
RaiseFunction raiseFunction = nullptr;
PrintFunction printFunction = nullptr;
KeepAliveFunction keepAliveFunction = nullptr;

// number of active vmMain calls, only the thread that runs vmMain
// may throw, any other thread sees 0 and falls back to raising directly
thread_local int depth = 0;

// error recorded in a nested vmMain call, or waiting to be raised,
// along with the depth of the call that failed
char pendingMessage[MAX_MESSAGE_LENGTH]{};
bool pendingUnexpected = false;
int pendingDepth = 0;

// set once the error has been handed to the engine, the engine calls back
// into the module (shutdown) before it longjmps out of trap_Error
bool raising = false;
char raiseMessage[MAX_MESSAGE_LENGTH]{};

void copyMessage(char *dest, const char *message, const char *details) {
  std::snprintf(dest, MAX_MESSAGE_LENGTH, "%s%s", message ? message : "",
                details ? details : "");
}

#ifdef _WIN32
// Engines built with MSVC longjmp by unwinding SEH style, which needs the
// unwind data of every frame it passes, including the vmMain frame that
// raised the error. The engine unloads the module before it longjmps,
// so an extra reference keeps the module mapped until the engine loads
// it again, at which point the reference is dropped in initialize().
// That load reuses the same image, so C++ globals keep their state instead
// of being constructed again, and until then the file stays locked.
// On Linux, every reload works like this anyway: the modules define
// STB_GNU_UNIQUE symbols (from libstdc++ headers), so glibc marks them
// as NODELETE and dlclose never unmaps them.
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
                const KeepAliveFunction keepAliveFunc) {
  raiseFunction = raiseFunc;
  printFunction = printFunc;
  keepAliveFunction = keepAliveFunc;

  // the engine might have longjmp'd over active vmMain calls,
  // either through trap_Error or due to an error of its own
  depth = 0;
  detail::hasPending = false;
  raising = false;
  pendingMessage[0] = '\0';
  pendingUnexpected = false;
  pendingDepth = 0;

#ifdef _WIN32
  releaseModule();
#endif
}

void throwFatal(const char *message) {
  if (depth > 0 && std::uncaught_exceptions() == 0) {
    throw FatalError(message ? message : "");
  }

  // outside of vmMain, or called by a destructor during stack unwinding,
  // throwing isn't possible, so this is no worse than before
  raiseError(message);
}

void raiseError(const char *message) {
  copyMessage(raiseMessage, message, nullptr);

  detail::hasPending = false;
  raising = true;
  depth = 0;

#ifdef _WIN32
  acquireModule();
#endif

  if (raiseFunction) {
    raiseFunction(raiseMessage);
  }

  // trap_Error never returns
  std::abort();
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

namespace detail {
void rethrowPending() {
  // only the call whose syscall contained the failed call rethrows it,
  // other calls the engine makes into the module meanwhile (e.g. its own
  // shutdown command when it errors out) must run normally
  if (depth == 0 || depth != pendingDepth - 1 ||
      std::uncaught_exceptions() > 0) {
    return;
  }

  hasPending = false;
  throw FatalError(pendingMessage, pendingUnexpected);
}

int enter() { return depth++; }

void capture(const char *message, const char *details, const bool unexpected) {
  // keep the first error, anything after it is most likely a consequence
  if (hasPending) {
    return;
  }

  copyMessage(pendingMessage, message, details);
  pendingUnexpected = unexpected;
  pendingDepth = depth;
  hasPending = true;
}

bool leave(const int entryDepth, const DeferFunction defer) {
  // restored rather than decremented, the module might have been loaded
  // again while this call was active (e.g. an engine command executed
  // inside a syscall that restarts the module), which resets the depth
  depth = entryDepth;

  if (depth > 0 || !hasPending) {
    return false;
  }

  if (raising) {
    // the engine is already handling an error and called back into the
    // module, raising another one would turn into a fatal recursive error
    hasPending = false;

    if (printFunction) {
      char buffer[MAX_MESSAGE_LENGTH + 64];
      std::snprintf(buffer, sizeof(buffer),
                    "Ignoring fatal error during error handling: %s\n",
                    pendingMessage);
      printFunction(buffer);
    }

    return false;
  }

  if (defer && defer(pendingMessage, pendingUnexpected)) {
    hasPending = false;
    return false;
  }

  return true;
}

void raisePending() { raiseError(pendingMessage); }
} // namespace detail
} // namespace FatalErrorBoundary
} // namespace ETJump
