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

#include "etj_ignore_completions.h"
#include "cg_local.h"

namespace ETJump {
bool IgnoreCompletions::complete(const std::vector<std::string> &args) const {
  if (args.size() > 1) {
    return false;
  }

  const std::string currentArg = !args.empty() ? args[0] : "";

  for (const auto &ci : cgs.clientinfo) {
    if (!ci.infoValid || ci.clientNum == cg.clientNum) {
      continue;
    }

    // current argument is already a valid client name
    if (!Q_stricmp(ci.name, currentArg.c_str()) ||
        !Q_stricmp(ci.cleanname, currentArg.c_str())) {
      return false;
    }

    completeArg(ci.cleanname);
  }

  return true;
}
} // namespace ETJump
