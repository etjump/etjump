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

#include "etj_fireteam_completions.h"
#include "cg_local.h"
#include "etj_command_complete_ext.h"

#include "../game/etj_string_utilities.h"

namespace ETJump {
FireteamCompletions::FireteamCompletions() { setupCompletions(); }

void FireteamCompletions::setupCompletions() {
  argCompletions.emplace_back("create");
  argCompletions.emplace_back(
      "apply",
      [](const std::vector<std::string> &args) { return apply(args); });
  argCompletions.emplace_back("leave");
  argCompletions.emplace_back("disband");
  argCompletions.emplace_back(
      "invite",
      [](const std::vector<std::string> &args) { return invite(args); });
  argCompletions.emplace_back("kick", [](const std::vector<std::string> &args) {
    return kickOrWarn(args);
  });
  argCompletions.emplace_back("warn", [](const std::vector<std::string> &args) {
    return kickOrWarn(args);
  });
  argCompletions.emplace_back(
      "propose",
      [](const std::vector<std::string> &args) { return propose(args); });
  argCompletions.emplace_back(
      "rules",
      [](const std::vector<std::string> &args) { return rules(args); });
  argCompletions.emplace_back(
      "teamjump", std::vector<std::string>{"tj"},
      [](const std::vector<std::string> &args) { return teamjump(args); });
  argCompletions.emplace_back("countdown");
}

bool FireteamCompletions::complete(const std::vector<std::string> &args) const {
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

bool FireteamCompletions::apply(const std::vector<std::string> &args) {
  // should not be called unless we have 'apply' as args already
  assert(!args.empty());

  if (args.size() > 2) {
    return false;
  }

  const std::string currentArg = args.size() > 1 ? args[1] : "";
  const auto *const ftData = cgs.clientinfo[cg.clientNum].fireteamData;

  for (int32_t i = 0; i < MAX_FIRETEAMS; i++) {
    const auto *const ft = &cg.fireTeams[i];

    if (!ft || !ft->inuse) {
      continue;
    }

    // this is our current fireteam
    if (ftData == ft) {
      continue;
    }

    const std::string ftName =
        StringUtils::toLowerCase(bg_fireteamNames[ft->ident]);

    // stop completion if we have a valid arg
    if (ftName == currentArg) {
      return false;
    }

    completeArg(ftName);
  }

  return true;
}

bool FireteamCompletions::invite(const std::vector<std::string> &args) {
  // should not be called unless we have 'invite' as args already
  assert(!args.empty());

  if (args.size() > 2) {
    return false;
  }

  const auto *const ftData = cgs.clientinfo[cg.clientNum].fireteamData;

  // we are not in a fireteam
  if (!ftData) {
    return false;
  }

  const std::string currentArg = args.size() > 1 ? args[1] : "";

  // skip adding 'spectator' and 'allied' aliases, no need to show both
  completeArg("axis");
  completeArg("allies");
  completeArg("spectators");
  completeArg("all");

  // stop completion if one of the special strings is matched
  // this intentionally skips the one character team names,
  // it would be quite annoying to have every 'b', 'r' and 's' character
  // to be treated as a completed completion
  // we also skip 'all' and 'spectator' to allow still
  // suggesting 'allies' and 'spectators'
  if (currentArg == "axis" || currentArg == "allies" ||
      currentArg == "allied" || currentArg == "spectators") {
    return false;
  }

  for (const auto &ci : cgs.clientinfo) {
    if (!ci.infoValid || ci.clientNum == cg.clientNum) {
      continue;
    }

    // player is already on a fireteam
    if (ci.fireteamData) {
      continue;
    }

    // stop completion if we have a valid client name as arg
    if (!Q_stricmp(ci.cleanname, currentArg.c_str()) ||
        !Q_stricmp(ci.name, currentArg.c_str())) {
      return false;
    }

    completeArg(ci.cleanname);
  }

  return true;
}

bool FireteamCompletions::kickOrWarn(const std::vector<std::string> &args) {
  // should not be called unless we have 'kick' or 'warn' as args already
  assert(!args.empty());

  if (args.size() > 2) {
    return false;
  }

  const auto *const ftData = cgs.clientinfo[cg.clientNum].fireteamData;

  // we are not in a fireteam
  if (!ftData) {
    return false;
  }

  const std::string currentArg = args.size() > 1 ? args[1] : "";

  for (const auto &ci : cgs.clientinfo) {
    if (!ci.infoValid || ci.clientNum == cg.clientNum) {
      continue;
    }

    // not in same fireteam
    if (ci.fireteamData != ftData) {
      continue;
    }

    // stop completion once we have a valid client name as arg
    if (!Q_stricmp(ci.cleanname, currentArg.c_str()) ||
        !Q_stricmp(ci.name, currentArg.c_str())) {
      return false;
    }

    completeArg(ci.cleanname);
  }

  return true;
}

bool FireteamCompletions::propose(const std::vector<std::string> &args) {
  // should not be called unless we have 'propose' as args already
  assert(!args.empty());

  if (args.size() > 2) {
    return false;
  }

  const auto *const ftData = cgs.clientinfo[cg.clientNum].fireteamData;

  // we are not in a fireteam
  if (!ftData) {
    return false;
  }

  const std::string currentArg = args.size() > 1 ? args[1] : "";

  for (const auto &ci : cgs.clientinfo) {
    if (!ci.infoValid || ci.clientNum == cg.clientNum) {
      continue;
    }

    // client is already in a fireteam
    if (ci.fireteamData) {
      continue;
    }

    // stop completion once we have a valid client name as arg
    if (!Q_stricmp(ci.cleanname, currentArg.c_str()) ||
        !Q_stricmp(ci.name, currentArg.c_str())) {
      return false;
    }

    completeArg(ci.cleanname);
  }

  return true;
}

bool FireteamCompletions::rules(const std::vector<std::string> &args) {
  // should not be called unless we have 'rules' as args already
  assert(!args.empty());

  if (args.size() > 3) {
    return false;
  }

  const auto *const ftData = cgs.clientinfo[cg.clientNum].fireteamData;

  // only fireteam leader may use these
  if (!ftData || ftData->leader != cg.clientNum) {
    return false;
  }

  const std::string mainArg = args.size() > 1 ? args[1] : "";
  const std::string secondaryArg = args.size() > 2 ? args[2] : "";

  // complete secondary args if we already have a main arg as input
  // these should maybe be in some sort of container if more are added
  if (mainArg == "savelimit" || mainArg == "noghost") {
    if (mainArg == "savelimit" && secondaryArg != "reset") {
      completeArg("reset");

      return true;
    }

    if (mainArg == "noghost" &&
        (secondaryArg != "on" && secondaryArg != "off")) {
      completeArg("on");
      completeArg("off");

      return true;
    }

    return false;
  }

  completeArg("savelimit");
  completeArg("noghost");

  return true;
}

bool FireteamCompletions::teamjump(const std::vector<std::string> &args) {
  // should not be called unless we have 'teamjump' or 'tj' as args already
  assert(!args.empty());

  if (args.size() > 2) {
    return false;
  }

  const auto *const ftData = cgs.clientinfo[cg.clientNum].fireteamData;

  // only fireteam leader may use these
  if (!ftData || ftData->leader != cg.clientNum) {
    return false;
  }

  const std::string currentArg = args.size() > 1 ? args[1] : "";

  if (currentArg == "on" || currentArg == "off") {
    return false;
  }

  completeArg("on");
  completeArg("off");

  return true;
}
} // namespace ETJump
