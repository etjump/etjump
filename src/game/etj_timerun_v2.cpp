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
#include "etj_log.h"
#include "etj_printer.h"
#include "etj_synchronization_context.h"
#include "etj_timerun_repository.h"

ETJump::TimerunV2::TimerunV2(
    std::string currentMap, std::unique_ptr<TimerunRepository> repository,
    std::unique_ptr<Log> logger,
    std::unique_ptr<SynchronizationContext> synchronizationContext)
  : _currentMap(std::move(currentMap)), _repository(std::move(repository)),
    _logger(std::move(logger)),
    _sc(std::move(synchronizationContext)) {
}

std::string millisToString(int millis);
std::string diffToString(int selfTime, int otherTime);

const ETJump::Timerun::Record *ETJump::TimerunV2::Player::getRecord(
    const std::string &runName) const {
  for (const auto &r : records) {
    if (r.run == runName) {
      return &r;
    }
  }
  return nullptr;
}

void ETJump::TimerunV2::initialize() {
  try {
    _activeSeasonsIds = std::vector<int>();
    _activeSeasons = std::vector<Timerun::Season>();

    _activeSeasons = _repository->getActiveSeasons(getCurrentTime());

    _mostRelevantSeason = getMostRelevantSeason();

    _activeSeasonsIds = Utilities::map(
        _activeSeasons, [](const Timerun::Season &s) {
          return s.id;
        });

    _logger->info(
        "Active seasons: %s",
        StringUtil::join(Utilities::map(_activeSeasons,
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
    : runs(runs) {
  }

  std::vector<ETJump::Timerun::Record> runs;
};

void ETJump::TimerunV2::clientConnect(int clientNum, int userId) {
  _sc->postTask(
      [this, clientNum, userId] {

        auto parameters = StringUtil::join(Utilities::map(_activeSeasonsIds,
                                             [](int season) {
                                               return std::to_string(season);
                                             }),
                                           ", ");

        auto runs = _repository->getRecordsForPlayer(
            _activeSeasonsIds, _currentMap, userId);

        return std::make_unique<ClientConnectResult>(runs);
      },
      [this, clientNum, userId](
      std::unique_ptr<SynchronizationContext::ResultBase> result) {
        auto clientConnectResult =
            static_cast<ClientConnectResult *>(result.get());

        _players[clientNum] = std::make_unique<Player>(
            clientNum, userId, clientConnectResult->runs);
      },
      [this, clientNum, userId](std::runtime_error error) {
        _logger->info("Unable to load player information for clientNum: `%d` "
                      "userId: `%d`: %s",
                      clientNum, userId, error.what());
        Printer::SendChatMessage(
            clientNum, "Unable to load player information. Any timeruns will "
            "not work. Try to reconnect or file a bug report at "
            "github.com/etjump/etjump.");
        Printer::SendConsoleMessage(clientNum,
                                    stringFormat("cause: %s", error.what()));
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
    Printer::SendChatMessage(clientNum,
                             "Unable to start timerun. Reconnect and if this persists, report the bug at github.com/etjump/etjump");
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

  for (auto &cp : player->checkpointTimes) {
    cp = -1;
  }

  startNotify(player);

  Utilities::startRun(clientNum);
}

void ETJump::TimerunV2::checkpoint(const std::string &runName,
                                   int checkpointIndex, int clientNum,
                                   int currentTimeMs) {
  Player *player = _players[clientNum].get();

  if (player == nullptr) {
    return;
  }

  if (!player->running) {
    return;
  }

  player->checkpointTimes[checkpointIndex] =
      currentTimeMs - player->startTime.value();
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

  Printer::SendCommand(clientNum, stringFormat("timerun_stop %d \"%s\"", millis,
                                               player->activeRunName));
  auto spectators = Utilities::getSpectators(clientNum);
  Printer::SendCommand(spectators,
                       stringFormat("timerun_stop_spec %d %d \"%s\"", clientNum,
                                    millis, player->activeRunName));
  Printer::SendCommandToAll(stringFormat("timerun stop %d %d \"%s\"", clientNum,
                                         millis, player->activeRunName));

  player->activeRunName = "";
  Utilities::stopRun(clientNum);
}

class AddSeasonResult : public ETJump::SynchronizationContext::ResultBase {
public:
  AddSeasonResult(std::string message)
    : message(message) {
  }

  std::string message;
};

void ETJump::TimerunV2::addSeason(Timerun::AddSeasonParams season) {
  _sc->postTask([this, season]() {
                  try {
                    _repository->addSeason(season);
                    return std::make_unique<AddSeasonResult>(
                        stringFormat("Successfully added season `%s`",
                                     season.name));
                  } catch (const std::runtime_error &e) {
                    return std::make_unique<AddSeasonResult>(e.what());
                  }
                },
                [this, season](
                std::unique_ptr<SynchronizationContext::ResultBase> result) {

                  auto addSeasonResult =
                      static_cast<AddSeasonResult *>(result.get());

                  Printer::SendConsoleMessage(season.clientNum,
                                              addSeasonResult->message + "\n");
                },
                [this, season](const std::runtime_error &e) {
                  const char *what = e.what();
                  Printer::SendConsoleMessage(
                      season.clientNum,
                      stringFormat("Unable to add season: %s\n", e.what()));
                }
      );
}

void ETJump::TimerunV2::interrupt(int clientNum) {
  Player *player = _players[clientNum].get();

  if (player == nullptr || !player->running) {
    return;
  }

  player->running = false;
  player->activeRunName = "";

  Utilities::stopRun(clientNum);
  Printer::SendCommand(clientNum, "timerun_interrupt");
  Printer::SendCommandToAll(
      stringFormat("timerun interrupt %d", clientNum));
}

void ETJump::TimerunV2::connectNotify(int clientNum) {
  for (int idx = 0; idx < 64; ++idx) {
    auto player = _players[idx].get();
    if (player && player->activeRunName.length() > 0) {
      auto previousRecord = player->getRecord(player->activeRunName);

      int fastestCompletionTime = -1;
      if (previousRecord) {
        fastestCompletionTime = previousRecord->time;
      }
      Printer::SendCommand(clientNum,
                           stringFormat("timerun start %d %d \"%s\" %d", idx,
                                        player->startTime.value(),
                                        player->activeRunName,
                                        fastestCompletionTime));
    }
  }
}

void ETJump::TimerunV2::startNotify(Player *player) {
  auto spectators = Utilities::getSpectators(player->clientNum);
  auto previousRecord = player->getRecord(player->activeRunName);

  int fastestCompletionTime = -1;
  if (previousRecord) {
    fastestCompletionTime = previousRecord->time;
  }
  Printer::SendCommand(
      player->clientNum,
      stringFormat("timerun_start %d \"%s\" %d", player->startTime.value(),
                   player->activeRunName, fastestCompletionTime));
  Printer::SendCommand(
      spectators,
      stringFormat("timerun_start_spec %d %d \"%s\" %d",
                   player->clientNum, player->startTime.value(),
                   player->activeRunName,
                   fastestCompletionTime));
  Printer::SendCommandToAll(stringFormat(
      "timerun start %d %d \"%s\" %d", player->clientNum,
      player->startTime.value(),
      player->activeRunName, fastestCompletionTime));
}

bool ETJump::TimerunV2::isDebugging(int clientNum) {
  std::vector<std::string> debuggers;

  if (g_debugTimeruns.integer > 0) {
    debuggers.push_back("Timerun");
  }

  if (g_debugTrackers.integer > 0) {
    debuggers.push_back("Tracker");
  }

  if (debuggers.size()) {
    Printer::SendPopupMessage(clientNum, "Record not saved:\n");

    for (auto &debugger : debuggers) {
      std::string fmt =
          stringFormat("- ^3%s ^7debugging enabled.\n", debugger);
      Printer::SendPopupMessage(clientNum, fmt);
    }

    return true;
  }

  return false;
}

class CheckRecordResult : public ETJump::SynchronizationContext::ResultBase {
public:
  struct NewRecord {
    ETJump::Timerun::Record record;
    std::string seasonName;
    ETJump::opt<int> previousTime;
  };

  CheckRecordResult()
    : clientNum(-1), isNewTopRecordForRelevantSeason(false) {
  }

  int clientNum;
  // Most relevant season record
  ETJump::opt<NewRecord> newRelevantRecord;
  bool isNewTopRecordForRelevantSeason{};
  ETJump::opt<ETJump::Timerun::Record> topRecordForRelevantSeason;
  // Any other season records
  std::vector<NewRecord> otherNewRecords;
};

void ETJump::TimerunV2::checkRecord(Player *player) {
  auto clientNum = player->clientNum;
  auto activeRunName = player->activeRunName;
  auto userId = player->userId;
  auto completionTime = player->completionTime;
  std::map<std::string, std::string> metadata = {{"mod_version", GAME_VERSION}};
  std::string playerName = player->name;
  auto checkpoints =
      Utilities::map(player->checkpointTimes, [](int time) { return time; });

  _sc->postTask(
      [this, activeRunName, userId, completionTime, playerName, metadata,
        checkpoints,
        clientNum]() {
        auto records = _repository->getRecordsForPlayer(
            _activeSeasonsIds, _currentMap, activeRunName, userId);

        auto topRecord = _repository->getTopRecord(
            _mostRelevantSeason->id, _currentMap, activeRunName);

        std::map<int, Timerun::Record> seasonIdToNewRecord{};
        std::map<int, const Timerun::Record *> seasonIdToPreviousRecord{};
        for (const auto &r : records) {
          seasonIdToPreviousRecord[r.seasonId] = &r;
        }

        std::map<int, bool> seasonIdToIsNewRecord{};
        for (auto seasonId : _activeSeasonsIds) {
          Timerun::Record record;
          record.seasonId = seasonId;
          record.map = _currentMap;
          record.run = activeRunName;
          record.userId = userId;
          record.time = completionTime.value();
          record.checkpoints = checkpoints;
          record.recordDate = getCurrentTime();
          record.playerName = playerName;
          record.metadata = metadata;

          auto insert = false;
          auto isNewRecord = false;
          if (seasonIdToPreviousRecord.count(seasonId) == 0) {
            isNewRecord = true;
            insert = true;
          } else {
            insert = false;
            isNewRecord = completionTime.value() <
                          seasonIdToPreviousRecord[seasonId]->time;
          }
          seasonIdToIsNewRecord[seasonId] = isNewRecord;

          if (isNewRecord) {
            if (insert) {
              _repository->insertRecord(record);
            } else {
              _repository->updateRecord(record);
            }
            seasonIdToNewRecord[seasonId] = std::move(record);
          }
        }

        auto result = std::make_unique<CheckRecordResult>();
        result->clientNum = clientNum;
        result->topRecordForRelevantSeason = topRecord;
        if (seasonIdToIsNewRecord[_mostRelevantSeason->id]) {
          auto seasonName = _mostRelevantSeason->name;
          auto previousTime =
              seasonIdToPreviousRecord.count(_mostRelevantSeason->id) > 0
                ? opt<int>(
                    seasonIdToPreviousRecord[_mostRelevantSeason->id]->time)
                : opt<int>();

          result->newRelevantRecord = opt<CheckRecordResult::NewRecord>(
          {seasonIdToNewRecord[_mostRelevantSeason->id], seasonName,
           previousTime});

          if (!topRecord.hasValue()) {
            result->isNewTopRecordForRelevantSeason = true;
          } else if (completionTime.value() < topRecord.value().time) {
            result->isNewTopRecordForRelevantSeason = true;
            result->topRecordForRelevantSeason = topRecord;
          }
        }

        for (const auto &r : seasonIdToIsNewRecord) {
          if (!r.second) {
            // not a record
            continue;
          }

          if (r.first == _mostRelevantSeason->id) {
            // already handled
            continue;
          }

          std::string seasonName;
          for (const auto &s : _activeSeasons) {
            if (s.id == r.first) {
              seasonName = s.name;
            }
          }

          auto otherSeasonRecord = seasonIdToNewRecord[r.first];
          auto previousTime = seasonIdToPreviousRecord.count(r.first) > 0
                                ? opt<int>(seasonIdToNewRecord[r.first].time)
                                : opt<int>();
          result->otherNewRecords.push_back(CheckRecordResult::NewRecord{
              otherSeasonRecord, seasonName, previousTime});
        }

        return std::move(result);
      }
      ,
      [this, completionTime, playerName, clientNum](
      std::unique_ptr<SynchronizationContext::ResultBase> result) {
        auto checkRecordResult = static_cast<CheckRecordResult *>(result.
          get());

        if (checkRecordResult->newRelevantRecord.hasValue()) {
          if (checkRecordResult->isNewTopRecordForRelevantSeason) {
            auto relRec = &checkRecordResult->newRelevantRecord.value();
            auto seasonName = relRec->seasonName;
            auto runName = sanitize(relRec->record.run);
            auto completionTimeStr = millisToString(
                completionTime.value());
            std::string diffString = "";
            if (checkRecordResult->topRecordForRelevantSeason.
                                   hasValue()) {
              auto diff = diffToString(
                  completionTime.value(),
                  checkRecordResult->topRecordForRelevantSeason.value().
                                     time);
              diffString = stringFormat("^z(%s^z)", diff);
            }

            Printer::BroadCastBannerMessage(ETJump::stringFormat(
                "^7%s ^7broke the server record on season %s for ^3%s\n^7with ^3%s %s"
                "^7!!!\n",
                playerName,
                checkRecordResult->newRelevantRecord.value().seasonName,
                sanitize(
                    checkRecordResult->newRelevantRecord.value().record.
                                       run),
                millisToString(completionTime.value()),
                diffString));
          }

          // refresh the records cache
          for (auto &oldCachedRecord : _players[clientNum]->records) {
            if (oldCachedRecord.seasonId == checkRecordResult->
                                            newRelevantRecord
                                            .value().record.seasonId) {
              oldCachedRecord =
                  checkRecordResult->newRelevantRecord.value().record;

            }
          }
        }

        for (const auto &r : checkRecordResult->otherNewRecords) {
          std::string diffString = "";
          if (r.previousTime.hasValue()) {
            auto diff = diffToString(
                completionTime.value(),
                r.previousTime.value());
            diffString = stringFormat("^z(%s^z)", diff);
          }

          Printer::SendConsoleMessage(
              checkRecordResult->clientNum,
              stringFormat(
                  "^7New personal record on season %s for ^7^3%s "
                  "^7with ^3%s %s^7!\n",
                  r.seasonName, r.record.run,
                  millisToString(r.record.time),
                  diffString));

          // refresh the records cache
          for (auto &oldCachedRecord : _players[clientNum]->records) {
            oldCachedRecord = r.record;
          }
        }
      },
      [this, &activeRunName, &completionTime, clientNum](
      const std::runtime_error &e) {
        _logger->error("Unable to check the record (%s/%s/%d) for %s: %s",
                       _currentMap, activeRunName, completionTime.value(),
                       e.what());

        Printer::SendChatMessage(
            clientNum,
            "Unable to process your timerun record. Please report "
            "this as a bug at github.com/etjump/etjump.");
      }
      );
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