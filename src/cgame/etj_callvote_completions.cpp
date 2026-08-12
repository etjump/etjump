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

#include "etj_callvote_completions.h"
#include "cg_local.h"
#include "etj_command_complete_ext.h"
#include "etj_local.h"

#include "../game/etj_string_utilities.h"

namespace ETJump {
CallvoteCompletions::CallvoteCompletions() { setupCompletions(); }

void CallvoteCompletions::setupCompletions() {
  argCompletions.emplace_back(
      "map", [](const std::vector<std::string> &args) { return map(args); });
  argCompletions.emplace_back(
      "devmap", [](const std::vector<std::string> &args) { return map(args); });
  argCompletions.emplace_back("maprestart");
  argCompletions.emplace_back("randommap",
                              [](const std::vector<std::string> &args) {
                                return randomMapOrRtv(args);
                              });
  argCompletions.emplace_back("rtv", [](const std::vector<std::string> &args) {
    return randomMapOrRtv(args);
  });
  argCompletions.emplace_back("autoRtv");
  argCompletions.emplace_back("portalPredict");
}

bool CallvoteCompletions::complete(const std::vector<std::string> &args) const {
  if (args.empty()) {
    return defaultCompletion(argCompletions);
  }

  for (const auto &completion : argCompletions) {
    if (args[0] == completion.arg ||
        CommandCompletions::isAlias(args[0], completion.aliases)) {
      if (completion.fn == nullptr) {
        return false;
      }

      return completion.fn(args);
    }
  }

  return defaultCompletion(argCompletions);
}

bool CallvoteCompletions::map(const std::vector<std::string> &args) {
  // should not be called unless we have 'map' or 'devmap' as args
  assert(!args.empty());

  if (args.size() > 2) {
    return false;
  }

  const std::string currentArg = args.size() > 1 ? args[1] : "";

  for (const auto &map : cgame.serverMapList) {
    if (StringUtils::iEqual(currentArg, map)) {
      return false;
    }

    completeArg(map);
  }

  return true;
}

bool CallvoteCompletions::randomMapOrRtv(const std::vector<std::string> &args) {
  // should not be called unless we have 'randommap' or 'rtv' as args
  assert(!args.empty());

  if (args.size() > 2) {
    return false;
  }

  // realistically there should not be time for client to get here
  // before the amount of custom votes is requested, but check for this
  // just in case, since the info request needs the amount to be known
  if (!cg.numCustomvotesRequested) {
    return false;
  }

  // we haven't requested info yet (custom vote menu has never been opened),
  // so flip the boolean to trigger the requests to server
  if (!cg.customvoteInfoRequested) {
    cg.customvoteInfoRequested = true;
  }

  const std::string currentArg = args.size() > 1 ? args[1] : "";

  for (const auto &list : cgame.customVoteLists) {
    if (StringUtils::iEqual(currentArg, list)) {
      return false;
    }

    completeArg(list);
  }

  return true;
}
} // namespace ETJump
