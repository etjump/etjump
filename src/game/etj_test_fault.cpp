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

#include <cstring>
#include <stdexcept>
#include <string>

#include "etj_test_fault.h"
#include "etj_string_utilities.h"
#include "q_shared.h"

// provided by each module's syscall layer, with identical signatures
void trap_Cvar_Register(vmCvar_t *cvar, const char *var_name, const char *value,
                        int flags);
void trap_Cvar_Set(const char *var_name, const char *value);
void trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer,
                                    int bufsize);

namespace ETJump::TestFault {
namespace {
constexpr const char *CVAR_NAME = "etj_testFault";
constexpr const char *UNEXPECTED_SUFFIX = ":ex";

// registered on first use, so the cvar can be set by name from the console
// too (an unregistered name is dispatched as a console command instead)
vmCvar_t testFaultCvar;

// the vm call currently being dispatched, set by the scopes at the vmMain
// entry points; a fault only fires while this matches the 'etj_testFault' cvar
const char *currentVmCall = nullptr;

// logs a marker when the injected error unwinds this scope, which proves the
// error was raised with normal C++ semantics instead of an engine longjmp
// over the mod's frames
struct Probe {
  const char *name;
  const char *func;

  ~Probe() {
    Com_Printf("%s[testfault]%s %s: stack unwound from '%s' past the injection "
               "point (destructors ran successfully)\n",
               S_COLOR_GREEN, S_COLOR_WHITE, name, func);
  }
};

// raises the fault, as an unexpected C++ exception or a fatal error
void inject(const char *name, const char *func, const bool unexpected) {
  Com_Printf("%s[testfault]%s %s: injecting %s from '%s'\n", S_COLOR_CYAN,
             S_COLOR_WHITE, name,
             unexpected ? "an unexpected C++ exception" : "a fatal error",
             func);

  const Probe probe{name, func};

  if (unexpected) {
    throw std::runtime_error(StringUtils::format(
        "Test fault (unexpected exception): %s (%s)", name, func));
  }

  Com_Error(ERR_DROP, "Test fault: %s (%s)", name, func);
}
} // namespace

VmCallScope::VmCallScope(const char *name) : previous(currentVmCall) {
  static bool registered = false;

  if (!registered) {
    trap_Cvar_Register(&testFaultCvar, CVAR_NAME, "", 0);
    registered = true;
  }

  currentVmCall = name;
}

VmCallScope::~VmCallScope() { currentVmCall = previous; }

void maybeRaise(const char *func, const char *message) {
  char value[MAX_CVAR_VALUE_STRING]{};
  trap_Cvar_VariableStringBuffer(CVAR_NAME, value, sizeof(value));

  if (value[0] == '\0') {
    return;
  }

  bool unexpected = false;

  const size_t valueLength = std::strlen(value);
  const size_t suffixLength = std::strlen(UNEXPECTED_SUFFIX);

  if (valueLength > suffixLength &&
      Q_stricmp(value + valueLength - suffixLength, UNEXPECTED_SUFFIX) == 0) {
    value[valueLength - suffixLength] = '\0';
    unexpected = true;
  }

  if (currentVmCall == nullptr || Q_stricmp(value, currentVmCall) != 0) {
    return;
  }

  // one-shot, the fault only fires once per cvar set
  trap_Cvar_Set(CVAR_NAME, "");

  // the logs name the custom message if one was given, the enclosing
  // function otherwise
  inject(currentVmCall, message[0] != '\0' ? message : func, unexpected);
}
} // namespace ETJump::TestFault
