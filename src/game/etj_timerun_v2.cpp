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
  _logger->info("%s started timerun %s at %d", playerName, runName,
                currentTimeMs);

  Printer::SendChatMessage(clientNum,
                           stringFormat("%s started timerun %s at %d",
                                        playerName, runName, currentTimeMs));

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

  for (auto & cp : player->checkpointTimes) {
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

  player->checkpointTimes[checkpointIndex] = currentTimeMs - player->startTime.value();
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
                [this, season](const std::runtime_error& e) {
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

};

void ETJump::TimerunV2::checkRecord(Player *player) {
  auto activeRunName = player->activeRunName;
  auto userId = player->userId;
  auto completionTime = player->completionTime;
  std::map<std::string, std::string> metadata = {{"mod_version", GAME_VERSION}};
  std::string playerName = player->name;

  _sc->postTask(
      [this, activeRunName, userId, completionTime, playerName, metadata]() {
        auto records = _repository->getRecordsForPlayer(
            _activeSeasonsIds, _currentMap, activeRunName, userId);

        std::map<int, const Timerun::Record *> seasonIdToRecord{};
        for (const auto &r : records) {
          seasonIdToRecord[r.seasonId] = &r;
        }

        std::map<int, bool> seasonIdToIsNewRecord{};
        for (auto seasonId : _activeSeasonsIds) {
          Timerun::Record record;
          record.seasonId = seasonId;
          record.map = _currentMap;
          record.run = activeRunName;
          record.userId = userId;
          record.time = completionTime.value();
          record.checkpoints = std::vector<int>();
          record.recordDate = getCurrentTime();
          record.playerName = playerName;
          record.metadata = metadata;

          auto insert = false;
          auto isNewRecord = false;
          if (seasonIdToRecord.count(seasonId) == 0) {
            isNewRecord = true;
            insert = true;
          } else {
            insert = false;
            isNewRecord = completionTime.value() <
                          seasonIdToRecord[seasonId]->time;
          }
          seasonIdToIsNewRecord[seasonId] = isNewRecord;

          if (isNewRecord) {
            if (insert) {
              _repository->insertRecord(record);
            } else {
              _repository->updateRecord(record);
            }
          }
        }

        auto mostRelevantSeason = getMostRelevantSeason();
        return std::make_unique<CheckRecordResult>();
      },
      [this](std::unique_ptr<SynchronizationContext::ResultBase>) {
        _logger->info("Completed run");
      },
      [this](const std::runtime_error &e) {
        _logger->error("Run failed %s", e.what());
      });
}

const ETJump::Timerun::Season &ETJump::TimerunV2::getMostRelevantSeason() {
  // Most relevant = Most recently started
  Timerun::Season &mostRelevant = _activeSeasons[0];
  for (const auto &season : _activeSeasons) {
    if (mostRelevant.startTime < season.startTime) {
      mostRelevant = season;
    }
  }
  return mostRelevant;
}
