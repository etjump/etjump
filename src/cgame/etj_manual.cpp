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

#include "cg_local.h"

void CG_Manual_f(void) {
  int i = 0;
  int argc = trap_Argc();
  int len = sizeof(commandManuals) / sizeof(commandManuals[0]);
  const char *cmd = NULL;

  if (argc != 2) {
    CG_Printf("Manual\n\n");
    CG_Printf("Usage:\n");
    CG_Printf("/man [command]\n\n");
    CG_Printf("Description:\n");
    CG_Printf("Explains how to use admin commands.\n");
    return;
  }

  cmd = CG_Argv(1);

  for (; i < len; i++) {
    if (!Q_stricmp(cmd, commandManuals[i].cmd)) {
      CG_Printf("%s\n\nUsage:\n%s\n\nDescription:\n%s\n", cmd,
                commandManuals[i].usage, commandManuals[i].description);
      return;
    }
  }
  CG_Printf("Couldn't find manual for command \"%s\".\n", cmd);
}
