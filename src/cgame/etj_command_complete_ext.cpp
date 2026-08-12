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

#include "etj_command_complete_ext.h"
#include "cg_local.h"
#include "etj_fireteam_completions.h"
#include "etj_callvote_completions.h"

#include "../game/etj_string_utilities.h"

namespace ETJump {
CommandCompletions::CommandCompletions() { setupCompletions(); }

void CommandCompletions::setupCompletions() {
  completions.emplace_back("fireteam", std::make_unique<FireteamCompletions>());
  completions.emplace_back("callvote", std::vector<std::string>{"cv"},
                           std::make_unique<CallvoteCompletions>());
}

bool CommandCompletions::completeArgument() const {
  std::string cmd = StringUtils::toLowerCase(CG_Argv(0));

  if (cmd[0] == '\\' || cmd[0] == '/') {
    cmd.erase(0, 1);
  }

  for (const auto &completion : completions) {
    if (cmd == completion.cmd || isAlias(cmd, completion.aliases)) {
      const int32_t argc = trap_Argc();
      std::vector<std::string> args;
      args.reserve(argc);

      for (int32_t i = 1; i < argc; i++) {
        args.emplace_back(StringUtils::toLowerCase(CG_Argv(i)));
      }

      return completion.obj->complete(args);
    }
  }

  return false;
}

bool CommandCompletions::isAlias(const std::string_view arg,
                                 const std::vector<std::string> &aliases) {
  return std::any_of(aliases.cbegin(), aliases.cend(),
                     [&arg](const auto &alias) { return arg == alias; });
};

} // namespace ETJump
