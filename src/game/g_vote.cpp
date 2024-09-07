// g_vote.c: All callvote handling
// -------------------------------
//
#include "g_local.h"
#include "etj_local.h"
#include "etj_printer.h"
#include "etj_string_utilities.h"
#include "etj_map_statistics.h"
#include "etj_numeric_utilities.h"
#include "etj_rtv.h"
#include <set>

#define T_FFA 0x01
#define T_1V1 0x02
#define T_SP 0x04
#define T_TDM 0x08
#define T_CTF 0x10
#define T_WLF 0x20
#define T_WSW 0x40
#define T_WCP 0x80
#define T_WCH 0x100

static const char *ACTIVATED = "ACTIVATED";
static const char *DEACTIVATED = "DEACTIVATED";
static const char *ENABLED = "ENABLED";
static const char *DISABLED = "DISABLED";

//
// Update info:
//	1. Add line to aVoteInfo w/appropriate info
//	2. Add implementation for attempt and success (see an existing command
// for an example)
//
typedef struct {
  unsigned int dwGameTypes;
  const char *pszVoteName;
  int (*pVoteCommand)(gentity_t *ent, unsigned int dwVoteIndex, char *arg,
                      char *arg2);
  const char *pszVoteMessage;
  const char *pszVoteHelp;
} vote_reference_t;

static int G_Map_v_Wrapper(gentity_t *ent, unsigned int dwVoteIndex, char *arg,
                           char *arg2) {
  return G_Map_v(ent, dwVoteIndex, arg, arg2, false);
}

int G_DevMap_v_Wrapper(gentity_t *ent, unsigned int dwVoteIndex, char *arg,
                       char *arg2) {
  return G_Map_v(ent, dwVoteIndex, arg, arg2, true);
}

// VC optimizes for dup strings :)
static const vote_reference_t aVoteInfo[] = {
    {0x1ff, "map", G_Map_v_Wrapper, "Change map to",
     " <mapname>^7\n  Votes for a map to be loaded with cheats disabled"},
    {0x1ff, "devmap", G_DevMap_v_Wrapper, "Change map to",
     " <mapname>^7\n  Votes for a map to be loaded with cheats enabled"},
    {0x1ff, "maprestart", G_MapRestart_v, "Map restart",
     "^7\n  Restarts the current map in progress"},
    {0x1ff, "randommap", G_RandomMap_v, "Random map",
     " ^7\n Votes a new random map to be loaded"},
    {0x1ff, "rtv", ETJump::G_RockTheVote_v, "Rock The Vote",
     " ^7\n Initiates Rock The Vote"},
    {0x1ff, "autoRtv", ETJump::G_AutoRtv_v, "Auto RTV",
     " ^7\n Votes to set an interval for automatic Rock The Vote"},
    {0, nullptr, nullptr, nullptr}};

void G_cpmPrintf(gentity_t *ent, const char *fmt, ...) {
  va_list argptr;
  char text[1024];

  va_start(argptr, fmt);
  Q_vsnprintf(text, sizeof(text), fmt, argptr);
  va_end(argptr);

  if (ent == NULL) {
    trap_Printf(text);
  } else {
    CP(va("cpm \"%s\n\"", text));
  }
}

// Checks for valid custom callvote requests from the client.
int G_voteCmdCheck(gentity_t *ent, char *arg, char *arg2) {
  unsigned int i, cVoteCommands = sizeof(aVoteInfo) / sizeof(aVoteInfo[0]);

  for (i = 0; i < cVoteCommands; i++) {
    if (!Q_stricmp(arg, aVoteInfo[i].pszVoteName)) {
      int hResult = aVoteInfo[i].pVoteCommand(ent, i, arg, arg2);

      if (hResult == G_OK) {
        Com_sprintf(arg, VOTE_MAXSTRING, aVoteInfo[i].pszVoteMessage);
        level.voteInfo.vote_fn = aVoteInfo[i].pVoteCommand;
      } else {
        level.voteInfo.vote_fn = NULL;
      }

      return (hResult);
    }
  }

  return (G_NOTFOUND);
}

// Voting help summary.
void G_voteHelp(gentity_t *ent, qboolean fShowVote) {
  int i, rows = 0;
  int num_cmds =
      sizeof(aVoteInfo) / sizeof(aVoteInfo[0]) - 1; // Remove terminator;
  int vi[100]; // Just make it large static.
  auto clientNum = ClientNum(ent);

  if (fShowVote) {
    Printer::console(clientNum, "\nValid ^3callvote^7 commands are:\n"
                                "^3----------------------------\n");
  }

  for (i = 0; i < num_cmds; i++) {
    if (aVoteInfo[i].dwGameTypes & (1 << g_gametype.integer)) {
      vi[rows++] = i;
    }
  }

  num_cmds = rows;
  rows = num_cmds / HELP_COLUMNS;

  if (num_cmds % HELP_COLUMNS) {
    rows++;
  }

  for (i = 0; i < rows; i++) {
    if (i + rows * 3 + 1 <= num_cmds) {
      Printer::console(clientNum, va("^5%-17s%-17s%-17s%-17s",
                                     aVoteInfo[vi[i]].pszVoteName,
                                     aVoteInfo[vi[i + rows]].pszVoteName,
                                     aVoteInfo[vi[i + rows * 2]].pszVoteName,
                                     aVoteInfo[vi[i + rows * 3]].pszVoteName));
    } else if (i + rows * 2 + 1 <= num_cmds) {
      Printer::console(clientNum,
                       va("^5%-17s%-17s%-17s", aVoteInfo[vi[i]].pszVoteName,
                          aVoteInfo[vi[i + rows]].pszVoteName,
                          aVoteInfo[vi[i + rows * 2]].pszVoteName));
    } else if (i + rows + 1 <= num_cmds) {
      Printer::console(clientNum,
                       va("^5%-17s%-17s", aVoteInfo[vi[i]].pszVoteName,
                          aVoteInfo[vi[i + rows]].pszVoteName));
    } else {
      Printer::console(clientNum, va("^5%-17s", aVoteInfo[vi[i]].pszVoteName));
    }
  }

  if (fShowVote) {
    Printer::console(
        clientNum,
        "\n\nUsage: ^3callvote <command> <params>\n"
        "^7For current settings/help, use: ^3callvote <command> ?\n");
  }
}

// Set disabled votes for client UI
void G_voteFlags(void) {
  int i, flags = 0;

  if (g_enableVote.integer <= 0) {
    flags |= VOTING_DISABLED;
  } else {
    for (i = 0; i < numVotesAvailable; i++) {
      if (trap_Cvar_VariableIntegerValue(voteToggles[i].pszCvar) == 0) {
        flags |= voteToggles[i].flag;
      }
    }
  }
  if (flags != voteFlags.integer) {
    trap_Cvar_Set("voteFlags", va("%d", flags));
  }
}

// Prints specific callvote command help description.
qboolean G_voteDescription(gentity_t *ent, int cmd, bool argCountCheck = true) {
  char arg[MAX_TOKEN_CHARS];

  if (!ent) {
    return (qfalse);
  }

  trap_Argv(2, arg, sizeof(arg));
  if (!Q_stricmp(arg, "?") || (argCountCheck && trap_Argc() == 2)) {
    trap_Argv(1, arg, sizeof(arg));
    G_cpmPrintf(ent, "\nUsage: ^3\\vote %s%s\n", arg,
                aVoteInfo[cmd].pszVoteHelp);
    return (qtrue);
  }

  return (qfalse);
}

// Localize disable message info.
void G_voteDisableMessage(gentity_t *ent, const char *cmd) {
  G_cpmPrintf(ent, "Sorry, [lof]^3%s^7 [lon]voting has been disabled.", cmd);
}

// Player ID message stub.
void G_playersMessage(gentity_t *ent) {
  G_cpmPrintf(ent, "Use the ^3players^7 command to find a valid player ID.");
}

// Localize current parameter setting.
void G_voteCurrentSetting(gentity_t *ent, const char *cmd,
                          const char *setting) {
  G_cpmPrintf(ent, "^2%s^7 is currently ^3%s", cmd, setting);
}

// Vote toggling
int G_voteProcessOnOff(gentity_t *ent, char *arg, char *arg2, int curr_setting,
                       int vote_allow, int vote_type) {
  if (!vote_allow && ent) {
    G_voteDisableMessage(ent, aVoteInfo[vote_type].pszVoteName);
    G_voteCurrentSetting(ent, aVoteInfo[vote_type].pszVoteName,
                         ((curr_setting) ? ENABLED : DISABLED));
    return (G_INVALID);
  }
  if (G_voteDescription(ent, vote_type)) {
    G_voteCurrentSetting(ent, aVoteInfo[vote_type].pszVoteName,
                         ((curr_setting) ? ENABLED : DISABLED));
    return (G_INVALID);
  }

  if ((Q_atoi(arg2) && curr_setting) || (!Q_atoi(arg2) && !curr_setting)) {
    G_cpmPrintf(ent, "^3%s^5 is already %s!", aVoteInfo[vote_type].pszVoteName,
                ((curr_setting) ? ENABLED : DISABLED));
    return (G_INVALID);
  }

  Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, "%s", arg2);
  Com_sprintf(arg2, VOTE_MAXSTRING, "%s",
              (Q_atoi(arg2)) ? ACTIVATED : DEACTIVATED);

  return (G_OK);
}

//
// Several commands to help with cvar setting
//
void G_voteSetOnOff(const char *desc, const char *cvar) {
  AP(va("cpm \"^3%s is: ^5%s\n\"", desc,
        (Q_atoi(level.voteInfo.vote_value)) ? ENABLED : DISABLED));
  // trap_SendConsoleCommand(EXEC_APPEND, va("%s %s\n", cvar,
  // level.voteInfo.vote_value));
  trap_Cvar_Set(cvar, level.voteInfo.vote_value);
}

void G_voteSetValue(const char *desc, const char *cvar) {
  AP(va("cpm \"^3%s set to: ^5%s\n\"", desc, level.voteInfo.vote_value));
  // trap_SendConsoleCommand(EXEC_APPEND, va("%s %s\n", cvar,
  // level.voteInfo.vote_value));
  trap_Cvar_Set(cvar, level.voteInfo.vote_value);
}

void G_voteSetVoteString(const char *desc) {
  AP(va("print \"^3%s set to: ^5%s\n\"", desc, level.voteInfo.vote_value));
  trap_SendConsoleCommand(EXEC_APPEND, va("%s\n", level.voteInfo.voteString));
}

namespace ETJump {
static bool matchMap(const char *voteArg, std::string &resultedMap,
                     const bool &cheats) {
  if (voteArg[0] == '\0') {
    resultedMap = "^3callvote: ^7No map specified.\n";
    return false;
  }

  auto matchedMaps = G_MatchAllMaps(voteArg);

  if (matchedMaps.size() == 0) {
    resultedMap =
        stringFormat("^3callvote: ^7no maps found matching ^3%s^7.\n", voteArg);
    return false;
  }

  // Check if called map is an exact match to a map on the server
  for (auto &map : matchedMaps) {
    if (!Q_stricmp(map.c_str(), voteArg)) {
      resultedMap = map;
      break;
    }
  }

  if (matchedMaps.size() > 1 && resultedMap.empty()) {
    resultedMap =
        stringFormat("^3callvote: ^7found ^3%zu ^7maps matching ^3%s^7.\n",
                     matchedMaps.size(), voteArg);
    auto perRow = 3;
    auto mapsOnCurrentRow = 0;
    for (auto &map : matchedMaps) {
      ++mapsOnCurrentRow;
      if (mapsOnCurrentRow > perRow) {
        mapsOnCurrentRow = 1;
        resultedMap += ETJump::stringFormat("\n%-22s", map);
      } else {
        resultedMap += ETJump::stringFormat("%-22s", map);
      }
    }
    resultedMap += "\n";
    return false;
  } else if (resultedMap.empty()) {
    resultedMap = matchedMaps[0];
  }

  if (resultedMap == level.rawmapname &&
      cheats == static_cast<bool>(g_cheats.integer)) {
    resultedMap =
        stringFormat("^3callvote: ^7%s is the current map (cheats %s).\n",
                     level.rawmapname, cheats ? "enabled" : "disabled");
    return false;
  }

  if (MapStatistics::isBlockedMap(resultedMap)) {
    resultedMap = stringFormat("^3callvote: ^7Voting for %s is not allowed.\n",
                               resultedMap);
    return false;
  }

  return true;
}

bool matchRandomMap(const char *arg, std::string &map) {
  bool isCustomMapType = strlen(arg) > 0;

  if (isCustomMapType) {
    auto customMapType = CustomMapTypeExists(arg);
    if (!customMapType) {
      map = stringFormat("^3randommap: ^7Map type %s "
                         "does not exist\n.",
                         arg);
      return false;
    }

    map = GetRandomMapByType(arg);
  } else {
    map = GetRandomMap();
  }

  if (map.empty()) {
    if (isCustomMapType) {
      map = "^3randommap: ^7no map is available on the "
            "requested map list.\n";
    } else {
      map = "^3randommap: ^7no map is available.\n";
    }
    return false;
  }

  return true;
}
} // namespace ETJump

////////////////////////////////////////////////////////
//
// Actual vote command implementation
//
////////////////////////////////////////////////////////

int G_RandomMap_v(gentity_t *ent, unsigned dwVoteIndex, char *arg, char *arg2) {
  if (arg) {
    char serverinfo[MAX_INFO_STRING];
    trap_GetServerinfo(serverinfo, sizeof(serverinfo));

    if (vote_allow_randommap.integer <= 0) {
      G_voteDisableMessage(ent, arg);
      return G_INVALID;
    }

    if (G_voteDescription(ent, static_cast<int>(dwVoteIndex), false)) {
      return G_INVALID;
    }

    std::string map;
    if (!ETJump::matchRandomMap(arg2, map)) {
      Printer::console(ClientNum(ent), map);
      return G_INVALID;
    }

    const char *mapTypeDesc = CustomMapTypeExists(arg2);
    // array size is set in Cmd_CallVote_f
    Q_strncpyz(arg2, mapTypeDesc ? mapTypeDesc : "", MAX_STRING_TOKENS);
    Q_strncpyz(level.voteInfo.vote_value, map.c_str(),
               sizeof(level.voteInfo.vote_value));
  } else {
    trap_SendConsoleCommand(EXEC_APPEND,
                            va("map %s\n", level.voteInfo.vote_value));
  }

  return G_OK;
}

// *** Map - simpleton: we don't verify map is allowed/exists ***
int G_Map_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg, char *arg2,
            const bool &cheats) {
  const int clientNum = ClientNum(ent);
  // Vote request (vote is being initiated)
  if (arg) {
    char serverinfo[MAX_INFO_STRING];
    trap_GetServerinfo(serverinfo, sizeof(serverinfo));

    if (!g_dedicated.integer && ent) {
      G_cpmPrintf(
          ent, "Sorry, [lof]^3%s^7 [lon]voting is disabled on localhost.", arg);
      return (G_INVALID);
    }
    if (vote_allow_map.integer <= 0 && ent) {
      G_voteDisableMessage(ent, arg);
      return (G_INVALID);
    }
    if (G_voteDescription(ent, static_cast<int>(dwVoteIndex))) {
      return (G_INVALID);
    }

    std::string map;
    if (!ETJump::matchMap(arg2, map, cheats)) {
      Printer::console(clientNum, map);
      return G_INVALID;
    }

    // array size is set in Cmd_CallVote_f
    Q_strncpyz(arg2, map.c_str(), MAX_STRING_TOKENS);
    Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, "%s", arg2);
    G_increaseCallvoteCount(arg2);

    // Vote action (vote has passed)
  } else {
    char s[MAX_STRING_CHARS];

    G_increasePassedCount(level.voteInfo.vote_value);

    Svcmd_ResetMatch_f(qfalse);
    trap_Cvar_VariableStringBuffer("nextmap", s, sizeof(s));
    trap_SendConsoleCommand(EXEC_APPEND,
                            va("%s %s%s\n", cheats ? "devmap" : "map",
                               level.voteInfo.vote_value,
                               ((*s) ? va("; set nextmap \"%s\"", s) : "")));
  }

  return (G_OK);
}

// *** Map Restart ***
int G_MapRestart_v(gentity_t *ent, unsigned int dwVoteIndex, char *arg,
                   char *arg2) {
  // Vote request (vote is being initiated)
  if (arg) {
    if (vote_allow_matchreset.integer <= 0) {
      G_voteDisableMessage(ent, arg);
      return G_INVALID;
    }

    if (G_voteDescription(ent, static_cast<int>(dwVoteIndex), false)) {
      return (G_INVALID);
    }
    // Vote action (vote has passed)
  } else {
    // Restart the map
    Svcmd_ResetMatch_f(qtrue);
  }

  return (G_OK);
}

namespace ETJump {
int G_RockTheVote_v(gentity_t *ent, unsigned dwVoteIndex, char *arg,
                    char *arg2) {
  if (arg) {
    if (!level.voteInfo.isAutoRtvVote) {
      if (!vote_allow_rtv.integer) {
        G_voteDisableMessage(ent, arg);
        return G_INVALID;
      }

      if (G_voteDescription(ent, static_cast<int>(dwVoteIndex), false)) {
        return (G_INVALID);
      }
    }

    std::vector<std::string> maps{};
    int numMaps;
    const int clientNum = !level.voteInfo.isAutoRtvVote
                              ? ClientNum(ent)
                              : level.voteInfo.voter_cn;

    if (arg2[0]) {
      if (!CustomMapTypeExists(arg2)) {
        Printer::popup(
            clientNum,
            stringFormat("Specified custom vote type ^3'%s' ^7does not exist.",
                         arg2));
        return G_INVALID;
      }

      maps = getMapsOnList(arg2);
      numMaps = static_cast<int>(maps.size());

      // in case someone made an empty map list, or none are available
      if (numMaps == 0) {
        Printer::popup(clientNum,
                       stringFormat("Specified custom vote type ^3'%s' ^7is "
                                    "empty or none of the maps are available.",
                                    arg2));
        return G_INVALID;
      }

      // exclude current map if present on the list
      for (const auto &map : maps) {
        if (level.rawmapname == map) {
          numMaps -= 1;
          break;
        }
      }
    } else {
      maps = (*game.mapStatistics->getCurrentMaps());
      // -1 to exclude current map
      numMaps = static_cast<int>(maps.size() - 1);
    }

    if (numMaps < 2) {
      // auto rtv
      if (ent == nullptr) {
        G_LogPrintf("Automatic %s is not possible on a server with less than 3 "
                    "valid maps, consider adding more maps to the server to "
                    "use this feature.\n",
                    arg);
      } else {
        Printer::popup(clientNum,
                       stringFormat("Sorry, calling [lof]^3%s^7[lon] with less "
                                    "than 3 valid maps on %s is not possible.",
                                    arg, arg2[0] ? "a list" : "the server"));
      }
      return G_INVALID;
    }

    const size_t maxMaps =
        Numeric::clamp(g_rtvMapCount.integer, 2, std::min(numMaps, 9));
    std::set<std::string> uniqueMaps;
    auto rtvMaps = game.rtv->getRtvMaps();

    game.rtv->clearRtvMaps();

    // just copy all the maps if we don't have more maps than requested
    if (numMaps <= maxMaps) {
      std::copy(maps.begin(), maps.end(),
                std::inserter(uniqueMaps, uniqueMaps.begin()));
      uniqueMaps.erase(level.rawmapname);
    } else {
      while (uniqueMaps.size() <= maxMaps) {
        // neither of these functions ever return the current map
        const char *map = arg2[0] ? GetRandomMapByType(arg2) : GetRandomMap();
        uniqueMaps.insert(map);
      }
    }

    std::string cs;
    auto it = uniqueMaps.begin();
    rtvMaps->resize(maxMaps);

    for (size_t i = 0; i < maxMaps; ++i, ++it) {
      (*rtvMaps)[i].first = *it;
      cs += stringFormat("%s\\0%s", (*rtvMaps)[i].first,
                         i == maxMaps - 1 ? "" : "\\");
    }

    const char *mapTypeDesc = CustomMapTypeExists(arg2);
    // array size is set in Cmd_CallVote_f
    Q_strncpyz(arg2, mapTypeDesc ? mapTypeDesc : "", MAX_STRING_TOKENS);

    // this will never overflow as MAX_QPATH is 64 and rtv supports max 9 maps
    trap_SetConfigstring(CS_VOTE_YES, cs.c_str());
  } else {
    game.rtv->setRtvWinner();
    trap_SendConsoleCommand(EXEC_APPEND,
                            va("map %s\n", level.voteInfo.vote_value));
  }

  return G_OK;
}

int G_AutoRtv_v(gentity_t *ent, unsigned dwVoteIndex, char *arg, char *arg2) {
  if (arg) {
    if (!vote_allow_autoRtv.integer) {
      G_voteDisableMessage(ent, arg);
      return G_INVALID;
    }

    if (G_voteDescription(ent, static_cast<int>(dwVoteIndex), false)) {
      return G_INVALID;
    }

    // abs so that big values are capped to max time
    // instead of resulting in < 0 due to overflow
    const int time = std::abs(Q_atoi(arg2));

    if (time < 0) {
      Q_strncpyz(arg2, "0", MAX_STRING_TOKENS);
    } else if (time > game.rtv->AUTORTV_MAX_TIME) {
      Q_strncpyz(arg2, std::to_string(game.rtv->AUTORTV_MAX_TIME).c_str(),
                 MAX_STRING_TOKENS);
    }

    // if auto rtv is already off, no point turning it off again
    if (Q_atoi(arg2) == 0 && !g_autoRtv.integer) {
      Printer::popup(ClientNum(ent),
                     "^gAutomatic Rock The Vote is already turned off.");
      return G_INVALID;
    }

    Com_sprintf(level.voteInfo.vote_value, VOTE_MAXSTRING, arg2);
  } else {
    trap_SendConsoleCommand(EXEC_APPEND,
                            va("g_autoRtv %s", level.voteInfo.vote_value));

    if (!Q_stricmp(level.voteInfo.vote_value, "0")) {
      Printer::popupAll("^gAutomatic Rock The Vote has been turned off.");
    } else {
      std::string voteMsg;

      if (!g_autoRtv.integer) {
        voteMsg = "^gAutomatic Rock The Vote has been turned on!";
      } else {
        voteMsg = "^gAutomatic Rock The Vote interval was adjusted!";
      }

      const int interval = Q_atoi(level.voteInfo.vote_value);
      const int nextVoteTime =
          interval - ((level.time - game.rtv->autoRtvStartTime) / (1000 * 60));
      const char *minutesStr = nextVoteTime == 1 ? "minute" : "minutes";

      Printer::popupAll(voteMsg);
      if (nextVoteTime > 0) {
        Printer::popupAll(
            stringFormat("^gNext vote will be called in ^3%i ^g%s.",
                         nextVoteTime, minutesStr));
      }
    }
  }

  return G_OK;
}
} // namespace ETJump
