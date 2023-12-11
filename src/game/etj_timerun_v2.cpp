/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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

#include <utility>

#include "etj_timerun_v2.h"

#include "etj_container_utilities.h"
#include "etj_log.h"
#include "etj_printer.h"
#include "etj_synchronization_context.h"
#include "etj_timerun_repository.h"
#include "etj_timerun_shared.h"
#include "etj_local.h"
#include "etj_map_statistics.h"

ETJump::TimerunV2::TimerunV2(
    std::string currentMap, std::unique_ptr<TimerunRepository> repository,
    std::unique_ptr<Log> logger,
    std::unique_ptr<SynchronizationContext> synchronizationContext)
    : _currentMap(std::move(currentMap)), _repository(std::move(repository)),
      _logger(std::move(logger)), _sc(std::move(synchronizationContext)) {}

const ETJump::Timerun::Record *
ETJump::TimerunV2::Player::getRecord(int seasonId,
                                     const std::string &runName) const {
  for (const auto &r : records) {
    if (r.seasonId == seasonId && r.run == runName) {
      return &r;
    }
  }
  return nullptr;
}

class ComputeRanksResult : public ETJump::SynchronizationContext::ResultBase {
public:
  explicit ComputeRanksResult(
      const std::map<int, std::vector<ETJump::TimerunV2::Ranking>> &rankings)
      : rankings(rankings) {}

  std::map<int, std::vector<ETJump::TimerunV2::Ranking>> rankings;
};

void ETJump::TimerunV2::computeRanks() {
  _sc->postTask(
      [this]() {
        auto start = std::chrono::high_resolution_clock::now();
        auto records = _repository->getRecords();
        auto now = std::chrono::high_resolution_clock::now();
        _logger->info("loaded all records for rankings computation in %fs",
                      static_cast<double>((now - start).count()) / 1000.0 /
                          1000.0 / 1000.0);

        start = now;

        int seasonId = 1;
        std::string map;
        std::string run;
        int rank = 1;
        int topTime = 0;

        using SeasonId = int;
        using UserId = int;

        std::map<SeasonId, std::map<UserId, double>> scores;
        std::map<UserId, std::string> latestName{};
        const double maxPointsPerRun = 1000.0;

        for (const auto &r : records) {
          // we don't want to compute score for maps not on the server,
          // e.g. when a new version of a map is released
          if (!game.mapStatistics->mapExists(r.map)) {
            continue;
          }
          if (scores.count(r.seasonId) == 0) {
            scores[r.seasonId] = {};
          }
          if (scores[r.seasonId].count(r.userId) == 0) {
            scores[r.seasonId][r.userId] = 0;
          }
          latestName[r.userId] = r.playerName;
          if (seasonId != r.seasonId || map != r.map || run != r.run) {
            seasonId = r.seasonId;
            map = r.map;
            run = r.run;
            rank = 1;
            topTime = r.time;

            scores[r.seasonId][r.userId] += maxPointsPerRun;
          } else {
            if (topTime == 0 || r.time == 0) {
              continue;
            }

            // c1 == (#1 time) / (time of the current player)
            double c1 =
                static_cast<double>(topTime) / static_cast<double>(r.time);

            const double pctLoss2to50 = 1.0;
            const double pctLoss51to100 = 0.5;
            const double pctLoss101onwards = 0.25;

            double c2 = 100.0 - pctLoss2to50 *
                                    std::min(static_cast<double>(rank), 50.0);
            if (rank > 50)
              c2 -= pctLoss51to100 *
                    (std::min(static_cast<double>(rank), 100.0) - 50.0);
            if (rank > 100)
              c2 -= pctLoss101onwards * static_cast<double>(rank - 100);

            c2 /= 100;

            if (c2 < 0) {
              continue;
            }

            scores[r.seasonId][r.userId] += maxPointsPerRun * c1 * c2;
          }
          ++rank;
        }

        std::map<SeasonId, std::vector<Ranking>> rankings;

        for (const auto &season : scores) {
          if (rankings.count(season.first) == 0) {
            rankings[season.first] = {};
          }

          for (const auto &user : season.second) {
            // exclude users with 0 points from rankings
            if (user.second == 0) {
              continue;
            }
            rankings[season.first].emplace_back(
                0, user.first, latestName[user.first], user.second);
          }
        }

        for (auto &season : rankings) {
          std::sort(begin(season.second), end(season.second),
                    [](const auto lhs, const auto rhs) {
                      return lhs.score > rhs.score;
                    });

          for (unsigned i = 0, len = season.second.size(); i < len; ++i) {
            season.second[i].rank = static_cast<int>(i) + 1;
          }
        }

        now = std::chrono::high_resolution_clock::now();

        _logger->info("computed rankings in %fs",
                      static_cast<double>((now - start).count()) / 1000.0 /
                          1000.0 / 1000.0);

        return std::make_unique<ComputeRanksResult>(rankings);
      },

      [this](auto r) {
        auto result = dynamic_cast<ComputeRanksResult *>(r.get());

        _rankingsPerSeason = std::move(result->rankings);
      },
      [this](auto e) {
        _logger->error("failed to compute rankings: %s", e.what());
      });
}

void ETJump::TimerunV2::updateSeasonStates() {
  const auto seasons = _repository->getSeasons();
  const auto currentTime = getCurrentTime();

  _activeSeasonsIds = std::vector<int>();
  _activeSeasons = std::vector<Timerun::Season>();

  _pastSeasonsIds = std::vector<int>();
  _pastSeasons = std::vector<Timerun::Season>();

  _upcomingSeasonsIds = std::vector<int>();
  _upcomingSeasons = std::vector<Timerun::Season>();

  for (const auto &season : seasons) {
    // upcoming seasons
    if (season.startTime > currentTime) {
      _upcomingSeasons.push_back(season);
    }
    // active seasons
    else if (!season.endTime.hasValue() ||
             season.endTime.value() > currentTime) {
      _activeSeasons.push_back(season);
    }
    // past seasons
    else {
      _pastSeasons.push_back(season);
    }
  }

  auto getSeasonId = [](const Timerun::Season &s) { return s.id; };

  _activeSeasonsIds = Container::map(_activeSeasons, getSeasonId);
  _pastSeasonsIds = Container::map(_pastSeasons, getSeasonId);
  _upcomingSeasonsIds = Container::map(_upcomingSeasons, getSeasonId);

  _logger->info("Successfully updated season states: Active seasons: (%d), "
                "Upcoming seasons: (%d) Past seasons: (%d)",
                _activeSeasonsIds.size(), _upcomingSeasonsIds.size(),
                _pastSeasonsIds.size());
}

void ETJump::TimerunV2::initialize() {
  try {
    _repository->initialize();

    updateSeasonStates();

    _mostRelevantSeason = getMostRelevantSeason();

    _logger->info("Active seasons: %s",
                  StringUtil::join(Container::map(_activeSeasons,
                                                  [](const Timerun::Season &s) {
                                                    return stringFormat(
                                                        "%s (%d)", s.name,
                                                        s.id);
                                                  }),
                                   ", "));

  } catch (const std::exception &e) {
    Printer::LogPrintln(std::string("Unable to initialize timerun database") +
                        e.what());
  }

  _sc->startWorkerThreads(1);
  computeRanks();
}

void ETJump::TimerunV2::shutdown() {
  _repository->shutdown();
  _repository = nullptr;
  _sc->stopWorkerThreads();
}

void ETJump::TimerunV2::runFrame() { _sc->processCompletedTasks(); }

class ClientConnectResult : public ETJump::SynchronizationContext::ResultBase {
public:
  explicit ClientConnectResult(std::vector<ETJump::Timerun::Record> runs)
      : runs(std::move(runs)) {}

  std::vector<ETJump::Timerun::Record> runs;
};

void ETJump::TimerunV2::clientConnect(int clientNum, int userId) {
  _sc->postTask(
      [this, clientNum, userId] {
        auto parameters = StringUtil::join(
            Container::map(_activeSeasonsIds,
                           [](int season) { return std::to_string(season); }),
            ", ");

        auto runs = _repository->getRecordsForPlayer(_activeSeasonsIds,
                                                     _currentMap, userId);

        return std::make_unique<ClientConnectResult>(runs);
      },
      [this, clientNum,
       userId](std::unique_ptr<SynchronizationContext::ResultBase> result) {
        auto clientConnectResult =
            dynamic_cast<ClientConnectResult *>(result.get());

        _players[clientNum] = std::make_unique<Player>(
            clientNum, userId, clientConnectResult->runs);
      },
      [this, clientNum, userId](const std::runtime_error &error) {
        _logger->info("Unable to load player information for clientNum: `%d` "
                      "userId: `%d`: %s",
                      clientNum, userId, error.what());
        Printer::SendChatMessage(
            clientNum, "Unable to load player information. Any timeruns will "
                       "not work. Try to reconnect or file a bug report at "
                       "github.com/etjump/etjump.");
        Printer::SendConsoleMessage(clientNum,
                                    stringFormat("cause: %s\n", error.what()));
      });
}

void ETJump::TimerunV2::clientDisconnect(int clientNum) {
  _players[clientNum] = nullptr;
}

void ETJump::TimerunV2::startTimer(const std::string &runName, int clientNum,
                                   const std::string &playerName,
                                   int currentTimeMs) {
  auto player = _players[clientNum].get();
  if (!player) {
    _logger->error("Trying to start run `%s` for client `%d` but no player "
                   "object available.",
                   runName, clientNum);
    Printer::SendChatMessage(
        clientNum, "Unable to start timerun. Reconnect and if this persists, "
                   "report the bug at github.com/etjump/etjump");
    return;
  }

  if (player->running) {
    return;
  }

  player->running = true;
  player->name = playerName;
  player->startTime = opt<int>(currentTimeMs);
  player->completionTime = opt<int>();
  player->activeRunName = runName;
  player->runHasCheckpoints =
      level.checkpointsCount[indexForRunname(runName)] != 0;
  player->checkpointTimes.fill(TIMERUN_CHECKPOINT_NOT_SET);
  player->checkpointIndexesHit.fill(false);
  player->nextCheckpointIdx = 0;

  startNotify(player);

  Utilities::startRun(clientNum);
}

void ETJump::TimerunV2::checkpoint(const std::string &runName, int clientNum,
                                   int checkpointIndex, int currentTimeMs) {
  Player *player = _players[clientNum].get();

  if (player == nullptr) {
    return;
  }

  if (!player->running || player->activeRunName != runName) {
    return;
  }

  if (player->nextCheckpointIdx >= MAX_TIMERUN_CHECKPOINTS) {
    return;
  }

  if (player->checkpointIndexesHit[checkpointIndex]) {
    return;
  }

  player->checkpointIndexesHit[checkpointIndex] = true;

  player->checkpointTimes[player->nextCheckpointIdx++] =
      currentTimeMs - player->startTime.value();

  Printer::SendCommandToAll(
      TimerunCommands::Checkpoint(
          clientNum, player->nextCheckpointIdx - 1,
          player->checkpointTimes[player->nextCheckpointIdx - 1],
          player->activeRunName)
          .serialize());
}

void ETJump::TimerunV2::stopTimer(const std::string &runName, int clientNum,
                                  int currentTimeMs) {
  Player *player = _players[clientNum].get();

  if (player == nullptr) {
    return;
  }

  if (!player->running || player->activeRunName != runName) {
    return;
  }

  auto millis = currentTimeMs - player->startTime.value();
  player->completionTime = opt<int>(millis);

  if (!g_cheats.integer && !isDebugging(clientNum)) {
    checkRecord(player);
  }

  player->running = false;

  Printer::SendCommandToAll(
      TimerunCommands::Stop(clientNum, millis, player->activeRunName)
          .serialize());

  player->activeRunName = "";
  Utilities::stopRun(clientNum);
}

class AddSeasonResult : public ETJump::SynchronizationContext::ResultBase {
public:
  explicit AddSeasonResult(std::string message) : message(std::move(message)) {}

  std::string message;
};

// do not pass by ref! postTask runs asynchronously and passing by
// ref will lead to undefined behavior
// NOLINTNEXTLINE(performance-unnecessary-value-param)
void ETJump::TimerunV2::addSeason(Timerun::AddSeasonParams season) {
  _sc->postTask(
      [this, season]() {
        try {
          _repository->addSeason(season);
          updateSeasonStates();
          return std::make_unique<AddSeasonResult>(
              stringFormat("Successfully added season `%s`", season.name));
        } catch (const std::runtime_error &e) {
          return std::make_unique<AddSeasonResult>(e.what());
        }
      },
      [this,
       season](std::unique_ptr<SynchronizationContext::ResultBase> result) {
        auto addSeasonResult = dynamic_cast<AddSeasonResult *>(result.get());

        Printer::SendConsoleMessage(season.clientNum,
                                    addSeasonResult->message + "\n");
      },
      [this, season](const std::runtime_error &e) {
        const char *what = e.what();
        Printer::SendConsoleMessage(
            season.clientNum,
            stringFormat("Unable to add season: %s\n", e.what()));
      });
}

class EditSeasonResult : public ETJump::SynchronizationContext::ResultBase {
public:
  explicit EditSeasonResult(std::string message)
      : message(std::move(message)) {}

  std::string message;
};

// do not pass by ref! postTask runs asynchronously and passing by
// ref will lead to undefined behavior
// NOLINTNEXTLINE(performance-unnecessary-value-param)
void ETJump::TimerunV2::editSeason(Timerun::EditSeasonParams params) {
  _sc->postTask(
      [this, params]() {
        try {
          _repository->editSeason(params);
          updateSeasonStates();
          return std::make_unique<EditSeasonResult>(
              stringFormat("Successfully edited season `%s`", params.name));
        } catch (const std::runtime_error &e) {
          return std::make_unique<EditSeasonResult>(e.what());
        }
      },
      [this, params](auto r) {
        auto editSeasonResult = dynamic_cast<EditSeasonResult *>(r.get());
        Printer::SendConsoleMessage(params.clientNum,
                                    editSeasonResult->message + "\n");
      },
      [this, params](const std::runtime_error &e) {
        Printer::SendConsoleMessage(
            params.clientNum,
            stringFormat("Unable to edit season: %s\n", e.what()));
      });
}

void ETJump::TimerunV2::interrupt(int clientNum) {
  Player *player = _players[clientNum].get();

  if (player == nullptr || !player->running) {
    return;
  }

  player->running = false;
  player->activeRunName = "";

  Utilities::stopRun(clientNum);
  Printer::SendCommandToAll(TimerunCommands::Interrupt(clientNum).serialize());
}

void ETJump::TimerunV2::connectNotify(int clientNum) {
  for (int idx = 0; idx < MAX_CLIENTS; ++idx) {
    auto player = _players[idx].get();
    if (player && player->activeRunName.length() > 0) {
      auto previousRecord =
          player->getRecord(defaultSeasonId, player->activeRunName);

      int fastestCompletionTime = -1;
      if (previousRecord) {
        fastestCompletionTime = previousRecord->time;
      }

      std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints{};
      checkpoints.fill(TIMERUN_CHECKPOINT_NOT_SET);

      if (player->overriddenCheckpoints.count(player->activeRunName)) {
        checkpoints = player->overriddenCheckpoints[player->activeRunName];
      } else {
        if (previousRecord) {
          checkpoints = toCheckpointsArray(&previousRecord->checkpoints);
        } else {
          checkpoints = player->checkpointTimes;
        }
      }

      Printer::SendCommand(
          clientNum, TimerunCommands::Start(
                         idx, player->startTime.value(), player->activeRunName,
                         fastestCompletionTime, player->runHasCheckpoints,
                         checkpoints, player->checkpointTimes)
                         .serialize());
    }
  }
}

class PrintRecordsResult : public ETJump::SynchronizationContext::ResultBase {
public:
  PrintRecordsResult(std::vector<ETJump::Timerun::Record> records,
                     std::vector<ETJump::Timerun::Season> seasons)
      : records(std::move(records)), seasons(std::move(seasons)) {}

  std::vector<ETJump::Timerun::Record> records;
  std::vector<ETJump::Timerun::Season> seasons;
};

std::string rankToString(int rank) {
  switch (rank) {
    case 1:
      return "^3#1";
    case 2:
      return "^z#2";
    case 3:
      return "^l#3";
    default:
      return "^7#" + std::to_string(rank);
  }
}

// do not pass by ref! postTask runs asynchronously and passing by
// ref will lead to undefined behavior
// NOLINTNEXTLINE(performance-unnecessary-value-param)
void ETJump::TimerunV2::printRecords(Timerun::PrintRecordsParams params) {
  _sc->postTask(
      [this, params] {
        auto records = _repository->getRecords(params);
        auto seasons =
            _repository->getSeasonsForName(params.season.value(), false);
        return std::make_unique<PrintRecordsResult>(std::move(records),
                                                    std::move(seasons));
      },
      [this, params](auto p) {
        auto result = dynamic_cast<PrintRecordsResult *>(p.get());

        if (result->records.empty()) {
          Printer::SendConsoleMessage(params.clientNum,
                                      "No records available.\n");
          return;
        }

        std::map<int, Timerun::Season> seasonIdToName{};
        for (const auto &s : result->seasons) {
          seasonIdToName[s.id] = s;
        }

        std::string message;

        using SeasonId = int;
        using MapName = std::string;
        using RunName = std::string;

        std::map<
            SeasonId,
            std::map<MapName,
                     std::map<RunName, std::vector<const Timerun::Record *>>>>
            processedRecords;
        std::map<SeasonId, std::map<MapName, std::map<RunName, int>>>
            ownRecords;

        for (const auto &r : result->records) {
          if (processedRecords.count(r.seasonId) == 0) {
            processedRecords[r.seasonId] = {};
            ownRecords[r.seasonId] = {};
          }

          if (processedRecords[r.seasonId].count(r.map) == 0) {
            processedRecords[r.seasonId][r.map] = {};
            ownRecords[r.seasonId][r.map] = {};
          }

          if (processedRecords[r.seasonId][r.map].count(r.run) == 0) {
            processedRecords[r.seasonId][r.map][r.run] =
                std::vector<const Timerun::Record *>();
          }

          if (r.userId == params.userId) {
            ownRecords[r.seasonId][r.map][r.run] = r.time;
          }
          processedRecords[r.seasonId][r.map][r.run].push_back(&r);
        }

        for (const auto &skvp : processedRecords) {
          auto seasonIt = seasonIdToName.find(skvp.first);
          const Timerun::Season *season = &seasonIt->second;

          if (seasonIt->first == defaultSeasonId) {
            message += stringFormat("^2Overall records for map ^7%s\n",
                                    result->records[0].map);
          } else {
            message += stringFormat("^2Records for map ^7%s ^2on season ^7%s\n",
                                    result->records[0].map, season->name);
          }

          for (const auto &mkvp : skvp.second) {
            auto mapName = mkvp.first;
            for (const auto &rkvp : mkvp.second) {
              auto runName = rkvp.first;
              int rank = 1;
              int rank1Time = !rkvp.second.empty() ? rkvp.second[0]->time : 0;
              int ownTime = ownRecords[season->id][mapName].count(runName) > 0
                                ? ownRecords[season->id][mapName][runName]
                                : rank1Time;
              // clang-format off
              message += "^g-------------------------------------------------------------\n";
              // clang-format on
              message += stringFormat(" ^2Run: ^7%s\n\n", rkvp.first);

              const int rankWidth = 4;
              const int timeWidth = 10;
              const int diffWidth = 11;

              message += "^g Rank Time       Difference  Player\n";
              std::string ownRecordString;
              for (const auto &r : rkvp.second) {
                auto isOnVisiblePage =
                    rank > (params.page - 1) * params.pageSize &&
                    rank <= (params.page) * params.pageSize;
                auto isOwnRecord = r->userId == params.userId;

                if (isOnVisiblePage || isOwnRecord) {
                  auto ownRecord = r->userId == params.userId;
                  auto rankString = rankToString(rank);
                  auto millisString = millisToString(r->time);
                  std::string diffString;
                  if (ownRecord || rank == 1 && ownTime == r->time) {
                    diffString = "";
                  } else {
                    diffString = diffToString(ownTime, r->time);
                  }
                  auto playerNameString =
                      ownRecord ? r->playerName + " ^g(You)" : r->playerName;

                  auto rankPadding =
                      rankWidth + StringUtil::countExtraPadding(rankString);
                  auto millisPadding =
                      timeWidth + StringUtil::countExtraPadding(millisString);
                  auto diffPadding =
                      diffWidth + StringUtil::countExtraPadding(diffString);

                  auto formatString =
                      stringFormat("^7 %%-%ds^7 %%-%ds^7 %%-%ds^7 %%s^7\n",
                                   rankPadding, millisPadding, diffPadding);

                  // we want to print our own record as the last one if it's not
                  // visible
                  if (isOnVisiblePage) {
                    message +=
                        stringFormat(formatString, rankString, millisString,
                                     diffString, playerNameString);
                  } else {
                    ownRecordString =
                        stringFormat(formatString, rankString, millisString,
                                     diffString, playerNameString);
                  }
                }
                rank++;
              }

              if (ownRecordString.length() > 0) {
                message += "\n" + ownRecordString;
              }
            }
          }
        }

        Printer::SendConsoleMessage(params.clientNum, message);
      },
      [this, params](const std::runtime_error &e) {
        Printer::SendConsoleMessage(params.clientNum,
                                    e.what() + std::string("\n"));
      });
}

class LoadCheckpointsResult
    : public ETJump::SynchronizationContext::ResultBase {
public:
  explicit LoadCheckpointsResult(std::string matchedRun,
                                 std::vector<int> checkpoints)
      : matchedRun(std::move(matchedRun)), checkpoints(std::move(checkpoints)) {
  }

  std::string matchedRun;
  std::vector<int> checkpoints;
};

// do not pass by ref! postTask runs asynchronously and passing by
// ref will lead to undefined behavior
// NOLINTBEGIN(performance-unnecessary-value-param)
void ETJump::TimerunV2::loadCheckpoints(int clientNum, std::string mapName,
                                        std::string runName, int rank) {
  // NOLINTEND(performance-unnecessary-value-param)
  _sc->postTask(
      [this, clientNum, mapName, runName, rank] {
        std::string matchedRun;
        const std::string sanitizedRunName = sanitize(runName, true);
        std::string errMsg;
        int matchedCount = 0;
        const auto matchedRuns =
            _repository->getRunsForName(mapName, runName, false, true);
        auto it = std::find(matchedRuns.cbegin(), matchedRuns.cend(), runName);

        if (it != matchedRuns.cend()) {
          matchedRun = *it;
        } else {
          // exact match not found, try partial match with sanitized name
          for (const auto &run : matchedRuns) {
            if (run.find(sanitizedRunName) != std::string::npos) {
              matchedCount++;
              matchedRun = run;
            }
          }
        }

        if (matchedCount > 1) {
          const int runsPerRow = 3;
          int runsOnCurrentRow = 0;
          errMsg = stringFormat("Multiple matches found for run ^3`%s`^7.\n",
                                runName);

          for (const auto &matches : matchedRuns) {
            ++runsOnCurrentRow;
            if (runsOnCurrentRow > runsPerRow) {
              runsOnCurrentRow = 1;
              errMsg += stringFormat("\n%-16s", sanitize(matches, false));
            } else {
              errMsg += stringFormat("%-16s", sanitize(matches, false));
            }
          }
          throw std::runtime_error(errMsg);
        }
        if (rank == -1) {
          _players[clientNum]->overriddenCheckpoints = {};
          // not really a runtime error but can't return here so bleh
          throw std::runtime_error(stringFormat(
              "^7Cleared loaded checkpoints for run ^3`%s`", matchedRun));
        }

        const auto record = _repository->getRecord(mapName, matchedRun, rank);

        if (!record.hasValue()) {
          throw std::runtime_error(stringFormat(
              "^7Could not find a record on map ^3`%s` ^7for run ^3`%s` ^7for "
              "rank ^3`%d`",
              mapName, matchedRun.empty() ? runName : matchedRun, rank));
        }

        return std::make_unique<LoadCheckpointsResult>(
            matchedRun, record.value().checkpoints);
      },
      [this, clientNum, rank](auto r) {
        const auto result = dynamic_cast<LoadCheckpointsResult *>(r.get());
        const auto runName = result->matchedRun;

        // bail out if no checkpoints are present
        if (result->checkpoints[0] == TIMERUN_CHECKPOINT_NOT_SET) {
          throw std::runtime_error(
              stringFormat("^7No checkpoint times found for run ^3`%s`^7 "
                           "from rank ^3`%d`^7 record.",
                           runName, rank));
        }

        _players[clientNum]->overriddenCheckpoints[runName] = {};

        auto checkpointsToCopy = std::min(
            result->checkpoints.size(),
            _players[clientNum]->overriddenCheckpoints[runName].size());

        std::copy_n(begin(result->checkpoints), checkpointsToCopy,
                    begin(_players[clientNum]->overriddenCheckpoints[runName]));

        Printer::SendConsoleMessage(
            clientNum, stringFormat("^7Loaded checkpoints for run ^3`%s`^7 "
                                    "from rank ^3`%d`^7 record.\n",
                                    runName, rank));

        if (_players[clientNum]->running) {
          Printer::SendConsoleMessage(clientNum,
                                      "^7You need to restart the run for the "
                                      "changes to take effect.\n");
        }
      },
      [clientNum](auto e) {
        Printer::SendConsoleMessage(clientNum, e.what() + std::string("\n"));
      });
}

class PrintResult : public ETJump::SynchronizationContext::ResultBase {
public:
  explicit PrintResult(std::string message) : message(std::move(message)) {}

  std::string message;
};

std::string ETJump::TimerunV2::getRankingsStringFor(
    const std::vector<Ranking> *rankings,
    const Timerun::PrintRankingsParams &params) {
  std::string message;
  message += "^gRank  Player                                      Score\n";
  for (size_t i = 0, len = rankings->size(); i < len; ++i) {
    unsigned rank = i + 1;
    const auto *r = &(*rankings)[i];
    auto isOnVisiblePage = rank > (params.page) * params.pageSize &&
                           rank <= (params.page + 1) * params.pageSize;
    auto isOwnRanking = r->userId == params.userId;

    if (isOnVisiblePage) {
      auto rankString = rankToString(i + 1);
      auto rankStringWidth = 5 + StringUtil::countExtraPadding(rankString);

      auto name = isOwnRanking ? (r->name + " ^g(You)") : r->name;
      auto nameStringWidth =
          MAX_NAME_LENGTH + 1 + 5 + StringUtil::countExtraPadding(name);

      std::string formatString = stringFormat("^7%%-%ds ^7%%-%ds  ^7%%.0f\n",
                                              rankStringWidth, nameStringWidth);

      message += stringFormat(formatString, rankString, name, r->score);
    }

    if (isOwnRanking && !isOnVisiblePage) {
      auto rankString = rankToString(i + 1);
      auto rankStringWidth = 5 + StringUtil::countExtraPadding(rankString);
      auto name = isOwnRanking ? (r->name + " ^g(You)") : r->name;
      auto nameStringWidth =
          MAX_NAME_LENGTH + 1 + 5 + StringUtil::countExtraPadding(name);

      std::string formatString = stringFormat("\n^7%%-%ds ^7%%-%ds  ^7%%.0f\n",
                                              rankStringWidth, nameStringWidth);

      message += stringFormat(formatString, rankString, name, r->score);
    }
  }
  return message;
}

// do not pass by ref! postTask runs asynchronously and passing by
// ref will lead to undefined behavior
// NOLINTNEXTLINE(performance-unnecessary-value-param)
void ETJump::TimerunV2::printRankings(Timerun::PrintRankingsParams params) {
  _sc->postTask(
      [this, params] {
        std::string message;
        if (params.season.hasValue()) {
          auto matchingSeasons =
              _repository->getSeasonsForName(params.season.value(), false);

          if (matchingSeasons.empty()) {
            message = stringFormat("No matching season for name `%s`",
                                   params.season.value());
          } else {
            for (const auto &s : matchingSeasons) {
              if (_rankingsPerSeason.count(s.id) == 0) {
                message = stringFormat("No records for season `%s`", s.name);
              } else {
                // clang-format off
                message =
                    stringFormat(
                        "^g=============================================================\n"
                        " ^gRankings for season: ^2%s^7\n"
                        "^g=============================================================\n",
                        s.name);
                // clang-format on
                message += this->getRankingsStringFor(&_rankingsPerSeason[s.id],
                                                      params);
              }
            }
          }

        } else {
          if (_rankingsPerSeason.count(defaultSeasonId) == 0) {
            message += "No overall records";
          } else {
            // clang-format off
            message =
                "^g=============================================================\n"
                " ^gOverall rankings^7\n"
                "^g=============================================================\n";
            // clang-format on
            message += this->getRankingsStringFor(
                &_rankingsPerSeason[defaultSeasonId], params);
          }
        }

        return std::make_unique<PrintResult>(message);
      },
      [this, params](auto r) {
        auto result = dynamic_cast<PrintResult *>(r.get());

        Printer::SendConsoleMessage(params.clientNum, result->message);
      },
      [params](auto e) {
        Printer::SendConsoleMessage(
            params.clientNum,
            stringFormat("Unable to print rankings: %s\n", e.what()));
      });
}

// do not pass by ref! postTask runs asynchronously and passing by
// ref will lead to undefined behavior
// NOLINTNEXTLINE(performance-unnecessary-value-param)
void ETJump::TimerunV2::printSeasons(int clientNum) {
  _sc->postTask(
      [this] {
        // 1 active season means only default season is active
        if (_activeSeasonsIds.size() == 1 && _upcomingSeasonsIds.empty() &&
            _pastSeasonsIds.empty()) {
          return std::make_unique<PrintResult>("No seasons found.\n");
        }

        std::string seasonHeader = stringFormat(
            "\n^g %-30s %-15s%s\n", "Season", "Start Date", "End date");

        // clang-format off
        std::string message =
            "\n ^2Seasons^7\n"
            "^g------------------------------------------------------------\n";
        // clang-format on

        if (_activeSeasonsIds.size() > 1) {
          message += " ^gActive seasons\n" + seasonHeader;

          for (const auto &s : _activeSeasons) {
            // default season is always active and cannot be deleted,
            // don't bother listing it
            if (s.id == defaultSeasonId) {
              continue;
            }

            std::string formatString =
                stringFormat(" ^7%%-%ds ^7%%-15s%%s\n",
                             30 + StringUtil::countExtraPadding(s.name));

            message += stringFormat(
                formatString, s.name, s.startTime.toAbbrevMonthDateString(),
                s.endTime.hasValue()
                    ? s.endTime.value().toAbbrevMonthDateString()
                    : "*");
          }
        }

        if (!_upcomingSeasonsIds.empty()) {
          // clang-format off
          message += "\n^g------------------------------------------------------------\n";
          // clang-format on
          message += " ^gUpcoming seasons\n" + seasonHeader;

          for (const auto &s : _upcomingSeasons) {
            std::string formatString =
                stringFormat(" ^7%%-%ds ^7%%-15s%%s\n",
                             30 + StringUtil::countExtraPadding(s.name));

            message += stringFormat(
                formatString, s.name, s.startTime.toAbbrevMonthDateString(),
                s.endTime.hasValue()
                    ? s.endTime.value().toAbbrevMonthDateString()
                    : "*");
          }
        }

        if (!_pastSeasonsIds.empty()) {
          // clang-format off
          message += "\n^g------------------------------------------------------------\n";
          // clang-format on
          message += " ^gPast seasons\n" + seasonHeader;

          for (const auto &s : _pastSeasons) {
            std::string formatString =
                stringFormat(" ^9%%-%ds ^9%%-15s%%s\n",
                             30 + StringUtil::countExtraPadding(s.name));

            message += stringFormat(
                formatString, s.name, s.startTime.toAbbrevMonthDateString(),
                s.endTime.hasValue()
                    ? s.endTime.value().toAbbrevMonthDateString()
                    : "*");
          }
        }

        return std::make_unique<PrintResult>(message);
      },
      [clientNum](auto r) {
        auto result = dynamic_cast<PrintResult *>(r.get());

        Printer::SendConsoleMessage(clientNum, result->message);
      },
      [clientNum](auto e) {
        Printer::SendConsoleMessage(
            clientNum, stringFormat("Unable to print seasons: %s\n", e.what()));
      });
}

class DeleteSeasonResult : public ETJump::SynchronizationContext::ResultBase {
public:
  explicit DeleteSeasonResult(std::string message)
      : message(std::move(message)) {}

  std::string message;
};

void ETJump::TimerunV2::deleteSeason(int clientNum, const std::string &name) {
  _sc->postTask(
      [this, name]() {
        try {
          _repository->deleteSeason(name);
          updateSeasonStates();
          return std::make_unique<DeleteSeasonResult>(
              stringFormat("Successfully deleted season `%s`", name));
        } catch (const std::runtime_error &e) {
          return std::make_unique<DeleteSeasonResult>(e.what());
        }
      },
      [this,
       clientNum](std::unique_ptr<SynchronizationContext::ResultBase> result) {
        auto deleteSeasonResult =
            dynamic_cast<DeleteSeasonResult *>(result.get());

        Printer::SendConsoleMessage(clientNum,
                                    deleteSeasonResult->message + "\n");
      },
      [this, clientNum](const std::runtime_error &e) {
        const char *what = e.what();
        Printer::SendConsoleMessage(
            clientNum, stringFormat("Unable to delete season: %s\n", e.what()));
      });
}

void ETJump::TimerunV2::startNotify(Player *player) const {
  auto spectators = Utilities::getSpectators(player->clientNum);
  auto previousRecord =
      player->getRecord(defaultSeasonId, player->activeRunName);

  int fastestCompletionTime = -1;
  if (previousRecord) {
    fastestCompletionTime = previousRecord->time;
  }

  std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints{};
  checkpoints.fill(TIMERUN_CHECKPOINT_NOT_SET);

  if (player->overriddenCheckpoints.count(player->activeRunName)) {
    checkpoints = player->overriddenCheckpoints[player->activeRunName];
  } else if (previousRecord) {
    checkpoints = toCheckpointsArray(&previousRecord->checkpoints);
  } else {
    checkpoints = player->checkpointTimes;
  }

  Printer::SendCommandToAll(
      TimerunCommands::Start(player->clientNum, player->startTime.value(),
                             player->activeRunName, fastestCompletionTime,
                             player->runHasCheckpoints, checkpoints,
                             player->checkpointTimes)
          .serialize());
}

bool ETJump::TimerunV2::isDebugging(int clientNum) {
  std::vector<std::string> debuggers;

  if (g_debugTimeruns.integer > 0) {
    debuggers.emplace_back("Timerun");
  }

  if (g_debugTrackers.integer > 0) {
    debuggers.emplace_back("Tracker");
  }

  if (!debuggers.empty()) {
    Printer::SendPopupMessage(clientNum, "Record not saved:\n");

    for (auto &debugger : debuggers) {
      std::string fmt = stringFormat("- ^3%s ^7debugging enabled.\n", debugger);
      Printer::SendPopupMessage(clientNum, fmt);
    }

    return true;
  }

  return false;
}

int ETJump::TimerunV2::indexForRunname(const std::string &runName) {
  int index;
  std::string currentRun;
  std::string activeRun = sanitize(runName, true);

  for (index = 0; index < level.timerunNamesCount; index++) {
    currentRun = sanitize(level.timerunNames[index], true);
    if (currentRun == activeRun) {
      break;
    }
  }

  return index;
}

class CheckRecordResult : public ETJump::SynchronizationContext::ResultBase {
public:
  struct NewRecord {
    ETJump::Timerun::Record record;
    std::string seasonName;
    ETJump::opt<int> previousTime;
  };

  CheckRecordResult() : clientNum(-1) {}

  int clientNum;
  // Most relevant season record
  std::map<int, bool> isTopRecordPerSeason{};
  std::map<int, NewRecord> newOwnRecordsPerSeason{};

  // our previous overall record
  ETJump::opt<ETJump::Timerun::Record> playerPreviousOverallRecord;
  // our previous seasonal record
  ETJump::opt<ETJump::Timerun::Record> playerPreviousSeasonalRecord;

  // previous overall record
  ETJump::opt<ETJump::Timerun::Record> previousOverallRecord;
  // previous seasonal record
  ETJump::opt<ETJump::Timerun::Record> previousSeasonalRecord;
};

void ETJump::TimerunV2::checkRecord(Player *player) {
  const auto clientNum = player->clientNum;
  const auto activeRunName = player->activeRunName;
  const auto userId = player->userId;
  const auto completionTime = player->completionTime.value();
  const std::map<std::string, std::string> metadata = {
      {"mod_version", GAME_VERSION}};
  const std::string playerName = player->name;
  const auto checkpoints =
      Container::map(player->checkpointTimes, [](int time) { return time; });

  _sc->postTask(
      [this, activeRunName, userId, completionTime, playerName, metadata,
       checkpoints, clientNum]() {
        /*
         * We want to check the record for all seasons.
         *
         * If player makes a new overall record, it is broadcasted
         * If player makes a new relevant season record and there's no overall
         * record, it is broadcasted If player makes a new season record, it is
         * printed for the player but not for others
         */

        const auto topRecords = _repository->getTopRecords(
            _activeSeasonsIds, _currentMap, activeRunName);

        std::map<int, Timerun::Record> topRecordForSeason{};

        // get the top times of the run we just finished for active seasons
        for (const auto &tr : topRecords) {
          topRecordForSeason[tr.seasonId] = tr;
        }

        auto playerRecords = _repository->getRecordsForPlayer(
            _activeSeasonsIds, _currentMap, activeRunName, userId);

        std::map<int, const Timerun::Record *> playerTopRecordForSeason{};

        // get our previous top times of the run we just finished
        // for active seasons
        for (const auto &r : playerRecords) {
          playerTopRecordForSeason[r.seasonId] = &r;
        }

        std::map<int, Timerun::Record> playerNewTopRecordForSeason{};
        std::map<int, bool> isNewRecordForSeason{};

        // go through active seasons and check if we beat any of the old records
        // if we did, we store the record in playerNewTopRecordForSeason
        // this only checks against our own personal previous times,
        // not against other player's times
        for (auto seasonId : _activeSeasonsIds) {
          Timerun::Record record;
          record.seasonId = seasonId;
          record.map = _currentMap;
          record.run = activeRunName;
          record.userId = userId;
          record.time = completionTime;
          record.checkpoints = checkpoints;
          record.recordDate = getCurrentTime();
          record.playerName = playerName;
          record.metadata = metadata;

          auto insert = false;
          auto isNewRecord = false;
          if (playerTopRecordForSeason.count(seasonId) == 0) {
            isNewRecord = true;
            insert = true;
          } else {
            insert = false;
            isNewRecord =
                completionTime < playerTopRecordForSeason[seasonId]->time;
          }
          isNewRecordForSeason[seasonId] = isNewRecord;

          if (isNewRecord) {
            if (insert) {
              _repository->insertRecord(record);
            } else {
              _repository->updateRecord(record);
            }
            playerNewTopRecordForSeason[seasonId] = std::move(record);
          }
        }

        auto result = std::make_unique<CheckRecordResult>();
        result->clientNum = clientNum;

        // go through old records and update any records we might have beaten
        for (const auto &s : isNewRecordForSeason) {
          if (!s.second) {
            continue;
          }

          auto seasonId = s.first;

          CheckRecordResult::NewRecord record{};

          for (const auto &activeSeason : _activeSeasons) {
            if (seasonId == activeSeason.id) {
              record.seasonName = activeSeason.name;
              break;
            }
          }

          record.previousTime = playerTopRecordForSeason.count(seasonId) > 0
                                    ? playerTopRecordForSeason[seasonId]->time
                                    : opt<int>();
          record.record = playerNewTopRecordForSeason[seasonId];

          result->newOwnRecordsPerSeason[seasonId] = record;

          // see if we beat the previous record time for this season
          auto topRecordIt = topRecordForSeason.find(seasonId);
          if (topRecordIt != end(topRecordForSeason)) {
            auto topTime = topRecordIt->second.time;

            // store the old record
            if (seasonId == defaultSeasonId) {
              result->previousOverallRecord = topRecordIt->second;
            } else {
              result->previousSeasonalRecord = topRecordIt->second;
            }

            if (record.record.time < topTime) {
              result->isTopRecordPerSeason[seasonId] = true;
            }
          } else {
            result->isTopRecordPerSeason[seasonId] = true;
          }
        }

        for (const auto &seasonId : _activeSeasonsIds) {
          if (result->isTopRecordPerSeason.count(seasonId) == 0) {
            result->isTopRecordPerSeason[seasonId] = false;
          }
        }

        auto playerPreviousOverallRecordIt =
            playerTopRecordForSeason.find(defaultSeasonId);

        // store our own previous overall record
        if (playerPreviousOverallRecordIt != end(playerTopRecordForSeason)) {
          result->playerPreviousOverallRecord =
              *playerPreviousOverallRecordIt->second;
        }

        return std::move(result);
      },
      [this, completionTime, activeRunName, playerName,
       clientNum](std::unique_ptr<SynchronizationContext::ResultBase> result) {
        const auto checkRecordResult =
            dynamic_cast<CheckRecordResult *>(result.get());
        bool isNewRecord = false;

        std::string diffString;
        int previousTopRecordTime;

        const int playerPreviousRecord =
            checkRecordResult->playerPreviousOverallRecord.hasValue()
                ? checkRecordResult->playerPreviousOverallRecord.value().time
                : 0;

        // Overall record
        if (checkRecordResult->isTopRecordPerSeason[defaultSeasonId]) {
          const auto record =
              checkRecordResult->newOwnRecordsPerSeason[defaultSeasonId];

          previousTopRecordTime =
              checkRecordResult->previousOverallRecord.hasValue()
                  ? checkRecordResult->previousOverallRecord.value().time
                  : 0;

          // print banner only if there was an existing overall record
          if (completionTime < previousTopRecordTime) {
            diffString = "^7(" +
                         diffToString(completionTime, previousTopRecordTime) +
                         "^7)";

            Printer::BroadCastBannerMessage(stringFormat(
                // clang-format off
              "^7%s ^7broke the overall server record for ^3%s\n^7with ^3%s %s ^7!!!\n",
                // clang-format on
                playerName, sanitize(record.record.run),
                millisToString(record.record.time), diffString));
          }
          Printer::SendCommandToAll(
              TimerunCommands::Record(clientNum, record.record.time,
                                      record.previousTime.valueOr(
                                          TimerunCommands::NO_PREVIOUS_RECORD),
                                      record.record.run)
                  .serialize());
          isNewRecord = true;
        }
        // Relevant season record
        else if (checkRecordResult
                     ->isTopRecordPerSeason[_mostRelevantSeason->id]) {
          const auto record =
              checkRecordResult
                  ->newOwnRecordsPerSeason[_mostRelevantSeason->id];

          previousTopRecordTime =
              checkRecordResult->previousSeasonalRecord.hasValue()
                  ? checkRecordResult->previousSeasonalRecord.value().time
                  : 0;

          // print banner only if there was an existing seasonal record
          if (completionTime < previousTopRecordTime) {
            diffString = "^7(" +
                         diffToString(completionTime, previousTopRecordTime) +
                         "^7)";

            Printer::BroadCastBannerMessage(stringFormat(
                // clang-format off
              "^7%s ^7broke the server record on ^3%s^7 season for ^3%s\n^7with ^3%s %s ^7!!!\n",
                // clang-format on
                playerName, record.seasonName, sanitize(record.record.run),
                millisToString(record.record.time), diffString));
          }

          // Send a record only if we beat our old PB
          if (completionTime < playerPreviousRecord) {
            Printer::SendCommandToAll(
                TimerunCommands::Record(
                    clientNum, record.record.time,
                    record.previousTime.valueOr(
                        TimerunCommands::NO_PREVIOUS_RECORD),
                    record.record.run)
                    .serialize());
            isNewRecord = true;
          }
        }

        // By default, print the other seasons to console
        for (const auto &record : checkRecordResult->newOwnRecordsPerSeason) {
          const auto seasonId = record.first;
          if (seasonId == defaultSeasonId ||
              seasonId == _mostRelevantSeason->id) {
            continue;
          }

          if (record.second.previousTime.hasValue()) {
            diffString = "^7(" +
                         diffToString(record.second.record.time,
                                      record.second.previousTime.value()) +
                         "^7)";
          }
          Printer::SendConsoleMessage(
              checkRecordResult->clientNum,
              stringFormat("^7New personal record on season ^3%s^7 for ^7^3%s "
                           "^7with ^3%s %s^7!\n",
                           record.second.seasonName, record.second.record.run,
                           millisToString(record.second.record.time),
                           diffString));
        }

        for (const auto &newRecord :
             checkRecordResult->newOwnRecordsPerSeason) {
          bool foundCachedRecord = false;

          for (auto &cachedRecord : _players[clientNum]->records) {
            if (cachedRecord.isSameRunAs(&newRecord.second.record)) {
              cachedRecord = newRecord.second.record;
              foundCachedRecord = true;
            }
          }

          if (!foundCachedRecord) {
            _players[clientNum]->records.push_back(newRecord.second.record);
          }
        }

        // send a completion if we didn't make a new record,
        // but had a previous record set for this run
        // otherwise the player didn't have a time set on the run at all,
        // so we should send a record so autodemo can save the demo
        if (!isNewRecord) {
          if (playerPreviousRecord && playerPreviousRecord < completionTime) {
            Printer::SendCommandToAll(
                TimerunCommands::Completion(
                    clientNum, completionTime,
                    checkRecordResult->playerPreviousOverallRecord.hasValue()
                        ? checkRecordResult->playerPreviousOverallRecord.value()
                              .time
                        : opt<int>(),
                    activeRunName)
                    .serialize());
          } else {
            Printer::SendCommandToAll(
                TimerunCommands::Record(
                    clientNum, completionTime,
                    checkRecordResult->playerPreviousOverallRecord.hasValue()
                        ? checkRecordResult->playerPreviousOverallRecord.value()
                              .time
                        : opt<int>(),
                    activeRunName)
                    .serialize());
          }
        }
      },
      [this, &activeRunName, &completionTime,
       clientNum](const std::runtime_error &e) {
        _logger->error("Unable to check the record (%s/%s/%d) for %s: %s",
                       _currentMap, activeRunName, completionTime, e.what());

        Printer::SendChatMessage(
            clientNum, "Unable to process your timerun record. Please report "
                       "this as a bug at github.com/etjump/etjump.");
      });
}

std::array<int, MAX_TIMERUN_CHECKPOINTS>
ETJump::TimerunV2::toCheckpointsArray(const std::vector<int> *input) {
  std::array<int, MAX_TIMERUN_CHECKPOINTS> arr{};
  arr.fill(TIMERUN_CHECKPOINT_NOT_SET);

  auto checkpointsToCopy = std::min(input->size(), arr.size());

  std::copy_n(begin(*input), checkpointsToCopy, begin(arr));

  return arr;
}

const ETJump::Timerun::Season *ETJump::TimerunV2::getMostRelevantSeason() {
  // Most relevant = Most recently started
  const Timerun::Season *mostRelevant = &_activeSeasons[0];
  for (const auto &season : _activeSeasons) {
    if (mostRelevant->startTime < season.startTime) {
      mostRelevant = &season;
    }
  }
  return mostRelevant;
}
