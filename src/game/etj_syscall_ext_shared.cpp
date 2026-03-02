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

#include "etj_syscall_ext_shared.h"

#if defined(GAMEDLL)
  #include "g_local.h"
#elif defined(CGAMEDLL)
  #include "../cgame/cg_local.h"
#elif defined(UIDLL)
  #include "../ui/ui_local.h"
#endif

namespace ETJump {
void SyscallExt::setupExtensions() {
  trap_Cvar_VariableStringBuffer(entryPointCvar, value, sizeof(value));

  // extension system not supported by the engine
  if (value[0] == '\0') {
    return;
  }

  dll_com_trapGetValue = Q_atoi(value);

#ifdef CGAMEDLL
  for (auto &[name, trap] : cgameExtensions) {
    setupExtensionTrap(value, name, trap);
  }
#endif
}

void SyscallExt::setupExtensionTrap(char *value, const std::string &name,
                                    int &trap) {
  if (trap_GetValue(value, MAX_CVAR_VALUE_STRING, name.c_str())) {
    trap = Q_atoi(value);
  }
}
} // namespace ETJump
