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

#include <utility>

#include "etj_timerun_repository.h"

#include "etj_container_utilities.h"
#include "etj_database_v2.h"
#include "q_shared.h"

namespace ETJump {
Timerun::Record getRecordFromStandardQueryResult(
    int seasonId, std::string map, std::string runName, int userId, int time,
    std::string checkpointsString, std::string recordDate,
    std::string playerName, std::string metadataString) {
  auto checkpoints = Container::map(
      Container::filter(StringUtils::split(checkpointsString, ","),
                        [](const std::string &input) {
                          return StringUtils::trim(input).length() > 0;
                        }),
      [](const std::string &checkpoint) {
        try {
          return std::stoi(StringUtils::trim(checkpoint));
        } catch (const std::logic_error &) {
          return TIMERUN_CHECKPOINT_NOT_SET;
        }
      });
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
            seasonId, map, runName, userId, time, checkpointsString, recordDate,
            playerName, metadataString);

        records.push_back(record);
      };

  return records;
}

std::vector<Timerun::Record>
TimerunRepository::getRecordsForRun(const std::string &map,
                                    const std::string &run) const {
  throw std::runtime_error("Not implemented");
}

void TimerunRepository::insertRecord(const Timerun::Record &record) {
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

void TimerunRepository::updateRecord(const Timerun::Record &record) {
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
            seasonId, map, runName, userId, time, checkpointsString, recordDate,
            playerName, metadataString));
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
        seasonId, map, runName, userId, time, checkpointsString, recordDate,
        playerName, metadataString));
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

  TimeUtils::Time newStartTime = startTime;
  std::optional<TimeUtils::Time> newEndTime = endTime;

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
TimerunRepository::getMapsForName(const std::string &map, bool exact) {

  std::string mapFilter = exact ? "map=?" : "map like ?";
  std::string mapSearchString = exact ? map : "%" + map + "%";

  std::vector<std::string> maps;
  _database->sql << StringUtils::format(R"(
    select
      distinct map
    from record
    where %s
    collate nocase
  )",
                                        mapFilter)
                 << mapSearchString >>
      [&maps](std::string map) { maps.push_back(map); };
  return maps;
}

std::vector<std::string>
TimerunRepository::getRunsForName(const std::string &map,
                                  const std::string &run, bool exact,
                                  bool sanitizeResults) {

  std::string runFilter = exact ? "lsanitize(run)=?" : "lsanitize(run) like ?";
  std::string runSearchString = exact ? run : "%" + run + "%";

  std::vector<std::string> runs;
  _database->sql << StringUtils::format(R"(
    select
      distinct run
    from record
    where %s
      and map = ?
    collate nocase
  )",
                                        runFilter)
                 << runSearchString << map >>
      [&runs, sanitizeResults](const std::string &run) {
        runs.push_back(sanitizeResults ? StringUtils::sanitize(run, true)
                                       : run);
      };
  return runs;
}

std::string
TimerunRepository::resolveMapName(const std::string &map, bool exact,
                                  const std::string &commandPrefix) {
  const auto maps = getMapsForName(map, exact);

  if (maps.size() > 1 && !Container::isIn(maps, map)) {
    throw std::runtime_error(
        buildMatchSuggestionsError(commandPrefix, "map", map, maps));
  }

  return maps.empty() ? map : maps[0];
}

std::string TimerunRepository::resolveRunName(const std::string &map,
                                              const std::string &run,
                                              bool exact) {
  const auto runs = getRunsForName(map, run, exact, true);

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
    order by season_id, map, run, time;
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
    order by season_id, map, run, time asc, record_date asc
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
            seasonId, map, runName, userId, time, checkpointsString, recordDate,
            playerName, metadataString);
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

  DatabaseV2::TransactionGuard txn(*_database);

  _database->sql << "delete from record where season_id=?;" << id;
  // make sure we also purge 'removed_records',
  // so we don't leave records from any nonexistent seasons in the table
  _database->sql << "delete from removed_records where season_id=?;" << id;
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
  int count = 0;
  _oldDatabase->sql
          << "select count(*) from sqlite_master where tbl_name='records'" >>
      count;
  if (count == 0) {
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
        r.map = map;
        r.run = run;
        r.time = time;
        r.recordDate = TimeUtils::Time::fromInt(recordDate);
        r.userId = userId;
        r.playerName = playerName;
        r.checkpoints = std::vector<int>(MAX_TIMERUN_CHECKPOINTS,
                                         TIMERUN_CHECKPOINT_NOT_SET);
        r.metadata = {{"mod_version", "unknown(imported)"}};
        oldRecords.push_back(r);
      };

  _database->sql << "begin;";

  for (const auto &r : oldRecords) {
    insertRecord(r);
  }

  _database->sql << "commit;";
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

  _database->applyMigrations();

  int count = 0;
  _database->sql << "select count(*) from record" >> count;

  if (count == 0) {
    tryToMigrateRecords();
  }
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
        seasonId, map, runName, userId, time, checkpointsString, recordDate,
        playerName, metadataString);

    records.push_back(record);
  };
  return records;
}
} // namespace ETJump
