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

#include <cstdint>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "../src/game/etj_database_v2.h"
#include "../src/game/etj_time_utilities.h"
#include "../src/game/etj_timerun_models.h"
#include "../src/game/etj_timerun_repository.h"

namespace {
using ETJump::DatabaseV2;
using ETJump::TimerunRepository;

constexpr int32_t DEFAULT_SEASON = ETJump::Timerun::DEFAULT_SEASON_ID;
constexpr const char *TEST_MAP   = "test_map";
constexpr const char *TEST_RUN   = "test_run";

// A file-backed database for the rare test that needs the data to survive a close/reopen,
// such as the seeding done when upgrading an existing database.
class TempDatabase {
public:
  explicit TempDatabase(const std::string &name) {
    static int32_t counter = 0;
    fspath                 = std::filesystem::path(testing::TempDir()) / (name + std::to_string(++counter) + ".db");
  }

  ~TempDatabase() {
    removeFiles();
  }

  TempDatabase(const TempDatabase &)            = delete;
  TempDatabase &operator=(const TempDatabase &) = delete;
  TempDatabase(TempDatabase &&)                 = delete;
  TempDatabase &operator=(TempDatabase &&)      = delete;

  [[nodiscard]] std::string path() const {
    return fspath.string();
  }

  void removeFiles() const {
    for (const auto *suffix : {"", "-wal", "-shm"}) {
      std::error_code ec;
      std::filesystem::remove(fspath.string() + suffix, ec);
    }
  }

private:
  std::filesystem::path fspath;
};

// Every test gets a pair of independent in-memory SQLite databases,
// so the repository runs its real migrations and queries without touching the disk.
class TimerunRepositoryTests : public testing::Test {
protected:
  void SetUp() override {
    repository = std::make_unique<TimerunRepository>(std::make_unique<DatabaseV2>("timerun-tests", ":memory:"),
                                                     std::make_unique<DatabaseV2>("timerun-tests-legacy", ":memory:"));
    repository->initialize();
  }

  static TimeUtils::Time parseTime(const std::string &value) {
    return TimeUtils::Time::fromString(value);
  }

  static ETJump::Timerun::Record makeRecord(const int32_t userId, const int32_t time,
                                            std::vector<int32_t> checkpoints = {},
                                            const std::string &date          = "2026-01-01 12:00:00") {
    ETJump::Timerun::Record record{};
    record.seasonId    = DEFAULT_SEASON;
    record.map         = TEST_MAP;
    record.run         = TEST_RUN;
    record.userId      = userId;
    record.time        = time;
    record.checkpoints = std::move(checkpoints);
    record.checkpoints.resize(MAX_TIMERUN_CHECKPOINTS, TIMERUN_CHECKPOINT_NOT_SET);
    record.recordDate = parseTime(date);
    record.playerName = "player" + std::to_string(userId);
    record.metadata   = {
      {"mod_version", "test"}
    };
    return record;
  }

  // serializes checkpoints the same way the game writes them into the database
  static std::string checkpointString(std::vector<int32_t> checkpoints) {
    checkpoints.resize(MAX_TIMERUN_CHECKPOINTS, TIMERUN_CHECKPOINT_NOT_SET);
    return StringUtils::join(checkpoints, ",");
  }

  static std::vector<ETJump::Timerun::Record> runTopRecords(TimerunRepository &repo) {
    return repo.getTopRecords({DEFAULT_SEASON}, TEST_MAP, TEST_RUN);
  }

  static ETJump::Timerun::RecordCompletion completion(ETJump::Timerun::Record record, const bool isNewRecord = true,
                                                      const bool insert = true) {
    return ETJump::Timerun::RecordCompletion{std::move(record), isNewRecord, insert};
  }

  static ETJump::Timerun::ListRemovedRecordsParams listRemovedParams() {
    ETJump::Timerun::ListRemovedRecordsParams params{};
    params.page     = 1;
    params.pageSize = 20;
    return params;
  }

  static ETJump::Timerun::ListRemovedSegmentsParams listRemovedSegmentsParams() {
    ETJump::Timerun::ListRemovedSegmentsParams params{};
    params.page     = 1;
    params.pageSize = 20;
    return params;
  }

  static ETJump::Timerun::RecordHistoryParams historyParams(const int32_t userId) {
    ETJump::Timerun::RecordHistoryParams params{};
    params.userId   = userId;
    params.season   = "Default";
    params.map      = TEST_MAP;
    params.run      = TEST_RUN;
    params.exactMap = true;
    return params;
  }

  static ETJump::Timerun::RemoveRecordParams recordRemoval(const int32_t userId, const int32_t removedBy,
                                                           const std::string &season = "",
                                                           const std::string &run    = TEST_RUN) {
    ETJump::Timerun::RemoveRecordParams params{};
    params.map       = TEST_MAP;
    params.run       = run;
    params.userId    = userId;
    params.removedBy = removedBy;
    params.season    = season;
    return params;
  }

  static ETJump::Timerun::Season addSeason(TimerunRepository &repo, const std::string &name,
                                           const TimeUtils::Date &start = TimeUtils::Date{2026, 1, 1}) {
    ETJump::Timerun::AddSeasonParams params{};
    params.name      = name;
    params.startTime = TimeUtils::Time::fromDate(start);
    return repo.addSeason(params);
  }

  std::unique_ptr<TimerunRepository> repository;
};

TEST_F(TimerunRepositoryTests, Initialize_CreatesDefaultSeason) {
  const auto seasons = repository->getSeasons();
  ASSERT_EQ(seasons.size(), 1u);
  EXPECT_EQ(seasons[0].id, DEFAULT_SEASON);
  EXPECT_EQ(seasons[0].name, "Default");
  EXPECT_EQ(seasons[0].startTime.toDateTimeString(), "2000-01-01 00:00:00");
  EXPECT_FALSE(seasons[0].endTime.has_value());
}

TEST_F(TimerunRepositoryTests, Initialize_ImportsLegacyRecords) {
  auto database       = std::make_unique<DatabaseV2>("timerun-tests-migrate", ":memory:");
  auto legacyDatabase = std::make_unique<DatabaseV2>("timerun-tests-legacy-source", ":memory:");

  legacyDatabase->sql << R"(
    create table records (
      id integer primary key autoincrement,
      time integer not null,
      record_date integer not null,
      map text not null,
      run text not null,
      user_id integer not null,
      player_name text not null
    );
  )";
  legacyDatabase->sql << "insert into records (time, record_date, map, run, user_id, player_name) "
                         "values (?, ?, ?, ?, ?, ?);"
                      << 1234 << 1700000000 << "legacy_map" << "legacy_run" << 42 << "legacy_player";

  TimerunRepository legacyRepository(std::move(database), std::move(legacyDatabase));
  legacyRepository.initialize();

  const auto records = legacyRepository.getRecords();
  ASSERT_EQ(records.size(), 1u);
  EXPECT_EQ(records[0].seasonId, DEFAULT_SEASON);
  EXPECT_EQ(records[0].map, "legacy_map");
  EXPECT_EQ(records[0].run, "legacy_run");
  EXPECT_EQ(records[0].userId, 42);
  EXPECT_EQ(records[0].time, 1234);
  EXPECT_EQ(records[0].playerName, "legacy_player");
  EXPECT_EQ(records[0].metadata.at("mod_version"), "unknown(imported)");

  // the legacy import fills the full checkpoint array with "not set"
  ASSERT_EQ(records[0].checkpoints.size(), MAX_TIMERUN_CHECKPOINTS);
  EXPECT_EQ(records[0].checkpoints.front(), TIMERUN_CHECKPOINT_NOT_SET);
}

TEST_F(TimerunRepositoryTests, Initialize_SeedsHistoryAndSegmentsForExistingRecords) {
  TempDatabase database("etjump-timerun-seed");

  // create the schema, then close it, so the record can be written
  // the way an older version of the mod would have left it behind
  {
    TimerunRepository first(std::make_unique<DatabaseV2>("seed", database.path()),
                            std::make_unique<DatabaseV2>("seed-legacy", ":memory:"));
    first.initialize();
  }

  {
    DatabaseV2 raw("seed", database.path());
    raw.sql << R"(
      insert into record (
        season_id, map, run, user_id, time, checkpoints, record_date, player_name, metadata
      ) values (?, ?, ?, ?, ?, ?, ?, ?, ?);
    )" << DEFAULT_SEASON
            << "seed_map" << "seed_run" << 7 << 300 << checkpointString({100, 200}) << "2026-01-01 12:00:00"
            << "seed_player" << "mod_version=seed";
  }

  const auto openRepository = [&database] {
    auto repo = std::make_unique<TimerunRepository>(std::make_unique<DatabaseV2>("seed", database.path()),
                                                    std::make_unique<DatabaseV2>("seed-legacy", ":memory:"));
    repo->initialize();
    return repo;
  };

  const auto upgraded = openRepository();

  const auto records = upgraded->getRecords();
  ASSERT_EQ(records.size(), 1u);
  EXPECT_EQ(records[0].time, 300);

  ETJump::Timerun::RecordHistoryParams seedHistoryParams{};
  seedHistoryParams.userId   = 7;
  seedHistoryParams.season   = "Default";
  seedHistoryParams.map      = "seed_map";
  seedHistoryParams.run      = "seed_run";
  seedHistoryParams.exactMap = true;

  const auto history = upgraded->getHistoricalRecords(seedHistoryParams);
  ASSERT_EQ(history.size(), 1u);
  EXPECT_EQ(history[0].r.time, 300);
  EXPECT_EQ(history[0].rank, 1);
  EXPECT_FALSE(history[0].removed);

  const auto global = upgraded->getBestSegments(DEFAULT_SEASON, "seed_map").at("seed_run");
  ASSERT_EQ(global.size(), 3u);
  EXPECT_EQ(global[0], 100);
  EXPECT_EQ(global[1], 100);
  EXPECT_EQ(global[2], 100);

  const auto personal = upgraded->getPlayerBestSegments(DEFAULT_SEASON, "seed_map", 7).at("seed_run");
  ASSERT_EQ(personal.size(), 3u);
  EXPECT_EQ(personal[0], 100);
  EXPECT_EQ(personal[1], 100);
  EXPECT_EQ(personal[2], 100);

  // seeding is one-shot, reopening the database does not duplicate anything
  const auto reopened = openRepository();
  EXPECT_EQ(reopened->getHistoricalRecords(seedHistoryParams).size(), 1u);
  EXPECT_EQ(reopened->getBestSegments(DEFAULT_SEASON, "seed_map").at("seed_run").size(), 3u);
}

TEST_F(TimerunRepositoryTests, Initialize_SkipsLegacyImportWhenNewRecordsExist) {
  TempDatabase database("etjump-timerun-migrate-skip");

  // create the schema first
  {
    TimerunRepository schema(std::make_unique<DatabaseV2>("schema", database.path()),
                             std::make_unique<DatabaseV2>("schema-legacy", ":memory:"));
    schema.initialize();
  }

  // a record written by the current version
  {
    DatabaseV2 raw("current", database.path());
    raw.sql << R"(
      insert into record (
        season_id, map, run, user_id, time, checkpoints, record_date, player_name, metadata
      ) values (?, ?, ?, ?, ?, ?, ?, ?, ?);
    )" << DEFAULT_SEASON
            << "current_map" << "current_run" << 7 << 300 << checkpointString({100, 200}) << "2026-01-01 12:00:00"
            << "current_player" << "mod_version=test";
  }

  // an old database with legacy records that must not be imported
  auto legacyDatabase = std::make_unique<DatabaseV2>("legacy", ":memory:");
  legacyDatabase->sql << R"(
    create table records (
      id integer primary key autoincrement,
      time integer not null,
      record_date integer not null,
      map text not null,
      run text not null,
      user_id integer not null,
      player_name text not null
    );
  )";
  legacyDatabase->sql << "insert into records (time, record_date, map, run, user_id, player_name) "
                         "values (?, ?, ?, ?, ?, ?);"
                      << 1234 << 1700000000 << "legacy_map" << "legacy_run" << 42 << "legacy_player";

  TimerunRepository upgraded(std::make_unique<DatabaseV2>("current", database.path()), std::move(legacyDatabase));
  upgraded.initialize();

  const auto records = upgraded.getRecords();
  ASSERT_EQ(records.size(), 1u);
  EXPECT_EQ(records[0].map, "current_map");
  EXPECT_EQ(records[0].run, "current_run");
  EXPECT_EQ(records[0].userId, 7);
}

TEST_F(TimerunRepositoryTests, Initialize_SeedExcludesRemovedRunsAndSplits) {
  TempDatabase database("etjump-timerun-seed-excluded");

  // create the schema first
  {
    TimerunRepository schema(std::make_unique<DatabaseV2>("schema", database.path()),
                             std::make_unique<DatabaseV2>("schema-legacy", ":memory:"));
    schema.initialize();
  }

  {
    DatabaseV2 raw("seed", database.path());
    const std::string checkpoints = checkpointString({100, 200});

    raw.sql << R"(
      insert into record (
        season_id, map, run, user_id, time, checkpoints, record_date, player_name, metadata
      ) values (?, ?, ?, ?, ?, ?, ?, ?, ?);
    )" << DEFAULT_SEASON
            << TEST_MAP << "kept_run" << 1 << 300 << checkpoints << "2026-01-01 12:00:00" << "player1"
            << "mod_version=test";

    raw.sql << R"(
      insert into record (
        season_id, map, run, user_id, time, checkpoints, record_date, player_name, metadata
      ) values (?, ?, ?, ?, ?, ?, ?, ?, ?);
    )" << DEFAULT_SEASON
            << TEST_MAP << "removed_run" << 2 << 300 << checkpoints << "2026-01-02 12:00:00" << "player2"
            << "mod_version=test";

    raw.sql << R"(
      insert into removed_records (
        season_id, map, run, user_id, time, checkpoints, record_date, player_name, metadata,
        removed_by, removed_at, reason
      ) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )" << DEFAULT_SEASON
            << TEST_MAP << "removed_run" << 2 << 300 << checkpoints << "2026-01-02 12:00:00" << "player2"
            << "mod_version=test" << 9 << "2026-02-01 12:00:00" << nullptr;

    raw.sql << "delete from record where map=? and run=?;" << TEST_MAP << "removed_run";

    raw.sql << R"(
      insert into record (
        season_id, map, run, user_id, time, checkpoints, record_date, player_name, metadata
      ) values (?, ?, ?, ?, ?, ?, ?, ?, ?);
    )" << DEFAULT_SEASON
            << TEST_MAP << "split_run" << 3 << 300 << checkpoints << "2026-01-03 12:00:00" << "player3"
            << "mod_version=test";

    raw.sql << R"(
      insert into removed_segments (
        season_id, map, run, run_sanitized, user_id, segment, time, record_date, player_name,
        metadata, removed_by, removed_at, reason
      ) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )" << DEFAULT_SEASON
            << TEST_MAP << "split_run" << "split_run" << 3 << 2 << 100 << "2026-01-03 12:00:00" << "player3" << "" << 9
            << "2026-02-01 12:00:00" << nullptr;
  }

  TimerunRepository upgraded(std::make_unique<DatabaseV2>("seed", database.path()),
                             std::make_unique<DatabaseV2>("seed-legacy", ":memory:"));
  upgraded.initialize();

  const auto global = upgraded.getBestSegments(DEFAULT_SEASON, TEST_MAP);

  // the archived run contributes nothing
  EXPECT_EQ(global.count("removed_run"), 0u);
  ASSERT_EQ(global.count("kept_run"), 1u);

  const auto &kept = global.at("kept_run");
  ASSERT_EQ(kept.size(), 3u);
  EXPECT_EQ(kept[0], 100);
  EXPECT_EQ(kept[1], 100);
  EXPECT_EQ(kept[2], 100);

  // the individually removed split is skipped, but the rest of the run seeds
  ASSERT_EQ(global.count("split_run"), 1u);
  const auto &split = global.at("split_run");
  ASSERT_EQ(split.size(), 3u);
  EXPECT_EQ(split[0], 100);
  EXPECT_EQ(split[1], 0);
  EXPECT_EQ(split[2], 100);

  EXPECT_TRUE(upgraded.getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 2).empty());
  EXPECT_EQ(upgraded.getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 3).at("split_run")[1], 0);
}

TEST_F(TimerunRepositoryTests, AddSeason_InsertsSeason) {
  ETJump::Timerun::AddSeasonParams params{};
  params.name      = "Season 2";
  params.startTime = TimeUtils::Time::fromDate(TimeUtils::Date{2026, 1, 1});

  auto season = repository->addSeason(params);
  EXPECT_EQ(season.name, "Season 2");

  auto seasons = repository->getSeasons();
  ASSERT_EQ(seasons.size(), 2u);
  EXPECT_EQ(seasons[1].id, season.id);
  EXPECT_EQ(seasons[1].name, "Season 2");
  EXPECT_EQ(seasons[1].startTime.toDateTimeString(), "2026-01-01 00:00:00");
  EXPECT_FALSE(seasons[1].endTime.has_value());

  params.name    = "Season 3";
  params.endTime = TimeUtils::Time::fromDate(TimeUtils::Date{2026, 2, 1});
  season         = repository->addSeason(params);
  EXPECT_EQ(season.name, "Season 3");

  seasons = repository->getSeasons();
  ASSERT_EQ(seasons.size(), 3u);
  EXPECT_EQ(seasons[2].id, season.id);
  EXPECT_EQ(seasons[2].name, "Season 3");
  EXPECT_EQ(seasons[2].startTime.toDateTimeString(), "2026-01-01 00:00:00");
  EXPECT_EQ(seasons[2].endTime.value().toDateTimeString(), "2026-02-01 00:00:00");
}

TEST_F(TimerunRepositoryTests, AddSeason_DuplicateName_Throws) {
  ETJump::Timerun::AddSeasonParams params{};
  params.name      = "Season 2";
  params.startTime = TimeUtils::Time::fromDate(TimeUtils::Date{2026, 1, 1});

  repository->addSeason(params);
  EXPECT_THROW(repository->addSeason(params), std::runtime_error);
}

TEST_F(TimerunRepositoryTests, AddSeason_StartTimeAfterEndTime_Throws) {
  ETJump::Timerun::AddSeasonParams params{};
  params.name      = "Season 2";
  params.startTime = TimeUtils::Time::fromDate(TimeUtils::Date{2026, 2, 1});
  params.endTime   = TimeUtils::Time::fromDate(TimeUtils::Date{2026, 1, 1});

  EXPECT_THROW(repository->addSeason(params), std::runtime_error);
}

TEST_F(TimerunRepositoryTests, EditSeason_UpdatesTimesAndValidates) {
  const auto season = addSeason(*repository, "Season 2");

  ETJump::Timerun::EditSeasonParams edit{};
  edit.name      = "Season 2";
  edit.startTime = TimeUtils::Time::fromDate(TimeUtils::Date{2026, 2, 1});
  edit.endTime   = TimeUtils::Time::fromDate(TimeUtils::Date{2026, 6, 1});

  repository->editSeason(edit);

  auto findSeason = [this] {
    const auto seasons = repository->getSeasonsForName("Season 2", true);
    EXPECT_EQ(seasons.size(), 1u);
    return seasons.front();
  };

  auto updated = findSeason();
  EXPECT_EQ(updated.id, season.id);
  EXPECT_EQ(updated.startTime.toDateTimeString(), "2026-02-01 00:00:00");
  ASSERT_TRUE(updated.endTime.has_value());
  EXPECT_EQ(updated.endTime->toDateTimeString(), "2026-06-01 00:00:00");

  // changing only the end time keeps the stored start time
  ETJump::Timerun::EditSeasonParams endOnly{};
  endOnly.name    = "Season 2";
  endOnly.endTime = TimeUtils::Time::fromDate(TimeUtils::Date{2026, 7, 1});

  repository->editSeason(endOnly);

  updated = findSeason();
  EXPECT_EQ(updated.startTime.toDateTimeString(), "2026-02-01 00:00:00");
  ASSERT_TRUE(updated.endTime.has_value());
  EXPECT_EQ(updated.endTime->toDateTimeString(), "2026-07-01 00:00:00");

  // an end before the start is rejected without touching the stored row
  ETJump::Timerun::EditSeasonParams invalid{};
  invalid.name      = "Season 2";
  invalid.startTime = TimeUtils::Time::fromDate(TimeUtils::Date{2026, 8, 1});
  invalid.endTime   = TimeUtils::Time::fromDate(TimeUtils::Date{2026, 7, 1});

  EXPECT_THROW(repository->editSeason(invalid), std::runtime_error);

  updated = findSeason();
  EXPECT_EQ(updated.startTime.toDateTimeString(), "2026-02-01 00:00:00");
  ASSERT_TRUE(updated.endTime.has_value());
  EXPECT_EQ(updated.endTime->toDateTimeString(), "2026-07-01 00:00:00");

  ETJump::Timerun::EditSeasonParams unknown{};
  unknown.name = "does not exist";
  EXPECT_THROW(repository->editSeason(unknown), std::runtime_error);
}

TEST_F(TimerunRepositoryTests, DeleteSeason_PurgesAllSeasonData) {
  const auto secondSeason = addSeason(*repository, "Season 2");

  auto defaultSeasonRecord    = makeRecord(1, 300, {100, 200});
  auto secondSeasonRecord     = defaultSeasonRecord;
  secondSeasonRecord.seasonId = secondSeason.id;

  repository->recordCompletion({completion(defaultSeasonRecord), completion(secondSeasonRecord)});
  ASSERT_EQ(repository->getRecords().size(), 2u);

  // archive one of Season 2's splits, so the purge has removed segments to work on too
  ETJump::Timerun::RemoveSegmentParams removeSegmentParams{};
  removeSegmentParams.season    = "Season 2";
  removeSegmentParams.map       = TEST_MAP;
  removeSegmentParams.run       = TEST_RUN;
  removeSegmentParams.userId    = 1;
  removeSegmentParams.segment   = 2;
  removeSegmentParams.removedBy = 9;
  ASSERT_EQ(repository->removeSegment(removeSegmentParams, parseTime("2026-03-01 12:00:00")).size(), 1u);

  // an explicit season targets only that copy, leaving the overall season copy intact
  const auto removed = repository->removeRecord(recordRemoval(1, 9, "Season 2"), parseTime("2026-03-01 12:00:00"));
  ASSERT_EQ(removed.size(), 1u);
  EXPECT_EQ(removed[0].seasonId, secondSeason.id);

  const auto records = repository->getRecords();
  ASSERT_EQ(records.size(), 1u);
  EXPECT_EQ(records[0].seasonId, DEFAULT_SEASON);

  const auto archived = repository->getRemovedRecords(listRemovedParams());
  ASSERT_EQ(archived.entries.size(), 1u);
  ASSERT_EQ(archived.entries[0].records.size(), 1u);
  EXPECT_EQ(archived.entries[0].records[0].record.seasonId, secondSeason.id);

  // the season name must match exactly for deletion too
  EXPECT_THROW(repository->deleteSeason("Season"), std::runtime_error);
  EXPECT_EQ(repository->getSeasonsForName("Season 2", true).size(), 1u);

  repository->deleteSeason("Season 2");

  const auto seasons = repository->getSeasons();
  ASSERT_EQ(seasons.size(), 1u);
  EXPECT_EQ(seasons[0].id, DEFAULT_SEASON);

  // overall season data survives; Season 2's live, derived and archived data is gone
  const auto remaining = repository->getRecords();
  ASSERT_EQ(remaining.size(), 1u);
  EXPECT_EQ(remaining[0].seasonId, DEFAULT_SEASON);
  EXPECT_EQ(remaining[0].time, 300);

  EXPECT_TRUE(repository->getBestSegments(secondSeason.id, TEST_MAP).empty());
  EXPECT_TRUE(repository->getPlayerBestSegments(secondSeason.id, TEST_MAP, 1).empty());
  EXPECT_TRUE(repository->getRemovedRecords(listRemovedParams()).entries.empty());
  EXPECT_TRUE(repository->getRemovedSegments(listRemovedSegmentsParams()).entries.empty());

  // default season cannot be deleted
  EXPECT_THROW(repository->deleteSeason("Default"), std::runtime_error);
  EXPECT_THROW(repository->deleteSeason("default"), std::runtime_error);
}

TEST_F(TimerunRepositoryTests, RecordCompletion_NewRecord_StoresRecordAndHistory) {
  repository->recordCompletion({completion(makeRecord(7, 1000, {200, 400}))});

  const auto tops = runTopRecords(*repository);
  ASSERT_EQ(tops.size(), 1u);

  const auto &top = tops.front();
  EXPECT_EQ(top.userId, 7);
  EXPECT_EQ(top.time, 1000);
  EXPECT_EQ(top.recordDate.toDateTimeString(), "2026-01-01 12:00:00");
  EXPECT_EQ(top.playerName, "player7");
  EXPECT_EQ(top.metadata.at("mod_version"), "test");
  ASSERT_EQ(top.checkpoints.size(), MAX_TIMERUN_CHECKPOINTS);
  EXPECT_EQ(top.checkpoints[0], 200);
  EXPECT_EQ(top.checkpoints[1], 400);

  for (size_t i = 2; i < top.checkpoints.size(); i++) {
    EXPECT_EQ(top.checkpoints[i], TIMERUN_CHECKPOINT_NOT_SET);
  }

  const auto playerRecords = repository->getRecordsForPlayer({DEFAULT_SEASON}, TEST_MAP, 7);
  ASSERT_EQ(playerRecords.size(), 1u);
  EXPECT_EQ(playerRecords[0].run, TEST_RUN);

  ETJump::Timerun::RecordHistoryParams historyParams{};
  historyParams.userId   = 7;
  historyParams.season   = "Default";
  historyParams.map      = TEST_MAP;
  historyParams.run      = TEST_RUN;
  historyParams.exactMap = true;

  const auto history = repository->getHistoricalRecords(historyParams);
  ASSERT_EQ(history.size(), 1u);
  EXPECT_EQ(history[0].r.time, 1000);
  EXPECT_EQ(history[0].rank, 1);
  EXPECT_FALSE(history[0].removed);
}

TEST_F(TimerunRepositoryTests, RecordCompletion_FasterCompletion_UpdatesRecord) {
  repository->recordCompletion({completion(makeRecord(7, 1000))});
  repository->recordCompletion({completion(makeRecord(7, 900, {}, "2026-02-01 12:00:00"), true, false)});

  const auto records = repository->getRecords();
  ASSERT_EQ(records.size(), 1u);
  EXPECT_EQ(records[0].time, 900);
  EXPECT_EQ(records[0].recordDate.toDateTimeString(), "2026-02-01 12:00:00");

  // both completions are immutable history entries, newest first, each ranked
  const auto history = repository->getHistoricalRecords(historyParams(7));
  ASSERT_EQ(history.size(), 2u);
  EXPECT_EQ(history[0].r.time, 900);
  EXPECT_EQ(history[0].rank, 1);
  EXPECT_EQ(history[1].r.time, 1000);
  EXPECT_EQ(history[1].rank, 1);

  // a completion without checkpoint data does not write any segments
  EXPECT_TRUE(repository->getBestSegments(DEFAULT_SEASON, TEST_MAP).empty());
}

TEST_F(TimerunRepositoryTests, RecordCompletion_SlowerCompletion_StillUpdatesSegments) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}))});

  // slower overall run, but segments 1 and 2 are faster than the previous best
  const auto updates =
    repository->recordCompletion({completion(makeRecord(1, 320, {90, 180}, "2026-01-02 12:00:00"), false, false)});

  // the record is untouched, and a non-record completion does not add history
  const auto records = repository->getRecords();
  ASSERT_EQ(records.size(), 1u);
  EXPECT_EQ(records[0].time, 300);
  EXPECT_EQ(repository->getHistoricalRecords(historyParams(1)).size(), 1u);

  const auto &seasonUpdates = updates.at(DEFAULT_SEASON);
  ASSERT_EQ(seasonUpdates.size(), 2u);
  EXPECT_EQ(seasonUpdates[0].segment, 1);
  EXPECT_EQ(seasonUpdates[0].time, 90);
  EXPECT_EQ(seasonUpdates[1].segment, 2);
  EXPECT_EQ(seasonUpdates[1].time, 90);

  const auto personal = repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 1).at(TEST_RUN);
  ASSERT_EQ(personal.size(), 3u);
  EXPECT_EQ(personal[0], 90);
  EXPECT_EQ(personal[1], 90);
  EXPECT_EQ(personal[2], 100); // the finish did not improve
}

TEST_F(TimerunRepositoryTests, RecordCompletion_ConstraintViolation_RollsBackWholeBatch) {
  const auto first     = completion(makeRecord(1, 300, {100, 200}));
  const auto duplicate = completion(makeRecord(2, 400));

  // the second insert of 'duplicate' violates the primary key,
  // which must discard 'first' along with the rest of the batch
  EXPECT_THROW(repository->recordCompletion({first, duplicate, duplicate}), std::exception);

  EXPECT_TRUE(repository->getRecords().empty());
  EXPECT_TRUE(repository->getBestSegments(DEFAULT_SEASON, TEST_MAP).empty());
  EXPECT_TRUE(repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 1).empty());
}

TEST_F(TimerunRepositoryTests, RecordCompletion_MultipleSeasons_UpdatesEachSeason) {
  const auto secondSeason = addSeason(*repository, "Season 2");

  auto defaultSeasonRecord    = makeRecord(1, 300, {100, 200});
  auto secondSeasonRecord     = defaultSeasonRecord;
  secondSeasonRecord.seasonId = secondSeason.id;

  const auto updates = repository->recordCompletion({completion(defaultSeasonRecord), completion(secondSeasonRecord)});

  ASSERT_EQ(updates.size(), 2u);
  EXPECT_EQ(updates.at(DEFAULT_SEASON).size(), 3u);
  EXPECT_EQ(updates.at(secondSeason.id).size(), 3u);

  EXPECT_EQ(repository->getBestSegments(DEFAULT_SEASON, TEST_MAP).at(TEST_RUN).size(), 3u);
  EXPECT_EQ(repository->getBestSegments(secondSeason.id, TEST_MAP).at(TEST_RUN).size(), 3u);

  const auto topRecords = repository->getTopRecords({DEFAULT_SEASON, secondSeason.id}, TEST_MAP, TEST_RUN);
  ASSERT_EQ(topRecords.size(), 2u);

  std::set<int32_t> topRecordSeasons;
  for (const auto &record : topRecords) {
    topRecordSeasons.insert(record.seasonId);
  }

  ASSERT_EQ(topRecordSeasons.size(), 2u);
  EXPECT_TRUE(topRecordSeasons.count(DEFAULT_SEASON) > 0);
  EXPECT_TRUE(topRecordSeasons.count(secondSeason.id) > 0);

  // a season-filtered listing only sees the copy for that season
  ETJump::Timerun::PrintRecordsParams params{};
  params.season   = "Season 2";
  params.map      = TEST_MAP;
  params.run      = TEST_RUN;
  params.exactMap = true;
  params.page     = 1;
  params.pageSize = 10;

  const auto listed = repository->getRecords(params);
  ASSERT_EQ(listed.size(), 1u);
  EXPECT_EQ(listed[0].seasonId, secondSeason.id);
}

TEST_F(TimerunRepositoryTests, GetRecords_MultipleSeasonsAndUsers_AreOrdered) {
  const auto fall   = addSeason(*repository, "Fall 2026");
  const auto winter = addSeason(*repository, "Winter 2026");

  const auto makeSeasonRecord = [](const int32_t userId, const int32_t time, const int32_t seasonId) {
    auto record     = makeRecord(userId, time);
    record.seasonId = seasonId;
    return record;
  };

  repository->recordCompletion({completion(makeSeasonRecord(1, 300, fall.id)),
                                completion(makeSeasonRecord(2, 250, fall.id)),
                                completion(makeSeasonRecord(1, 280, winter.id)),
                                completion(makeSeasonRecord(2, 240, winter.id)), completion(makeRecord(1, 200))});

  ETJump::Timerun::PrintRecordsParams params{};
  params.season   = "2026";
  params.map      = TEST_MAP;
  params.run      = TEST_RUN;
  params.exactMap = true;
  params.page     = 1;
  params.pageSize = 10;

  // a partial season match returns records from every matching season
  const auto records = repository->getRecords(params);
  ASSERT_EQ(records.size(), 4u);

  // ordered by season, then by time within each season
  EXPECT_EQ(records[0].seasonId, fall.id);
  EXPECT_EQ(records[0].userId, 2);
  EXPECT_EQ(records[0].time, 250);
  EXPECT_EQ(records[1].seasonId, fall.id);
  EXPECT_EQ(records[1].userId, 1);
  EXPECT_EQ(records[1].time, 300);
  EXPECT_EQ(records[2].seasonId, winter.id);
  EXPECT_EQ(records[2].userId, 2);
  EXPECT_EQ(records[2].time, 240);
  EXPECT_EQ(records[3].seasonId, winter.id);
  EXPECT_EQ(records[3].userId, 1);
  EXPECT_EQ(records[3].time, 280);

  EXPECT_EQ(repository->getSeasonsForName("2026", false).size(), 2u);

  // the top records collapse each season to its fastest run
  const auto tops = repository->getTopRecords({fall.id, winter.id}, TEST_MAP, TEST_RUN);
  ASSERT_EQ(tops.size(), 2u);

  std::map<int32_t, ETJump::Timerun::Record> topBySeason;
  for (const auto &record : tops) {
    topBySeason[record.seasonId] = record;
  }

  ASSERT_EQ(topBySeason.size(), 2u);
  EXPECT_EQ(topBySeason.at(fall.id).userId, 2);
  EXPECT_EQ(topBySeason.at(fall.id).time, 250);
  EXPECT_EQ(topBySeason.at(winter.id).userId, 2);
  EXPECT_EQ(topBySeason.at(winter.id).time, 240);

  // season IDs not in the requested list are excluded
  const auto fallOnly = repository->getTopRecords({fall.id}, TEST_MAP, TEST_RUN);
  ASSERT_EQ(fallOnly.size(), 1u);
  EXPECT_EQ(fallOnly[0].seasonId, fall.id);
}

TEST_F(TimerunRepositoryTests, GetRecordsForPlayer_FiltersBySeasonListAndUser) {
  const auto secondSeason = addSeason(*repository, "Season 2");

  auto defaultRecord    = makeRecord(1, 300);
  auto seasonRecord     = makeRecord(1, 280, {}, "2026-02-01 12:00:00");
  seasonRecord.seasonId = secondSeason.id;

  repository->recordCompletion(
    {completion(defaultRecord), completion(seasonRecord), completion(makeRecord(2, 250, {}, "2026-03-01 12:00:00"))});

  const auto mapOnly = repository->getRecordsForPlayer({DEFAULT_SEASON, secondSeason.id}, TEST_MAP, 1);
  ASSERT_EQ(mapOnly.size(), 2u);

  std::set<int32_t> mapSeasons;
  for (const auto &record : mapOnly) {
    mapSeasons.insert(record.seasonId);
  }

  EXPECT_EQ(mapSeasons.size(), 2u);
  EXPECT_TRUE(mapSeasons.count(DEFAULT_SEASON) > 0);
  EXPECT_TRUE(mapSeasons.count(secondSeason.id) > 0);

  const auto defaultOnly = repository->getRecordsForPlayer({DEFAULT_SEASON}, TEST_MAP, 1);
  ASSERT_EQ(defaultOnly.size(), 1u);
  EXPECT_EQ(defaultOnly[0].seasonId, DEFAULT_SEASON);
  EXPECT_EQ(defaultOnly[0].time, 300);

  // the user filter is applied independently of the season list
  const auto otherUser = repository->getRecordsForPlayer({DEFAULT_SEASON, secondSeason.id}, TEST_MAP, 2);
  ASSERT_EQ(otherUser.size(), 1u);
  EXPECT_EQ(otherUser[0].seasonId, DEFAULT_SEASON);
  EXPECT_EQ(otherUser[0].time, 250);

  const auto runRecords = repository->getRecordsForPlayer({DEFAULT_SEASON, secondSeason.id}, TEST_MAP, TEST_RUN, 1);
  ASSERT_EQ(runRecords.size(), 2u);

  const auto seasonOnly = repository->getRecordsForPlayer({secondSeason.id}, TEST_MAP, TEST_RUN, 1);
  ASSERT_EQ(seasonOnly.size(), 1u);
  EXPECT_EQ(seasonOnly[0].seasonId, secondSeason.id);
  EXPECT_EQ(seasonOnly[0].time, 280);
}

TEST_F(TimerunRepositoryTests, GetRecords_ResolvesPartialMapNames) {
  auto testMap = makeRecord(1, 300);
  testMap.map  = "test_map";

  auto testMapFinal = makeRecord(2, 280);
  testMapFinal.map  = "test_map_final";

  auto oasis = makeRecord(3, 260);
  oasis.map  = "oasis";

  repository->recordCompletion({completion(testMap), completion(testMapFinal), completion(oasis)});

  ETJump::Timerun::PrintRecordsParams params{};
  params.season   = std::string("Default");
  params.run      = std::string(TEST_RUN);
  params.page     = 1;
  params.pageSize = 10;

  // a unique partial match resolves to the stored map name
  params.map      = "oas";
  params.exactMap = false;

  const auto partial = repository->getRecords(params);
  ASSERT_EQ(partial.size(), 1u);
  EXPECT_EQ(partial[0].map, "oasis");

  // a partial match hitting several maps is rejected with suggestions
  params.map = "test_ma";
  EXPECT_THROW(repository->getRecords(params), std::runtime_error);

  // an exact query does not match longer map names
  params.map      = "test_map";
  params.exactMap = true;

  const auto exact = repository->getRecords(params);
  ASSERT_EQ(exact.size(), 1u);
  EXPECT_EQ(exact[0].map, "test_map");
}

TEST_F(TimerunRepositoryTests, GetRecords_PrefersExactRunThenFallsBackToPartial) {
  auto shortRun = makeRecord(1, 300);
  shortRun.run  = "short";

  auto longerRun = makeRecord(2, 280);
  longerRun.run  = "shortrun";

  repository->recordCompletion({completion(shortRun), completion(longerRun)});

  ETJump::Timerun::PrintRecordsParams params{};
  params.season   = std::string("Default");
  params.map      = TEST_MAP;
  params.exactMap = true;
  params.page     = 1;
  params.pageSize = 10;

  // an existing exact run match takes precedence over longer runs
  params.run = std::string("short");

  const auto exact = repository->getRecords(params);
  ASSERT_EQ(exact.size(), 1u);
  EXPECT_EQ(exact[0].run, "short");

  // without an exact match, the run filter falls back to a partial match
  params.run = std::string("shor");

  const auto partial = repository->getRecords(params);
  ASSERT_EQ(partial.size(), 2u);

  std::set<std::string> runs;
  for (const auto &record : partial) {
    runs.insert(record.run);
  }

  EXPECT_EQ(runs.size(), 2u);
  EXPECT_TRUE(runs.count("short") > 0);
  EXPECT_TRUE(runs.count("shortrun") > 0);
}

TEST_F(TimerunRepositoryTests, TiedRecords_AreIndividuallyAddressableByRank) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}, "2026-01-01 12:00:00")),
                                completion(makeRecord(2, 300, {90, 200}, "2026-01-02 12:00:00"))});

  // top records still treats equal times as a tie for rank 1
  const auto tops = repository->getTopRecords({DEFAULT_SEASON}, TEST_MAP, TEST_RUN);
  ASSERT_EQ(tops.size(), 2u);

  // rank arguments address the tied rows individually, ordered by date then user ID
  const auto rank1 = repository->getRecord(TEST_MAP, TEST_RUN, 1);
  ASSERT_TRUE(rank1.has_value());
  EXPECT_EQ(rank1->userId, 1);

  const auto rank2 = repository->getRecord(TEST_MAP, TEST_RUN, 2);
  ASSERT_TRUE(rank2.has_value());
  EXPECT_EQ(rank2->userId, 2);

  EXPECT_FALSE(repository->getRecord(TEST_MAP, TEST_RUN, 3).has_value());

  ETJump::Timerun::ListCheckpointsParams checkpointParams{};
  checkpointParams.season   = "Default";
  checkpointParams.map      = TEST_MAP;
  checkpointParams.run      = TEST_RUN;
  checkpointParams.rank     = 1;
  checkpointParams.exactMap = true;

  auto checkpoints = repository->getCheckpoints(checkpointParams);
  ASSERT_EQ(checkpoints.size(), 1u);
  EXPECT_EQ(checkpoints[0].playerName, "player1");

  checkpointParams.rank = 2;
  checkpoints           = repository->getCheckpoints(checkpointParams);
  ASSERT_EQ(checkpoints.size(), 1u);
  EXPECT_EQ(checkpoints[0].playerName, "player2");

  checkpointParams.rank = 3;
  EXPECT_TRUE(repository->getCheckpoints(checkpointParams).empty());

  // history keeps the competition rank, where ties share rank 1
  const auto history1 = repository->getHistoricalRecords(historyParams(1));
  ASSERT_EQ(history1.size(), 1u);
  EXPECT_EQ(history1[0].rank, 1);

  const auto history2 = repository->getHistoricalRecords(historyParams(2));
  ASSERT_EQ(history2.size(), 1u);
  EXPECT_EQ(history2[0].rank, 1);
}

TEST_F(TimerunRepositoryTests, GetRecord_SelectsRanksWithinDefaultSeason) {
  repository->recordCompletion({completion(makeRecord(1, 300, {}, "2026-01-01 12:00:00")),
                                completion(makeRecord(2, 250, {}, "2026-01-02 12:00:00")),
                                completion(makeRecord(3, 280, {}, "2026-01-03 12:00:00"))});

  const auto secondSeason = addSeason(*repository, "Season 2");

  auto secondSeasonRecord     = makeRecord(4, 100, {}, "2026-01-04 12:00:00");
  secondSeasonRecord.seasonId = secondSeason.id;
  repository->recordCompletion({completion(secondSeasonRecord)});

  const auto rank1 = repository->getRecord(TEST_MAP, TEST_RUN, 1);
  ASSERT_TRUE(rank1.has_value());
  EXPECT_EQ(rank1->userId, 2);
  EXPECT_EQ(rank1->time, 250);

  const auto rank2 = repository->getRecord(TEST_MAP, TEST_RUN, 2);
  ASSERT_TRUE(rank2.has_value());
  EXPECT_EQ(rank2->userId, 3);

  const auto rank3 = repository->getRecord(TEST_MAP, TEST_RUN, 3);
  ASSERT_TRUE(rank3.has_value());
  EXPECT_EQ(rank3->userId, 1);

  // the lookup is scoped to the default season, so the faster seasonal copy is invisible
  EXPECT_FALSE(repository->getRecord(TEST_MAP, TEST_RUN, 4).has_value());
}

TEST_F(TimerunRepositoryTests, GetRecordFromSeason_AddressesTiedRanksIndividually) {
  const auto secondSeason = addSeason(*repository, "Season 2");

  const auto makeSeasonRecord = [](const int32_t userId, const int32_t time, const std::string &date,
                                   const int32_t seasonId) {
    auto record     = makeRecord(userId, time, {}, date);
    record.seasonId = seasonId;
    return record;
  };

  // three tied times: two on the same date, one later, broken by user ID and date
  repository->recordCompletion({completion(makeSeasonRecord(4, 100, "2026-01-01 12:00:00", secondSeason.id)),
                                completion(makeSeasonRecord(7, 100, "2026-01-01 12:00:00", secondSeason.id)),
                                completion(makeSeasonRecord(6, 100, "2026-01-03 12:00:00", secondSeason.id)),
                                completion(makeSeasonRecord(5, 120, "2026-01-02 12:00:00", secondSeason.id))});

  const auto rank1 = repository->getRecordFromSeason(secondSeason.id, TEST_MAP, TEST_RUN, 1, true);
  ASSERT_EQ(rank1.size(), 1u);
  EXPECT_EQ(rank1[0].userId, 4);

  const auto rank2 = repository->getRecordFromSeason(secondSeason.id, TEST_MAP, TEST_RUN, 2, true);
  ASSERT_EQ(rank2.size(), 1u);
  EXPECT_EQ(rank2[0].userId, 7);

  const auto rank3 = repository->getRecordFromSeason(secondSeason.id, TEST_MAP, TEST_RUN, 3, true);
  ASSERT_EQ(rank3.size(), 1u);
  EXPECT_EQ(rank3[0].userId, 6);

  const auto rank4 = repository->getRecordFromSeason(secondSeason.id, TEST_MAP, TEST_RUN, 4, true);
  ASSERT_EQ(rank4.size(), 1u);
  EXPECT_EQ(rank4[0].userId, 5);

  EXPECT_TRUE(repository->getRecordFromSeason(secondSeason.id, TEST_MAP, TEST_RUN, 5, true).empty());
}

TEST_F(TimerunRepositoryTests, GetRecordFromSeason_ResolvesPartialMapNames) {
  const auto secondSeason = addSeason(*repository, "Season 2");

  auto record     = makeRecord(4, 100);
  record.seasonId = secondSeason.id;
  repository->recordCompletion({completion(record)});

  // a unique partial map match resolves to the stored map name
  const auto partial = repository->getRecordFromSeason(secondSeason.id, "test_ma", TEST_RUN, 1, false);
  ASSERT_EQ(partial.size(), 1u);
  EXPECT_EQ(partial[0].map, TEST_MAP);
  EXPECT_EQ(partial[0].userId, 4);

  // with exact matching enabled, the partial name matches nothing
  EXPECT_TRUE(repository->getRecordFromSeason(secondSeason.id, "test_ma", TEST_RUN, 1, true).empty());
}

TEST_F(TimerunRepositoryTests, GetHistoricalRecords_MultipleSeasons) {
  const auto seasonA = addSeason(*repository, "Season A 2026");
  const auto seasonB = addSeason(*repository, "Season B 2026");

  const auto makeSeasonRecord = [](const int32_t userId, const int32_t time, const std::string &date,
                                   const int32_t seasonId) {
    auto record     = makeRecord(userId, time, {}, date);
    record.seasonId = seasonId;
    return record;
  };

  repository->recordCompletion({completion(makeSeasonRecord(1, 300, "2026-01-01 12:00:00", seasonA.id)),
                                completion(makeSeasonRecord(1, 280, "2026-01-02 12:00:00", seasonA.id), true, false),
                                completion(makeSeasonRecord(1, 250, "2026-01-03 12:00:00", seasonB.id))});

  ETJump::Timerun::RecordHistoryParams params{};
  params.season   = std::string("2026");
  params.map      = TEST_MAP;
  params.run      = TEST_RUN;
  params.userId   = 1;
  params.exactMap = true;

  // ordered by season, then newest first within each season
  const auto history = repository->getHistoricalRecords(params);
  ASSERT_EQ(history.size(), 3u);

  EXPECT_EQ(history[0].r.seasonId, seasonA.id);
  EXPECT_EQ(history[0].r.time, 280);
  EXPECT_EQ(history[0].r.recordDate.toDateTimeString(), "2026-01-02 12:00:00");
  EXPECT_EQ(history[1].r.seasonId, seasonA.id);
  EXPECT_EQ(history[1].r.time, 300);
  EXPECT_EQ(history[1].r.recordDate.toDateTimeString(), "2026-01-01 12:00:00");
  EXPECT_EQ(history[2].r.seasonId, seasonB.id);
  EXPECT_EQ(history[2].r.time, 250);
  EXPECT_EQ(history[2].r.recordDate.toDateTimeString(), "2026-01-03 12:00:00");
}

TEST_F(TimerunRepositoryTests, GetCheckpoints_ParsesCheckpointsAndRanksRuns) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}))});
  repository->recordCompletion({completion(makeRecord(2, 250, {90, 190}, "2026-01-02 12:00:00"))});

  ETJump::Timerun::ListCheckpointsParams params{};
  params.season   = "Default";
  params.map      = TEST_MAP;
  params.run      = TEST_RUN;
  params.rank     = 1;
  params.exactMap = true;

  const auto rank1 = repository->getCheckpoints(params);
  ASSERT_EQ(rank1.size(), 1u);
  EXPECT_EQ(rank1[0].seasonID, DEFAULT_SEASON);
  EXPECT_EQ(rank1[0].map, TEST_MAP);
  EXPECT_EQ(rank1[0].run, TEST_RUN);
  EXPECT_EQ(rank1[0].runTime, 250);
  EXPECT_EQ(rank1[0].playerName, "player2");
  ASSERT_EQ(rank1[0].checkpoints.size(), MAX_TIMERUN_CHECKPOINTS);
  EXPECT_EQ(rank1[0].checkpoints[0], 90);
  EXPECT_EQ(rank1[0].checkpoints[1], 190);

  for (size_t i = 2; i < rank1[0].checkpoints.size(); i++) {
    EXPECT_EQ(rank1[0].checkpoints[i], TIMERUN_CHECKPOINT_NOT_SET);
  }

  params.rank = 2;

  const auto rank2 = repository->getCheckpoints(params);
  ASSERT_EQ(rank2.size(), 1u);
  EXPECT_EQ(rank2[0].runTime, 300);
  EXPECT_EQ(rank2[0].playerName, "player1");
  ASSERT_EQ(rank2[0].checkpoints.size(), MAX_TIMERUN_CHECKPOINTS);
  EXPECT_EQ(rank2[0].checkpoints[0], 100);
  EXPECT_EQ(rank2[0].checkpoints[1], 200);

  for (size_t i = 2; i < rank2[0].checkpoints.size(); i++) {
    EXPECT_EQ(rank2[0].checkpoints[i], TIMERUN_CHECKPOINT_NOT_SET);
  }

  params.rank = 3;
  EXPECT_TRUE(repository->getCheckpoints(params).empty());
}

TEST_F(TimerunRepositoryTests, GetCheckpoints_RanksRestartPerSeason) {
  const auto seasonA = addSeason(*repository, "Season A 2026");
  const auto seasonB = addSeason(*repository, "Season B 2026");

  const auto makeSeasonRecord = [](const int32_t userId, const int32_t time, const int32_t seasonId) {
    auto record     = makeRecord(userId, time, {100, 200});
    record.seasonId = seasonId;
    return record;
  };

  repository->recordCompletion(
    {completion(makeSeasonRecord(3, 300, seasonA.id)), completion(makeSeasonRecord(4, 250, seasonA.id)),
     completion(makeSeasonRecord(5, 320, seasonB.id)), completion(makeSeasonRecord(6, 240, seasonB.id))});

  ETJump::Timerun::ListCheckpointsParams params{};
  params.season   = std::string("2026");
  params.map      = TEST_MAP;
  params.run      = TEST_RUN;
  params.rank     = 1;
  params.exactMap = true;

  // a partial season match spans both seasons, and each season ranks its own
  // records, so rank 1 returns one row per season
  const auto rank1 = repository->getCheckpoints(params);
  ASSERT_EQ(rank1.size(), 2u);

  std::map<int32_t, const ETJump::Timerun::Checkpoints *> rank1BySeason;
  for (const auto &cp : rank1) {
    rank1BySeason[cp.seasonID] = &cp;
  }

  ASSERT_EQ(rank1BySeason.size(), 2u);
  EXPECT_EQ(rank1BySeason.at(seasonA.id)->runTime, 250);
  EXPECT_EQ(rank1BySeason.at(seasonA.id)->playerName, "player4");
  EXPECT_EQ(rank1BySeason.at(seasonB.id)->runTime, 240);
  EXPECT_EQ(rank1BySeason.at(seasonB.id)->playerName, "player6");

  params.rank = 2;

  const auto rank2 = repository->getCheckpoints(params);
  ASSERT_EQ(rank2.size(), 2u);

  std::map<int32_t, const ETJump::Timerun::Checkpoints *> rank2BySeason;
  for (const auto &cp : rank2) {
    rank2BySeason[cp.seasonID] = &cp;
  }

  ASSERT_EQ(rank2BySeason.size(), 2u);
  EXPECT_EQ(rank2BySeason.at(seasonA.id)->runTime, 300);
  EXPECT_EQ(rank2BySeason.at(seasonA.id)->playerName, "player3");
  EXPECT_EQ(rank2BySeason.at(seasonB.id)->runTime, 320);
  EXPECT_EQ(rank2BySeason.at(seasonB.id)->playerName, "player5");

  params.rank = 3;
  EXPECT_TRUE(repository->getCheckpoints(params).empty());
}

TEST_F(TimerunRepositoryTests, RecordCompletion_ComputesBestSegments) {
  const std::vector<int32_t> segments    = {200, 300, 500};
  const std::vector<int32_t> checkpoints = {200, 500};
  const auto updates                     = repository->recordCompletion({completion(makeRecord(1, 1000, checkpoints))});

  ASSERT_EQ(updates.count(DEFAULT_SEASON), 1u);
  const auto &seasonUpdates = updates.at(DEFAULT_SEASON);
  ASSERT_EQ(seasonUpdates.size(), 3u);

  for (size_t i = 0; i < seasonUpdates.size(); i++) {
    EXPECT_EQ(seasonUpdates[i].segment, static_cast<int32_t>(i) + 1);
    EXPECT_EQ(seasonUpdates[i].time, segments[i]);
    EXPECT_TRUE(seasonUpdates[i].personal);
    EXPECT_TRUE(seasonUpdates[i].global);
    EXPECT_FALSE(seasonUpdates[i].previousPersonal.has_value());
    EXPECT_FALSE(seasonUpdates[i].previousGlobal.has_value());
  }

  const auto global = repository->getBestSegments(DEFAULT_SEASON, TEST_MAP);
  ASSERT_EQ(global.count(TEST_RUN), 1u);
  const auto &globalSegments = global.at(TEST_RUN);
  ASSERT_EQ(globalSegments.size(), 3u);
  EXPECT_EQ(globalSegments[0], segments[0]);
  EXPECT_EQ(globalSegments[1], segments[1]);
  EXPECT_EQ(globalSegments[2], segments[2]);

  const auto personal = repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 1);
  ASSERT_EQ(personal.count(TEST_RUN), 1u);
  const auto &personalSegments = personal.at(TEST_RUN);
  ASSERT_EQ(personalSegments.size(), 3u);
  EXPECT_EQ(personalSegments[0], segments[0]);
  EXPECT_EQ(personalSegments[1], segments[1]);
  EXPECT_EQ(personalSegments[2], segments[2]);
}

TEST_F(TimerunRepositoryTests, RecordCompletion_Improvement_ReportsPreviousTimes) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}))});
  const auto firstImprovement =
    repository->recordCompletion({completion(makeRecord(2, 280, {90, 200}, "2026-01-02 12:00:00"))});

  const auto &firstUpdates = firstImprovement.at(DEFAULT_SEASON);
  ASSERT_EQ(firstUpdates.size(), 3u);

  // segment 1: 90 beats the global 100, and is player 2's first personal time
  EXPECT_EQ(firstUpdates[0].segment, 1);
  EXPECT_EQ(firstUpdates[0].time, 90);
  EXPECT_TRUE(firstUpdates[0].personal);
  EXPECT_TRUE(firstUpdates[0].global);
  EXPECT_FALSE(firstUpdates[0].previousPersonal.has_value());
  ASSERT_TRUE(firstUpdates[0].previousGlobal.has_value());
  EXPECT_EQ(*firstUpdates[0].previousGlobal, 100);

  // segment 2: 110 is only a personal best for player 2
  EXPECT_EQ(firstUpdates[1].segment, 2);
  EXPECT_EQ(firstUpdates[1].time, 110);
  EXPECT_TRUE(firstUpdates[1].personal);
  EXPECT_FALSE(firstUpdates[1].global);
  EXPECT_FALSE(firstUpdates[1].previousPersonal.has_value());
  ASSERT_TRUE(firstUpdates[1].previousGlobal.has_value());
  EXPECT_EQ(*firstUpdates[1].previousGlobal, 100);

  // segment 3: 80 beats the global 100
  EXPECT_EQ(firstUpdates[2].segment, 3);
  EXPECT_EQ(firstUpdates[2].time, 80);
  EXPECT_TRUE(firstUpdates[2].personal);
  EXPECT_TRUE(firstUpdates[2].global);
  EXPECT_FALSE(firstUpdates[2].previousPersonal.has_value());
  ASSERT_TRUE(firstUpdates[2].previousGlobal.has_value());
  EXPECT_EQ(*firstUpdates[2].previousGlobal, 100);

  // now player 2 runs 80, 120, 70 - only segments 1 and 3 improve,
  // because 120 is slower than their previous 110 on segment 2
  const auto secondImprovement =
    repository->recordCompletion({completion(makeRecord(2, 270, {80, 200}, "2026-01-03 12:00:00"), true, false)});

  const auto &secondUpdates = secondImprovement.at(DEFAULT_SEASON);
  ASSERT_EQ(secondUpdates.size(), 2u);

  EXPECT_EQ(secondUpdates[0].segment, 1);
  EXPECT_EQ(secondUpdates[0].time, 80);
  EXPECT_TRUE(secondUpdates[0].personal);
  EXPECT_TRUE(secondUpdates[0].global);
  ASSERT_TRUE(secondUpdates[0].previousPersonal.has_value());
  EXPECT_EQ(*secondUpdates[0].previousPersonal, 90);
  ASSERT_TRUE(secondUpdates[0].previousGlobal.has_value());
  EXPECT_EQ(*secondUpdates[0].previousGlobal, 90);

  EXPECT_EQ(secondUpdates[1].segment, 3);
  EXPECT_EQ(secondUpdates[1].time, 70);
  EXPECT_TRUE(secondUpdates[1].personal);
  EXPECT_TRUE(secondUpdates[1].global);
  ASSERT_TRUE(secondUpdates[1].previousPersonal.has_value());
  EXPECT_EQ(*secondUpdates[1].previousPersonal, 80);
  ASSERT_TRUE(secondUpdates[1].previousGlobal.has_value());
  EXPECT_EQ(*secondUpdates[1].previousGlobal, 80);

  const auto global = repository->getBestSegments(DEFAULT_SEASON, TEST_MAP).at(TEST_RUN);
  ASSERT_EQ(global.size(), 3u);
  EXPECT_EQ(global[0], 80);
  EXPECT_EQ(global[1], 100);
  EXPECT_EQ(global[2], 70);

  const auto personal = repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 2).at(TEST_RUN);
  ASSERT_EQ(personal.size(), 3u);
  EXPECT_EQ(personal[0], 80);
  EXPECT_EQ(personal[1], 110);
  EXPECT_EQ(personal[2], 70);
}

TEST_F(TimerunRepositoryTests, RecordCompletion_FullCheckpointArray_AddsFinishSegment) {
  std::vector<int32_t> checkpoints;
  checkpoints.reserve(MAX_TIMERUN_CHECKPOINTS);

  for (int32_t i = 1; i <= MAX_TIMERUN_CHECKPOINTS; i++) {
    checkpoints.push_back(i * 1000);
  }

  const auto updates = repository->recordCompletion({completion(makeRecord(1, 17000, checkpoints))});

  const auto &seasonUpdates = updates.at(DEFAULT_SEASON);
  ASSERT_EQ(seasonUpdates.size(), static_cast<size_t>(MAX_TIMERUN_CHECKPOINTS) + 1);

  for (size_t i = 0; i < seasonUpdates.size(); i++) {
    EXPECT_EQ(seasonUpdates[i].segment, static_cast<int32_t>(i) + 1);
    EXPECT_EQ(seasonUpdates[i].time, 1000);
  }

  // 16 checkpoint legs plus the finishing split
  const auto global = repository->getBestSegments(DEFAULT_SEASON, TEST_MAP).at(TEST_RUN);
  ASSERT_EQ(global.size(), static_cast<size_t>(MAX_TIMERUN_CHECKPOINTS) + 1);
  EXPECT_EQ(global.back(), 1000);
}

TEST_F(TimerunRepositoryTests, GetBestSegmentDetails_ReportsGlobalHoldersAndPersonalRanks) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}))});
  repository->recordCompletion({completion(makeRecord(2, 280, {90, 200}, "2026-01-02 12:00:00"))});

  ETJump::Timerun::BestSegmentsParams globalParams{};
  globalParams.season   = "Default";
  globalParams.map      = TEST_MAP;
  globalParams.run      = TEST_RUN;
  globalParams.global   = true;
  globalParams.exactMap = true;

  const auto global = repository->getBestSegmentDetails(globalParams);
  ASSERT_EQ(global.size(), 3u);
  EXPECT_EQ(global[0].segment, 1);
  EXPECT_EQ(global[0].time, 90);
  EXPECT_EQ(global[0].userId, 2);
  EXPECT_EQ(global[1].segment, 2);
  EXPECT_EQ(global[1].time, 100);
  EXPECT_EQ(global[1].userId, 1);
  EXPECT_EQ(global[2].segment, 3);
  EXPECT_EQ(global[2].time, 80);
  EXPECT_EQ(global[2].userId, 2);

  ETJump::Timerun::BestSegmentsParams personalParams = globalParams;
  personalParams.global                              = false;
  personalParams.userId                              = 1;

  const auto personal = repository->getBestSegmentDetails(personalParams);
  ASSERT_EQ(personal.size(), 3u);
  EXPECT_EQ(personal[0].time, 100);
  EXPECT_EQ(personal[0].rank, 2); // player 2's 90 is faster
  EXPECT_EQ(personal[1].time, 100);
  EXPECT_EQ(personal[1].rank, 1); // player 2's 110 is slower
  EXPECT_EQ(personal[2].time, 100);
  EXPECT_EQ(personal[2].rank, 2); // player 2's 80 is faster
}

TEST_F(TimerunRepositoryTests, GetBestSegmentHistory_FlagsRemovedRunsAndSplits) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}))});

  const auto historyFor = [this](const int32_t segment) {
    ETJump::Timerun::BestSegmentHistoryParams params{};
    params.season   = "Default";
    params.map      = TEST_MAP;
    params.run      = TEST_RUN;
    params.userId   = 1;
    params.segment  = segment;
    params.exactMap = true;
    return repository->getBestSegmentHistory(params);
  };

  auto segment1 = historyFor(1);
  ASSERT_EQ(segment1.size(), 1u);
  EXPECT_FALSE(segment1[0].removed);

  // removing a single split flags only that segment's history row
  ETJump::Timerun::RemoveSegmentParams removeParams{};
  removeParams.map       = TEST_MAP;
  removeParams.run       = TEST_RUN;
  removeParams.userId    = 1;
  removeParams.segment   = 2;
  removeParams.removedBy = 9;
  repository->removeSegment(removeParams, parseTime("2026-03-01 12:00:00"));

  segment1 = historyFor(1);
  ASSERT_EQ(segment1.size(), 1u);
  EXPECT_FALSE(segment1[0].removed);

  auto segment2 = historyFor(2);
  ASSERT_EQ(segment2.size(), 1u);
  EXPECT_TRUE(segment2[0].removed);

  // archiving the run flags all of its history rows
  ASSERT_EQ(repository->removeRecord(recordRemoval(1, 9), parseTime("2026-03-02 12:00:00")).size(), 1u);

  segment1 = historyFor(1);
  ASSERT_EQ(segment1.size(), 1u);
  EXPECT_TRUE(segment1[0].removed);

  // restoring the run clears the flag for the surviving splits,
  // while the split that is still archived stays flagged
  const auto removedPage = repository->getRemovedRecords(listRemovedParams());
  ASSERT_EQ(removedPage.entries.size(), 1u);

  ETJump::Timerun::RestoreRecordParams restoreParams{};
  restoreParams.callerId = 9;
  restoreParams.recordId = removedPage.entries[0].records[0].id;
  restoreParams.isAdmin  = true;
  repository->restoreRecord(restoreParams, parseTime("2026-03-03 12:00:00"));

  segment1 = historyFor(1);
  ASSERT_EQ(segment1.size(), 1u);
  EXPECT_FALSE(segment1[0].removed);

  segment2 = historyFor(2);
  ASSERT_EQ(segment2.size(), 1u);
  EXPECT_TRUE(segment2[0].removed);
}

TEST_F(TimerunRepositoryTests, GetBestSegmentHistory_OrdersImprovements) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}, "2026-01-01 12:00:00"))});
  repository->recordCompletion({completion(makeRecord(1, 350, {90, 200}, "2026-01-02 12:00:00"), false, false)});
  repository->recordCompletion({completion(makeRecord(1, 280, {80, 200}, "2026-01-03 12:00:00"), true, false)});

  ETJump::Timerun::BestSegmentHistoryParams params{};
  params.season   = "Default";
  params.map      = TEST_MAP;
  params.run      = TEST_RUN;
  params.userId   = 1;
  params.segment  = 1;
  params.exactMap = true;

  // newest improvement first, all still present despite later ones beating them
  const auto history = repository->getBestSegmentHistory(params);
  ASSERT_EQ(history.size(), 3u);

  EXPECT_EQ(history[0].recordDate.toDateTimeString(), "2026-01-03 12:00:00");
  EXPECT_EQ(history[0].time, 80);
  EXPECT_EQ(history[1].recordDate.toDateTimeString(), "2026-01-02 12:00:00");
  EXPECT_EQ(history[1].time, 90);
  EXPECT_EQ(history[2].recordDate.toDateTimeString(), "2026-01-01 12:00:00");
  EXPECT_EQ(history[2].time, 100);

  for (const auto &entry : history) {
    EXPECT_EQ(entry.segment, 1);
    EXPECT_FALSE(entry.removed);
  }
}

TEST_F(TimerunRepositoryTests, GetBestSegmentDetails_ResolvesPartialRunNames) {
  auto firstRun = makeRecord(1, 300, {100, 200});
  firstRun.run  = "first_run";

  auto secondRun = makeRecord(2, 280, {90, 200}, "2026-01-02 12:00:00");
  secondRun.run  = "second_run";

  repository->recordCompletion({completion(firstRun), completion(secondRun)});

  ETJump::Timerun::BestSegmentsParams params{};
  params.season   = std::string("Default");
  params.map      = TEST_MAP;
  params.global   = true;
  params.exactMap = true;

  // a unique partial run match resolves to the stored run name
  params.run = "first_r";

  const auto partial = repository->getBestSegmentDetails(params);
  ASSERT_EQ(partial.size(), 3u);
  EXPECT_EQ(partial[0].run, "first_run");

  // a partial match hitting several runs is rejected with suggestions
  params.run = "run";
  EXPECT_THROW(repository->getBestSegmentDetails(params), std::runtime_error);
}

TEST_F(TimerunRepositoryTests, TiedGlobalSegment_PrefersEarlierDateThenLowerUserId) {
  ETJump::Timerun::BestSegmentsParams params{};
  params.season   = "Default";
  params.map      = TEST_MAP;
  params.run      = TEST_RUN;
  params.global   = true;
  params.exactMap = true;

  // user 2 sets the global with the first run
  repository->recordCompletion({completion(makeRecord(2, 300, {100, 200}, "2026-01-01 12:00:00"))});

  // the same time and date from a lower user ID takes over
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}, "2026-01-01 12:00:00"))});

  auto global = repository->getBestSegmentDetails(params);
  ASSERT_EQ(global.size(), 3u);

  for (const auto &segment : global) {
    EXPECT_EQ(segment.userId, 1);
  }

  // an equal time from a later date must not take it back
  repository->recordCompletion({completion(makeRecord(3, 300, {100, 200}, "2026-01-02 12:00:00"))});

  global = repository->getBestSegmentDetails(params);
  ASSERT_EQ(global.size(), 3u);

  for (const auto &segment : global) {
    EXPECT_EQ(segment.userId, 1);
    EXPECT_EQ(segment.recordDate.toDateTimeString(), "2026-01-01 12:00:00");
  }
}

TEST_F(TimerunRepositoryTests, TiedPersonalSegment_KeepsEarlierDate) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}, "2026-01-01 12:00:00"))});

  // equal time on a later date is not an improvement for the player either
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}, "2026-01-02 12:00:00"), false, false)});

  ETJump::Timerun::BestSegmentsParams params{};
  params.season   = "Default";
  params.map      = TEST_MAP;
  params.run      = TEST_RUN;
  params.global   = false;
  params.userId   = 1;
  params.exactMap = true;

  const auto personal = repository->getBestSegmentDetails(params);
  ASSERT_EQ(personal.size(), 3u);

  for (const auto &segment : personal) {
    EXPECT_EQ(segment.time, 100);
    EXPECT_EQ(segment.recordDate.toDateTimeString(), "2026-01-01 12:00:00");
  }
}

TEST_F(TimerunRepositoryTests, RemoveRecord_ArchivesRecordAndVoidsSegments) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}))});

  ETJump::Timerun::RemoveRecordParams params{};
  params.map       = TEST_MAP;
  params.run       = TEST_RUN;
  params.userId    = 1;
  params.removedBy = 9;
  params.reason    = "testing";

  const auto removed = repository->removeRecord(params, parseTime("2026-03-01 12:00:00"));
  ASSERT_EQ(removed.size(), 1u);
  EXPECT_EQ(removed[0].userId, 1);

  EXPECT_TRUE(repository->getRecords().empty());
  EXPECT_TRUE(repository->getBestSegments(DEFAULT_SEASON, TEST_MAP).empty());
  EXPECT_TRUE(repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 1).empty());

  const auto page = repository->getRemovedRecords(listRemovedParams());
  ASSERT_EQ(page.entries.size(), 1u);
  ASSERT_EQ(page.entries[0].records.size(), 1u);
  EXPECT_EQ(page.entries[0].records[0].record.time, 300);
  EXPECT_EQ(page.entries[0].records[0].removedBy, 9);
  ASSERT_TRUE(page.entries[0].records[0].reason.has_value());
  EXPECT_EQ(*page.entries[0].records[0].reason, "testing");

  // history is kept, and is now flagged as archived
  const auto history = repository->getHistoricalRecords(historyParams(1));
  ASSERT_EQ(history.size(), 1u);
  EXPECT_TRUE(history[0].removed);
}

TEST_F(TimerunRepositoryTests, RemoveRecord_RequiresExactMapAndRun) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}))});

  auto params = recordRemoval(1, 9);

  params.map = "test_ma";
  EXPECT_THROW(repository->removeRecord(params, parseTime("2026-03-01 12:00:00")), std::runtime_error);
  ASSERT_EQ(repository->getRecords().size(), 1u);
  EXPECT_TRUE(repository->getRemovedRecords(listRemovedParams()).entries.empty());

  params.map = TEST_MAP;
  params.run = "test_ru";
  EXPECT_THROW(repository->removeRecord(params, parseTime("2026-03-01 12:00:00")), std::runtime_error);
  ASSERT_EQ(repository->getRecords().size(), 1u);
  EXPECT_TRUE(repository->getRemovedRecords(listRemovedParams()).entries.empty());

  // the exact parameters still remove the record
  params.run = TEST_RUN;
  ASSERT_EQ(repository->removeRecord(params, parseTime("2026-03-01 12:00:00")).size(), 1u);
  EXPECT_TRUE(repository->getRecords().empty());
}

TEST_F(TimerunRepositoryTests, RemoveRecord_RequiresExactSeason) {
  const auto secondSeason = addSeason(*repository, "Season 2");

  auto defaultSeasonRecord    = makeRecord(1, 300);
  auto secondSeasonRecord     = defaultSeasonRecord;
  secondSeasonRecord.seasonId = secondSeason.id;

  repository->recordCompletion({completion(defaultSeasonRecord), completion(secondSeasonRecord)});
  ASSERT_EQ(repository->getRecords().size(), 2u);

  auto params = recordRemoval(1, 9, "Season");
  EXPECT_THROW(repository->removeRecord(params, parseTime("2026-03-01 12:00:00")), std::runtime_error);
  EXPECT_EQ(repository->getRecords().size(), 2u);
  EXPECT_TRUE(repository->getRemovedRecords(listRemovedParams()).entries.empty());

  // season names are matched exactly, but case-insensitively
  params.season = "season 2";

  const auto removed = repository->removeRecord(params, parseTime("2026-03-01 12:00:00"));
  ASSERT_EQ(removed.size(), 1u);
  EXPECT_EQ(removed[0].seasonId, secondSeason.id);
  EXPECT_EQ(repository->getRecords().size(), 1u);
}

TEST_F(TimerunRepositoryTests, RestoreRecord_RestoresRecordAndRebuildsSegments) {
  const std::vector<int32_t> segments    = {200, 300, 500};
  const std::vector<int32_t> checkpoints = {200, 500};
  const auto updates                     = repository->recordCompletion({completion(makeRecord(1, 1000, checkpoints))});

  ETJump::Timerun::RemoveRecordParams removeParams{};
  removeParams.map       = TEST_MAP;
  removeParams.run       = TEST_RUN;
  removeParams.userId    = 1;
  removeParams.removedBy = 9;
  repository->removeRecord(removeParams, parseTime("2026-03-01 12:00:00"));

  const auto page = repository->getRemovedRecords(listRemovedParams());
  ASSERT_EQ(page.entries.size(), 1u);

  ETJump::Timerun::RestoreRecordParams restoreParams{};
  restoreParams.callerId = 9;
  restoreParams.recordId = page.entries[0].records[0].id;
  restoreParams.isAdmin  = true;

  const auto result = repository->restoreRecord(restoreParams, parseTime("2026-03-02 12:00:00"));
  EXPECT_EQ(result.numTargeted, 1);
  ASSERT_EQ(result.restored.size(), 1u);
  EXPECT_TRUE(result.conflicts.empty());

  const auto records = repository->getRecords();
  ASSERT_EQ(records.size(), 1u);
  EXPECT_EQ(records[0].time, 1000);

  const auto global = repository->getBestSegments(DEFAULT_SEASON, TEST_MAP);
  ASSERT_EQ(global.count(TEST_RUN), 1u);
  const auto &globalSegments = global.at(TEST_RUN);
  ASSERT_EQ(globalSegments.size(), 3u);
  EXPECT_EQ(globalSegments[0], segments[0]);
  EXPECT_EQ(globalSegments[1], segments[1]);
  EXPECT_EQ(globalSegments[2], segments[2]);
}

TEST_F(TimerunRepositoryTests, RemoveRecord_WithoutSeason_CascadesAcrossSeasons) {
  ETJump::Timerun::AddSeasonParams addSeason{};
  addSeason.name          = "Season 2";
  addSeason.startTime     = TimeUtils::Time::fromDate(TimeUtils::Date{2026, 1, 1});
  const auto secondSeason = repository->addSeason(addSeason);

  auto defaultSeasonRecord    = makeRecord(1, 300);
  auto secondSeasonRecord     = defaultSeasonRecord;
  secondSeasonRecord.seasonId = secondSeason.id;

  repository->recordCompletion({completion(defaultSeasonRecord), completion(secondSeasonRecord)});
  ASSERT_EQ(repository->getRecords().size(), 2u);

  ETJump::Timerun::RemoveRecordParams removeParams{};
  removeParams.map       = TEST_MAP;
  removeParams.run       = TEST_RUN;
  removeParams.userId    = 1;
  removeParams.removedBy = 1;

  const auto removed = repository->removeRecord(removeParams, parseTime("2026-03-01 12:00:00"));
  EXPECT_EQ(removed.size(), 2u);
  EXPECT_TRUE(repository->getRecords().empty());

  // both season copies are grouped into a single removed completion
  const auto page = repository->getRemovedRecords(listRemovedParams());
  ASSERT_EQ(page.entries.size(), 1u);
  ASSERT_EQ(page.entries[0].records.size(), 2u);

  ETJump::Timerun::RestoreRecordParams restoreParams{};
  restoreParams.callerId = 1;
  restoreParams.recordId = page.entries[0].records[0].id;
  restoreParams.isAdmin  = true;

  const auto result = repository->restoreRecord(restoreParams, parseTime("2026-03-02 12:00:00"));
  EXPECT_EQ(result.numTargeted, 2);
  EXPECT_EQ(result.restored.size(), 2u);
  EXPECT_EQ(repository->getRecords().size(), 2u);
}

TEST_F(TimerunRepositoryTests, RemoveRecord_CascadeOnlyMatchesSameCompletionDate) {
  const auto secondSeason = addSeason(*repository, "Season 2");

  // the same run in another season, but from a different completion, so the cascade must not touch it
  auto defaultSeasonRecord      = makeRecord(1, 300, {}, "2026-01-01 12:00:00");
  auto secondSeasonRecord       = defaultSeasonRecord;
  secondSeasonRecord.seasonId   = secondSeason.id;
  secondSeasonRecord.time       = 320;
  secondSeasonRecord.recordDate = parseTime("2026-02-01 12:00:00");

  repository->recordCompletion({completion(defaultSeasonRecord), completion(secondSeasonRecord)});
  ASSERT_EQ(repository->getRecords().size(), 2u);

  const auto removed = repository->removeRecord(recordRemoval(1, 9), parseTime("2026-03-01 12:00:00"));
  ASSERT_EQ(removed.size(), 1u);
  EXPECT_EQ(removed[0].seasonId, DEFAULT_SEASON);

  const auto records = repository->getRecords();
  ASSERT_EQ(records.size(), 1u);
  EXPECT_EQ(records[0].seasonId, secondSeason.id);
  EXPECT_EQ(records[0].time, 320);
}

TEST_F(TimerunRepositoryTests, GetRemovedRecords_FiltersAndPages) {
  auto runA = makeRecord(1, 300);
  runA.run  = "run_a";

  auto runB = makeRecord(1, 320, {}, "2026-01-02 12:00:00");
  runB.run  = "run_b";

  repository->recordCompletion({completion(runA), completion(runB)});

  ASSERT_EQ(repository->removeRecord(recordRemoval(1, 9, "", "run_a"), parseTime("2026-03-01 12:00:00")).size(), 1u);
  ASSERT_EQ(repository->removeRecord(recordRemoval(1, 10, "", "run_b"), parseTime("2026-03-02 12:00:00")).size(), 1u);

  auto params     = listRemovedParams();
  params.pageSize = 1;

  // newest removal first, one entry per page
  auto page1 = repository->getRemovedRecords(params);
  EXPECT_EQ(page1.numPages, 2);
  ASSERT_EQ(page1.entries.size(), 1u);
  EXPECT_EQ(page1.entries[0].records[0].record.run, "run_b");

  params.page = 2;

  auto page2 = repository->getRemovedRecords(params);
  ASSERT_EQ(page2.entries.size(), 1u);
  EXPECT_EQ(page2.entries[0].records[0].record.run, "run_a");

  // an out-of-range page clamps to the last page
  params.page = 99;

  const auto clamped = repository->getRemovedRecords(params);
  EXPECT_EQ(clamped.page, 2);
  ASSERT_EQ(clamped.entries.size(), 1u);
  EXPECT_EQ(clamped.entries[0].records[0].record.run, "run_a");

  // filters are applied before grouping and paging
  auto filter      = listRemovedParams();
  filter.removedBy = 10;

  const auto byRemover = repository->getRemovedRecords(filter);
  ASSERT_EQ(byRemover.entries.size(), 1u);
  EXPECT_EQ(byRemover.entries[0].records[0].record.run, "run_b");

  filter.removedBy.reset();
  filter.userId = 1;
  EXPECT_EQ(repository->getRemovedRecords(filter).entries.size(), 2u);

  filter.userId.reset();
  filter.run = "un_a"; // partial match, like the listing commands accept

  const auto byRun = repository->getRemovedRecords(filter);
  ASSERT_EQ(byRun.entries.size(), 1u);
  EXPECT_EQ(byRun.entries[0].records[0].record.run, "run_a");
}

TEST_F(TimerunRepositoryTests, GetRemovedSegments_FiltersAndPages) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}))});

  ETJump::Timerun::RemoveSegmentParams removeParams{};
  removeParams.map       = TEST_MAP;
  removeParams.run       = TEST_RUN;
  removeParams.userId    = 1;
  removeParams.segment   = 1;
  removeParams.removedBy = 9;
  ASSERT_EQ(repository->removeSegment(removeParams, parseTime("2026-03-01 12:00:00")).size(), 1u);

  removeParams.segment   = 3;
  removeParams.removedBy = 10;
  ASSERT_EQ(repository->removeSegment(removeParams, parseTime("2026-03-02 12:00:00")).size(), 1u);

  auto params     = listRemovedSegmentsParams();
  params.pageSize = 1;

  // newest removal first, one entry per page
  auto page1 = repository->getRemovedSegments(params);
  EXPECT_EQ(page1.numPages, 2);
  ASSERT_EQ(page1.entries.size(), 1u);
  EXPECT_EQ(page1.entries[0].segments[0].segment, 3);
  EXPECT_EQ(page1.entries[0].segments[0].removedBy, 10);

  params.page = 2;

  auto page2 = repository->getRemovedSegments(params);
  ASSERT_EQ(page2.entries.size(), 1u);
  EXPECT_EQ(page2.entries[0].segments[0].segment, 1);
  EXPECT_EQ(page2.entries[0].segments[0].removedBy, 9);

  // an out-of-range page clamps to the last page
  params.page = 99;

  const auto clamped = repository->getRemovedSegments(params);
  EXPECT_EQ(clamped.page, 2);
  ASSERT_EQ(clamped.entries.size(), 1u);
  EXPECT_EQ(clamped.entries[0].segments[0].segment, 1);

  // filters are applied before grouping and paging
  auto filter      = listRemovedSegmentsParams();
  filter.removedBy = 10;

  const auto byRemover = repository->getRemovedSegments(filter);
  ASSERT_EQ(byRemover.entries.size(), 1u);
  EXPECT_EQ(byRemover.entries[0].segments[0].segment, 3);

  filter.removedBy.reset();
  filter.userId = 1;
  EXPECT_EQ(repository->getRemovedSegments(filter).entries.size(), 2u);

  filter.userId.reset();
  filter.map = "test_ma"; // partial match
  EXPECT_EQ(repository->getRemovedSegments(filter).entries.size(), 2u);

  filter.map = "";
  filter.run = "test_ru"; // partial match
  EXPECT_EQ(repository->getRemovedSegments(filter).entries.size(), 2u);

  filter.run     = "";
  filter.segment = 3;

  const auto bySegment = repository->getRemovedSegments(filter);
  ASSERT_EQ(bySegment.entries.size(), 1u);
  EXPECT_EQ(bySegment.entries[0].segments[0].segment, 3);

  filter.segment.reset();
  filter.season = "Default";
  EXPECT_EQ(repository->getRemovedSegments(filter).entries.size(), 2u);
}

TEST_F(TimerunRepositoryTests, RestoreRecord_Conflict_RequiresForceAndSwapsOccupant) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}))});
  const auto removed = repository->removeRecord(recordRemoval(1, 9), parseTime("2026-03-01 12:00:00"));
  ASSERT_EQ(removed.size(), 1u);

  // the player sets a slower run in the meantime, occupying the record slot
  repository->recordCompletion({completion(makeRecord(1, 320, {120, 200}, "2026-04-01 12:00:00"))});

  const auto archived = repository->getRemovedRecords(listRemovedParams());
  ASSERT_EQ(archived.entries.size(), 1u);
  const int32_t archivedId = archived.entries[0].records[0].id;

  ETJump::Timerun::RestoreRecordParams restore{};
  restore.callerId = 9;
  restore.recordId = archivedId;
  restore.isAdmin  = true;

  const auto conflict = repository->restoreRecord(restore, parseTime("2026-05-01 12:00:00"));
  EXPECT_EQ(conflict.numTargeted, 1);
  EXPECT_TRUE(conflict.restored.empty());
  ASSERT_EQ(conflict.conflicts.size(), 1u);
  EXPECT_EQ(conflict.conflicts[0].existing.time, 320);
  EXPECT_EQ(conflict.conflicts[0].toRestore.time, 300);

  // nothing was written, and the archive ID is still usable for a forced retry
  const auto occupant = runTopRecords(*repository);
  ASSERT_EQ(occupant.size(), 1u);
  EXPECT_EQ(occupant.front().time, 320);
  EXPECT_EQ(repository->getRemovedRecords(listRemovedParams()).entries.size(), 1u);

  restore.force     = true;
  const auto forced = repository->restoreRecord(restore, parseTime("2026-05-02 12:00:00"));
  ASSERT_EQ(forced.restored.size(), 1u);
  EXPECT_EQ(forced.restored[0].time, 300);
  ASSERT_EQ(forced.swapped.size(), 1u);
  EXPECT_EQ(forced.swapped[0].time, 320);

  const auto top = runTopRecords(*repository);
  ASSERT_EQ(top.size(), 1u);
  EXPECT_EQ(top.front().time, 300);

  // the displaced occupant is archived in place of the restored record
  const auto swappedPage = repository->getRemovedRecords(listRemovedParams());
  ASSERT_EQ(swappedPage.entries.size(), 1u);
  ASSERT_EQ(swappedPage.entries[0].records.size(), 1u);
  EXPECT_EQ(swappedPage.entries[0].records[0].record.time, 320);

  // the restored run re-derives its splits, and the displaced run's are gone
  const auto segments = repository->getBestSegments(DEFAULT_SEASON, TEST_MAP).at(TEST_RUN);
  ASSERT_EQ(segments.size(), 3u);
  EXPECT_EQ(segments[0], 100);
  EXPECT_EQ(segments[1], 100);
  EXPECT_EQ(segments[2], 100);
}

TEST_F(TimerunRepositoryTests, RestoreRecord_Permission_DeniesOthersRemoval) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}))});
  ASSERT_EQ(repository->removeRecord(recordRemoval(1, 9), parseTime("2026-03-01 12:00:00")).size(), 1u);

  const auto archived = repository->getRemovedRecords(listRemovedParams());
  ASSERT_EQ(archived.entries.size(), 1u);

  ETJump::Timerun::RestoreRecordParams restore{};
  restore.callerId = 1; // removed by user 9, and the caller is not an admin
  restore.recordId = archived.entries[0].records[0].id;
  restore.isAdmin  = false;

  EXPECT_THROW(repository->restoreRecord(restore, parseTime("2026-03-02 12:00:00")), std::runtime_error);
  EXPECT_TRUE(repository->getRecords().empty());
  EXPECT_EQ(repository->getRemovedRecords(listRemovedParams()).entries.size(), 1u);
}

TEST_F(TimerunRepositoryTests, RestoreRecord_Permission_RestoresOnlyOwnCopies) {
  const auto secondSeason = addSeason(*repository, "Season 2");

  auto defaultSeasonRecord    = makeRecord(1, 300, {100, 200});
  auto secondSeasonRecord     = defaultSeasonRecord;
  secondSeasonRecord.seasonId = secondSeason.id;

  repository->recordCompletion({completion(defaultSeasonRecord), completion(secondSeasonRecord)});

  // the same completion was removed from each season by a different user
  ASSERT_EQ(repository->removeRecord(recordRemoval(1, 9, "Default"), parseTime("2026-03-01 12:00:00")).size(), 1u);
  ASSERT_EQ(repository->removeRecord(recordRemoval(1, 10, "Season 2"), parseTime("2026-03-02 12:00:00")).size(), 1u);
  EXPECT_TRUE(repository->getRecords().empty());

  const auto archived = repository->getRemovedRecords(listRemovedParams());
  ASSERT_EQ(archived.entries.size(), 1u);
  ASSERT_EQ(archived.entries[0].records.size(), 2u);

  // caller 9 removed the default-season copy, but not the Season 2 one
  ETJump::Timerun::RestoreRecordParams restore{};
  restore.callerId = 9;
  restore.recordId = archived.entries[0].records[0].id; // default season, sorted first
  restore.isAdmin  = false;

  const auto result = repository->restoreRecord(restore, parseTime("2026-03-03 12:00:00"));
  EXPECT_EQ(result.numTargeted, 2);
  ASSERT_EQ(result.restored.size(), 1u);
  EXPECT_EQ(result.restored[0].seasonId, DEFAULT_SEASON);
  ASSERT_EQ(result.skipped.size(), 1u);
  EXPECT_EQ(result.skipped[0].record.seasonId, secondSeason.id);
  EXPECT_EQ(result.skipped[0].removedBy, 10);

  const auto records = repository->getRecords();
  ASSERT_EQ(records.size(), 1u);
  EXPECT_EQ(records[0].seasonId, DEFAULT_SEASON);

  const auto remaining = repository->getRemovedRecords(listRemovedParams());
  ASSERT_EQ(remaining.entries.size(), 1u);
  ASSERT_EQ(remaining.entries[0].records.size(), 1u);
  EXPECT_EQ(remaining.entries[0].records[0].record.seasonId, secondSeason.id);
  EXPECT_EQ(remaining.entries[0].records[0].removedBy, 10);
}

TEST_F(TimerunRepositoryTests, RemovedSegment_IsNotResurrectedByRecordRestore) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}))});
  repository->recordCompletion({completion(makeRecord(2, 280, {90, 200}, "2026-01-02 12:00:00"))});

  // player 1's 100 holds segment 2 globally, player 2 has 110 there
  const auto before = repository->getBestSegments(DEFAULT_SEASON, TEST_MAP).at(TEST_RUN);
  ASSERT_EQ(before.size(), 3u);
  EXPECT_EQ(before[1], 100);

  ETJump::Timerun::RemoveSegmentParams removeParams{};
  removeParams.map       = TEST_MAP;
  removeParams.run       = TEST_RUN;
  removeParams.userId    = 1;
  removeParams.segment   = 2;
  removeParams.removedBy = 9;
  removeParams.reason    = "testing";

  const auto removed = repository->removeSegment(removeParams, parseTime("2026-03-01 12:00:00"));
  ASSERT_EQ(removed.size(), 1u);
  EXPECT_EQ(removed[0].segment, 2);
  EXPECT_EQ(removed[0].time, 100);

  // the global falls back to player 2's 110, and player 1's split is gone
  EXPECT_EQ(repository->getBestSegments(DEFAULT_SEASON, TEST_MAP).at(TEST_RUN)[1], 110);
  EXPECT_EQ(repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 1).at(TEST_RUN)[1], 0);

  const auto removedPage = repository->getRemovedSegments(listRemovedSegmentsParams());
  ASSERT_EQ(removedPage.entries.size(), 1u);
  ASSERT_EQ(removedPage.entries[0].segments.size(), 1u);
  EXPECT_EQ(removedPage.entries[0].segments[0].userId, 1);
  EXPECT_EQ(removedPage.entries[0].segments[0].removedBy, 9);
  const int32_t archivedSegmentId = removedPage.entries[0].segments[0].id;

  // archive player 1's run as well
  const auto removedRecords = repository->removeRecord(recordRemoval(1, 9), parseTime("2026-03-02 12:00:00"));
  ASSERT_EQ(removedRecords.size(), 1u);

  const auto removedRecordPage = repository->getRemovedRecords(listRemovedParams());
  ASSERT_EQ(removedRecordPage.entries.size(), 1u);
  const int32_t archivedRecordId = removedRecordPage.entries[0].records[0].id;

  ETJump::Timerun::RestoreSegmentParams restoreSegmentParams{};
  restoreSegmentParams.callerId  = 9;
  restoreSegmentParams.segmentId = archivedSegmentId;
  restoreSegmentParams.isAdmin   = true;

  // a split cannot come back while its source run is archived
  EXPECT_THROW(repository->restoreSegment(restoreSegmentParams, parseTime("2026-03-03 12:00:00")), std::runtime_error);

  ETJump::Timerun::RestoreRecordParams restoreRecordParams{};
  restoreRecordParams.callerId = 9;
  restoreRecordParams.recordId = archivedRecordId;
  restoreRecordParams.isAdmin  = true;

  const auto restoredRecord = repository->restoreRecord(restoreRecordParams, parseTime("2026-03-04 12:00:00"));
  ASSERT_EQ(restoredRecord.restored.size(), 1u);

  // restoring the run skips the independently removed split
  EXPECT_EQ(repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 1).at(TEST_RUN)[1], 0);
  EXPECT_EQ(repository->getBestSegments(DEFAULT_SEASON, TEST_MAP).at(TEST_RUN)[1], 110);

  // once the run is back, the split can be restored and reclaims the global
  const auto restoredSegment = repository->restoreSegment(restoreSegmentParams, parseTime("2026-03-05 12:00:00"));
  ASSERT_EQ(restoredSegment.restored.size(), 1u);
  EXPECT_EQ(repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 1).at(TEST_RUN)[1], 100);
  EXPECT_EQ(repository->getBestSegments(DEFAULT_SEASON, TEST_MAP).at(TEST_RUN)[1], 100);
  EXPECT_TRUE(repository->getRemovedSegments(listRemovedSegmentsParams()).entries.empty());
}

TEST_F(TimerunRepositoryTests, RemoveSegment_WithoutSeason_CascadesAcrossSeasons) {
  const auto secondSeason                = addSeason(*repository, "Season 2");
  const std::vector<int32_t> segments    = {200, 300, 500};
  const std::vector<int32_t> checkpoints = {200, 500};

  auto defaultSeasonRecord    = makeRecord(1, 1000, checkpoints);
  auto secondSeasonRecord     = defaultSeasonRecord;
  secondSeasonRecord.seasonId = secondSeason.id;

  repository->recordCompletion({completion(defaultSeasonRecord), completion(secondSeasonRecord)});

  ETJump::Timerun::RemoveSegmentParams removeParams{};
  removeParams.map       = TEST_MAP;
  removeParams.run       = TEST_RUN;
  removeParams.userId    = 1;
  removeParams.segment   = 1;
  removeParams.removedBy = 9;

  const auto removed = repository->removeSegment(removeParams, parseTime("2026-03-01 12:00:00"));
  ASSERT_EQ(removed.size(), 2u);

  std::set<int32_t> seasonIds;
  for (const auto &segment : removed) {
    seasonIds.insert(segment.seasonId);
  }

  ASSERT_EQ(seasonIds.size(), 2u);
  EXPECT_TRUE(seasonIds.count(DEFAULT_SEASON) > 0);
  EXPECT_TRUE(seasonIds.count(secondSeason.id) > 0);

  for (const int32_t seasonId : {DEFAULT_SEASON, secondSeason.id}) {
    const auto bestSegments = repository->getBestSegments(seasonId, TEST_MAP).at(TEST_RUN);
    ASSERT_EQ(bestSegments.size(), 3u);
    EXPECT_EQ(bestSegments[0], 0); // removed split has no surviving personal row
    EXPECT_EQ(bestSegments[1], segments[1]);
    EXPECT_EQ(bestSegments[2], segments[2]);
  }

  // both season copies are grouped into a single removed split
  const auto page = repository->getRemovedSegments(listRemovedSegmentsParams());
  ASSERT_EQ(page.entries.size(), 1u);
  ASSERT_EQ(page.entries[0].segments.size(), 2u);
}

TEST_F(TimerunRepositoryTests, RemoveSegment_RequiresExactMapAndRun) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}))});

  ETJump::Timerun::RemoveSegmentParams params{};
  params.map       = "test_ma";
  params.run       = TEST_RUN;
  params.userId    = 1;
  params.segment   = 1;
  params.removedBy = 9;

  EXPECT_THROW(repository->removeSegment(params, parseTime("2026-03-01 12:00:00")), std::runtime_error);
  EXPECT_TRUE(repository->getRemovedSegments(listRemovedSegmentsParams()).entries.empty());
  EXPECT_EQ(repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 1).at(TEST_RUN)[0], 100);

  params.map = TEST_MAP;
  params.run = "test_ru";

  EXPECT_THROW(repository->removeSegment(params, parseTime("2026-03-01 12:00:00")), std::runtime_error);
  EXPECT_TRUE(repository->getRemovedSegments(listRemovedSegmentsParams()).entries.empty());
  EXPECT_EQ(repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 1).at(TEST_RUN)[0], 100);

  // the exact parameters still remove the split
  params.run = TEST_RUN;
  ASSERT_EQ(repository->removeSegment(params, parseTime("2026-03-01 12:00:00")).size(), 1u);
}

TEST_F(TimerunRepositoryTests, RemoveSegment_RequiresExactSeason) {
  const auto secondSeason = addSeason(*repository, "Season 2");

  auto defaultSeasonRecord    = makeRecord(1, 300, {100, 200});
  auto secondSeasonRecord     = defaultSeasonRecord;
  secondSeasonRecord.seasonId = secondSeason.id;

  repository->recordCompletion({completion(defaultSeasonRecord), completion(secondSeasonRecord)});

  ETJump::Timerun::RemoveSegmentParams params{};
  params.map       = TEST_MAP;
  params.run       = TEST_RUN;
  params.userId    = 1;
  params.segment   = 1;
  params.removedBy = 9;
  params.season    = "Season";

  EXPECT_THROW(repository->removeSegment(params, parseTime("2026-03-01 12:00:00")), std::runtime_error);
  EXPECT_TRUE(repository->getRemovedSegments(listRemovedSegmentsParams()).entries.empty());
  EXPECT_EQ(repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 1).at(TEST_RUN)[0], 100);
  EXPECT_EQ(repository->getPlayerBestSegments(secondSeason.id, TEST_MAP, 1).at(TEST_RUN)[0], 100);

  // season names are matched exactly, but case-insensitively
  params.season = "season 2";

  const auto removed = repository->removeSegment(params, parseTime("2026-03-01 12:00:00"));
  ASSERT_EQ(removed.size(), 1u);
  EXPECT_EQ(removed[0].seasonId, secondSeason.id);

  // only the targeted season's copy is removed
  EXPECT_EQ(repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 1).at(TEST_RUN)[0], 100);
  EXPECT_EQ(repository->getPlayerBestSegments(secondSeason.id, TEST_MAP, 1).at(TEST_RUN)[0], 0);

  const auto archived = repository->getRemovedSegments(listRemovedSegmentsParams());
  ASSERT_EQ(archived.entries.size(), 1u);
  ASSERT_EQ(archived.entries[0].segments.size(), 1u);
  EXPECT_EQ(archived.entries[0].segments[0].seasonId, secondSeason.id);
  EXPECT_EQ(archived.entries[0].segments[0].time, 100);
}

TEST_F(TimerunRepositoryTests, RestoreSegment_Conflict_RequiresForceAndSwapsOccupant) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}))});

  ETJump::Timerun::RemoveSegmentParams removeParams{};
  removeParams.map       = TEST_MAP;
  removeParams.run       = TEST_RUN;
  removeParams.userId    = 1;
  removeParams.segment   = 1;
  removeParams.removedBy = 9;
  ASSERT_EQ(repository->removeSegment(removeParams, parseTime("2026-03-01 12:00:00")).size(), 1u);

  const auto archivedPage = repository->getRemovedSegments(listRemovedSegmentsParams());
  ASSERT_EQ(archivedPage.entries.size(), 1u);
  const int32_t archivedId = archivedPage.entries[0].segments[0].id;

  // a slower overall run sets a faster first split in the meantime
  repository->recordCompletion({completion(makeRecord(1, 310, {90, 200}, "2026-01-02 12:00:00"), false, false)});

  ETJump::Timerun::RestoreSegmentParams restore{};
  restore.callerId  = 9;
  restore.segmentId = archivedId;
  restore.isAdmin   = true;

  const auto conflict = repository->restoreSegment(restore, parseTime("2026-03-02 12:00:00"));
  EXPECT_TRUE(conflict.restored.empty());
  ASSERT_EQ(conflict.conflicts.size(), 1u);
  EXPECT_EQ(conflict.conflicts[0].toRestore.time, 100);
  EXPECT_EQ(conflict.conflicts[0].occupant.time, 90);

  // nothing was written, and the archive ID is still usable for a forced retry
  EXPECT_EQ(repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 1).at(TEST_RUN)[0], 90);
  EXPECT_EQ(repository->getRemovedSegments(listRemovedSegmentsParams()).entries.size(), 1u);

  restore.force     = true;
  const auto forced = repository->restoreSegment(restore, parseTime("2026-03-03 12:00:00"));
  ASSERT_EQ(forced.restored.size(), 1u);
  EXPECT_EQ(forced.restored[0].time, 100);
  ASSERT_EQ(forced.swapped.size(), 1u);
  EXPECT_EQ(forced.swapped[0].time, 90);

  EXPECT_EQ(repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 1).at(TEST_RUN)[0], 100);
  EXPECT_EQ(repository->getBestSegments(DEFAULT_SEASON, TEST_MAP).at(TEST_RUN)[0], 100);

  // the displaced split is archived in place of the restored one
  const auto swappedPage = repository->getRemovedSegments(listRemovedSegmentsParams());
  ASSERT_EQ(swappedPage.entries.size(), 1u);
  EXPECT_EQ(swappedPage.entries[0].segments[0].time, 90);
  EXPECT_EQ(swappedPage.entries[0].segments[0].removedBy, 9);
}

TEST_F(TimerunRepositoryTests, RestoreSegment_Permission_DeniesOthersRemoval) {
  repository->recordCompletion({completion(makeRecord(1, 300, {100, 200}))});

  ETJump::Timerun::RemoveSegmentParams removeParams{};
  removeParams.map       = TEST_MAP;
  removeParams.run       = TEST_RUN;
  removeParams.userId    = 1;
  removeParams.segment   = 1;
  removeParams.removedBy = 9;
  ASSERT_EQ(repository->removeSegment(removeParams, parseTime("2026-03-01 12:00:00")).size(), 1u);

  const auto archived = repository->getRemovedSegments(listRemovedSegmentsParams());
  ASSERT_EQ(archived.entries.size(), 1u);
  ASSERT_EQ(archived.entries[0].segments.size(), 1u);

  ETJump::Timerun::RestoreSegmentParams restore{};
  restore.callerId  = 1; // removed by user 9, and the caller is not an admin
  restore.segmentId = archived.entries[0].segments[0].id;
  restore.isAdmin   = false;

  EXPECT_THROW(repository->restoreSegment(restore, parseTime("2026-03-02 12:00:00")), std::runtime_error);
  EXPECT_EQ(repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 1).at(TEST_RUN)[0], 0);
  EXPECT_EQ(repository->getRemovedSegments(listRemovedSegmentsParams()).entries.size(), 1u);
}

TEST_F(TimerunRepositoryTests, RestoreSegment_Permission_RestoresOnlyOwnCopies) {
  const auto secondSeason = addSeason(*repository, "Season 2");

  auto defaultSeasonRecord    = makeRecord(1, 300, {100, 200});
  auto secondSeasonRecord     = defaultSeasonRecord;
  secondSeasonRecord.seasonId = secondSeason.id;

  repository->recordCompletion({completion(defaultSeasonRecord), completion(secondSeasonRecord)});

  ETJump::Timerun::RemoveSegmentParams removeParams{};
  removeParams.map     = TEST_MAP;
  removeParams.run     = TEST_RUN;
  removeParams.userId  = 1;
  removeParams.segment = 1;

  // the same split was removed from each season by a different user
  removeParams.season    = "Default";
  removeParams.removedBy = 9;
  ASSERT_EQ(repository->removeSegment(removeParams, parseTime("2026-03-01 12:00:00")).size(), 1u);

  removeParams.season    = "Season 2";
  removeParams.removedBy = 10;
  ASSERT_EQ(repository->removeSegment(removeParams, parseTime("2026-03-02 12:00:00")).size(), 1u);

  const auto archived = repository->getRemovedSegments(listRemovedSegmentsParams());
  ASSERT_EQ(archived.entries.size(), 1u);
  ASSERT_EQ(archived.entries[0].segments.size(), 2u);

  // caller 9 removed the default-season copy, but not the Season 2 one
  ETJump::Timerun::RestoreSegmentParams restore{};
  restore.callerId  = 9;
  restore.segmentId = archived.entries[0].segments[0].id; // default season, sorted first
  restore.isAdmin   = false;

  const auto result = repository->restoreSegment(restore, parseTime("2026-03-03 12:00:00"));
  EXPECT_EQ(result.numTargeted, 2);
  ASSERT_EQ(result.restored.size(), 1u);
  EXPECT_EQ(result.restored[0].seasonId, DEFAULT_SEASON);
  ASSERT_EQ(result.skipped.size(), 1u);
  EXPECT_EQ(result.skipped[0].seasonId, secondSeason.id);
  EXPECT_EQ(result.skipped[0].removedBy, 10);

  EXPECT_EQ(repository->getPlayerBestSegments(DEFAULT_SEASON, TEST_MAP, 1).at(TEST_RUN)[0], 100);
  EXPECT_EQ(repository->getPlayerBestSegments(secondSeason.id, TEST_MAP, 1).at(TEST_RUN)[0], 0);

  const auto remaining = repository->getRemovedSegments(listRemovedSegmentsParams());
  ASSERT_EQ(remaining.entries.size(), 1u);
  ASSERT_EQ(remaining.entries[0].segments.size(), 1u);
  EXPECT_EQ(remaining.entries[0].segments[0].seasonId, secondSeason.id);
  EXPECT_EQ(remaining.entries[0].segments[0].removedBy, 10);
}
} // namespace
