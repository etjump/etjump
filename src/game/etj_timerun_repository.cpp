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

#include <iterator>
#include <set>
#include <tuple>
#include <utility>

#include "etj_timerun_repository.h"

#include "etj_container_utilities.h"
#include "etj_database_v2.h"
#include "q_shared.h"

namespace ETJump {
static std::vector<int32_t>
parseCheckpoints(const std::string &checkpointsString) {
  return Container::map(
      Container::filter(StringUtils::split(checkpointsString, ","),
                        [](const std::string &input) {
                          return !StringUtils::trim(input).empty();
                        }),
      [](const std::string &checkpoint) {
        try {
          return std::stoi(StringUtils::trim(checkpoint));
        } catch (const std::logic_error &) {
          return TIMERUN_CHECKPOINT_NOT_SET;
        }
      });
}

// Segment durations for a completed run: segment i is the time from the
// previous checkpoint (the run start, for the first) to checkpoint i, and the
// final segment runs from the last checkpoint to the finish. A run with no
// checkpoints has no splits at all, so it produces no segments - its best is
// already the record. Stops at the first unset checkpoint, or a non-increasing
// checkpoint, since such a segment cannot represent a real run and a zero or
// negative time would otherwise win every comparison and sit as permanent,
// unremovable gold segment.
static std::vector<int32_t>
computeSegmentTimes(const std::vector<int32_t> &checkpoints,
                    int32_t completionTime) {
  std::vector<int32_t> segments;
  int32_t previous = 0;

  for (const int32_t checkpoint : checkpoints) {
    if (checkpoint == TIMERUN_CHECKPOINT_NOT_SET || checkpoint <= previous) {
      break;
    }

    segments.push_back(checkpoint - previous);
    previous = checkpoint;
  }

  // the finish closes the run, but only for a run that has checkpoints:
  // a completion time at or before the last checkpoint means the finish was
  // itself a checkpoint, so there is nothing to add
  if (!segments.empty() && completionTime > previous) {
    segments.push_back(completionTime - previous);
  }

  return segments;
}

Timerun::Record getRecordFromStandardQueryResult(
    int seasonId, std::string map, std::string runName, int userId, int time,
    std::string checkpointsString, std::string recordDate,
    std::string playerName, std::string metadataString) {
  auto checkpoints = parseCheckpoints(checkpointsString);
  TimeUtils::Time recordDateTime = TimeUtils::Time::fromString(recordDate);

  std::map<std::string, std::string> metadata;
  for (const auto &kvp : Container::map(StringUtils::split(metadataString, ","),
                                        [](const std::string &kvp) {
                                          return StringUtils::split(kvp, "=");
                                        })) {
    if (kvp.size() != 2) {
      continue;
    }

    metadata[kvp[0]] = kvp[1];
  }

  Timerun::Record record;
  record.seasonId = seasonId;
  record.map = std::move(map);
  record.run = std::move(runName);
  record.userId = userId;
  record.time = time;
  record.recordDate = std::move(recordDateTime);
  record.checkpoints = std::move(checkpoints);
  record.playerName = std::move(playerName);
  record.metadata = std::move(metadata);

  return record;
}

static std::string buildMatchSuggestionsError(
    const std::string &commandPrefix, const std::string &matchItem,
    const std::string &searchTerm, const std::vector<std::string> &matches) {
  static constexpr int32_t MATCH_ITEMS_PER_ROW = 3;

  std::string error = StringUtils::format(
      "^3%s: ^7found %i %ss matching ^3'%s'^7\n", commandPrefix,
      static_cast<int>(matches.size()), matchItem, searchTerm);

  int32_t i = 0;
  for (const auto &match : matches) {
    if (i != 0 && i % MATCH_ITEMS_PER_ROW == 0) {
      error += "\n";
    }

    error += StringUtils::format("%-22s", match);
    ++i;
  }

  return error;
}

static std::string
createSeasonPredicate(const std::vector<Timerun::Season> &seasons) {
  return StringUtils::join(
      Container::map(seasons, [](const auto &) { return "season_id=?"; }),
      " or ");
}

void TimerunRepository::initialize() { migrate(); }

void TimerunRepository::shutdown() { _database = nullptr; }

std::vector<Timerun::Record>
TimerunRepository::getRecordsForPlayer(const std::vector<int> activeSeasons,
                                       const std::string &map, int userId) {
  auto parameters = StringUtils::join(
      Container::map(activeSeasons,
                     [](int season) { return std::to_string(season); }),
      ", ");

  auto binder = _database->sql
                << StringUtils::format(R"(
          select
            %s
          from record
          where season_id in (%s) and
            map=? and
            user_id=?;
        )",
                                       _defaultRecordFieldsStr, parameters)
                << map << userId;

  auto records = getRecordsFromQuery(binder);

  return records;
}

Timerun::Season TimerunRepository::addSeason(Timerun::AddSeasonParams params) {
  int count = 0;
  _database->sql << R"(
                      select count(name) from season where name=? collate nocase;
                    )"
                 << params.name >>
      count;

  if (count > 0) {
    throw std::runtime_error(StringUtils::format(
        "Cannot add season `%s` as it already exists.", params.name));
  }

  if (params.endTime.has_value()) {
    if (params.startTime >= params.endTime.value()) {
      throw std::runtime_error("Start time cannot be after end time");
    }
  }

  std::string insert = R"(
                    insert into season (
                      name,
                      start_time,
                      end_time
                    ) values (
                      ?,
                      ?,
                      ?
                    );
                  )";

  if (params.endTime.has_value())
    _database->sql << insert << params.name
                   << params.startTime.toDateTimeString()
                   << (*params.endTime).toDateTimeString();
  else
    _database->sql << insert << params.name
                   << params.startTime.toDateTimeString() << nullptr;

  return Timerun::Season{static_cast<int>(_database->sql.last_insert_rowid()),
                         params.name, params.startTime, params.endTime};
}

std::vector<Timerun::Record>
TimerunRepository::getRecordsForPlayer(const std::vector<int> &activeSeasons,
                                       const std::string &map,
                                       const std::string &run, int userId) {
  auto records = std::vector<Timerun::Record>();

  _database->sql << StringUtils::format(R"(
    select
      season_id,
      map,
      run,
      user_id,
      time,
      checkpoints,
      record_date,
      player_name,
      metadata
    from record
    where season_id in (%s) and
      map=? and
      run=? and
      user_id=?;
    )",
                                        StringUtils::join(activeSeasons, ", "))
                 << map << run << userId >>
      [&records](int seasonId, std::string map, std::string runName, int userId,
                 int time, std::string checkpointsString,
                 std::string recordDate, std::string playerName,
                 std::string metadataString) {
        auto record = getRecordFromStandardQueryResult(
            seasonId, std::move(map), std::move(runName), userId, time,
            std::move(checkpointsString), std::move(recordDate),
            std::move(playerName), std::move(metadataString));

        records.push_back(std::move(record));
      };

  return records;
}

std::vector<Timerun::Record>
TimerunRepository::getRecordsForRun(const std::string &map,
                                    const std::string &run) const {
  throw std::runtime_error("Not implemented");
}

// assembles (run, segment, time) rows into a run -> segment durations map;
// segment numbers are 1-based, so segment N lands at index N - 1
static std::map<std::string, std::vector<int32_t>>
bestSegmentsFromRows(sqlite::database_binder &binder) {
  std::map<std::string, std::vector<int32_t>> bestSegments;

  binder >> [&bestSegments](std::string run, int32_t segment, int32_t time) {
    auto &runSegments = bestSegments[std::move(run)];

    if (runSegments.size() < static_cast<size_t>(segment)) {
      runSegments.resize(segment);
    }

    runSegments[segment - 1] = time;
  };

  return bestSegments;
}

// assembles (segment, time) rows into a segment durations map
static std::map<int32_t, int32_t>
segmentTimesFromRows(sqlite::database_binder &binder) {
  std::map<int32_t, int32_t> segmentTimes;

  binder >> [&segmentTimes](int32_t segment, int32_t time) {
    segmentTimes[segment] = time;
  };

  return segmentTimes;
}

std::map<std::string, std::vector<int32_t>>
TimerunRepository::getBestSegments(int32_t seasonId,
                                   const std::string &map) const {
  auto binder = _database->sql << R"(
    select run, segment, time
    from best_segments
    where season_id = ? and map = ?
    order by run, segment;
  )" << seasonId << map;

  return bestSegmentsFromRows(binder);
}

std::map<std::string, std::vector<int32_t>>
TimerunRepository::getPlayerBestSegments(int32_t seasonId,
                                         const std::string &map,
                                         int userId) const {
  auto binder = _database->sql << R"(
    select run, segment, time
    from player_best_segments
    where season_id = ? and map = ? and user_id = ?
    order by run, segment;
  )" << seasonId << map << userId;

  return bestSegmentsFromRows(binder);
}

void TimerunRepository::insertRecordRow(const Timerun::Record &record) {
  _database->sql << R"(
    insert into record (
      season_id,
      map,
      run,
      user_id,
      time,
      checkpoints,
      record_date,
      player_name,
      metadata
    ) values (
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?
    );
  )" << record.seasonId
                 << record.map << record.run << record.userId << record.time
                 << StringUtils::join(record.checkpoints, ",")
                 << record.recordDate.toDateTimeString() << record.playerName
                 << serializeMetadata(record.metadata);
}

// the rank a record holds among the current records of the same run, stored
// when the record is written: the number of strictly faster times plus one,
// so records that share a time share a rank, matching how '/records' displays
// tied records
static constexpr const char *RECORD_RANK_SQL =
    "      (select count(*) + 1\n"
    "        from record r2\n"
    "        where r2.season_id = r.season_id\n"
    "          and r2.map = r.map\n"
    "          and r2.run = r.run\n"
    "          and r2.time < r.time)";

void TimerunRepository::insertRecordHistory(const Timerun::Record &record) {
  _database->sql << StringUtils::format(R"(
    insert into record_history (
      season_id,
      map,
      run,
      user_id,
      time,
      rank,
      checkpoints,
      record_date,
      player_name,
      metadata
    )
    select
      r.season_id,
      r.map,
      r.run,
      r.user_id,
      r.time,
%s,
      r.checkpoints,
      r.record_date,
      r.player_name,
      r.metadata
    from record r
    where
      r.season_id = ? and
      r.map = ? and
      r.run = ? and
      r.user_id = ?;
  )",
                                        RECORD_RANK_SQL)
                 << record.seasonId << record.map << record.run
                 << record.userId;
}

void TimerunRepository::insertRecord(const Timerun::Record &record) {
  DatabaseV2::TransactionGuard txn(*_database);

  insertRecordRow(record);
  insertRecordHistory(record);

  txn.commit();
}

void TimerunRepository::updateRecord(const Timerun::Record &record) {
  DatabaseV2::TransactionGuard txn(*_database);

  _database->sql << R"(
    update
      record
    set
      time=?,
      checkpoints=?,
      record_date=?,
      player_name=?,
      metadata=?
    where
      season_id=? and
      map=? and
      run=? and
      user_id=?;
  )" << record.time
                 << StringUtils::join(record.checkpoints, ",")
                 << record.recordDate.toDateTimeString() << record.playerName
                 << serializeMetadata(record.metadata) << record.seasonId
                 << record.map << record.run << record.userId;

  insertRecordHistory(record);

  txn.commit();
}

std::vector<Timerun::BestSegmentUpdate>
TimerunRepository::updateBestSegments(const Timerun::Record &record) {
  std::vector<Timerun::BestSegmentUpdate> updates;

  const auto segments = computeSegmentTimes(record.checkpoints, record.time);

  // a run with no usable checkpoint data has no segments to record
  if (segments.empty()) {
    return updates;
  }

  DatabaseV2::TransactionGuard txn(*_database);

  const auto recordDate = record.recordDate.toDateTimeString();

  // Read what these segments are about to beat, before the upserts overwrite
  // it, so a completion can report what it improved (and stay quiet about
  // segments it set for the first time, which beat nothing). Scoped so the
  // binders are closed before the upserts run.
  std::map<int32_t, int32_t> personalPriors;
  std::map<int32_t, int32_t> globalPriors;

  {
    auto binder = _database->sql << R"(
      select segment, time
      from player_best_segments
      where season_id = ? and map = ? and run = ? and user_id = ?;
    )" << record.seasonId << record.map
                                 << record.run << record.userId;
    personalPriors = segmentTimesFromRows(binder);
  }

  {
    auto binder = _database->sql << R"(
      select segment, time
      from best_segments
      where season_id = ? and map = ? and run = ?;
    )" << record.seasonId << record.map
                                 << record.run;
    globalPriors = segmentTimesFromRows(binder);
  }

  for (size_t i = 0; i < segments.size(); ++i) {
    // 1-based: segment i is the leg ending at checkpoint i, and the finish is
    // the last segment
    const auto segment = static_cast<int32_t>(i) + 1;

    // improve-only: the new time must beat the stored one, ties going to the
    // earlier record date (and, for the global, the lower user ID), so the
    // holder matches what the seed would have picked
    _database->sql << R"(
      insert into player_best_segments (
        season_id,
        map,
        run,
        run_sanitized,
        user_id,
        segment,
        time,
        record_date,
        player_name,
        metadata
      ) values (
        ?,
        ?,
        ?,
        ?,
        ?,
        ?,
        ?,
        ?,
        ?,
        ?
      )
      on conflict (season_id, map, run, user_id, segment) do update
      set
        time = excluded.time,
        record_date = excluded.record_date,
        player_name = excluded.player_name,
        metadata = excluded.metadata
      where
        excluded.time < time or
        (excluded.time = time and excluded.record_date < record_date);
    )" << record.seasonId
                   << record.map << record.run
                   << StringUtils::sanitize(record.run, true) << record.userId
                   << segment << segments[i] << recordDate << record.playerName
                   << serializeMetadata(record.metadata);

    const bool personal = _database->sql.rows_modified() > 0;

    // a personal improvement is also the next step in the progression
    if (personal) {
      _database->sql << R"(
        insert into player_best_segments_history (
          season_id,
          map,
          run,
          run_sanitized,
          user_id,
          segment,
          time,
          rank,
          record_date,
          player_name,
          metadata
        ) values (
          ?,
          ?,
          ?,
          ?,
          ?,
          ?,
          ?,
          (select count(*) + 1
             from player_best_segments
            where season_id = ? and map = ? and run = ? and segment = ?
              and time < ?),
          ?,
          ?,
          ?
        );
      )" << record.seasonId
                     << record.map << record.run
                     << StringUtils::sanitize(record.run, true) << record.userId
                     << segment << segments[i] << record.seasonId << record.map
                     << record.run << segment << segments[i] << recordDate
                     << record.playerName << serializeMetadata(record.metadata);
    }

    _database->sql << R"(
      insert into best_segments (
        season_id,
        map,
        run,
        run_sanitized,
        segment,
        time,
        user_id,
        record_date,
        player_name
      ) values (
        ?,
        ?,
        ?,
        ?,
        ?,
        ?,
        ?,
        ?,
        ?
      )
      on conflict (season_id, map, run, segment) do update
      set
        time = excluded.time,
        user_id = excluded.user_id,
        record_date = excluded.record_date,
        player_name = excluded.player_name
      where
        excluded.time < time or
        (excluded.time = time and
         (excluded.record_date < record_date or
          (excluded.record_date = record_date and excluded.user_id < user_id)));
    )" << record.seasonId
                   << record.map << record.run
                   << StringUtils::sanitize(record.run, true) << segment
                   << segments[i] << record.userId << recordDate
                   << record.playerName;

    const bool global = _database->sql.rows_modified() > 0;

    if (personal || global) {
      Timerun::BestSegmentUpdate update{segment, segments[i], personal, global};

      const auto personalIt = personalPriors.find(segment);
      if (personalIt != personalPriors.end()) {
        update.previousPersonal = personalIt->second;
      }

      const auto globalIt = globalPriors.find(segment);
      if (globalIt != globalPriors.end()) {
        update.previousGlobal = globalIt->second;
      }

      updates.push_back(std::move(update));
    }
  }

  txn.commit();

  return updates;
}

std::optional<Timerun::Record>
TimerunRepository::getTopRecord(int seasonId, const std::string &map,
                                const std::string &run) {
  std::optional<Timerun::Record> record;

  _database->sql << R"(
    select
      season_id,
      map,
      run,
      user_id,
      time,
      checkpoints,
      record_date,
      player_name,
      metadata
    from record
    where
      season_id=? and
      map=? and
      run=?
    order by time asc
    limit 1
    )" << seasonId
                 << map << run >>
      [&record](int seasonId, std::string map, std::string runName, int userId,
                int time, std::string checkpointsString, std::string recordDate,
                std::string playerName, std::string metadataString) {
        record = Timerun::Record(getRecordFromStandardQueryResult(
            seasonId, std::move(map), std::move(runName), userId, time,
            std::move(checkpointsString), std::move(recordDate),
            std::move(playerName), std::move(metadataString)));
      };

  return record;
}

std::vector<Timerun::Record>
TimerunRepository::getTopRecords(const std::vector<int> &seasonIds,
                                 const std::string &map,
                                 const std::string &run) const {
  auto seasonIdsPlaceholder = DatabaseV2::createPlaceholderString(seasonIds);

  std::string query = StringUtils::format(
      R"(
        select *
        from (select season_id,
                     map,
                     run,
                     user_id,
                     time,
                     checkpoints,
                     record_date,
                     player_name,
                     metadata,
                     rank() over (partition by season_id, map, run order by time asc) as rank
              FROM record
              where season_id in (%s)
                and map = ?
                and run = ?) as ranked_records
        where rank = 1;
      )",
      seasonIdsPlaceholder);

  auto binder = _database->sql << query;

  for (const auto &seasonId : seasonIds) {
    binder << seasonId;
  }

  binder << map << run;

  std::vector<Timerun::Record> records;
  binder >> [&records](int seasonId, std::string map, std::string runName,
                       int userId, int time, std::string checkpointsString,
                       std::string recordDate, std::string playerName,
                       std::string metadataString, int rank) {
    records.push_back(getRecordFromStandardQueryResult(
        seasonId, std::move(map), std::move(runName), userId, time,
        std::move(checkpointsString), std::move(recordDate),
        std::move(playerName), std::move(metadataString)));
  };

  return records;
}

void TimerunRepository::editSeason(const Timerun::EditSeasonParams &params) {
  int seasonId = -1;
  TimeUtils::Time startTime;
  std::optional<TimeUtils::Time> endTime;

  _database->sql << R"(
    select
      id,
      start_time,
      end_time
    from season
    where name=?
    collate nocase
  )" << params.name >>
      [&](int sid, std::string s, std::unique_ptr<std::string> e) {
        seasonId = sid;
        startTime = TimeUtils::Time::fromString(s);
        if (e) {
          endTime = TimeUtils::Time(TimeUtils::Time::fromString(*e));
        }
      };

  if (seasonId < 0) {
    throw std::runtime_error(
        StringUtils::format("No season matching name `%s`", params.name));
  }

  std::vector<std::string> updatedFields;
  std::vector<std::string> updatedParams;
  bool anythingToUpdate = false;

  TimeUtils::Time newStartTime = std::move(startTime);
  std::optional<TimeUtils::Time> newEndTime = std::move(endTime);

  if (params.startTime.has_value()) {
    newStartTime = params.startTime.value();
    anythingToUpdate = true;
    updatedFields.emplace_back("start_time");
    updatedParams.push_back(params.startTime.value().toDateTimeString());
  }
  if (params.endTime.has_value()) {
    newEndTime = params.endTime;
    anythingToUpdate = true;
    updatedFields.emplace_back("end_time");
    updatedParams.push_back(params.endTime.value().toDateTimeString());
  }

  if (newEndTime.has_value() && newEndTime.value() < newStartTime) {
    throw std::runtime_error("End time cannot be before start time.");
  }

  if (!anythingToUpdate) {
    return;
  }

  std::string updatedFieldsString = StringUtils::join(
      Container::map(updatedFields, [](auto a) { return a + "=?"; }), ",");

  auto query = StringUtils::format(R"(
    update
      season
    set
      %s
    where
      id=?
  )",
                                   updatedFieldsString);

  auto q = _database->sql << query;

  for (const auto &p : updatedParams) {
    q << p;
  }
  q << seasonId;
}

std::vector<std::string>
TimerunRepository::getMapsForName(const std::string &map, bool exact,
                                  bool fromHistory) {
  const std::string table = fromHistory ? "record_history" : "record";

  std::string mapFilter = exact ? "map=?" : "map like ?";
  std::string mapSearchString = exact ? map : "%" + map + "%";

  std::vector<std::string> maps;
  _database->sql << StringUtils::format(R"(
    select
      distinct map
    from %s
    where %s
    collate nocase
  )",
                                        table, mapFilter)
                 << mapSearchString >>
      [&maps](std::string map) { maps.push_back(map); };
  return maps;
}

std::vector<std::string>
TimerunRepository::getRunsForName(const std::string &map,
                                  const std::string &run, bool exact,
                                  bool sanitizeResults, bool fromHistory) {
  const std::string table = fromHistory ? "record_history" : "record";

  std::string runFilter = exact ? "lsanitize(run)=?" : "lsanitize(run) like ?";
  std::string runSearchString = exact ? run : "%" + run + "%";

  std::vector<std::string> runs;
  _database->sql << StringUtils::format(R"(
    select
      distinct run
    from %s
    where %s
      and map = ?
    collate nocase
  )",
                                        table, runFilter)
                 << runSearchString << map >>
      [&runs, sanitizeResults](const std::string &run) {
        runs.push_back(sanitizeResults ? StringUtils::sanitize(run, true)
                                       : run);
      };
  return runs;
}

std::string TimerunRepository::resolveMapName(const std::string &map,
                                              bool exact,
                                              const std::string &commandPrefix,
                                              bool fromHistory) {
  const auto maps = getMapsForName(map, exact, fromHistory);

  if (maps.size() > 1 && !Container::isIn(maps, map)) {
    throw std::runtime_error(
        buildMatchSuggestionsError(commandPrefix, "map", map, maps));
  }

  return maps.empty() ? map : maps[0];
}

std::string TimerunRepository::resolveRunName(const std::string &map,
                                              const std::string &run,
                                              bool exact, bool fromHistory) {
  const auto runs = getRunsForName(map, run, exact, true, fromHistory);

  return runs.size() == 1 ? runs[0] : "%" + run + "%";
}

std::vector<Timerun::Season>
TimerunRepository::getSeasonsFromQuery(sqlite::database_binder &binder) {
  std::vector<Timerun::Season> seasons;

  binder >> [&seasons](int id, const std::string &name,
                       const std::string &startTime,
                       std::unique_ptr<std::string> endTime) {
    seasons.push_back(
        Timerun::Season{id, name, TimeUtils::Time::fromString(startTime),
                        endTime ? std::make_optional<TimeUtils::Time>(
                                      TimeUtils::Time::fromString(*endTime))
                                : std::nullopt});
  };

  return seasons;
}

std::vector<Timerun::Record> TimerunRepository::getRecords() {
  auto binder = _database->sql << R"(
    select
      season_id,
      map,
      run,
      user_id,
      time,
      checkpoints,
      record_date,
      player_name,
      metadata
    from record
    order by season_id, map, run, time asc, record_date asc, user_id asc;
  )";

  return getRecordsFromQuery(binder);
}

std::vector<Timerun::Record>
TimerunRepository::getRecords(const Timerun::PrintRecordsParams &params) {
  const auto season = params.season.value_or("Default");
  const std::string &map = params.map;
  const std::string &run = params.run.value_or("");
  const bool runSpecified = !run.empty();

  const auto seasons = getSeasonsForName(season, false);

  if (seasons.empty()) {
    throw std::runtime_error(
        StringUtils::format("No season matches name `%s`", season));
  }

  const std::string resolvedMap =
      resolveMapName(map, params.exactMap, "records");

  // try to match a single run, so in scenarios where a map has runs
  // 'foo' and 'foobar' and query has 'foo' as the run param,
  // we get the exact match for the run 'foo' instead of exact and
  // partial matches to both 'foo' and 'foobar'
  std::string runPlaceholder;
  std::string runBinder;

  if (runSpecified) {
    runPlaceholder = "and lsanitize(run) like ?";
    runBinder = resolveRunName(resolvedMap, run, true);
  }

  const std::string seasonPlaceholders = createSeasonPredicate(seasons);

  const std::string query =
      StringUtils::format(R"(
    select
      season_id,
      map,
      run,
      user_id,
      time,
      checkpoints,
      record_date,
      player_name,
      metadata
    from record
    where
      (%s) and
      map=?
      %s
    collate nocase
    order by season_id, map, run, time asc, record_date asc, user_id asc
  )",
                          seasonPlaceholders, runPlaceholder);

  auto binder = _database->sql << query;

  for (const auto &s : seasons) {
    binder << s.id;
  }

  binder << StringUtils::toLowerCase(resolvedMap);

  if (runSpecified) {
    binder << runBinder;
  }

  return getRecordsFromQuery(binder);
}

std::vector<Timerun::Season>
TimerunRepository::getSeasonsForName(const std::string &name, bool exact) {
  const std::string query =
      exact
          ? R"(select id, name, start_time, end_time from season where name=? collate nocase;)"
          : R"(select id, name, start_time, end_time from season where name like ? collate nocase;)";
  const std::string searchString = exact ? name : "%" + name + "%";

  auto binder = _database->sql << query << searchString;

  return getSeasonsFromQuery(binder);
}

std::optional<Timerun::Record>
TimerunRepository::getRecord(const std::string &map, const std::string &run,
                             int rank) {
  std::optional<Timerun::Record> record;

  _database->sql << R"(
    select *
      from (
        select
          season_id,
          map,
          run,
          user_id,
          time,
          checkpoints,
          record_date,
          player_name,
          metadata,
          rank() over (partition by season_id, map, run order by time asc) as rank
        FROM record
        where season_id=1 and map=? and lsanitize(run)=?
      ) as ranked_records
      where rank = ?;
  )" << map << run
                 << rank >>
      [&record](int seasonId, std::string map, std::string runName, int userId,
                int time, std::string checkpointsString, std::string recordDate,
                std::string playerName, std::string metadataString, int rank) {
        record = getRecordFromStandardQueryResult(
            seasonId, std::move(map), std::move(runName), userId, time,
            std::move(checkpointsString), std::move(recordDate),
            std::move(playerName), std::move(metadataString));
      };

  return record;
}

std::vector<Timerun::Record> TimerunRepository::getRecordFromSeason(
    const int32_t seasonId, const std::string &map, const std::string &run,
    const int32_t rank, const bool exactMap) {
  std::vector<Timerun::Record> records;

  const std::string resolvedMap =
      resolveMapName(map, exactMap, "record-details");

  const std::string runPlaceHolder = "lsanitize(run) like ?";
  const std::string runBinder = resolveRunName(resolvedMap, run, false);

  const std::string query = StringUtils::format(R"(
  select *
    from (
      select
        season_id,
        map,
        run,
        user_id,
        time,
        checkpoints,
        record_date,
        player_name,
        metadata,
        rank() over (partition by season_id, map, run order by time asc) as record_rank
      FROM record
      where
        season_id=? and map=? and %s
    ) as ranked_records
    where record_rank = ?;
  )",
                                                runPlaceHolder);

  auto binder = _database->sql << query;
  binder << seasonId << resolvedMap << runBinder << rank;

  binder >> [&records](int32_t seasonId, const std::string &map,
                       const std::string &runName, int32_t userId, int32_t time,
                       const std::string &checkpointsString,
                       const std::string &recordDate,
                       const std::string &playerName,
                       const std::string &metadataString) {
    records.emplace_back(getRecordFromStandardQueryResult(
        seasonId, map, runName, userId, time, checkpointsString, recordDate,
        playerName, metadataString));
  };

  return records;
}

std::vector<Timerun::HistoricalRecord> TimerunRepository::getHistoricalRecords(
    const Timerun::RecordHistoryParams &params) {
  const auto seasons = getSeasonsForName(params.season, false);

  if (seasons.empty()) {
    throw std::runtime_error(
        StringUtils::format("No season matches name `%s`", params.season));
  }

  const std::string resolvedMap =
      resolveMapName(params.map, params.exactMap, "record-history", true);

  // match runs the same way '/records' does: prefer a single exact match,
  // otherwise fall back to partial matches
  const std::string runPlaceholder = "lsanitize(run) like ?";
  const std::string runBinder =
      resolveRunName(resolvedMap, params.run, true, true);

  const std::string seasonPlaceholders = createSeasonPredicate(seasons);

  const std::string query =
      StringUtils::format(R"(
    select
      season_id,
      map,
      run,
      user_id,
      time,
      rank,
      checkpoints,
      record_date,
      player_name,
      metadata,
      exists(select 1 from removed_records rr
              where rr.season_id = rh.season_id
                and rr.map = rh.map
                and rr.run = rh.run
                and rr.user_id = rh.user_id
                and rr.record_date = rh.record_date) as removed
    from record_history rh
    where
      (%s) and
      map=? collate nocase and
      %s and
      user_id=?
    order by season_id, map, run, record_date desc, id desc;
  )",
                          seasonPlaceholders, runPlaceholder);

  auto binder = _database->sql << query;

  for (const auto &s : seasons) {
    binder << s.id;
  }

  binder << StringUtils::toLowerCase(resolvedMap);
  binder << runBinder;
  binder << params.userId;

  std::vector<Timerun::HistoricalRecord> records;

  binder >> [&records](int32_t seasonId, const std::string &map,
                       const std::string &runName, int32_t userId, int32_t time,
                       int32_t rank, const std::string &checkpointsString,
                       const std::string &recordDate,
                       const std::string &playerName,
                       const std::string &metadataString, int32_t removed) {
    Timerun::HistoricalRecord historicalRecord;
    historicalRecord.r = getRecordFromStandardQueryResult(
        seasonId, map, runName, userId, time, checkpointsString, recordDate,
        playerName, metadataString);
    // 'rank' is null for records seeded from 'removed_records', which
    // sqlite_modern_cpp reads as 0
    historicalRecord.rank = rank;
    historicalRecord.removed = removed != 0;
    records.emplace_back(std::move(historicalRecord));
  };

  return records;
}

std::vector<Timerun::Season> TimerunRepository::getSeasons() {
  auto binder = _database->sql
                << R"(select id, name, start_time, end_time from season;)";

  return getSeasonsFromQuery(binder);
}

void TimerunRepository::deleteSeason(const std::string &name) {
  if (name == "default") {
    throw std::runtime_error("Cannot delete default season.");
  }
  int id = 0;
  _database->sql << "select coalesce((select id from season where name=? "
                    "collate nocase), -1);"
                 << name >>
      id;
  if (id < 0) {
    throw std::runtime_error(
        StringUtils::format("Season `%s` does not exist.", name));
  }

  // for some reason someone named it something else
  if (id == 1) {
    throw std::runtime_error("Cannot delete default season.");
  }

  constexpr std::array<const char *, 7> purgedTables = {
      "record",
      "record_history",
      "removed_records",
      "best_segments",
      "player_best_segments",
      "player_best_segments_history",
      "removed_segments",
  };

  DatabaseV2::TransactionGuard txn(*_database);

  // delete season is a destructive operation - we purge all records from all
  // tables that match the season ID, leaving nothing behind
  for (const auto &table : purgedTables) {
    _database->sql << StringUtils::format("delete from %s where season_id=?;",
                                          table)
                   << id;
  }

  // all tables purged, finally delete the actual season
  _database->sql << "delete from season where id=?" << id;

  txn.commit();
}

std::vector<Timerun::Record>
TimerunRepository::removeRecord(const Timerun::RemoveRecordParams &params,
                                const TimeUtils::Time &removedAt) {
  // If a record is present in multiple seasons, and the caller did not specify
  // the season to target, we find the record on the overall season,
  // and cascade the removal to every other season that contains that
  // specific record, as identified by the record date. If season is explicitly
  // targeted, we only remove the record from that season.
  const bool cascade = params.season.empty();
  static constexpr int32_t DEFAULT_SEASON_ID = 1;

  // resolve the targeted season name -> id
  int32_t targetSeasonId = 0;

  if (cascade) {
    targetSeasonId = DEFAULT_SEASON_ID;
  } else {
    auto seasons = getSeasonsForName(params.season, true);

    // if we don't get an exact match, we always fail, but give suggestions
    // for partial matches to the user regardless
    if (seasons.empty()) {
      seasons = getSeasonsForName(params.season, false);

      // this is always a partial match, so saying "no season found matching..."
      // is correct - we didn't find the exact season we were looking for
      if (seasons.size() <= 1) {
        throw std::runtime_error(StringUtils::format(
            "^3remove-record: ^7no season found matching ^3'%s'^7.",
            params.season));
      }

      const auto seasonNames = Container::map(
          seasons, [](const Timerun::Season &s) { return s.name; });

      throw std::runtime_error(buildMatchSuggestionsError(
          "remove-record", "season", params.season, seasonNames));
    }

    targetSeasonId = seasons[0].id;
  }

  const std::string resolvedMap = StringUtils::sanitize(params.map, true);

  // if we don't get an exact match, we always fail, but give suggestions
  // for partial matches to the user regardless
  if (getMapsForName(resolvedMap, true).empty()) {
    const auto maps = getMapsForName(resolvedMap, false);

    // this is always a partial match, so saying "no map found matching..."
    // is correct - we didn't find the exact map we were looking for
    if (maps.size() <= 1) {
      throw std::runtime_error(StringUtils::format(
          "^3remove-record: ^7no map found matching ^3'%s'^7.", params.map));
    }

    throw std::runtime_error(
        buildMatchSuggestionsError("remove-record", "map", params.map, maps));
  }

  const std::string resolvedRun = StringUtils::sanitize(params.run, true);

  if (getRunsForName(resolvedMap, resolvedRun, true, true).empty()) {
    throw std::runtime_error(StringUtils::format(
        "^3remove-record: ^7no run found matching ^3'%s'^7 on map ^3'%s'^7.",
        params.run, params.map));
  }

  // fetch the record(s) for the targeted season
  const std::string basePredicate =
      "map=? collate nocase and lsanitize(run)=? collate nocase and "
      "user_id=?";

  std::vector<Timerun::Record> targetRecords;
  {
    auto binder = _database->sql
                  << StringUtils::format(
                         "select %s from record where season_id=? and %s;",
                         _defaultRecordFieldsStr, basePredicate)
                  << targetSeasonId << resolvedMap << resolvedRun
                  << params.userId;
    targetRecords = getRecordsFromQuery(binder);
  }

  if (targetRecords.empty()) {
    throw std::runtime_error(StringUtils::format(
        "^3remove-record: ^7no record found for ^3'%s'^7 by user ID ^3'%i'^7.",
        params.run, params.userId));
  }

  // expand to the set of season ids we will remove from
  std::vector<int> seasonIds;

  if (cascade) {
    // the targeted row is the Default season copy; find every other season
    // that holds a copy of the same completion (same record_date)
    const auto dates = Container::map(targetRecords, [](const auto &r) {
      return r.recordDate.toDateTimeString();
    });

    auto binder = _database->sql
                  << StringUtils::format(
                         "select distinct season_id from record "
                         "where %s and record_date in (%s);",
                         basePredicate,
                         DatabaseV2::createPlaceholderString(dates))
                  << resolvedMap << resolvedRun << params.userId;

    for (const auto &date : dates) {
      binder << date;
    }

    binder >> [&seasonIds](int seasonId) { seasonIds.push_back(seasonId); };
  } else {
    seasonIds.push_back(targetSeasonId);
  }

  // final predicate shared by insert-select / remove
  const std::string predicate = StringUtils::format(
      "season_id in (%s) and %s",
      DatabaseV2::createPlaceholderString(seasonIds), basePredicate);

  const std::string archiveQuery = StringUtils::format(
      R"(
    insert into removed_records (
      season_id,
      map,
      run,
      user_id,
      time,
      checkpoints,
      record_date,
      player_name,
      metadata,
      removed_by,
      removed_at,
      reason
    )
    select
      season_id,
      map,
      run,
      user_id,
      time,
      checkpoints,
      record_date,
      player_name,
      metadata,
      ?,
      ?,
      ?
    from record
    where %s
  )",
      predicate);

  const std::string removeQuery = StringUtils::format(
      R"(
    delete from record
    where %s
  )",
      predicate);

  const auto bind = [&seasonIds, &resolvedMap, &resolvedRun,
                     &params](auto &binder) {
    for (const auto &seasonId : seasonIds) {
      binder << seasonId;
    }
    binder << resolvedMap << resolvedRun << params.userId;
  };

  // fetch all records that are about to be removed, so the result
  // reflects cascaded seasons too, not just the targeted season
  std::vector<Timerun::Record> removedRecords;
  {
    auto binder = _database->sql
                  << StringUtils::format("select %s from record where %s;",
                                         _defaultRecordFieldsStr, predicate);
    bind(binder);
    removedRecords = getRecordsFromQuery(binder);
  }

  DatabaseV2::TransactionGuard txn(*_database);

  auto archiveBinder = _database->sql << archiveQuery;
  archiveBinder << params.removedBy << removedAt.toDateTimeString();

  if (params.reason.has_value()) {
    archiveBinder << params.reason.value();
  } else {
    archiveBinder << nullptr;
  }

  bind(archiveBinder);
  archiveBinder++; // execute the archive now, inside the transaction

  auto removeBinder = _database->sql << removeQuery;
  bind(removeBinder);
  removeBinder++; // execute the removal now, inside the transaction

  txn.commit();

  return removedRecords;
}

Timerun::RestoreRecordResult
TimerunRepository::restoreRecord(const Timerun::RestoreRecordParams &params,
                                 const TimeUtils::Time &restoredAt) {
  // load the removed record the caller is targeting
  std::vector<Timerun::RemovedRecord> removedRecords;
  {
    auto binder = _database->sql << R"(
        select
          id,
          season_id,
          map,
          run,
          user_id,
          time,
          checkpoints,
          record_date,
          player_name,
          metadata,
          removed_by,
          removed_at,
          reason
        from removed_records
        where id=?;
      )" << params.recordId;
    removedRecords = getRemovedRecordsFromQuery(binder);
  }

  if (removedRecords.empty()) {
    throw std::runtime_error(StringUtils::format(
        "^3restore-record: ^7no removed record found with ID ^3%i^7.",
        params.recordId));
  }

  // the targeted record is a single season copy of a completion; find every
  // other season copy of the same completion so the restoration cascades
  const auto &base = removedRecords[0].record;
  const std::string resolvedRun = StringUtils::sanitize(base.run, true);

  {
    auto binder = _database->sql << R"(
        select
          id,
          season_id,
          map,
          run,
          user_id,
          time,
          checkpoints,
          record_date,
          player_name,
          metadata,
          removed_by,
          removed_at,
          reason
        from removed_records
        where map=? collate nocase and lsanitize(run)=? collate nocase
          and user_id=? and record_date=?
        order by season_id;
      )" << base.map << resolvedRun
                                 << base.userId
                                 << base.recordDate.toDateTimeString();
    removedRecords = getRemovedRecordsFromQuery(binder);
  }

  // non-admins can only restore the records they removed themselves; any
  // other season copies of the completion are skipped and stay archived
  std::vector<Timerun::RemovedRecord> restorable;
  std::vector<Timerun::RemovedRecord> skipped;

  for (auto &removedRecord : removedRecords) {
    if (params.isAdmin || removedRecord.removedBy == params.callerId) {
      restorable.emplace_back(std::move(removedRecord));
    } else {
      skipped.emplace_back(std::move(removedRecord));
    }
  }

  if (restorable.empty()) {
    throw std::runtime_error(StringUtils::format(
        "^3restore-record: ^7you do not have permission to restore %s.",
        skipped.size() == 1 ? "this record" : "these records"));
  }

  Timerun::RestoreRecordResult result;
  // the cascade set is the total number of records this restore targets
  // 'removedRecords' still holds all of them, the split only moves them out
  result.numTargeted = static_cast<int32_t>(removedRecords.size());
  result.skipped = std::move(skipped);

  // find the record currently occupying a given slot, if any
  const auto findOccupant = [this](const Timerun::Record &record) {
    const std::string run = StringUtils::sanitize(record.run, true);

    std::vector<Timerun::Record> existing;
    {
      auto binder = _database->sql
                    << StringUtils::format(
                           "select %s from record where season_id=? and "
                           "map=? collate nocase and lsanitize(run)=? "
                           "collate nocase and user_id=?;",
                           _defaultRecordFieldsStr)
                    << record.seasonId << record.map << run << record.userId;
      existing = getRecordsFromQuery(binder);
    }

    if (existing.empty()) {
      return std::optional<Timerun::Record>{};
    }

    return std::optional<Timerun::Record>{std::move(existing[0])};
  };

  DatabaseV2::TransactionGuard txn(*_database);

  // detect all conflicts up front; if any exist and we're not forcing the
  // restore, we abort without touching anything, so the same record ID can
  // be used to retry the restore with --force
  std::vector<Timerun::RestoreConflict> conflicts;

  for (auto &removedRecord : restorable) {
    const auto &record = removedRecord.record;
    auto occupant = findOccupant(record);

    if (occupant.has_value()) {
      conflicts.emplace_back(
          Timerun::RestoreConflict{std::move(*occupant), record});
    }
  }

  if (!conflicts.empty() && !params.force) {
    result.conflicts = std::move(conflicts);
    return result;
  }

  for (auto &removedRecord : restorable) {
    const auto &record = removedRecord.record;
    const std::string run = StringUtils::sanitize(record.run, true);
    auto occupant = findOccupant(record);

    if (occupant.has_value()) {
      // force: archive the occupant in place of the restored record
      _database->sql
          << R"(
        insert into removed_records (
          season_id,
          map,
          run,
          user_id,
          time,
          checkpoints,
          record_date,
          player_name,
          metadata,
          removed_by,
          removed_at,
          reason
        )
        select
          season_id,
          map,
          run,
          user_id,
          time,
          checkpoints,
          record_date,
          player_name,
          metadata,
          ?,
          ?,
          ?
        from record
        where season_id=? and map=? collate nocase and lsanitize(run)=?
          collate nocase and user_id=?;
      )" << params.callerId
          << restoredAt.toDateTimeString()
          << "Automatically removed by 'restore-record' (conflict resolution)"
          << record.seasonId << record.map << run << record.userId;

      // free up the slot for the restored record
      _database->sql << R"(
        delete from record
        where season_id=? and map=? collate nocase and lsanitize(run)=?
          collate nocase and user_id=?;
      )" << record.seasonId
                     << record.map << run << record.userId;

      result.swapped.emplace_back(std::move(*occupant));
    }

    // restore the record into 'record'
    _database->sql << R"(
      insert into record (
        season_id,
        map,
        run,
        user_id,
        time,
        checkpoints,
        record_date,
        player_name,
        metadata
      ) values (
        ?,
        ?,
        ?,
        ?,
        ?,
        ?,
        ?,
        ?,
        ?
      );
    )" << record.seasonId
                   << record.map << record.run << record.userId << record.time
                   << StringUtils::join(record.checkpoints, ",")
                   << record.recordDate.toDateTimeString() << record.playerName
                   << serializeMetadata(record.metadata);

    // remove the archived copy
    _database->sql << "delete from removed_records where id=?;"
                   << removedRecord.id;

    result.restored.emplace_back(record);
  }

  txn.commit();

  return result;
}

Timerun::RemovedRecordsPage TimerunRepository::getRemovedRecords(
    const Timerun::ListRemovedRecordsParams &params) {
  std::vector<Timerun::Season> seasons;

  if (!params.season.empty()) {
    seasons = getSeasonsForName(params.season, false);

    if (seasons.empty()) {
      throw std::runtime_error(
          StringUtils::format("No season matches name '%s'", params.season));
    }
  }

  std::vector<std::string> whereClauses;

  if (!params.season.empty()) {
    whereClauses.emplace_back("(" + createSeasonPredicate(seasons) + ")");
  }

  if (!params.map.empty()) {
    whereClauses.emplace_back("map like ? collate nocase");
  }

  if (!params.run.empty()) {
    whereClauses.emplace_back("lsanitize(run) like ? collate nocase");
  }

  if (params.userId.has_value()) {
    whereClauses.emplace_back("user_id=?");
  }

  if (params.removedBy.has_value()) {
    whereClauses.emplace_back("removed_by=?");
  }

  const std::string whereString =
      whereClauses.empty()
          ? ""
          : " where " + StringUtils::join(whereClauses, " and ");

  const auto bindFilters = [&params, &seasons](auto &binder) {
    if (!params.season.empty()) {
      for (const auto &season : seasons) {
        binder << season.id;
      }
    }

    if (!params.map.empty()) {
      binder << "%" + params.map + "%";
    }

    if (!params.run.empty()) {
      binder << "%" + params.run + "%";
    }

    if (params.userId.has_value()) {
      binder << params.userId.value();
    }

    if (params.removedBy.has_value()) {
      binder << params.removedBy.value();
    }
  };

  const std::string query = StringUtils::format(R"(
    select
      id,
      season_id,
      map,
      run,
      user_id,
      time,
      checkpoints,
      record_date,
      player_name,
      metadata,
      removed_by,
      removed_at,
      reason
    from removed_records%s
    order by removed_at desc, id desc;
  )",
                                                whereString);

  auto binder = _database->sql << query;
  bindFilters(binder);

  std::vector<Timerun::RemovedRecord> records =
      getRemovedRecordsFromQuery(binder);

  // group the season copies of the same completion into a single entry,
  // unless a season was explicitly targeted, in which case each record is
  // listed on its own. rows are ordered by 'removed_at' desc, so the first
  // time a group key appears also gives the most recently removed first.
  std::vector<Timerun::RemovedRecordEntry> entries;

  if (params.season.empty()) {
    std::map<std::tuple<std::string, std::string, int32_t, std::string>, size_t>
        groupIndex;

    for (auto &record : records) {
      const auto key = std::make_tuple(
          record.record.map, record.record.run, record.record.userId,
          record.record.recordDate.toDateTimeString());

      const auto [it, inserted] = groupIndex.emplace(key, entries.size());

      if (inserted) {
        entries.emplace_back();
      }

      entries[it->second].records.emplace_back(std::move(record));
    }
  } else {
    entries.reserve(records.size());

    for (auto &record : records) {
      entries.emplace_back();
      entries.back().records.emplace_back(std::move(record));
    }
  }

  // sort the season copies within each entry by season id, so the Default
  // season is always first and 'entry.records[0]' is the representative
  // "base" record for all the copies of a given record
  for (auto &entry : entries) {
    std::sort(entry.records.begin(), entry.records.end(),
              [](const auto &a, const auto &b) {
                return a.record.seasonId < b.record.seasonId;
              });
  }

  // cap the requested page so we never return an empty page,
  // and instead return the last page of entries
  const auto totalCount = static_cast<int32_t>(entries.size());
  const int32_t pageSize = std::max(params.pageSize, 1);
  const int32_t numPages = std::max(1, (totalCount + pageSize - 1) / pageSize);
  const int32_t page = std::clamp(params.page, 1, numPages);
  const int32_t start = (page - 1) * pageSize;
  const int32_t end =
      std::min(start + pageSize, static_cast<int32_t>(entries.size()));

  Timerun::RemovedRecordsPage result;
  result.page = page;
  result.numPages = numPages;
  result.entries.insert(result.entries.end(),
                        std::make_move_iterator(entries.begin() + start),
                        std::make_move_iterator(entries.begin() + end));

  return result;
}

std::vector<Timerun::Checkpoints> TimerunRepository::getCheckpoints(
    const Timerun::ListCheckpointsParams &params) {
  std::vector<Timerun::Checkpoints> checkpoints;

  const std::string season = params.season.value_or("Default");
  const auto seasons = getSeasonsForName(season, false);

  if (seasons.empty()) {
    throw std::runtime_error(
        StringUtils::format("No seasons found matching name '%s'", season));
  }

  const std::string map = params.map;
  const std::string resolvedMap =
      resolveMapName(map, params.exactMap, "records");

  const std::string runPlaceHolder = "and lsanitize(run) like ?";
  const std::string runBinder = resolveRunName(resolvedMap, params.run, true);

  const std::string seasonPlaceholders = createSeasonPredicate(seasons);

  std::string query = StringUtils::format(R"(
    select *
      from (
        select
          season_id,
          map,
          run,
          time,
          checkpoints,
          player_name,
          rank() over (partition by season_id, map, run order by time asc) as rank
        FROM record
        where (%s) and map=? %s
      ) as ranked_records
      where rank = ?;
  )",
                                          seasonPlaceholders, runPlaceHolder);

  sqlite::database_binder binder = _database->sql << query;

  for (const auto &s : seasons) {
    binder << s.id;
  }

  binder << StringUtils::toLowerCase(resolvedMap);
  binder << runBinder;
  binder << params.rank;

  binder >> [&checkpoints](const int32_t seasonID, const std::string &map,
                           const std::string &run, const int32_t runTime,
                           const std::string &checkpointsStr,
                           const std::string &playerName) {
    Timerun::Checkpoints cp;

    cp.seasonID = seasonID;
    cp.map = map;
    cp.run = run;
    cp.runTime = runTime;

    for (const auto &time : StringUtils::split(checkpointsStr, ",")) {
      cp.checkpoints.emplace_back(Q_atoi(time));
    }

    cp.playerName = playerName;

    checkpoints.emplace_back(cp);
  };

  return checkpoints;
}

void TimerunRepository::tryToMigrateRecords() {
  int32_t existingRecords = 0;

  _database->sql << "select exists(select 1 from record);" >> existingRecords;

  // records already migrated
  if (existingRecords > 0) {
    return;
  }

  _oldDatabase->sql
          << R"(select exists(select 1 from sqlite_master where tbl_name='records');)" >>
      existingRecords;

  // no old records to migrate
  if (existingRecords == 0) {
    return;
  }

  std::vector<Timerun::Record> oldRecords;

  _oldDatabase->sql << R"(
    select
        id,
        time,
        record_date,
        map,
        run,
        user_id,
        player_name
    from records;
  )" >>
      [&oldRecords](int id, int time, int recordDate, std::string map,
                    std::string run, int userId, std::string playerName) {
        Timerun::Record r{};
        r.seasonId = 1;
        r.map = std::move(map);
        r.run = std::move(run);
        r.time = time;
        r.recordDate = TimeUtils::Time::fromInt(recordDate);
        r.userId = userId;
        r.playerName = std::move(playerName);
        r.checkpoints = std::vector<int>(MAX_TIMERUN_CHECKPOINTS,
                                         TIMERUN_CHECKPOINT_NOT_SET);
        r.metadata = {{"mod_version", "unknown(imported)"}};
        oldRecords.push_back(std::move(r));
      };

  DatabaseV2::TransactionGuard txn(*_database);

  for (const auto &r : oldRecords) {
    insertRecordRow(r);
  }

  txn.commit();
}

void TimerunRepository::seedRecordHistory() {
  // this must run after tryToMigrateRecords(), which imports the legacy
  // 'records' table into 'record'. as a plain migration statement it would run
  // during applyMigrations(), before that import, and seed an empty table
  int32_t haveHistory = 0;

  _database->sql << "select exists(select 1 from record_history)" >>
      haveHistory;

  // history already seeded
  if (haveHistory > 0) {
    return;
  }

  DatabaseV2::TransactionGuard txn(*_database);

  _database->sql << StringUtils::format(R"(
    insert into record_history (
      season_id,
      map,
      run,
      user_id,
      time,
      rank,
      checkpoints,
      record_date,
      player_name,
      metadata
    )
    select
      r.season_id,
      r.map,
      r.run,
      r.user_id,
      r.time,
%s,
      r.checkpoints,
      r.record_date,
      r.player_name,
      r.metadata
    from record r
    where not exists (
      select 1 from record_history h
       where h.season_id = r.season_id
         and h.map = r.map
         and h.run = r.run
         and h.user_id = r.user_id
         and h.record_date = r.record_date
    );
  )",
                                        RECORD_RANK_SQL);

  _database->sql << R"(
    insert into record_history (
      season_id, map, run, user_id, time, rank,
      checkpoints, record_date, player_name, metadata
    )
    select
      rr.season_id, rr.map, rr.run, rr.user_id, rr.time,
      null,
      rr.checkpoints, rr.record_date, rr.player_name, rr.metadata
    from removed_records rr
    where not exists (
      select 1 from record_history h
       where h.season_id = rr.season_id
         and h.map = rr.map
         and h.run = rr.run
         and h.user_id = rr.user_id
         and h.record_date = rr.record_date
    );
  )";

  txn.commit();
}

void TimerunRepository::seedBestSegments() {
  // Guarded by an "is the table empty?" check, the same sentinel the other
  // seeds use: the best segments are derived, and a server whose runs have no
  // checkpoints simply has nothing to seed, so re-deriving is a no-op there.
  // The tables only stay empty until the first checkpointed completion lands,
  // after which this skips forever.
  //
  // Seeds the personal and global tables and the initial history rows, from
  // 'record_history' minus removed runs ('removed_records') minus individually
  // removed splits ('removed_segments').
  int32_t haveBest = 0;

  _database->sql << "select exists(select 1 from player_best_segments);" >>
      haveBest;

  if (haveBest > 0) {
    return;
  }

  // Per-player bests, keyed by (season_id, map, run, user_id), each holding a
  // map of segment index -> the fastest time seen for that segment and the run
  // that set it. Held in memory while we scan history, so the tables are never
  // read and written through the same open cursor. Removed runs and removed
  // splits never contribute - history is immutable, so removal is a filter here
  // rather than a mutation.
  struct SegmentBest {
    int32_t time;
    std::string recordDate;
    std::string playerName;
    // serialized metadata of the run that set the split, so removals can be
    // audited against the mod version that produced it
    std::string metadata;
  };

  using PlayerKey = std::tuple<int32_t, std::string, std::string, int32_t>;
  std::map<PlayerKey, std::map<int32_t, SegmentBest>> personal;

  // Splits removed standalone are excluded per segment. Whole removed runs are
  // filtered out of the history scan below through 'removed_records'; this
  // covers the other archive, so a rebuild cannot re-admit a voided split.
  using RemovedSegmentKey = std::tuple<int32_t, std::string, std::string,
                                       int32_t, std::string, int32_t>;
  std::set<RemovedSegmentKey> removedSegments;

  _database->sql << R"(
    select season_id, map, run, user_id, record_date, segment
    from removed_segments;
  )" >>
      [&removedSegments](int32_t seasonId, std::string map, std::string run,
                         int32_t userId, std::string recordDate,
                         int32_t segment) {
        removedSegments.emplace(seasonId, std::move(map), std::move(run),
                                userId, std::move(recordDate), segment);
      };

  _database->sql << R"(
    select
      h.season_id,
      h.map,
      h.run,
      h.user_id,
      h.checkpoints,
      h.time,
      h.record_date,
      h.player_name,
      h.metadata
    from record_history h
    where not exists (
      select 1 from removed_records rr
      where rr.season_id = h.season_id
        and rr.map = h.map
        and rr.run = h.run
        and rr.user_id = h.user_id
        and rr.record_date = h.record_date
    );
  )" >>
      [&personal, &removedSegments](
          int32_t seasonId, std::string map, std::string run, int32_t userId,
          const std::string &checkpointsString, int32_t completionTime,
          const std::string &recordDate, const std::string &playerName,
          const std::string &metadata) {
        const auto segments = computeSegmentTimes(
            parseCheckpoints(checkpointsString), completionTime);

        // a run with no usable checkpoint data contributes no segments - bail
        // before touching 'personal', since operator[] would otherwise create
        // an entry for it, and the fold would then write rows with no segments
        if (segments.empty()) {
          return;
        }

        auto &playerSegments =
            personal[std::make_tuple(seasonId, map, run, userId)];

        for (size_t i = 0; i < segments.size(); ++i) {
          // 1-based: segment i is the leg ending at checkpoint i, and the
          // finish is the last segment
          const auto segment = static_cast<int32_t>(i) + 1;

          // a split removed standalone never contributes, whatever the run
          // that set it
          if (removedSegments.find(RemovedSegmentKey{seasonId, map, run, userId,
                                                     recordDate, segment}) !=
              removedSegments.end()) {
            continue;
          }

          const auto it = playerSegments.find(segment);

          // lower time wins; ties break to the earlier record date, so the
          // source run is deterministic
          if (it == playerSegments.end() || segments[i] < it->second.time ||
              (segments[i] == it->second.time &&
               recordDate < it->second.recordDate)) {
            playerSegments[segment] = {segments[i], recordDate, playerName,
                                       metadata};
          }
        }
      };

  // The global best is a materialised min over the per-player bests: for each
  // segment, whichever player holds the fastest time, breaking ties by the
  // earlier record date and then the lower user ID.
  struct GlobalBest {
    int32_t time;
    int32_t userId;
    std::string recordDate;
    std::string playerName;
  };

  using RunKey = std::tuple<int32_t, std::string, std::string>;
  std::map<RunKey, std::map<int32_t, GlobalBest>> global;

  for (const auto &[playerKey, playerSegments] : personal) {
    const auto &[seasonId, map, run, userId] = playerKey;
    auto &runSegments = global[std::make_tuple(seasonId, map, run)];

    for (const auto &[segment, best] : playerSegments) {
      const auto it = runSegments.find(segment);
      const bool better =
          it == runSegments.end() || best.time < it->second.time ||
          (best.time == it->second.time &&
           std::tie(best.recordDate, userId) <
               std::tie(it->second.recordDate, it->second.userId));

      if (better) {
        runSegments[segment] = {best.time, userId, best.recordDate,
                                best.playerName};
      }
    }
  }

  DatabaseV2::TransactionGuard txn(*_database);

  for (const auto &[key, segments] : personal) {
    const auto &[seasonId, map, run, userId] = key;
    // the raw run is the identity and stays displayed; the sanitized copy is
    // what queries match on (color codes stripped, lowercased)
    const auto runSanitized = StringUtils::sanitize(run, true);

    for (const auto &[segment, best] : segments) {
      _database->sql << R"(
        insert into player_best_segments (
          season_id,
          map,
          run,
          run_sanitized,
          user_id,
          segment,
          time,
          record_date,
          player_name,
          metadata
        ) values (
          ?,
          ?,
          ?,
          ?,
          ?,
          ?,
          ?,
          ?,
          ?,
          ?
        );
      )" << seasonId << map
                     << run << runSanitized << userId << segment << best.time
                     << best.recordDate << best.playerName << best.metadata;

      // the initial bests become the initial history rows, so the progression
      // starts from what the live tables started from. seeded rows are
      // unranked: the rank they held is not known retroactively, and the first
      // live improvement records the rank it beat
      _database->sql << R"(
        insert into player_best_segments_history (
          season_id,
          map,
          run,
          run_sanitized,
          user_id,
          segment,
          time,
          rank,
          record_date,
          player_name,
          metadata
        ) values (
          ?,
          ?,
          ?,
          ?,
          ?,
          ?,
          ?,
          null,
          ?,
          ?,
          ?
        );
      )" << seasonId << map
                     << run << runSanitized << userId << segment << best.time
                     << best.recordDate << best.playerName << best.metadata;
    }
  }

  for (const auto &[key, segments] : global) {
    const auto &[seasonId, map, run] = key;
    const auto runSanitized = StringUtils::sanitize(run, true);

    for (const auto &[segment, best] : segments) {
      _database->sql << R"(
        insert into best_segments (
          season_id,
          map,
          run,
          run_sanitized,
          segment,
          time,
          user_id,
          record_date,
          player_name
        ) values (
          ?,
          ?,
          ?,
          ?,
          ?,
          ?,
          ?,
          ?,
          ?
        );
      )" << seasonId << map
                     << run << runSanitized << segment << best.time
                     << best.userId << best.recordDate << best.playerName;
    }
  }

  txn.commit();
}

void TimerunRepository::migrate() {
  _database->addMigration(
      // clang-format off
      "initial",
      {R"(
          create table season (
              id integer primary key autoincrement,
              name text not null,
              start_time timestamp not null,
              end_time timestamp null
          );
        )",
       R"(
            insert into season (
              id,
              name,
              start_time,
              end_time
            ) values (
              1,
              'Default',
              '2000-01-01 00:00:00',
              null
            );
          )",
       R"(
            create table record (
              season_id integer not null,
              map text not null,
              run text not null,
              user_id int not null,
              time int not null,
              checkpoints text not null,
              record_date timestamp not null,
              player_name text not null,
              metadata text not null default '',
              primary key (season_id, map, run, user_id),
              foreign key (season_id) references season(id)
            );
          )",
       "create index idx_season_id_map on record(season_id, map);",
       "create index idx_season_id_map_run on record(season_id, map, run);",
       "create index idx_season_id_map_run_user_id on record(season_id, map, run, user_id);"});

  _database->addMigration(
      // clang-format off
      "removed_records",
      {R"(
          create table removed_records (
            id integer primary key autoincrement,
            season_id integer not null,
            map text not null,
            run text not null,
            user_id int not null,
            time int not null,
            checkpoints text not null,
            record_date timestamp not null,
            player_name text not null,
            metadata text not null default '',
            removed_by int not null,
            removed_at timestamp not null,
            reason text null
          );
        )",
       "create index idx_removed_records_user_id on removed_records(user_id);"});
  // clang-format on

  _database->addMigration(
      "record_history",
      {R"(
        create table record_history (
          id integer primary key autoincrement,
          season_id integer not null,
          map text not null,
          run text not null,
          user_id int not null,
          time int not null,
          rank int null,
          checkpoints text not null,
          record_date timestamp not null,
          player_name text not null,
          metadata text not null default '',
          foreign key (season_id) references season(id)
        );
      )",
       R"(create index idx_record_history_user on record_history(user_id, season_id, map, run, record_date);)",
       R"(create index idx_record_history_lookup on record_history(season_id, map, run, record_date);)"});

  _database->addMigration(
      // clang-format off
      "best_segments",
      {R"(
        create table player_best_segments (
          season_id integer not null,
          map text not null,
          run text not null,
          run_sanitized text not null default '',
          user_id int not null,
          segment integer not null,
          time int not null,
          record_date timestamp not null,
          player_name text not null,
          metadata text not null default '',
          primary key (season_id, map, run, user_id, segment),
          foreign key (season_id) references season(id)
        );
      )",
       R"(
        create table player_best_segments_history (
          id integer primary key autoincrement,
          season_id integer not null,
          map text not null,
          run text not null,
          run_sanitized text not null default '',
          user_id int not null,
          segment integer not null,
          time int not null,
          rank integer null,
          record_date timestamp not null,
          player_name text not null,
          metadata text not null default ''
        );
      )",
       R"(
        create index idx_player_best_segments_global
          on player_best_segments(season_id, map, run, segment, time, record_date, user_id);
      )",
       R"(
        create index idx_player_best_segments_user
          on player_best_segments(season_id, map, user_id, run_sanitized, segment, time, run);
      )",
       R"(
        create index idx_player_best_segments_history_listing
          on player_best_segments_history(user_id, map, run_sanitized, segment, season_id, record_date desc, id desc);
      )",
       R"(
        create index idx_player_best_segments_history_lookup
          on player_best_segments_history(season_id, map, run, segment, record_date);
      )",
       R"(
        create table best_segments (
          season_id integer not null,
          map text not null,
          run text not null,
          run_sanitized text not null default '',
          segment integer not null,
          time int not null,
          user_id int not null,
          record_date timestamp not null,
          player_name text not null,
          primary key (season_id, map, run, segment),
          foreign key (season_id) references season(id)
        );
      )",
       R"(
        create table removed_segments (
          id integer primary key autoincrement,
          season_id integer not null,
          map text not null,
          run text not null,
          run_sanitized text not null default '',
          user_id int not null,
          segment integer not null,
          time int not null,
          record_date timestamp not null,
          player_name text not null,
          metadata text not null default '',
          removed_by int not null,
          removed_at timestamp not null,
          reason text null
        );
      )",
       R"(
        create unique index idx_removed_segments_identity
          on removed_segments(season_id, map, run, user_id, record_date, segment);
      )",
       R"(
        create index idx_removed_segments_user_id on removed_segments(user_id);
      )"});
  // clang-format on

  _database->applyMigrations();

  tryToMigrateRecords();
  seedRecordHistory();
  seedBestSegments();
}

std::string TimerunRepository::serializeMetadata(
    std::map<std::string, std::string> metadata) {
  std::string result;
  for (const auto &kvp : metadata) {
    // NOTE: we're not escaping = so if payload contains = this will fail...
    result += kvp.first + "=" + kvp.second;
  }
  return result;
}

std::vector<Timerun::Record>
TimerunRepository::getRecordsFromQuery(sqlite::database_binder &binder) {
  std::vector<Timerun::Record> records;
  binder >> [&records](int seasonId, std::string map, std::string runName,
                       int userId, int time, std::string checkpointsString,
                       std::string recordDate, std::string playerName,
                       std::string metadataString) {
    auto record = getRecordFromStandardQueryResult(
        seasonId, std::move(map), std::move(runName), userId, time,
        std::move(checkpointsString), std::move(recordDate),
        std::move(playerName), std::move(metadataString));

    records.push_back(std::move(record));
  };

  return records;
}

std::vector<Timerun::RemovedRecord>
TimerunRepository::getRemovedRecordsFromQuery(sqlite::database_binder &binder) {
  std::vector<Timerun::RemovedRecord> records;

  binder >> [&records](int32_t id, int32_t seasonId, std::string map,
                       std::string run, int32_t userId, int32_t time,
                       std::string checkpointsString, std::string recordDate,
                       std::string playerName, std::string metadataString,
                       int32_t removedBy, const std::string &removedAt,
                       std::unique_ptr<std::string> reason) {
    Timerun::RemovedRecord removedRecord;

    removedRecord.id = id;
    removedRecord.record = getRecordFromStandardQueryResult(
        seasonId, std::move(map), std::move(run), userId, time,
        std::move(checkpointsString), std::move(recordDate),
        std::move(playerName), std::move(metadataString));
    removedRecord.removedBy = removedBy;
    removedRecord.removedAt = TimeUtils::Time::fromString(removedAt);

    if (reason) {
      removedRecord.reason = *reason;
    }

    records.emplace_back(std::move(removedRecord));
  };

  return records;
}
} // namespace ETJump
