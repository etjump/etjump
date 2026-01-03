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

#pragma once

#include <array>
#include <map>
#include <utility>

#include "etj_log.h"
#include "etj_synchronization_context.h"
#include "etj_timerun_models.h"
#include "g_local.h"
#include "etj_savepos_shared.h"

namespace ETJump {
class TimerunRepository;

class TimerunV2 {
public:
  static constexpr int defaultSeasonId = 1;

  TimerunV2(std::string currentMap,
            std::unique_ptr<TimerunRepository> repository,
            std::unique_ptr<Log> logger,
            std::unique_ptr<SynchronizationContext> synchronizationContext);

  struct Player {
    Player(int clientNum, int userId, const std::vector<Timerun::Record> &runs)
        : clientNum(clientNum), userId(userId), records(runs) {}

    int clientNum;
    int userId;
    std::vector<Timerun::Record> records;
    bool running{};
    std::string name;
    std::optional<int> startTime;
    std::optional<int> completionTime;
    std::string activeRunName;
    bool runHasCheckpoints{};
    int nextCheckpointIdx{};
    std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpointTimes{};
    std::array<bool, MAX_TIMERUN_CHECKPOINTS> checkpointIndicesHit{};
    // /loadcheckpoints stores checkpoints here
    std::map<std::string, std::array<int, MAX_TIMERUN_CHECKPOINTS>>
        overriddenCheckpoints;

    [[nodiscard]] const Timerun::Record *
    getRecord(int seasonId, const std::string &runName) const;
  };

  struct Ranking {
    Ranking(int rank, int userId, std::string name, double score)
        : rank(rank), userId(userId), name(std::move(name)), score(score) {}

    int rank;
    int userId;
    std::string name;
    double score;
  };

  void computeRanks();
  void initialize();
  void shutdown();
  void runFrame();
  void clientConnect(int clientNum, int userId);
  void clientDisconnect(int clientNum);
  void startTimer(const std::string &runName, int clientNum,
                  const std::string &playerName, int currentTimeMs);

  /*
   * Starts a timerun for player, except grabs previous record and previous
   * record checkpoints from savepos data instead of records database
   */
  void startSaveposTimer(int clientNum, const std::string &playerName,
                         int currentTimeMs, const SavePosData &data);

  void checkpoint(const std::string &runName, int clientNum,
                  int checkpointIndex, int currentTimeMs) const;
  void stopTimer(const std::string &runName, int clientNum, int currentTimeMs);
  void addSeason(const Timerun::AddSeasonParams &season);
  void editSeason(const Timerun::EditSeasonParams &params);
  void interrupt(int clientNum);
  void connectNotify(int clientNum);
  void printRecords(const Timerun::PrintRecordsParams &params);
  void loadCheckpoints(int clientNum, const std::string &mapName,
                       const std::string &runName, int rank);
  void printRankings(const Timerun::PrintRankingsParams &params);
  void printSeasons(int clientNum);
  void deleteSeason(int clientNum, const std::string &name);

  [[nodiscard]] int32_t getRunStartTime(int32_t clientNum) const;

  static void removeDisallowedWeapons(gentity_t *ent);
  static void removePlayerProjectiles(gentity_t *ent);

  // returns true if weapon is considered an explosive by the
  // "No explosives" timerun spawnflag
  static bool weaponIsExplosivePickup(int weapon);

private:
  void startNotify(Player *player) const;
  static bool isDebugging(int clientNum);
  void checkRecord(Player *player);

  /*
   * returns nullptr if player object is unavailable,
   * or if the player is already running
   */
  [[nodiscard]] Player *setupPlayerData(int clientNum,
                                        const std::string &runName,
                                        const std::string &playerName,
                                        int currentTimeMs) const;

  static std::array<int, MAX_TIMERUN_CHECKPOINTS>
  toCheckpointsArray(const std::vector<int> *vector);
  static int indexForRunname(const std::string &runName);
  /**
   * We can have multiple seasons running at once. This will
   * figure out which one is the most relevant for the user
   */
  const Timerun::Season *getMostRelevantSeason();
  void updateSeasonStates();
  static std::string
  getRankingsStringFor(const std::vector<Ranking> *vector,
                       const Timerun::PrintRankingsParams &params);

  std::string _currentMap;
  std::unique_ptr<TimerunRepository> _repository;
  std::unique_ptr<Log> _logger;
  std::unique_ptr<SynchronizationContext> _sc;
  std::array<std::unique_ptr<Player>, MAX_CLIENTS> _players;

  std::vector<int> _activeSeasonsIds;
  std::vector<Timerun::Season> _activeSeasons;
  std::vector<int> _pastSeasonsIds;
  std::vector<Timerun::Season> _pastSeasons;
  std::vector<int> _upcomingSeasonsIds;
  std::vector<Timerun::Season> _upcomingSeasons;

  const Timerun::Season *_mostRelevantSeason{};
  std::map<int, std::vector<Ranking>> _rankingsPerSeason;
};
} // namespace ETJump
