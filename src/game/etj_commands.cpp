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

#include <bitset>
#include "etj_commands.h"

#include "etj_command_parser.h"
#include "etj_local.h"
#include "etj_save_system.h"
#include "etj_session.h"
#include "etj_custom_map_votes.h"
#include "g_local.h"
#include "etj_map_statistics.h"
#include "etj_utilities.h"
#include "etj_tokens.h"
#include "etj_string_utilities.h"
#include "etj_printer.h"
#include "etj_timerun_v2.h"
#include "etj_chat_replay.h"
#include "etj_filesystem.h"
#include "etj_savepos_command_handler.h"

typedef std::function<bool(gentity_t *ent, Arguments argv)> Command;
typedef std::pair<std::function<bool(gentity_t *ent, Arguments argv)>, char>
    AdminCommandPair;
typedef std::map<std::string,
                 std::function<bool(gentity_t *ent, Arguments argv)>>::
    const_iterator ConstCommandIterator;
typedef std::map<std::string,
                 std::pair<std::function<bool(gentity_t *ent, Arguments argv)>,
                           char>>::const_iterator ConstAdminCommandIterator;
typedef std::map<std::string,
                 std::function<bool(gentity_t *ent, Arguments argv)>>::iterator
    CommandIterator;
typedef std::map<std::string,
                 std::pair<std::function<bool(gentity_t *ent, Arguments argv)>,
                           char>>::iterator AdminCommandIterator;

namespace ETJump {
std::optional<CommandParser::Command>
getOptCommand(const std::string &commandPrefix, int clientNum,
              const CommandParser::CommandDefinition &def, Arguments args) {
  auto command = ETJump::CommandParser(def, *args).parse();

  if (command.helpRequested) {
    Printer::chat(
        clientNum,
        ETJump::stringFormat("^3%s: ^7check console for help.", commandPrefix));
    Printer::console(clientNum, def.help());
    return std::nullopt;
  }

  if (!command.errors.empty()) {
    Printer::chat(
        clientNum,
        stringFormat(
            "^3%s: ^7operation failed. Check console for more information.",
            commandPrefix));
    Printer::console(clientNum, command.getErrorMessage() + "\n");

    return std::nullopt;
  }

  return std::move(command);
}
} // namespace ETJump

namespace ClientCommands {

bool BackupLoad(gentity_t *ent, Arguments argv) {
  ETJump::saveSystem->loadBackupPosition(ent);
  return true;
}

bool Load(gentity_t *ent, Arguments argv) {
  ETJump::saveSystem->load(ent);
  return true;
}

bool Save(gentity_t *ent, Arguments argv) {
  ETJump::saveSystem->save(ent);
  return true;
}

bool Unload(gentity_t *ent, Arguments argv) {
  ETJump::saveSystem->unload(ent);
  return true;
}

bool listCustomVotes(gentity_t *ent, Arguments argv) {
  const int clientNum = ClientNum(ent);
  const std::string &cmd = argv->at(0);

  if (argv->size() != 2) {
    std::string types = game.customMapVotes->listTypes();

    if (types.empty()) {
      Printer::console(clientNum, "No custom map votes found on the server.\n");
      return true;
    }

    Printer::console(
        clientNum,
        ETJump::stringFormat(
            "^gAvailable custom map vote lists:\n^z %s\n\n^gUse ^3%s "
            "[listname] ^gto view maps in the specified list.\n",
            types, cmd));
    return true;
  }

  const auto &type = argv->at(1);
  const std::string maplist = game.customMapVotes->listInfo(type);
  if (maplist.empty()) {
    Printer::console(
        clientNum, ETJump::stringFormat("^3%s: ^gcould not find list ^3'%s'\n",
                                        cmd, type));
    return false;
  }
  Printer::console(clientNum, maplist);
  return true;
}

bool Rankings(gentity_t *ent, Arguments argv) {
  // these are console commands but to make them more accessible
  // they were also made admin commands
  // server can't call these as they expect clientNum
  if (!ent) {
    return false;
  }
  auto args = ETJump::Container::skipFirstN(*argv, 1);

  auto optCommand = getOptCommand(
      "rankings", ClientNum(ent),
      ETJump::CommandParser::CommandDefinition::create(
          "rankings",
          "Displays the player rankings for a specific season. Uses the "
          "overall as the default.\n\n    /rankings --season <season> --page "
          "<page> --page-size <page size>\n\n    Has a shorthand format of:\n  "
          "  /rankings <season>")
          .addOption("season", "s", "Name of the season to list rankings for",
                     ETJump::CommandParser::OptionDefinition::Type::MultiToken,
                     false)
          .addOption("page", "p", "Which page of rankings to show",
                     ETJump::CommandParser::OptionDefinition::Type::Integer,
                     false)
          .addOption("page-size", "ps", "How many rankings to show per page",
                     ETJump::CommandParser::OptionDefinition::Type::Integer,
                     false),
      &args);

  if (!optCommand.has_value()) {
    return true;
  }

  auto command = optCommand.value();

  auto optSeason = command.getOptional("season");
  auto optPage = command.getOptional("page");
  auto optPageSize = command.getOptional("page-size");

  std::string season;

  if (!command.extraArgs.empty()) {
    season = command.extraArgs[0];
  }

  if (season.empty()) {
    season = optSeason.has_value() ? optSeason.value().text : "Default";
  }
  auto page = optPage.has_value() ? optPage.value().integer - 1 : 0;
  auto pageSize = optPageSize.has_value() ? optPageSize.value().integer : 20;

  pageSize = std::clamp(pageSize, 1, 100);

  auto userId = ETJump::session->GetId(ent);

  ETJump::Timerun::PrintRankingsParams params{
      ClientNum(ent), userId, std::move(season), page, pageSize};

  game.timerunV2->printRankings(params);
  return true;
}

bool ListSeasons(gentity_t *ent, Arguments argv) {
  // these are console commands but to make them more accessible
  // they were also made admin commands
  // server can't call these as they expect clientNum
  if (!ent) {
    return false;
  }
  game.timerunV2->printSeasons(ClientNum(ent));
  return true;
}

static bool listCheckpoints(gentity_t *ent, Arguments argv) {
  if (!ent) {
    return false;
  }

  const int32_t clientNum = ClientNum(ent);
  const std::string desc = R"(Print checkpoint times for a run.
    /listcheckpoints --season <season name> --map <map name> --run <run name> --rank <rank>

    Has a shorthand format of:
    /listcheckpoints <run name>
    /listcheckpoints <run name> <rank>
    /listcheckpoints <map name> <run name> <rank>
    /listcheckpoints <season name> <map name> <run name> <rank>)";

  const auto args = ETJump::Container::skipFirstN(*argv, 1);
  auto optCommand = ETJump::getOptCommand(
      "listcheckpoints", clientNum,
      ETJump::CommandParser::CommandDefinition::create("listcheckpoints", desc)
          .addOption("season", "s",
                     "Name of the season to list checkpoint times from. "
                     "Default is the overall season.",
                     ETJump::CommandParser::OptionDefinition::Type::MultiToken,
                     false)
          .addOption("map", "m",
                     "Name of the map to list checkpoints from. Default is the "
                     "current map.",
                     ETJump::CommandParser::OptionDefinition::Type::MultiToken,
                     false)
          .addOption(
              "run", "r", "Name of the run to list checkpoint times from.",
              ETJump::CommandParser::OptionDefinition::Type::MultiToken, false)
          .addOption(
              "rank", "rk",
              "Rank from which to list checkpoints from. Default is rank 1.",
              ETJump::CommandParser::OptionDefinition::Type::Integer, false),
      &args);

  if (!optCommand.has_value()) {
    return false;
  }

  const auto optSeason = optCommand.value().getOptional("season");
  const auto optMap = optCommand.value().getOptional("map");
  const auto optRun = optCommand.value().getOptional("run");
  const auto optRank = optCommand.value().getOptional("rank");

  std::string season;
  std::string map;
  std::string run;
  int32_t rank = 0;
  bool exactMap = false;

  if (!optCommand->extraArgs.empty()) {
    switch (optCommand->extraArgs.size()) {
      case 1:
        season = "Default";
        map = level.rawmapname;
        run = optCommand->extraArgs[0];
        rank = 1;
        break;
      case 2:
        season = "Default";
        map = level.rawmapname;
        run = optCommand->extraArgs[0];
        rank = Q_atoi(optCommand->extraArgs[1]);
        break;
      case 3:
        season = "Default";
        map = optCommand->extraArgs[0];
        run = optCommand->extraArgs[1];
        rank = Q_atoi(optCommand->extraArgs[2]);
        break;
      default: // >= 4
        season = optCommand->extraArgs[0];
        map = optCommand->extraArgs[1];
        run = optCommand->extraArgs[2];
        rank = Q_atoi(optCommand->extraArgs[3]);
    }
  }

  if (run.empty()) {
    // this is a bit awkward, but because we can use shorthand format
    // for specifying the run, we can't make it a required option,
    // so we can end up with a command with no run specified,
    // in which case mimic the error that an invalid command would print out
    // TODO: this should probably be a callable function in the command itself,
    // or we should make the parser use 'extraArgs' as the required option(s)
    if (!optRun.has_value()) {
      Printer::chat(clientNum, "^3listcheckpoints: ^7operation failed. Check "
                               "console for more information.");
      Printer::console(clientNum, "Required option `run` was not specified.\n");
      return false;
    }

    run = optRun.value().text;
  }

  if (season.empty()) {
    season = optSeason.has_value() ? optSeason.value().text : "Default";
  }

  if (map.empty()) {
    map = optMap.has_value() ? optMap.value().text : level.rawmapname;
    exactMap = !optMap.has_value();
  }

  if (rank == 0) {
    rank = optRank.has_value() ? optRank.value().integer : 1;
  }

  game.timerunV2->listCheckpoints(
      {clientNum, season, std::move(map), std::move(run), rank, exactMap});
  return true;
}

static bool compareCheckpoints(gentity_t *ent, Arguments argv) {
  if (!ent) {
    return false;
  }

  const int32_t clientNum = ClientNum(ent);
  const std::string desc = R"(Compares checkpoint times for a run.
    /comparecheckpoints --season <season name> --map <map name> --run <run name> --rank-1 <rank> --rank-2 <rank>

    Has a shorthand format of:
    /comparecheckpoints <run name> <rank-2>
    /comparecheckpoints <run name> <rank-1> <rank-2>)";

  const auto args = ETJump::Container::skipFirstN(*argv, 1);
  auto optCommand = ETJump::getOptCommand(
      "comparecheckpoints", clientNum,
      ETJump::CommandParser::CommandDefinition::create("comparecheckpoints",
                                                       desc)
          .addOption("season", "s",
                     "Name of the season to compare checkpoint times from. "
                     "Default is the overall season.",
                     ETJump::CommandParser::OptionDefinition::Type::MultiToken,
                     false)
          .addOption("map", "m",
                     "Name of the map to compare checkpoints from. Default is "
                     "the current map.",
                     ETJump::CommandParser::OptionDefinition::Type::MultiToken,
                     false)
          .addOption(
              "run", "r", "Name of the run to compare checkpoint times from.",
              ETJump::CommandParser::OptionDefinition::Type::MultiToken, false)
          .addOption("rank-1", "rk1",
                     "Rank to compare checkpoint times to. Default is rank 1.",
                     ETJump::CommandParser::OptionDefinition::Type::Integer,
                     false)
          .addOption("rank-2", "rk2",
                     "Rank to compare against the base checkpoint times.",
                     ETJump::CommandParser::OptionDefinition::Type::Integer,
                     false),
      &args);

  if (!optCommand.has_value()) {
    return false;
  }

  const auto optSeason = optCommand.value().getOptional("season");
  const auto optMap = optCommand.value().getOptional("map");
  const auto optRun = optCommand.value().getOptional("run");
  const auto optRankBase = optCommand.value().getOptional("rank-1");
  const auto optRankCmp = optCommand.value().getOptional("rank-2");

  std::string season;
  std::string map;
  std::string run;
  int32_t rankBase = 0;
  int32_t rankCmp = 0;
  bool exactMap = false;

  // shorthand commands need at least 2 arguments
  if (optCommand->extraArgs.size() > 1) {
    if (optCommand->extraArgs.size() == 2) {
      season = "Default";
      map = level.rawmapname;
      run = optCommand->extraArgs[0];
      rankBase = 1;
      rankCmp = Q_atoi(optCommand->extraArgs[1]);
    } else if (optCommand->extraArgs.size() >= 3) {
      season = "Default";
      map = level.rawmapname;
      run = optCommand->extraArgs[0];
      rankBase = Q_atoi(optCommand->extraArgs[1]);
      rankCmp = Q_atoi(optCommand->extraArgs[2]);
    }
  }

  // manual error handling like in 'listcheckpoints', more details there
  if (run.empty() || rankCmp == 0) {
    if (!optRun.has_value() || !optRankCmp.has_value()) {
      Printer::chat(clientNum,
                    "^3comparecheckpoints: ^7operation failed. Check "
                    "console for more information.");
      if (!optRun.has_value()) {
        Printer::console(clientNum,
                         "Required option `run` was not specified.\n");
      }

      if (!optRankCmp.has_value()) {
        Printer::console(clientNum,
                         "Required option `rank-2` was not specified.\n");
      }

      return false;
    }

    run = optRun.value().text;
    rankCmp = optRankCmp.value().integer;
  }

  if (season.empty()) {
    season = optSeason.has_value() ? optSeason.value().text : "Default";
  }

  if (map.empty()) {
    map = optMap.has_value() ? optMap.value().text : level.rawmapname;
    exactMap = !optMap.has_value();
  }

  if (rankBase == 0) {
    rankBase = optRankBase.has_value() ? optRankBase.value().integer : 1;
  }

  game.timerunV2->compareCheckpoints({clientNum, season, std::move(map),
                                      std::move(run), rankBase, rankCmp,
                                      exactMap});
  return true;
}

bool Records(gentity_t *ent, Arguments argv) {
  // these are console commands but to make them more accessible
  // they were also made admin commands
  // server can't call these as they expect clientNum
  if (!ent) {
    return false;
  }
  auto args = ETJump::Container::skipFirstN(*argv, 1);
  auto optCommand = getOptCommand(
      "records", ClientNum(ent),
      ETJump::CommandParser::CommandDefinition::create(
          "records",
          "Print the timerun records.\n    /records --season <season name> "
          "--map <map name> --run <run name>\n\n    Has a shorthand format "
          "of:\n    /records <run name>\n    /records <map name> <run "
          "name>\n  "
          "  /records <season name> <map name> <run name>")
          .addOption("season", "s",
                     "Name of the season to print the records for. Default is "
                     "the overall season.",
                     ETJump::CommandParser::OptionDefinition::Type::MultiToken,
                     false)
          .addOption("map", "m",
                     "Name of the map to print the records for. Default is the "
                     "current map.",
                     ETJump::CommandParser::OptionDefinition::Type::MultiToken,
                     false)
          .addOption(
              "run", "r",
              "Name of the run to print the records for. Default will print "
              "top 3 records for all runs on specified map and your record.",
              ETJump::CommandParser::OptionDefinition::Type::MultiToken, false)
          .addOption("page", "p", "Which page to display starting at 1",
                     ETJump::CommandParser::OptionDefinition::Type::Integer,
                     false)
          .addOption("page-size", "ps",
                     "How many records to show on a single page. Max page size "
                     "is 100 if a run is specified, otherwise 10.",
                     ETJump::CommandParser::OptionDefinition::Type::Integer,
                     false),
      &args);

  if (!optCommand.has_value()) {
    return true;
  }

  auto command = optCommand.value();

  auto optSeason = command.getOptional("season");
  auto optMap = command.getOptional("map");
  auto optRun = command.getOptional("run");
  auto optPage = command.getOptional("page");
  auto optPageSize = command.getOptional("page-size");

  std::string season;
  std::string map;
  std::optional<std::string> run;

  if (command.extraArgs.size() >= 3) {
    season = command.extraArgs[0];
    map = command.extraArgs[1];
    run = command.extraArgs[2];
  } else if (command.extraArgs.size() >= 2) {
    map = command.extraArgs[0];
    run = command.extraArgs[1];
  } else if (!command.extraArgs.empty()) {
    run = command.extraArgs[0];
  }

  if (season.empty()) {
    season = optSeason.has_value() ? optSeason.value().text : "Default";
  }
  bool exactMap{};
  if (map.empty()) {
    map = optMap.has_value() ? optMap.value().text : level.rawmapname;
    exactMap = !optMap.has_value();
  } else {
    exactMap = false;
  }
  if (!run.has_value()) {
    run = optRun.has_value()
              ? std::make_optional<std::string>(optRun.value().text)
              : std::nullopt;
  }

  ETJump::Timerun::PrintRecordsParams params;
  params.clientNum = ClientNum(ent);
  params.season = std::move(season);
  params.map = std::move(map);
  // use exact map search if user did not specify the map
  params.exactMap = exactMap;
  params.run = std::move(run);
  params.page = optPage.has_value() ? std::max(optPage.value().integer, 1) : 1;
  if (!params.run.has_value()) {
    params.pageSize = optPageSize.has_value()
                          ? std::clamp(optPageSize.value().integer, 1,
                                       ETJump::Timerun::MAX_PAGE_SIZE_ALL_RUNS)
                          : ETJump::Timerun::DEFAULT_PAGE_SIZE_ALL_RUNS;
  } else {
    params.pageSize =
        optPageSize.has_value()
            ? std::clamp(optPageSize.value().integer, 1,
                         ETJump::Timerun::MAX_PAGE_SIZE_SINGLE_RUN)
            : ETJump::Timerun::DEFAULT_PAGE_SIZE_SINGLE_RUN;
  }
  params.userId = ETJump::session->GetId(ent);

  game.timerunV2->printRecords(params);

  return true;
}

bool LoadCheckpoints(gentity_t *ent, Arguments argv) {
  // these are console commands but to make them more accessible
  // they were also made admin commands
  // server can't call these as they expect clientNum
  if (!ent) {
    return false;
  }
  auto args = ETJump::Container::skipFirstN(*argv, 1);
  auto optCommand = getOptCommand(
      "loadcheckpoints", ClientNum(ent),
      ETJump::CommandParser::CommandDefinition::create(
          "loadcheckpoints",
          "Load checkpoints from an existing record.\n    /loadcheckpoints "
          "--run <run name> --rank <rank>\n\n"
          "    Has a shorthand format of:\n"
          "    /loadcheckpoints <run name> <rank>\n"
          "    /loadcheckpoints <run name>")
          .addOption("run", "r", "Name of the run to load the records from.",
                     ETJump::CommandParser::OptionDefinition::Type::Token, true,
                     0)
          .addOption("rank", "rk",
                     "Rank to load checkpoints from. Defaults to 1. Value -1 "
                     "clears loaded checkpoints.",
                     ETJump::CommandParser::OptionDefinition::Type::Integer,
                     false, 1),
      &args);

  if (!optCommand.has_value()) {
    return true;
  }

  const auto &runName = optCommand.value().options.at("run").text;
  auto optRank = optCommand.value().getOptional("rank");
  auto rank = optRank.has_value() ? optRank.value().integer : 1;

  game.timerunV2->loadCheckpoints(ClientNum(ent), level.rawmapname, runName,
                                  rank);

  return true;
}

bool GetChatReplay(gentity_t *ent, Arguments argv) {
  game.chatReplay->sendChatMessages(ent);
  return true;
}

static bool sendMaplist(gentity_t *ent, Arguments argv) {
  std::string mapList =
      ETJump::StringUtil::join(game.mapStatistics->getMaps(), " ");
  const std::string prefix = "maplist ";
  const size_t msgLen = BYTES_PER_PACKET - prefix.length() - 1;

  // split to multiple commands to ensure client gets the full list
  // each command is prefixed with 'maplist' so client recognizes
  // this command is part of the map list, and parses it correctly
  auto splits = ETJump::wrapWords(mapList, ' ', msgLen);
  for (auto &split : splits) {
    split.insert(0, prefix);
    trap_SendServerCommand(ClientNum(ent), std::string(split + '\n').c_str());
  }

  return true;
}

static bool sendNumCustomvotes(gentity_t *ent, Arguments argv) {
  const auto numlists =
      static_cast<int>(game.customMapVotes->getNumVotelists());
  trap_SendServerCommand(ClientNum(ent), va("numcustomvotes %i\n", numlists));
  return true;
}

static bool sendCustomvoteInfo(gentity_t *ent, Arguments argv) {
  if (argv->size() < 2) {
    Printer::console(
        ent, ETJump::stringFormat("^3%s: ^7no list given as an argument.\n",
                                  __func__));
    return false;
  }

  const int index = Q_atoi(argv->at(1).c_str());
  const auto list = game.customMapVotes->getVotelistByIndex(index);

  if (list == nullptr) {
    Printer::console(ent, ETJump::stringFormat(
                              "^3%s: ^7no list with a given index ^3'%i'^7.\n",
                              __func__, index));
    return false;
  }

  const std::string prefix = "customvotelist";
  const int clientNum = ClientNum(ent);

  /* The commands sent to client consist of following fields:
   *
   * cmd <type> <field> <data>
   *
   * Each command includes the list type as the first argument, so client
   * knows which list the data in this command belongs to. This way we can
   * ensure the client can construct the complete struct for any given list,
   * regardless of the order in which the packets arrive in. They *should*
   * arrive sequentially in the order we send them, this is just extra
   * guarantee.
   */

  const std::string cmd = ETJump::stringFormat("%s \"%s\"", prefix, list->type);

  // we could omit the type from the end here, but this simplifies parsing
  const std::string typeCmd = ETJump::stringFormat(
      "%s %s \"%s\"\n", cmd, ETJump::CUSTOMVOTE_TYPE, list->type);
  const std::string cvTextCmd = ETJump::stringFormat(
      "%s %s \"%s\"\n", cmd, ETJump::CUSTOMVOTE_CVTEXT, list->callvoteText);

  trap_SendServerCommand(clientNum, typeCmd.c_str());
  trap_SendServerCommand(clientNum, cvTextCmd.c_str());

  const std::string serverMapsCmd =
      ETJump::stringFormat("%s %s ", cmd, ETJump::CUSTOMVOTE_SERVERMAPS);
  const std::string otherMapsCmd =
      ETJump::stringFormat("%s %s ", cmd, ETJump::CUSTOMVOTE_OTHERMAPS);

  const std::string serverMaps =
      ETJump::StringUtil::join(list->mapsOnServer, " ");
  const std::string otherMaps = ETJump::StringUtil::join(list->otherMaps, " ");

  // we have to check if the combined string would be over max message length,
  // because we need to always attach the command prefix in front
  if (serverMapsCmd.length() + serverMaps.length() > BYTES_PER_PACKET - 1) {
    const auto splits = ETJump::wrapWords(
        serverMaps, ' ', BYTES_PER_PACKET - serverMapsCmd.length() - 1);

    for (const auto &split : splits) {
      trap_SendServerCommand(clientNum, (serverMapsCmd + split + '\n').c_str());
    }
  } else {
    trap_SendServerCommand(clientNum,
                           (serverMapsCmd + serverMaps + '\n').c_str());
  }

  if (otherMapsCmd.length() + otherMaps.length() > BYTES_PER_PACKET) {
    const auto splits = ETJump::wrapWords(
        otherMaps, ' ', BYTES_PER_PACKET - otherMapsCmd.length() - 1);

    for (const auto &split : splits) {
      trap_SendServerCommand(clientNum, (otherMapsCmd + split + '\n').c_str());
    }
  } else {
    trap_SendServerCommand(clientNum,
                           (otherMapsCmd + otherMaps + '\n').c_str());
  }

  return true;
}

static bool loadPos(gentity_t *ent, Arguments argv) {
  if (argv->size() < 2) {
    Printer::center(ent, "Invalid ^3loadpos ^7command");
    return false;
  }

  if (!g_cheats.integer) {
    Printer::center(ent, "^3loadpos ^7cannot be used without cheats");
    return false;
  }

  if (ent->client->sess.sessionTeam == TEAM_SPECTATOR) {
    Printer::center(ent, "^3loadpos ^7cannot be used while spectating");
    return false;
  }

  ETJump::SavePosHandler::execSaveposCommand(
      ent, ETJump::Container::skipFirstN(*argv, 1));
  return true;
}
} // namespace ClientCommands

void PrintManual(gentity_t *ent, const std::string &command) {
  if (ent) {
    Printer::chat(ent, va("^3%s: ^7check console for more information.",
                          command.c_str()));
    trap_SendServerCommand(ClientNum(ent), va("manual %s", command.c_str()));
  } else {
    int i = 0;
    int len = sizeof(commandManuals) / sizeof(commandManuals[0]);
    for (i = 0; i < len; i++) {
      if (!Q_stricmp(commandManuals[i].cmd, command.c_str())) {
        G_Printf("%s\n\nUsage:\n%s\n\nDescription:\n%s\n",
                 commandManuals[i].cmd, commandManuals[i].usage,
                 commandManuals[i].description);
        return;
      }
    }
    if (i == len) {
      G_Printf("Couldn't find manual for command \"%s\"\n", command.c_str());
    }
  }
}

bool IsTargetHigherLevel(gentity_t *ent, gentity_t *target,
                         bool equalIsHigher) {
  if (!ent) {
    return false;
  }

  if (equalIsHigher) {
    return ETJump::session->GetLevel(ent) <= ETJump::session->GetLevel(target);
  }

  return ETJump::session->GetLevel(ent) < ETJump::session->GetLevel(target);
}

bool IsTargetHigherLevel(gentity_t *ent, unsigned id, bool equalIsHigher) {
  if (equalIsHigher) {
    return ETJump::session->GetLevel(ent) <= ETJump::session->GetLevelById(id);
  }

  return ETJump::session->GetLevel(ent) < ETJump::session->GetLevelById(id);
}

namespace AdminCommands {
const int CMDS_OPEN = 1;
const int GREETING_OPEN = 2;
const int TITLE_OPEN = 4;

namespace Updated {
/*
const unsigned NONE      = 0;
const unsigned LEVEL     = 0x00001;
const unsigned LAST_SEEN = 0x00002;
const unsigned NAME      = 0x00004;
*/
const unsigned TITLE = 0x00008;
const unsigned COMMANDS = 0x00010;
const unsigned GREETING = 0x00020;
} // namespace Updated

bool Admintest(gentity_t *ent, Arguments argv) {
  if (!ent) {
    Printer::chatAll("^3admintest: ^7console is a level ? user.");
    return true;
  }

  ETJump::session->PrintAdmintest(ent);
  return true;
}

bool AddLevel(gentity_t *ent, Arguments argv) {
  // !addlevel [level] -cmds [commands] -greeting [greeting] -title [title]
  if (argv->size() < 2) {
    PrintManual(ent, "addlevel");
    return false;
  }
  int open = 0;

  int level = 0;
  std::string commands;
  std::string greeting;
  std::string title;

  if (!ToInt(argv->at(1), level)) {
    Printer::chat(
        ent, ETJump::stringFormat("^3addlevel: ^7'%s^7' is not an integer.",
                                  argv->at(1)));
    return false;
  }

  if (argv->size() > 2) {
    auto it = argv->begin() + 2;

    while (it != argv->end()) {
      if (*it == "-cmds" && it + 1 != argv->end()) {
        open = CMDS_OPEN;
      } else if (*it == "-greeting" && it + 1 != argv->end()) {
        open = GREETING_OPEN;
      } else if (*it == "-title" && it + 1 != argv->end()) {
        open = TITLE_OPEN;
      } else {
        switch (open) {
          case 0:
            Printer::chat(
                ent, va("^3addlevel: ^7ignored argument '%s^7'.", it->c_str()));
            break;
          case CMDS_OPEN:
            commands += *it;
            break;
          case GREETING_OPEN:
            greeting += *it + " ";
            break;
          case TITLE_OPEN:
            title += *it + " ";
            break;
          default:
            break;
        }
      }

      it++;
    }

    greeting = ETJump::trimEnd(greeting);
    title = ETJump::trimEnd(title);
  }

  if (!game.levels->Add(level, std::move(title), std::move(commands),
                        std::move(greeting))) {
    Printer::chat(ent, "^3addlevel: ^7" + game.levels->ErrorMessage());
    return false;
  }

  Printer::chat(ent, va("^3addlevel: ^7added level %d.", level));

  return true;
}

bool Ball8(gentity_t *ent, Arguments argv) {
  static const std::string Magical8BallResponses[] = {
      "It is certain",
      "It is decidedly so",
      "Without a doubt",
      "Yes - definitely",
      "You may rely on it",

      "As I see it, yes",
      "Most likely",
      "Outlook good",
      "Signs point to yes",
      "Yes",

      "Reply hazy, try again",
      "Ask again later",
      "Better not tell you now",
      "Cannot predict now",
      "Concentrate and ask again",

      "Don't count on it",
      "My reply is no",
      "My sources say no",
      "Outlook not so good",
      "Very doubtful"};

  constexpr int DELAY_8BALL = 3000; // in milliseconds

  if (ent && ent->client->last8BallTime + DELAY_8BALL > level.time) {
    const int remainingSeconds = std::ceil(
        (ent->client->last8BallTime + DELAY_8BALL - level.time) / 1000.0);
    Printer::chat(ent, "^3!8ball: ^7you must wait " +
                           ETJump::getSecondsString(remainingSeconds) +
                           " before using !8ball again.");
    return false;
  }

  if (argv->size() == 1) {
    PrintManual(ent, "8ball");
    return false;
  }

  int random = rand() % 20;
  const int POSITIVE = 10;
  const int NO_IDEA = 15;
  std::string msg;

  if (random < POSITIVE) {
    msg += "^3Magical 8 Ball: ^2" + Magical8BallResponses[random];
  } else if (random < NO_IDEA) {
    msg += "^3Magical 8 Ball: ^3" + Magical8BallResponses[random];
  } else {
    msg += "^3Magical 8 Ball: ^1" + Magical8BallResponses[random];
  }

  Printer::chatAll(msg);

  if (ent) {
    ent->client->last8BallTime = level.time;
  }
  return true;
}

bool Ban(gentity_t *ent, Arguments argv) {
  if (argv->size() == 1) {
    PrintManual(ent, "ban");
    return false;
  }
  // ban permanently
  time_t t;
  time(&t);
  std::string err;
  gentity_t *player = PlayerGentityFromString(argv->at(1).c_str(), err);

  if (!player) {
    Printer::chat(ent, "^3ban: ^7no player with name " + argv->at(1));
    return false;
  }

  if (IsTargetHigherLevel(ent, player, true)) {
    Printer::chat(ent, "^3ban: ^7you cannot ban a fellow admin.");
    return false;
  }

  unsigned expires = 0;
  std::string reason = "Banned by admin.";

  // !ban <name> <time> <reason>
  if (argv->size() >= 3) {
    if (!ToUnsigned(argv->at(2), expires)) {
      // TODO: 10m, 2h etc.
      Printer::chat(ent, "^3ban: ^7time was not a number.");
      return false;
    }

    if (expires != 0) {
      expires = static_cast<unsigned>(t) + expires;
    }
  }

  if (argv->size() >= 4) {
    reason = "";
    for (size_t i = 3; i < argv->size(); i++) {
      if (i + 1 == argv->size()) {
        reason += argv->at(i);
      } else {
        reason += argv->at(i) + " ";
      }
    }
  }
  if (!ETJump::session->Ban(ent, player, expires, std::move(reason))) {
    Printer::chat(ent, "^3ban: ^7" + ETJump::session->GetMessage());
    return false;
  }

  trap_DropClient(ClientNum(player), "You are banned", 0);
  return true;
}

bool Cancelvote(gentity_t *ent, Arguments argv) {
  if (level.voteInfo.voteTime) {
    level.voteInfo.voteCanceled = qtrue;
    Printer::chatAll("^3cancelvote: ^7vote has been canceled.");
  } else {
    Printer::chat(ent, "^3cancelvote: ^7no vote in progress.");
  }
  return true;
}

bool DeleteLevel(gentity_t *ent, Arguments argv) {
  if (argv->size() != 2) {
    PrintManual(ent, "deletelevel");
    return false;
  }

  int level = 0;
  if (!ToInt(argv->at(1), level)) {
    Printer::chat(ent, va("^3deletelevel: ^7'%s^7' is not an integer.",
                          argv->at(1).c_str()));
    return false;
  }

  if (!game.levels->Delete(level)) {
    Printer::chat(ent, "^3deletelevel: ^7couldn't find level.");
    return false;
  }

  int usersWithLevel = ETJump::session->LevelDeleted(level);

  Printer::chat(ent, "^3deletelevel: ^7deleted level. Set " +
                         ETJump::getPluralizedString(usersWithLevel, "user") +
                         " to level 0.");
  return true;
}

bool EditCommands(gentity_t *ent, Arguments argv) {
  // !editcommands level +command|-command +command|-command etc.
  if (argv->size() < 3) {
    PrintManual(ent, "editcommands");
    return false;
  }

  int level = 0;
  if (!ToInt(argv->at(1), level)) {
    Printer::chat(ent, "^3editcommands: ^7defined level '" + (*argv)[1] +
                           "' is not an integer.");
    return false;
  }

  if (!game.levels->LevelExists(level)) {
    Printer::chat(ent,
                  "^3editcommands: ^7level " + (*argv)[1] + " does not exist.");
    return false;
  }

  auto it = argv->begin() + 2;
  auto end = argv->end();

  std::string currentPermissions = game.levels->GetLevel(level)->commands;

  bool add = true;
  std::string currentCommand;
  std::string addCommands = "+";
  std::string deleteCommands = "-";
  for (; it != end; it++) {
    if ((*it)[0] == '-') {
      add = false;
      currentCommand = (*it).substr(1);
    } else if ((*it)[0] == '+') {
      add = true;
      currentCommand = (*it).substr(1);
    } else {
      add = true;
      currentCommand = (*it);
    }
    char flag = game.commands->FindCommandFlag(currentCommand);
    if (flag == 0) {
      Printer::chat(ent, "^3editcommands: ^7command '" + currentCommand +
                             "' doesn't match any known command.");
      continue;
    }
    if (add) {
      addCommands += flag;
    } else {
      deleteCommands += flag;
    }
  }

  std::string duplicateFlags;
  for (char addCommand : addCommands) {
    if (deleteCommands.find(addCommand) != std::string::npos) {
      Printer::chat(ent, va("^3editcommands: ^7ignoring command flag '%c'. Are "
                            "you trying to add or delete it?",
                            addCommand));
      duplicateFlags.push_back(addCommand);
    }
  }

  if (duplicateFlags.length() > 0) {
    std::string temp;
    for (char addCommand : addCommands) {
      if (duplicateFlags.find(addCommand) == std::string::npos) {
        temp += addCommand;
      }
    }
    addCommands = temp;

    temp.clear();
    for (char deleteCommand : deleteCommands) {
      if (duplicateFlags.find(deleteCommand) == std::string::npos) {
        temp += deleteCommand;
      }
    }
    deleteCommands = std::move(temp);
  }

  // always has + in it
  if (addCommands.length() > 1) {
    currentPermissions += addCommands;
  }
  // always has - in it
  if (deleteCommands.length() > 1) {
    currentPermissions += deleteCommands;
  }
  game.levels->Edit(level, "", currentPermissions, "", 1);

  Printer::chat(ent, "^3editcommands: ^7edited level " + (*argv)[1] +
                         " permissions. New permissions are: " +
                         game.levels->GetLevel(level)->commands);
  return true;
}

bool EditLevel(gentity_t *ent, Arguments argv) {
  if (argv->size() < 4) {
    PrintManual(ent, "editlevel");
    return false;
  }

  int updated = 0;
  int open = 0;

  int adminLevel = 0;
  std::string commands;
  std::string greeting;
  std::string title;

  if (!ToInt(argv->at(1), adminLevel)) {
    Printer::chat(
        ent, ETJump::stringFormat("^3editlevel: ^7'%s^7' is not an integer.",
                                  argv->at(1)));
    return false;
  }

  if (argv->size() > 2) {
    ConstArgIter it = argv->begin() + 2;

    while (it != argv->end()) {
      if (*it == "-cmds" && it + 1 != argv->end()) {
        open = CMDS_OPEN;
        updated |= CMDS_OPEN;
      } else if (*it == "-greeting" && it + 1 != argv->end()) {
        open = GREETING_OPEN;
        updated |= GREETING_OPEN;
      } else if (*it == "-title" && it + 1 != argv->end()) {
        open = TITLE_OPEN;
        updated |= TITLE_OPEN;
      } else if (*it == "-clear" && it + 1 != argv->end()) {
        ConstArgIter nextIt = it + 1;
        if (*nextIt == "cmds") {
          commands = "";
          updated |= CMDS_OPEN;

        } else if (*nextIt == "greeting") {
          greeting = "";
          updated |= GREETING_OPEN;
        } else if (*nextIt == "title") {
          title = "";
          updated |= TITLE_OPEN;
        } else {
          it++;
        }
      } else {
        switch (open) {
          case 0:
            if (updated == 0) {
              Printer::chat(ent, va("^3editlevel: ^7ignored argument '%s^7'.",
                                    it->c_str()));
            }

            break;
          case CMDS_OPEN:
            commands += *it;
            break;
          case GREETING_OPEN:
            greeting += *it + " ";
            break;
          case TITLE_OPEN:
            title += *it + " ";
            break;
          default:
            break;
        }
      }

      it++;
    }

    greeting = ETJump::trimEnd(greeting);
    title = ETJump::trimEnd(title);
  }

  game.levels->Edit(adminLevel, title, commands, greeting, updated);

  for (int i = 0; i < level.numConnectedClients; i++) {
    int num = level.sortedClients[i];

    ETJump::session->ParsePermissions(num);
  }

  Printer::chat(ent, va("^3editlevel: ^7updated level %d.", adminLevel));
  return true;
}

bool EditUser(gentity_t *ent, Arguments argv) {
  if (argv->size() < 4) {
    PrintManual(ent, "edituser");
    return false;
  }

  unsigned id = 0;
  if (!ToUnsigned(argv->at(1), id)) {
    Printer::chat(ent, "^3edituser: ^7invalid id " + argv->at(1));
    return false;
  }

  if (!ETJump::database->UserExists(id)) {
    Printer::chat(ent, "^3edituser: ^7user does not exist.");
    return false;
  }

  int updated = 0;
  int open = 0;

  std::string commands;
  std::string greeting;
  std::string title;

  ConstArgIter it = argv->begin() + 2;
  while (it != argv->end()) {
    if (*it == "-cmds" && it + 1 != argv->end()) {
      open = CMDS_OPEN;
      updated |= Updated::COMMANDS;
    } else if (*it == "-greeting" && it + 1 != argv->end()) {
      open = GREETING_OPEN;
      updated |= Updated::GREETING;
    } else if (*it == "-title" && it + 1 != argv->end()) {
      open = TITLE_OPEN;
      updated |= Updated::TITLE;
    } else if (*it == "-clear" && it + 1 != argv->end()) {
      ConstArgIter nextIt = it + 1;
      if (*nextIt == "cmds") {
        commands = "";
        updated |= Updated::COMMANDS;
      } else if (*nextIt == "greeting") {
        greeting = "";
        updated |= Updated::GREETING;
      } else if (*nextIt == "title") {
        title = "";
        updated |= Updated::TITLE;
      }
      it++;
    } else {
      switch (open) {
        case 0:
          Printer::chat(
              ent, va("^3edituser: ^7ignored argument '%s^7'.", it->c_str()));
          break;
        case CMDS_OPEN:
          commands += *it;
          break;
        case GREETING_OPEN:
          greeting += *it + " ";
          break;
        case TITLE_OPEN:
          title += *it + " ";
          break;
        default:
          break;
      }
    }

    it++;
  }

  greeting = ETJump::trimEnd(greeting);
  title = ETJump::trimEnd(title);

  Printer::chat(ent, va("^3edituser: ^7updating user %d", id));
  return ETJump::database->UpdateUser(ent, id, commands, greeting, title,
                                      updated);
}

bool FindUser(gentity_t *ent, Arguments argv) {
  if (argv->size() != 2) {
    Printer::chat(ent, "^3usage: ^7!finduser <name>");
    return false;
  }

  ETJump::database->FindUser(ent, argv->at(1));

  return true;
}

bool FindMap(gentity_t *ent, Arguments argv) {
  if (argv->size() < 2) {
    Printer::chat(ent, "^3usage: ^7!findmap <map> [maps per row]");
    return false;
  }

  auto perRow = 3;
  if (argv->size() == 3) {
    try {
      perRow = std::stoi(argv->at(2));
    } catch (...) {
      perRow = 3;
    }

    if (perRow <= 0) {
      Printer::chat(ent, "^3findmap: ^7third argument must be over 0");
      return false;
    }

    if (perRow > 5) {
      perRow = 5;
    }
  }

  auto maps = game.mapStatistics->getMaps();
  std::sort(maps.begin(), maps.end());
  std::vector<std::string> matching;
  for (auto &map : maps) {
    if (map.find(argv->at(1)) != std::string::npos) {
      matching.push_back(map);
    }
  }

  auto mapsOnCurrentRow = 0;
  std::string buffer = "^zFound ^3" +
                       ETJump::getPluralizedString(matching.size(), "^zmap") +
                       " matching ^3" + argv->at(1) + "^z:\n^7";
  for (auto &map : matching) {
    // color every other column grey for readability
    buffer += mapsOnCurrentRow % 2 == 0 ? "^7" : "^z";

    ++mapsOnCurrentRow;
    if (mapsOnCurrentRow > perRow) {
      mapsOnCurrentRow = 1;
      buffer += ETJump::stringFormat("\n^7%-23s", map);
    } else {
      buffer += ETJump::stringFormat("%-23s", map);
    }
  }

  buffer += "\n";

  Printer::console(ClientNum(ent), buffer);
  return true;
}

bool ListUserNames(gentity_t *ent, Arguments argv) {
  if (argv->size() != 2) {
    Printer::chat(ent, "^3usage: ^7!listusernames <id>");
    return false;
  }

  int id;
  if (!ToInt(argv->at(1), id)) {
    Printer::chat(
        ent, va("^3listusernames: ^7%s is not an id", argv->at(1).c_str()));
    return false;
  }

  ETJump::database->ListUserNames(ent, id);
  return true;
}

bool AdminCommands(gentity_t *ent, Arguments argv) {
  Printer::chat(ent, "AdminCommands is not implemented.");
  return true;
}

bool Finger(gentity_t *ent, Arguments argv) {
  if (argv->size() != 2) {
    PrintManual(ent, "finger");
    return false;
  }

  std::string err;
  gentity_t *target = PlayerGentityFromString(argv->at(1), err);
  if (!target) {
    Printer::chat(ent, "^3finger: ^7" + err);
    return false;
  }

  ETJump::session->PrintFinger(ent, target);
  return true;
}

bool Help(gentity_t *ent, Arguments argv) {
  if (argv->size() == 1) {
    game.commands->List(ent);
  } else {
    PrintManual(ent, argv->at(1));
  }

  return true;
}

bool Kick(gentity_t *ent, Arguments argv) {
  const unsigned MIN_ARGS = 2;
  if (argv->size() < MIN_ARGS) {
    PrintManual(ent, "kick");
    return false;
  }

  std::string error;
  gentity_t *target = PlayerGentityFromString(argv->at(1), error);
  if (!target) {
    Printer::chat(ent, "^3kick: " + error);
    return false;
  }

  if (ent) {
    if (ent == target) {
      Printer::chat(ent, "^3kick: ^7you can't kick yourself.");
      return false;
    }

    if (IsTargetHigherLevel(ent, target, true)) {
      Printer::chat(ent, "^3kick: ^7you can't kick a fellow admin.");
      return false;
    }
  }

  int timeout = 0;
  if (argv->size() >= 3) {
    if (!ToInt(argv->at(2), timeout)) {
      Printer::chat(ent, "^3kick: ^7invalid timeout '" + argv->at(2) +
                             "' specified.");
      return false;
    }
  }

  std::string reason;
  if (argv->size() >= 4) {
    reason = argv->at(3);
  }

  trap_DropClient(ClientNum(target), reason.c_str(), timeout);
  return true;
}

std::string playedTimeFmtString(int seconds) {
  auto minutes = seconds / 60;
  seconds -= minutes * 60;
  auto hours = minutes / 60;
  minutes -= hours * 60;
  auto days = hours / 24;
  hours -= days * 24;
  auto weeks = days / 7;
  days -= weeks * 7;

  std::string str;
  if (weeks) {
    str = ETJump::getWeeksString(weeks);
  } else if (days) {
    str = ETJump::getDaysString(days);
  } else if (hours || minutes || seconds) {
    str = ETJump::getHoursString(hours) + " " +
          ETJump::getMinutesString(minutes) + " " +
          ETJump::getSecondsString(seconds);
  }

  if (str.length() == 0) {
    str = "never";
  }
  return str;
}

bool LeastPlayed(gentity_t *ent, Arguments argv) {
  auto mapsToList = 10;
  if (argv->size() == 2) {
    try {
      mapsToList = std::stoi(argv->at(1), nullptr, 10);
    } catch (const std::invalid_argument &) {
      Printer::chat(ent, ETJump::stringFormat(
                             "^3Error: ^7'%s^7' is not a number", argv->at(1)));
      return false;
    } catch (const std::out_of_range &) {
      mapsToList = 10;
    }

    if (mapsToList <= 0) {
      Printer::chat(ent, "^3leastplayed: ^7second argument must be over 0");
      return false;
    }

    if (mapsToList > 100) {
      mapsToList = 100;
    }
  }

  auto leastPlayed = game.mapStatistics->getLeastPlayed();
  auto listedMaps = 0;
  std::string buffer = "^zLeast played maps are:\n"
                       "^gMap                    Played                        "
                       " Last played       Times played\n";
  auto green = false;
  for (auto &map : leastPlayed) {
    if (listedMaps >= mapsToList) {
      break;
    }

    buffer += green ? "^g" : "^7";
    buffer += ETJump::stringFormat(
        "%-22s %-30s %-17s     %d\n", map->name,
        playedTimeFmtString(map->secondsPlayed),
        Utilities::timestampToString(map->lastPlayed), map->timesPlayed);
    green = !green;

    ++listedMaps;
  }

  Printer::console(ClientNum(ent), buffer);
  return true;
}

bool LevelInfo(gentity_t *ent, Arguments argv) {
  if (argv->size() == 1) {
    game.levels->PrintLevelInfo(ent);
  } else {
    int level = 0;
    if (!ToInt(argv->at(1), level)) {
      Printer::chat(ent, "^3levelinfo: ^7invalid level.");
      return false;
    }

    game.levels->PrintLevelInfo(ent, level);
  }
  return true;
}

bool ListBans(gentity_t *ent, Arguments argv) {
  int page = 1;

  if (argv->size() > 1) {
    if (!ToInt(argv->at(1), page)) {
      Printer::chat(ent, "^3listbans: ^7page is not a number.");
      return false;
    }
  }

  if (page < 1)
    page = 1;

  ETJump::database->ListBans(ent, page);

  return true;
}

bool ListFlags(gentity_t *ent, Arguments argv) {
  game.commands->ListCommandFlags(ent);
  return true;
}

bool ListMaps(gentity_t *ent, Arguments argv) {
  auto perRow = 5;

  if (argv->size() == 2) {
    try {
      perRow = std::stoi(argv->at(1), nullptr, 10);
    } catch (const std::invalid_argument &) {
      Printer::chat(ent,
                    ETJump::stringFormat("^3listmaps: ^7'%s^7' is not a number",
                                         argv->at(1)));
      return false;
    } catch (const std::out_of_range &) {
      perRow = 5;
    }

    if (perRow <= 0) {
      Printer::chat(ent, "^3listmaps: ^7second argument must be over 0");
      return false;
    }

    if (perRow > 5) {
      perRow = 5;
    }
  }

  std::string buffer = "^zListing all maps on server:\n^7";
  auto maps = game.mapStatistics->getMaps();
  std::sort(maps.begin(), maps.end());
  auto mapsOnCurrentRow = 0;
  for (auto &map : maps) {
    // color every other column grey for readability
    buffer += mapsOnCurrentRow % 2 == 0 ? "^7" : "^z";

    ++mapsOnCurrentRow;
    if (mapsOnCurrentRow > perRow) {
      mapsOnCurrentRow = 1;
      buffer += ETJump::stringFormat("\n^7%-23s", map);
    } else {
      buffer += ETJump::stringFormat("%-23s", map);
    }
  }

  buffer += "\n";

  buffer += "\n^zFound ^3" + ETJump::getPluralizedString(maps.size(), "^zmap") +
            " on the server.\n";

  Printer::console(ent, buffer);
  return true;
}

bool ListUsers(gentity_t *ent, Arguments argv) {
  int page = 1;

  if (argv->size() > 1) {
    if (!ToInt(argv->at(1), page)) {
      Printer::chat(ent, "^3listusers: ^7page was not a number.");
      return false;
    }
  }

  ETJump::database->ListUsers(ent, page);
  return true;
}

bool ListPlayers(gentity_t *ent, Arguments argv) {
  if (argv->size() != 1) {
    return false;
  }

  if (!level.numConnectedClients) {
    Printer::console(ent, "There are currently no connected players.\n");
    return true;
  }

  if (level.numConnectedClients == 1) {
    Printer::console(ent, "^7There is currently 1 connected player.\n");
  } else {
    Printer::console(ent, ETJump::stringFormat(
                              "^7There are currently %d connected players.\n",
                              level.numConnectedClients));
  }

  std::string msg = "#  ETJumpID  Level  Player\n";

  for (int i = 0; i < level.numConnectedClients; i++) {
    const int clientNum = level.sortedClients[i];
    gentity_t *player = g_entities + clientNum;
    const int id = ETJump::session->GetId(player);

    msg += ETJump::stringFormat("^7%-2d %-9s %-6d %-s\n", clientNum,
                                id == -1 ? "-" : std::to_string(id),
                                ETJump::session->GetLevel(player),
                                player->client->pers.netname);
  }

  Printer::console(ent, msg);
  return true;
}

bool Map(gentity_t *ent, Arguments argv) {
  if (argv->size() != 2) {
    // PrintManual(ent, "map");
    return false;
  }

  std::string requestedMap = ETJump::StringUtil::toLowerCase(argv->at(1));

  if (!ETJump::FileSystem::exists("maps/" + requestedMap + ".bsp")) {
    Printer::chat(ent, "^3map: ^7'" + requestedMap + "' is not on the server.");
    return false;
  }

  if (ETJump::MapStatistics::isBlockedMap(requestedMap)) {
    Printer::chat(ent, "^3map: ^7'" + requestedMap +
                           "' cannot be played on this server.");
    return false;
  }

  trap_SendConsoleCommand(EXEC_APPEND, va("map %s\n", requestedMap.c_str()));
  return true;
}

bool MapInfo(gentity_t *ent, Arguments argv) {
  auto mi = game.mapStatistics->getMapInformation(
      argv->size() > 1 ? argv->at(1) : level.rawmapname);
  auto currentMap = game.mapStatistics->getCurrentMap();

  if (mi == nullptr) {
    Printer::chat(ent, "^3mapinfo: ^7Could not find the map");
    return false;
  }

  int seconds = mi->secondsPlayed;
  int minutes = seconds / 60;
  seconds = seconds - minutes * 60;
  int hours = minutes / 60;
  minutes = minutes - hours * 60;
  int days = hours / 24;
  hours = hours - days * 24;

  std::string message;
  if (mi == currentMap) {
    message = "^3mapinfo: ^7" + mi->name +
              " is the current map on the server. It has been played for a "
              "total of " +
              ETJump::getDaysString(days) + " " +
              ETJump::getHoursString(hours) + " " +
              ETJump::getMinutesString(minutes) + " " +
              ETJump::getSecondsString(seconds);
  } else {
    if (mi->lastPlayed == 0) {
      message = ETJump::stringFormat("^3mapinfo: ^7%s has never been played.",
                                     mi->name);
    } else {
      message = "^3mapinfo: ^7" + mi->name + " was last played on " +
                Utilities::timestampToString(mi->lastPlayed) +
                ". It has been played for a total of " +
                ETJump::getDaysString(days) + " " +
                ETJump::getHoursString(hours) + " " +
                ETJump::getMinutesString(minutes) + " " +
                ETJump::getSecondsString(seconds);
    }
  }

  Printer::chat(ent, message);
  return true;
}

bool MostPlayed(gentity_t *ent, Arguments argv) {
  auto mapsToList = 10;
  if (argv->size() == 2) {
    try {
      mapsToList = std::stoi(argv->at(1), nullptr, 10);
    } catch (const std::invalid_argument &) {
      Printer::chat(ent, ETJump::stringFormat(
                             "^3Error: ^7'%s^7' is not a number", argv->at(1)));
      return false;
    } catch (const std::out_of_range &) {
      mapsToList = 10;
    }

    if (mapsToList <= 0) {
      Printer::chat(ent, "^3mostplayed: ^7second argument must be over 0");
      return false;
    }

    if (mapsToList > 100) {
      mapsToList = 100;
    }
  }

  auto mostPlayed = game.mapStatistics->getMostPlayed();
  auto listedMaps = 0;
  std::string buffer = "^zMost played maps are:\n"
                       "^gMap                    Played                        "
                       " Last played       Times played\n";
  auto green = false;
  for (auto &map : mostPlayed) {
    if (listedMaps >= mapsToList) {
      break;
    }

    buffer += green ? "^g" : "^7";
    buffer += ETJump::stringFormat(
        "%-22s %-30s %-17s     %d\n", map->name,
        playedTimeFmtString(map->secondsPlayed),
        Utilities::timestampToString(map->lastPlayed), map->timesPlayed);
    green = !green;

    ++listedMaps;
  }

  Printer::console(ClientNum(ent), buffer);

  return true;
}

void MutePlayer(gentity_t *target) {
  target->client->sess.muted = qtrue;

  char userinfo[MAX_INFO_STRING] = "\0";
  char *ip = NULL;

  trap_GetUserinfo(target - g_entities, userinfo, sizeof(userinfo));
  ip = Info_ValueForKey(userinfo, "ip");

  G_AddIpMute(ip);
}

bool Mute(gentity_t *ent, Arguments argv) {
  if (argv->size() != 2) {
    PrintManual(ent, "mute");
    return false;
  }

  std::string errorMsg;

  gentity_t *target = PlayerGentityFromString(argv->at(1), errorMsg);

  if (!target) {
    Printer::chat(ent, "^3!mute: ^7" + errorMsg);
    return false;
  }

  if (ent) {
    if (ent == target) {
      Printer::chat(ent, "^3mute: ^7you cannot mute yourself.");
      return false;
    }

    if (IsTargetHigherLevel(ent, target, true)) {
      Printer::chat(ent, "^3mute: ^7you cannot mute a fellow admin.");
      return false;
    }
  }

  if (target->client->sess.muted == qtrue) {
    Printer::chat(ent, "^3mute: " + std::string(target->client->pers.netname) +
                           " ^7is already muted.");
    return false;
  }

  MutePlayer(target);
  Printer::center(target, "^5You've been muted");
  Printer::chat(ent, std::string(target->client->pers.netname) +
                         " ^7has been muted.");
  return true;
}

bool Noclip(gentity_t *ent, Arguments argv) {
  if (!g_cheats.integer && level.noNoclip) {
    Printer::chat(ent, "^3noclip: ^7noclip is disabled on this map.");
    return false;
  }

  if (argv->size() == 1) {
    if (!ent) {
      return false;
    }

    if (!g_cheats.integer && ent->client->sess.timerunActive) {
      Printer::chat(ent,
                    "^3noclip: ^7cheats are disabled while timerun is active.");
      return false;
    }

    ent->client->noclip = ent->client->noclip ? qfalse : qtrue;
  } else {
    int count = 1;
    if (argv->size() == 3) {
      if (!ToInt(argv->at(2), count)) {
        count = 1;
      }
    }
    std::string err;
    gentity_t *other = PlayerGentityFromString(argv->at(1), err);
    if (!other) {
      Printer::chat(ent, "^3noclip: ^7" + err);
      return false;
    }

    if (!g_cheats.integer && other->client->sess.timerunActive) {
      Printer::chat(other,
                    "^3noclip: ^7cheats are disabled while timerun is active.");
      return false;
    }

    if (count > 1) {
      Printer::chat(other,
                    va("^3noclip: ^7you can use /noclip %d times.", count));
      Printer::chat(ent, va("^3noclip: ^7%s^7 can use /noclip %d times.",
                            other->client->pers.netname, count));
    } else if (count < 0) {
      Printer::chat(other, "^3noclip: ^7you can use /noclip infinitely.");
      Printer::chat(ent, va("^3noclip: ^7%s^7 can use /noclip infinitely.",
                            other->client->pers.netname));
    } else {
      Printer::chat(other, "^3noclip: ^7you can use /noclip once.");
      Printer::chat(ent, va("^3noclip: ^7%s^7 can use /noclip once.",
                            other->client->pers.netname));
    }

    other->client->pers.noclipCount = count;
  }

  return true;
}

bool Passvote(gentity_t *ent, Arguments argv) {
  if (level.voteInfo.voteTime) {
    if (level.voteInfo.vote_fn == ETJump::G_RockTheVote_v) {
      Printer::chat(
          ent, ETJump::stringFormat("^3passvote:^7 %s cannot be force passed.",
                                    level.voteInfo.voteString));
    } else {
      level.voteInfo.forcePass = qtrue;
      Printer::chatAll("^3passvote:^7 vote has been passed.");
    }
  } else {
    Printer::chat(ent, "^3passvote:^7 no vote in progress.");
  }
  return qtrue;
}

bool ReadConfig(gentity_t *ent, Arguments argv) {
  Printer::chat(ent, "ReadConfig is not implemented.");
  return true;
}

bool Rename(gentity_t *ent, Arguments argv) {
  if (argv->size() < 3) {
    PrintManual(ent, "rename");
    return false;
  }

  std::string err;
  gentity_t *target = PlayerGentityFromString(argv->at(1), err);
  if (!target) {
    Printer::chat(ent, "^3rename: ^7" + err);
    return false;
  }

  if (ent) {
    if (IsTargetHigherLevel(ent, target, true)) {
      if (ent != target) {
        Printer::chat(ent, "^3rename: ^7you cannot rename a fellow admin.");
        return false;
      }
    }

    if (ent != target) {
      target->client->forceRename = true;
    }
  }

  const std::string newName =
      ETJump::StringUtil::join(ETJump::Container::skipFirstN(*argv, 2), " ");

  if (newName.length() > MAX_NETNAME) {
    Printer::chat(
        ent, ETJump::stringFormat("^3rename: ^7new name is too long (%i > %i)",
                                  newName.length(), MAX_NETNAME));
    return false;
  }

  char userinfo[MAX_INFO_STRING] = "\0";
  const int cn = ClientNum(target);
  trap_GetUserinfo(cn, userinfo, sizeof(userinfo));

  const char *oldName = Info_ValueForKey(userinfo, "name");
  Printer::chatAll(
      va("^3rename: ^7%s^7 has been renamed to %s", oldName, newName.c_str()));
  Info_SetValueForKey(userinfo, "name", newName.c_str());
  trap_SetUserinfo(cn, userinfo);
  ClientUserinfoChanged(cn);
  trap_SendServerCommand(cn, va("set_name %s", newName.c_str()));
  return true;
}

bool Restart(gentity_t *ent, Arguments argv) {
  Svcmd_ResetMatch_f(qtrue);
  return true;
}

bool SetLevel(gentity_t *ent, Arguments argv) {
  // !setlevel <player> <level>
  // !setlevel -id id level

  if (argv->size() == 3) {
    std::string err;
    gentity_t *target = PlayerGentityFromString(argv->at(1), err);

    if (!target) {
      Printer::chat(ent, err);
      return false;
    }

    int level = 0;
    if (!ToInt(argv->at(2), level)) {
      Printer::chat(ent, "^3setlevel: ^7invalid level " + argv->at(2));
      return false;
    }

    if (ent) {
      if (IsTargetHigherLevel(ent, target, false)) {
        Printer::chat(
            ent, "^3setlevel: ^7you can't set the level of a fellow admin.");
        return false;
      }

      if (level > ETJump::session->GetLevel(ent)) {
        Printer::chat(ent, "^3setlevel: ^7you're not allowed to setlevel "
                           "higher than your own level.");
        return false;
      }
    }

    if (!game.levels->LevelExists(level)) {
      Printer::chat(ent, "^3setlevel: ^7level does not exist.");
      return false;
    }

    if (!ETJump::session->SetLevel(target, level)) {
      Printer::chat(
          ent, va("^3setlevel: ^7%s", ETJump::session->GetMessage().c_str()));
      return false;
    }

    Printer::chat(ent, va("^3setlevel: ^7%s^7 is now a level %d user.",
                          target->client->pers.netname, level));
    Printer::chat(target,
                  va("^3setlevel: ^7you are now a level %d user.", level));

    return true;
  }

  if (argv->size() == 4) {
    unsigned id = 0;
    if (!ToUnsigned(argv->at(2), id)) {
      Printer::chat(ent, "^3setlevel: ^7invalid id " + argv->at(2));
      return false;
    }

    if (!ETJump::session->UserExists(id)) {
      Printer::chat(ent, "^3setlevel: ^7user with id " + argv->at(2) +
                             " doesn't exist.");
      return false;
    }

    int level = 0;
    if (!ToInt(argv->at(3), level)) {
      Printer::chat(ent, "^3setlevel: ^7invalid level " + argv->at(2));
      return false;
    }

    if (ent) {
      if (IsTargetHigherLevel(ent, id, false)) {
        Printer::chat(
            ent, "^3setlevel: ^7you can't set the level of a fellow admin.");
        return false;
      }

      if (level > ETJump::session->GetLevel(ent)) {
        Printer::chat(ent, "^3setlevel: ^7you're not allowed to setlevel "
                           "higher than your own level.");
        return false;
      }
    }

    if (!game.levels->LevelExists(level)) {
      Printer::chat(ent, "^3setlevel: ^7level does not exist.");
      return false;
    }

    if (!ETJump::session->SetLevel(id, level)) {
      Printer::chat(
          ent, va("^3setlevel: ^7%s", ETJump::session->GetMessage().c_str()));
      return false;
    }

    Printer::chat(
        ent,
        va("^3setlevel: ^7user with id %d is now a level %d user.", id, level));
  }

  return true;
}

bool Spectate(gentity_t *ent, Arguments argv) {
  if (!ent) {
    return qfalse;
  }

  if (argv->size() != 2) {
    if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
      SetTeam(ent, "spectator", qfalse, static_cast<weapon_t>(-1),
              static_cast<weapon_t>(-1), qfalse);
    }

    return qtrue;
  }

  std::string error;
  // ETJump: match only players that are in game, filter out spectators
  gentity_t *target =
      PlayerGentityFromString(argv->at(1), error, TEAM_SPECTATOR);

  if (!target) {
    Printer::chat(ent, "^3spectate: ^7" + error);
    return false;
  }

  if (target->client->sess.sessionTeam == TEAM_SPECTATOR) {
    Printer::chat(ent, "^3!spectate:^7 you can't spectate a spectator.");
    return qfalse;
  }

  if (!G_AllowFollow(ent, target)) {
    Printer::chat(ent, va("^3!spectate: %s ^7is locked from spectators.",
                          target->client->pers.netname));
    return qfalse;
  }

  if (ent->client->sess.sessionTeam != TEAM_SPECTATOR) {
    SetTeam(ent, "spectator", qfalse, static_cast<weapon_t>(-1),
            static_cast<weapon_t>(-1), qfalse);
  }

  ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
  ent->client->sess.spectatorClient = target->client->ps.clientNum;
  return qtrue;
}

bool createToken(gentity_t *ent, Arguments argv) {
  if (ent) {
    if (argv->size() != 3) {
      Printer::chat(ent,
                    "^3usage: ^7!tokens create <easy (e)|medium (m)|hard (h)>");
      return false;
    }
  } else {
    if (argv->size() != 6) {
      Printer::chat(ent, "^3usage: ^7!tokens create <easy (e)|medium (m)|hard "
                         "(h)> <x> <y> <z>");
      return false;
    }
  }

  std::array<float, 3> coordinates{};
  if (argv->size() < 6) {
    VectorCopy(ent->r.currentOrigin, coordinates);
    // move closer to ground, but not quite to ground level
    // to avoid clipping into slopes a bit
    coordinates[2] += ent->client->ps.mins[2] + 2;
  } else {
    try {
      coordinates[0] = std::stof((*argv)[3]);
      coordinates[1] = std::stof((*argv)[4]);
      coordinates[2] = std::stof((*argv)[5]);
    } catch (const std::invalid_argument &) {
      Printer::chat(ent, "^3tokens: ^7coordinates are not numbers.");
      return false;
    } catch (const std::out_of_range &) {
      Printer::chat(ent, "^3tokens: ^7coordinates are out of range.");
      return false;
    }

    if (abs(coordinates[0]) > MAX_MAP_SIZE) {
      Printer::chat(ent, "^3tokens: ^7x coordinate is out of range.");
      return false;
    }
    if (abs(coordinates[1]) > MAX_MAP_SIZE) {
      Printer::chat(ent, "^3tokens: ^7y coordinate is out of range.");
      return false;
    }
    if (abs(coordinates[2]) > MAX_MAP_SIZE) {
      Printer::chat(ent, "^3tokens: ^7z coordinate is out of range.");
      return false;
    }
  }

  ETJump::Tokens::Difficulty difficulty;
  if ((*argv)[2] == "easy" || ((*argv)[2]) == "e") {
    difficulty = ETJump::Tokens::Easy;
  } else if ((*argv)[2] == "medium" || ((*argv)[2]) == "m") {
    difficulty = ETJump::Tokens::Medium;
  } else if ((*argv)[2] == "hard" || ((*argv)[2]) == "h") {
    difficulty = ETJump::Tokens::Hard;
  } else {
    Printer::chat(ent, "^3tokens: ^7difficulty must be either easy (e), medium "
                       "(m) or hard (h)");
    return false;
  }

  Printer::chat(ent, ETJump::stringFormat(
                         "Creating a token at (%f, %f, %f) for difficulty '%s'",
                         coordinates[0], coordinates[1], coordinates[2],
                         ETJump::Tokens::tokenDifficultyToString(difficulty)));

  auto result = game.tokens->createToken(difficulty, coordinates);
  if (!result.first) {
    Printer::chat(ent, "^3error: ^7" + result.second);
    return false;
  }
  return true;
}

bool moveToken(gentity_t *ent) {
  if (!ent) {
    Printer::chat(ent, "^3usage: ^7!tokens move can only be used by players.");
    return false;
  }
  std::array<float, 3> coordinates{};
  VectorCopy(ent->r.currentOrigin, coordinates);
  // move closer to ground, but not quite to ground level
  // to avoid clipping into slopes a bit
  coordinates[2] += ent->client->ps.mins[2] + 2;

  const auto result = game.tokens->moveNearestToken(coordinates);
  if (!result.first) {
    Printer::chat(ent, "^3error: ^7" + result.second);
    return false;
  }

  Printer::chat(ent, "^3tokens: ^7" + result.second);

  return true;
}

bool deleteToken(gentity_t *ent, Arguments argv) {
  if (!ent) {
    if (argv->size() != 4) {
      Printer::chat(
          ent,
          "^3usage: ^7!tokens <delete> <easy (e)|medium (m)|hard (h)> <1-6>");
      return false;
    }
  }

  if (argv->size() == 2) {
    std::array<float, 3> coordinates{};
    VectorCopy(ent->r.currentOrigin, coordinates);
    const auto result = game.tokens->deleteNearestToken(coordinates);
    if (!result.first) {
      Printer::chat(ent, "^3error: ^7" + result.second);
      return false;
    }

    Printer::chat(ent, "^3tokens: ^7" + result.second);

    return true;
  }

  if (argv->size() == 4) {
    ETJump::Tokens::Difficulty difficulty;
    if ((*argv)[2] == "easy" || ((*argv)[2]) == "e") {
      difficulty = ETJump::Tokens::Easy;
    } else if ((*argv)[2] == "medium" || ((*argv)[2]) == "m") {
      difficulty = ETJump::Tokens::Medium;
    } else if ((*argv)[2] == "hard" || ((*argv)[2]) == "h") {
      difficulty = ETJump::Tokens::Hard;
    } else {
      Printer::chat(ent, "^3tokens: ^7difficulty must be either easy (e), "
                         "medium (m) or hard (h)");
      return false;
    }

    auto num = 1;
    try {
      num = std::stoi((*argv)[3]);
    } catch (const std::invalid_argument &) {
      Printer::chat(ent, "^3tokens: ^7" + (*argv)[3] + " is not a number.");
      return false;
    } catch (const std::out_of_range &) {
      Printer::chat(ent, "^3tokens: ^7" + (*argv)[3] +
                             " is out of range (too large).");
      return false;
    }

    if (num < 1 || num > 6) {
      Printer::chat(ent, "^3tokens: ^7number should be between 1 and 6.");
      return false;
    }

    Printer::chat(
        ent, va("^3tokens: ^7deleting token %s #%d", (*argv)[2].c_str(), num));
    auto result = game.tokens->deleteToken(difficulty, num - 1);

    if (!result.first) {
      Printer::chat(ent, "^3error: ^7" + result.second);
      return false;
    }

    Printer::chat(ent, "^3tokens: ^7" + result.second);

    return true;
  }
  return true;
}

bool Tokens(gentity_t *ent, Arguments argv) {
  if (!g_tokensMode.integer) {
    Printer::chat(ent, "^3tokens: ^7tokens mode is disabled. Set g_tokensMode "
                       "'1' and restart map to enable tokens mode.");
    return false;
  }

  if (ent) {
    if (argv->size() < 2) {
      Printer::chat(ent, "^3usage: ^7check console for more information");
      Printer::console(
          ent, "^7!tokens create <easy (e)|medium (m)|hard (h)> ^9| Creates a "
               "new token\n"
               "^7!tokens move ^9| Moves nearest token to your location\n"
               "^7!tokens delete ^9| Deletes nearest token to your location\n"
               "^7!tokens delete <easy (e)|medium (m)|hard (h)> <1-6> ^9| "
               "Deletes specified token\n");
      return false;
    }
  } else {
    if (argv->size() < 4) {
      Printer::console(
          ent,
          "^3usage: \n"
          "^7!tokens <easy (e)|medium (m)|hard (h)> <difficulty> <x> <y> <z>\n"
          "!tokens <delete> <easy (e)|medium (m)|hard (h)> <1-6>\n");
      return false;
    }
  }

  if ((*argv)[1] == "create") {
    return createToken(ent, argv);
  }

  if ((*argv)[1] == "move") {
    return moveToken(ent);
  }

  if ((*argv)[1] == "delete") {
    return deleteToken(ent, argv);
  }

  return true;
}

bool Unban(gentity_t *ent, Arguments argv) {

  if (argv->size() == 1) {
    PrintManual(ent, "unban");
    return false;
  }

  int id;
  if (!ToInt(argv->at(1), id)) {
    Printer::chat(ent, "^3unban: ^7id is not a number.");
    return false;
  }

  if (!ETJump::database->Unban(ent, id)) {
    Printer::chat(ent, "^3unban: ^7" + ETJump::database->GetMessage());
    return false;
  }

  Printer::chat(ent, "^3unban: ^7removed ban with id " + argv->at(1));
  return true;
}

bool Unmute(gentity_t *ent, Arguments argv) {
  if (argv->size() != 2) {
    PrintManual(ent, "unmute");
    return false;
  }

  std::string error;
  gentity_t *target = PlayerGentityFromString(argv->at(1), error);
  if (!target) {
    Printer::chat(ent, "^3unmute: ^7" + error);
    return false;
  }

  if (!target->client->sess.muted) {
    Printer::chat(ent, "^3unmute: ^7target is not muted.");
    return false;
  }

  target->client->sess.muted = qfalse;

  char *ip = NULL;
  char userinfo[MAX_INFO_STRING] = "\0";
  trap_GetUserinfo(ClientNum(target), userinfo, sizeof(userinfo));
  ip = Info_ValueForKey(userinfo, "ip");

  G_RemoveIPMute(ip);

  Printer::center(target, "^5You've been unmuted.");
  Printer::chatAll(target->client->pers.netname +
                   std::string(" ^7has been unmuted."));

  return true;
}

bool UserInfo(gentity_t *ent, Arguments argv) {
  if (argv->size() == 1) {
    PrintManual(ent, "userinfo");
    return false;
  }

  int id;
  if (!ToInt(argv->at(1), id)) {
    Printer::chat(ent, "^3userinfo: ^7id was not a number.");
    return false;
  }

  ETJump::database->UserInfo(ent, id);

  return true;
}

bool MoverScale(gentity_t *ent, Arguments argv) {
  auto moverScaleValue = g_moverScale.value;
  if (argv->size() > 1) {
    if (!ToFloat(argv->at(1), moverScaleValue)) {
      moverScaleValue = 1.0f;
    }
    // scale range 0.1 - 5.0
    if (moverScaleValue > 5.0f) {
      moverScaleValue = 5.0f;
    } else if (moverScaleValue < 0.1f) {
      moverScaleValue = 0.1f;
    }
    trap_Cvar_Set("g_moverScale", va("%f", moverScaleValue));
  }

  Printer::chat(ent, "^3Mover scale is set to: ^7" +
                         std::to_string(moverScaleValue));
  return true;
}

bool NewMaps(gentity_t *ent, Arguments argv) {
  auto numMaps = 5;
  if (argv->size() > 1) {
    try {
      numMaps = std::stoi(argv->at(1), nullptr, 10);
    } catch (const std::invalid_argument &) {
      Printer::chat(ClientNum(ent),
                    ETJump::stringFormat("^3newmaps: ^7%s^7 is not a number",
                                         argv->at(1)));
      return false;
    } catch (const std::out_of_range &) {
      numMaps = 5;
    }

    if (numMaps <= 0) {
      Printer::chat(ClientNum(ent),
                    "^3newmaps: ^7second argument must be over 0");
      return false;
    }

    if (numMaps > 50) {
      numMaps = 50;
    }
  }

  auto maps = game.mapStatistics->getMaps();
  int totalMaps = static_cast<int>(maps.size());

  // very unlikely but can potentially happen
  if (numMaps > totalMaps) {
    numMaps = totalMaps;
  }

  std::string buffer = "^zLatest ^3" +
                       ETJump::getPluralizedString(numMaps, "^zmap") +
                       " added to server:\n\n";
  int lines = 0;
  for (int i = numMaps; i > 0; i--) {
    buffer += lines % 2 == 0 ? "^7" : "^z";
    buffer += ETJump::stringFormat("%s\n", maps.at(totalMaps - i));
    lines++;
  }

  Printer::console(ClientNum(ent), buffer);
  return true;
}

bool TimerunAddSeason(gentity_t *ent, Arguments argv) {
  const int clientNum = ClientNum(ent);

  // All commands should be refactored to use this but will do this like
  // this for now
  auto def = std::move(
      ETJump::CommandParser::CommandDefinition::create(
          "add-season", "Adds a new timerun season\n    !add-season --name "
                        "<name> --start-date <2000-01-01>")
          .addOption("name", "n", "Name of the season to add",
                     ETJump::CommandParser::OptionDefinition::Type::MultiToken,
                     true)
          .addOption("start-date", "sd",
                     "Start date for the timerun season in YYYY-MM-DD format "
                     "(e.g. 2000-01-01)",
                     ETJump::CommandParser::OptionDefinition::Type::Date, true)
          .addOption("end-date-exclusive", "ed",
                     "End date for the timerun season in YYYY-MM-DD format "
                     "(e.g. 2000-01-01)",
                     ETJump::CommandParser::OptionDefinition::Type::Date,
                     false));
  auto optCommand = getOptCommand("add-season", clientNum, def, argv);

  if (!optCommand.has_value()) {
    return true;
  }

  auto command = std::move(optCommand.value());

  const auto &name = command.options.at("name").text;
  auto start = command.options.at("start-date").date;
  auto end = command.options.count("end-date-exclusive") > 0
                 ? std::make_optional<ETJump::Time>(ETJump::Time::fromDate(
                       command.options.at("end-date-exclusive").date))
                 : std::nullopt;

  if (end.has_value()) {
    if ((*end).date < start) {
      Printer::chat(clientNum,
                    ETJump::stringFormat(
                        "^3addseason: ^7Start time `%s` is after end time `%s`",
                        start.toDateString(), end.value().date.toDateString()));
      return true;
    }
  }

  ETJump::Timerun::AddSeasonParams params{};
  params.clientNum = clientNum;
  params.startTime = ETJump::Time::fromDate(start);
  params.endTime = end;
  params.name = name;

  game.timerunV2->addSeason(params);

  return true;
}

bool TimerunEditSeason(gentity_t *ent, Arguments argv) {
  const int clientNum = ClientNum(ent);

  auto def = std::move(
      ETJump::CommandParser::CommandDefinition::create(
          "edit-season", "Edit an existing timerun season")
          .addOption("name", "n", "Name of the season to edit",
                     ETJump::CommandParser::OptionDefinition::Type::MultiToken,
                     true)
          .addOption("start-date", "sd",
                     "Start date for the timerun season in YYYY-MM-DD format "
                     "(e.g. 2000-01-01)",
                     ETJump::CommandParser::OptionDefinition::Type::Date, false)
          .addOption("end-date", "ed",
                     "End date for the timerun season in YYYY-MM-DD format "
                     "(e.g. 2000-01-01)",
                     ETJump::CommandParser::OptionDefinition::Type::Date,
                     false));

  auto optCommand = getOptCommand("edit-season", clientNum, def, argv);

  if (!optCommand.has_value()) {
    return true;
  }

  auto command = std::move(optCommand.value());

  const auto &name = command.options["name"].text;
  auto start = command.getOptional("start-date");
  auto end = command.getOptional("end-date");

  const auto &startTime = start.has_value()
                              ? std::make_optional<ETJump::Time>(
                                    ETJump::Time::fromDate(start.value().date))
                              : std::nullopt;
  const auto &endTime = end.has_value()
                            ? std::make_optional<ETJump::Time>(
                                  ETJump::Time::fromDate(end.value().date))
                            : std::nullopt;

  game.timerunV2->editSeason({clientNum, name, startTime, endTime});

  return true;
}

bool TimerunDeleteSeason(gentity_t *ent, Arguments argv) {
  const int clientNum = ClientNum(ent);

  auto def = std::move(
      ETJump::CommandParser::CommandDefinition::create(
          "delete-season", "Delete a season. This will delete all the "
                           "records within the season.")
          .addOption("name", "n", "Exact name of the season to delete",
                     ETJump::CommandParser::OptionDefinition::Type::MultiToken,
                     true));

  auto optCommand = getOptCommand("delete-season", clientNum, def, argv);
  if (!optCommand.has_value()) {
    return true;
  }

  auto command = std::move(optCommand.value());
  const auto &name = command.options["name"].text;

  game.timerunV2->deleteSeason(clientNum,
                               ETJump::StringUtil::toLowerCase(name));

  return true;
}

bool RockTheVote(gentity_t *ent, Arguments argv) {
  std::string cmd = "callvote rtv";

  if (argv->size() > 1) {
    cmd += " " + argv->at(1);
  }

  trap_SendServerCommand(ClientNum(ent), cmd.c_str());
  return true;
}

// ensures custom vote command options are valid
bool validateCustomVoteCommand(const std::string &cmdName, const int &clientNum,
                               const ETJump::CommandParser::Command &command) {
  bool commandOk = true;

  for (const auto &op : command.options) {
    if (ETJump::sanitize(op.second.text).empty()) {
      Printer::chat(clientNum,
                    ETJump::stringFormat("^3%s: ^7'%s' cannot be empty.",
                                         cmdName, op.first));
      commandOk = false;
    }

    if (op.first == "name" && !ETJump::isValidVoteString(op.second.text)) {
      Printer::chat(
          clientNum,
          ETJump::stringFormat("^3%s: ^7%s '%s' contains invalid characters.",
                               cmdName, op.first, op.second.text));
      commandOk = false;
    }
  }

  return commandOk;
}

bool addCustomVote(gentity_t *ent, Arguments argv) {
  const int clientNum = ClientNum(ent);

  const auto def = std::move(
      ETJump::CommandParser::CommandDefinition::create(
          "add-customvote", "Creates a new custom map vote list.")
          .addOption("name", "n", "Name of the custom vote list",
                     ETJump::CommandParser::OptionDefinition::Type::Token, true)
          .addOption(
              "full-name", "fn", "Full name, displayed in the callvote text.",
              ETJump::CommandParser::OptionDefinition::Type::MultiToken, true)
          .addOption(
              "maps", "m", "Maps to include in the list, space delimited.",
              ETJump::CommandParser::OptionDefinition::Type::MultiToken, true));

  const auto optCommand =
      ETJump::getOptCommand("add-customvote", clientNum, def, argv);

  if (!optCommand.has_value()) {
    return false;
  }

  auto command = optCommand.value();

  const std::string &name = command.options["name"].text;
  const std::string &fullName = command.options["full-name"].text;
  const std::string &maps = command.options["maps"].text;

  if (!validateCustomVoteCommand(def.name, clientNum, command)) {
    return false;
  }

  game.customMapVotes->addCustomvoteList(clientNum, name, fullName, maps);
  return true;
}

bool deleteCustomVote(gentity_t *ent, Arguments argv) {
  const int clientNum = ClientNum(ent);

  auto def = std::move(
      ETJump::CommandParser::CommandDefinition::create(
          "delete-customvote", "Deletes a custom map vote list.")
          .addOption("name", "n", "Name of the list to delete.",
                     ETJump::CommandParser::OptionDefinition::Type::Token,
                     true));

  const auto optCommand =
      ETJump::getOptCommand("delete-customvote", clientNum, def, argv);

  if (!optCommand.has_value()) {
    return false;
  }

  auto command = optCommand.value();
  const std::string &name = command.options["name"].text;

  game.customMapVotes->deleteCustomvoteList(clientNum, name);
  return true;
}

bool editCustomVote(gentity_t *ent, Arguments argv) {
  const int clientNum = ClientNum(ent);

  auto def = std::move(
      ETJump::CommandParser::CommandDefinition::create(
          "edit-customvote", "Edits a custom map vote list.")
          .addOption("list", "l", "Name of the list to edit.",
                     ETJump::CommandParser::OptionDefinition::Type::Token, true)
          .addOption("name", "n", "Name of the custom vote list.",
                     ETJump::CommandParser::OptionDefinition::Type::Token,
                     false)
          .addOption(
              "full-name", "fn", "Full name, displayed in the callvote text.",
              ETJump::CommandParser::OptionDefinition::Type::MultiToken, false)
          .addOption(
              "add-maps", "am", "Maps to add to the list, space delimited.",
              ETJump::CommandParser::OptionDefinition::Type::MultiToken, false)
          .addOption("remove-maps", "rm",
                     "Maps to remove from the list, space delimited.",
                     ETJump::CommandParser::OptionDefinition::Type::MultiToken,
                     false));

  const auto optCommand =
      ETJump::getOptCommand("edit-customvote", clientNum, def, argv);

  if (!optCommand.has_value()) {
    return false;
  }

  auto command = optCommand.value();

  const std::string &list = command.options["list"].text;
  const auto optName = command.getOptional("name");
  const auto optFullName = command.getOptional("full-name");
  const auto optAddMaps = command.getOptional("add-maps");
  const auto optRemoveMaps = command.getOptional("remove-maps");

  const std::string &name = optName.has_value() ? optName.value().text : "";
  const std::string &fullName =
      optFullName.has_value() ? optFullName.value().text : "";
  const std::string &addMaps =
      optAddMaps.has_value() ? optAddMaps.value().text : "";
  const std::string &removeMaps =
      optRemoveMaps.has_value() ? optRemoveMaps.value().text : "";

  if (!validateCustomVoteCommand(def.name, clientNum, command)) {
    return false;
  }

  game.customMapVotes->editCustomvoteList(clientNum, list, name, fullName,
                                          addMaps, removeMaps);
  return true;
}

} // namespace AdminCommands

Commands::Commands() {
  // using AdminCommands::AdminCommand;
  // adminCommands_["addlevel"] = AdminCommand(AdminCommands::AddLevel,
  // 'a');

  adminCommands_["addlevel"] =
      AdminCommandPair(AdminCommands::AddLevel, CommandFlags::EDIT);
  adminCommands_["admintest"] =
      AdminCommandPair(AdminCommands::Admintest, CommandFlags::BASIC);
  adminCommands_["8ball"] =
      AdminCommandPair(AdminCommands::Ball8, CommandFlags::BASIC);
  adminCommands_["ban"] =
      AdminCommandPair(AdminCommands::Ban, CommandFlags::BAN);
  adminCommands_["cancelvote"] =
      AdminCommandPair(AdminCommands::Cancelvote, CommandFlags::CANCELVOTE);
  adminCommands_["deletelevel"] =
      AdminCommandPair(AdminCommands::DeleteLevel, CommandFlags::EDIT);
  // adminCommands_["deleteuser"] =
  // AdminCommandPair(AdminCommands::DeleteUser, CommandFlags::EDIT);
  adminCommands_["editcommands"] =
      AdminCommandPair(AdminCommands::EditCommands, CommandFlags::EDIT);
  adminCommands_["editlevel"] =
      AdminCommandPair(AdminCommands::EditLevel, CommandFlags::EDIT);
  adminCommands_["edituser"] =
      AdminCommandPair(AdminCommands::EditUser, CommandFlags::EDIT);
  adminCommands_["finduser"] =
      AdminCommandPair(AdminCommands::FindUser, CommandFlags::EDIT);
  adminCommands_["listusernames"] =
      AdminCommandPair(AdminCommands::ListUserNames, CommandFlags::EDIT);
  adminCommands_["finger"] =
      AdminCommandPair(AdminCommands::Finger, CommandFlags::FINGER);
  adminCommands_["findmap"] =
      AdminCommandPair(AdminCommands::FindMap, CommandFlags::BASIC);
  adminCommands_["help"] =
      AdminCommandPair(AdminCommands::Help, CommandFlags::BASIC);
  adminCommands_["kick"] =
      AdminCommandPair(AdminCommands::Kick, CommandFlags::KICK);
  adminCommands_["leastplayed"] =
      AdminCommandPair(AdminCommands::LeastPlayed, CommandFlags::BASIC);
  adminCommands_["levelinfo"] =
      AdminCommandPair(AdminCommands::LevelInfo, CommandFlags::EDIT);
  adminCommands_["listbans"] =
      AdminCommandPair(AdminCommands::ListBans, CommandFlags::LISTBANS);
  // adminCommands_["listcmds"] =
  // AdminCommandPair(AdminCommands::ListCommands, CommandFlags::BASIC);
  adminCommands_["listflags"] =
      AdminCommandPair(AdminCommands::ListFlags, CommandFlags::EDIT);
  adminCommands_["listmaps"] =
      AdminCommandPair(AdminCommands::ListMaps, CommandFlags::BASIC);
  adminCommands_["listplayers"] =
      AdminCommandPair(AdminCommands::ListPlayers, CommandFlags::LISTPLAYERS);
  adminCommands_["listusers"] =
      AdminCommandPair(AdminCommands::ListUsers, CommandFlags::EDIT);
  adminCommands_["map"] =
      AdminCommandPair(AdminCommands::Map, CommandFlags::MAP);
  adminCommands_["mapinfo"] =
      AdminCommandPair(AdminCommands::MapInfo, CommandFlags::BASIC);
  adminCommands_["mostplayed"] =
      AdminCommandPair(AdminCommands::MostPlayed, CommandFlags::BASIC);
  adminCommands_["mute"] =
      AdminCommandPair(AdminCommands::Mute, CommandFlags::MUTE);
  adminCommands_["noclip"] =
      AdminCommandPair(AdminCommands::Noclip, CommandFlags::NOCLIP);
  adminCommands_["passvote"] =
      AdminCommandPair(AdminCommands::Passvote, CommandFlags::PASSVOTE);
  //    adminCommands_["readconfig"] =
  //    AdminCommandPair(AdminCommands::ReadConfig,
  //    CommandFlags::READCONFIG);
  adminCommands_["rename"] =
      AdminCommandPair(AdminCommands::Rename, CommandFlags::RENAME);
  adminCommands_["restart"] =
      AdminCommandPair(AdminCommands::Restart, CommandFlags::RESTART);
  adminCommands_["setlevel"] =
      AdminCommandPair(AdminCommands::SetLevel, CommandFlags::SETLEVEL);
  adminCommands_["spectate"] =
      AdminCommandPair(AdminCommands::Spectate, CommandFlags::BASIC);
  adminCommands_["tokens"] =
      AdminCommandPair(AdminCommands::Tokens, CommandFlags::TOKENS);
  adminCommands_["unban"] =
      AdminCommandPair(AdminCommands::Unban, CommandFlags::BAN);
  adminCommands_["unmute"] =
      AdminCommandPair(AdminCommands::Unmute, CommandFlags::MUTE);
  adminCommands_["userinfo"] =
      AdminCommandPair(AdminCommands::UserInfo, CommandFlags::EDIT);
  adminCommands_["moverscale"] =
      AdminCommandPair(AdminCommands::MoverScale, CommandFlags::MOVERSCALE);
  adminCommands_["newmaps"] =
      AdminCommandPair(AdminCommands::NewMaps, CommandFlags::BASIC);
  adminCommands_["add-season"] = AdminCommandPair(
      AdminCommands::TimerunAddSeason, CommandFlags::TIMERUN_MANAGEMENT);
  adminCommands_["edit-season"] = AdminCommandPair(
      AdminCommands::TimerunEditSeason, CommandFlags::TIMERUN_MANAGEMENT);
  adminCommands_["delete-season"] = AdminCommandPair(
      AdminCommands::TimerunDeleteSeason, CommandFlags::TIMERUN_MANAGEMENT);
  adminCommands_["records"] =
      AdminCommandPair(ClientCommands::Records, CommandFlags::BASIC);
  adminCommands_["ranks"] =
      AdminCommandPair(ClientCommands::Records, CommandFlags::BASIC);
  adminCommands_["times"] =
      AdminCommandPair(ClientCommands::Records, CommandFlags::BASIC);
  adminCommands_["top"] =
      AdminCommandPair(ClientCommands::Records, CommandFlags::BASIC);
  adminCommands_["rankings"] =
      AdminCommandPair(ClientCommands::Rankings, CommandFlags::BASIC);
  adminCommands_["loadcheckpoints"] =
      AdminCommandPair(ClientCommands::LoadCheckpoints, CommandFlags::BASIC);
  adminCommands_["seasons"] =
      AdminCommandPair(ClientCommands::ListSeasons, CommandFlags::BASIC);
  adminCommands_["rtv"] =
      AdminCommandPair(AdminCommands::RockTheVote, CommandFlags::BASIC);
  adminCommands_["listcheckpoints"] =
      AdminCommandPair(ClientCommands::listCheckpoints, CommandFlags::BASIC);
  adminCommands_["comparecheckpoints"] =
      AdminCommandPair(ClientCommands::compareCheckpoints, CommandFlags::BASIC);
  adminCommands_["add-customvote"] =
      AdminCommandPair(AdminCommands::addCustomVote, CommandFlags::CUSTOMVOTES);
  adminCommands_["delete-customvote"] = AdminCommandPair(
      AdminCommands::deleteCustomVote, CommandFlags::CUSTOMVOTES);
  adminCommands_["edit-customvote"] = AdminCommandPair(
      AdminCommands::editCustomVote, CommandFlags::CUSTOMVOTES);

  commands_["backup"] = ClientCommands::BackupLoad;
  commands_["save"] = ClientCommands::Save;
  commands_["load"] = ClientCommands::Load;
  commands_["unload"] = ClientCommands::Unload;
  //    commands_["race"] = ClientCommands::Race;
  commands_["listinfo"] = ClientCommands::listCustomVotes;
  commands_["customvotes"] = ClientCommands::listCustomVotes;
  commands_["records"] = ClientCommands::Records;
  commands_["times"] = ClientCommands::Records;
  commands_["ranks"] = ClientCommands::Records;
  commands_["top"] = ClientCommands::Records;
  commands_["loadcheckpoints"] = ClientCommands::LoadCheckpoints;
  commands_["load-checkpoints"] = ClientCommands::LoadCheckpoints;
  commands_["rankings"] = ClientCommands::Rankings;
  commands_["seasons"] = ClientCommands::ListSeasons;
  commands_["listcheckpoints"] = ClientCommands::listCheckpoints;
  commands_["comparecheckpoints"] = ClientCommands::compareCheckpoints;
  commands_["getchatreplay"] = ClientCommands::GetChatReplay;
  commands_["requestmaplist"] = ClientCommands::sendMaplist;
  commands_["requestnumcustomvotes"] = ClientCommands::sendNumCustomvotes;
  commands_["requestcustomvoteinfo"] = ClientCommands::sendCustomvoteInfo;
  commands_["loadpos"] = ClientCommands::loadPos;
}

bool Commands::ClientCommand(gentity_t *ent, const std::string &commandStr) {
  G_DPrintf("Commands::ClientCommand called for %d\n", ClientNum(ent));

  ConstCommandIterator command = commands_.find(commandStr);
  if (command == commands_.end()) {
    return false;
  }

  command->second(ent, GetArgs());

  return true;
}

bool Commands::List(gentity_t *ent) {
  ConstAdminCommandIterator it = adminCommands_.begin(),
                            end = adminCommands_.end();
  const int clienNum = ClientNum(ent);
  std::string helpMsg;

  Printer::chat(clienNum, "^3help: ^gcheck console for more information.");

  int i = 1;
  std::bitset<256> perm = ETJump::session->Permissions(ent);
  for (; it != end; it++) {
    if (!perm[it->second.second]) {
      continue;
    }

    helpMsg += va("%-20s ", it->first.c_str());
    if (i != 0 && i % 3 == 0) {
      helpMsg += "\n";
    }

    i++;
  }

  // Add a newline if last row is incomplete
  if (i % 3 != 1) {
    helpMsg += "\n";
  }

  // Let client know if they have access to silent commands
  if (ent && ETJump::session->HasPermission(ent, '/')) {
    helpMsg += "\n^gUse admin commands silently with ^3/!command\n";
  }

  Printer::console(clienNum, helpMsg);
  return true;
}

bool Commands::AdminCommand(gentity_t *ent) {
  std::string command, arg = SayArgv(0);
  int skip = 0;

  if (arg == "say" || arg == "enc_say") {
    arg = SayArgv(1);
    skip = 1;
  } else {
    if (ent && !ETJump::session->HasPermission(ent, '/')) {
      return false;
    }
  }
  Arguments argv = GetSayArgs(skip);

  if (arg.length() == 0) {
    return false;
  }

  if (arg[0] == '!') {
    if (arg.length() == 1) {
      return false;
    }
    command = &arg[1];
  } else {
    return false;
  }

  command = ETJump::StringUtil::toLowerCase(command);

  ConstAdminCommandIterator it = adminCommands_.lower_bound(command);

  if (it == adminCommands_.end()) {
    return false;
  }
  std::bitset<256> permissions = ETJump::session->Permissions(ent);
  std::vector<ConstAdminCommandIterator> foundCommands;
  while (it != adminCommands_.end() &&
         it->first.compare(0, command.length(), command) == 0) {
    if (permissions[it->second.second]) {
      if (it->first == command) {
        foundCommands.clear();
        foundCommands.push_back(it);
        break;
      }
      foundCommands.push_back(it);
    }
    ++it;
  }

  if (foundCommands.size() == 1) {
    foundCommands[0]->second.first(ent, argv);

    // check if we should log this command
    const char flag = FindCommandFlag(foundCommands[0]->first);

    if (std::find(std::begin(CommandFlags::loggedCommandFlags),
                  std::end(CommandFlags::loggedCommandFlags),
                  flag) != std::end(CommandFlags::loggedCommandFlags)) {

      const std::string name =
          ent ? ETJump::sanitize(ent->client->pers.netname) : "Console";

      // skip the first arg because we might have partially matched the command
      const std::string cmdArgs = ETJump::StringUtil::join(
          ETJump::Container::skipFirstN(*argv, 1), " ");
      Printer::logAdminLn(ETJump::stringFormat(
          "admincommand: %s%s used '%s%s'",
          ent ? std::to_string(ent->s.number) + " " : "", name,
          foundCommands[0]->first, cmdArgs.empty() ? "" : " " + cmdArgs));
    }

    return true;
  }

  if (foundCommands.size() > 1) {
    Printer::chat(ent, "^3server: ^7multiple matching commands found. Check "
                       "console for more information");
    std::string msg;

    for (const auto &cmd : foundCommands) {
      msg += ETJump::stringFormat("* %s\n", cmd->first);
    }

    Printer::console(ent, msg);
  }

  return false;
}

qboolean AdminCommandCheck(gentity_t *ent) {
  return game.commands->AdminCommand(ent) ? qtrue : qfalse;
}

void Commands::ListCommandFlags(gentity_t *ent) {
  Printer::chat(ent, "^3listflags: ^7check console for more information.");

  std::string msg;

  for (const auto &cmd : adminCommands_) {
    msg += ETJump::stringFormat("%c %s\n", cmd.second.second, cmd.first);
  }

  // manually add this since it's not an actual command
  msg += "/ [silent commands]\n";

  Printer::console(ent, msg);
}

char Commands::FindCommandFlag(const std::string &command) {
  AdminCommandsIterator it = adminCommands_.begin();
  AdminCommandsIterator end = adminCommands_.end();
  while (it != end) {
    if ((*it).first == command) {
      return (*it).second.second;
    }
    it++;
  }
  return 0;
}
