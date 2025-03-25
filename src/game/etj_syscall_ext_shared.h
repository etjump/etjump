/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
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

#if defined(GAMEDLL)
  #include "g_local.h"
#elif defined(CGAMEDLL)
  #include "../cgame/cg_local.h"
#elif defined(UIDLL)
  #include "../ui/ui_local.h"
#endif

#include <unordered_map>

namespace ETJump {
class SyscallExt {
public:
#ifdef CGAMEDLL
  const char *flashWindowETLegacy = "trap_SysFlashWindow_Legacy";
  const char *cmdBackupExt = "trap_CmdBackup_Ext_Legacy";

  std::unordered_map<const char *, int> cgameExtensions = {
      {flashWindowETLegacy, 0},
      {cmdBackupExt, 0},
  };

  // defined by SDL2
  enum class FlashWindowState {
    SDL_FLASH_CANCEL = 0,
    SDL_FLASH_BRIEFLY = 1,
    SDL_FLASH_UNTIL_FOCUSED = 2,
  };

  static void trap_SysFlashWindowETLegacy(FlashWindowState state);
  static void trap_CmdBackup_Ext();
#endif

  // entry point for extensions
  int dll_com_trapGetValue{};

  // the implementations are in module-specific *_syscalls.cpp files
  // due to SystemCall macro expansion
  static bool trap_GetValue(char *value, int size, const char *key);

  SyscallExt() = default;
  ~SyscallExt() = default;
  void setupExtensions();

private:
  static void setupExtensionTrap(char *value, const std::string &name,
                                 int &trap);

  // the entry point cvar into the extension system in engine
  const char *entryPointCvar = "//trap_GetValue";
  char value[MAX_CVAR_VALUE_STRING]{};
};
} // namespace ETJump
