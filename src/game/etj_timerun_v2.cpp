#include "etj_timerun_v2.h"

#include <utility>
#include <ctime>

#include "etj_log.h"
#include "etj_printer.h"
#include "etj_synchronization_context.h"

ETJump::TimerunV2::TimerunV2(
    std::string currentMap,
    std::unique_ptr<DatabaseV2> database,
    std::unique_ptr<Log> logger,
    std::unique_ptr<SynchronizationContext> synchronizationContext)
  : _currentMap(std::move(currentMap)), _database(std::move(database)),
    _logger(std::move(logger)),
    _sc(std::move(synchronizationContext)) {
}


void ETJump::TimerunV2::initialize() {
  try {
    _database->addMigration(
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
              current_timestamp,
              null
            );
          )",
         R"(
            create table run (
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
         "create index idx_season_id_map on run(season_id, map);",
         "create index idx_season_id_map_run on run(season_id, map, run);",
         "create index idx_season_id_map_run_user_id on run(season_id, map, run, user_id);"
        }
        );

    _database->applyMigrations();

    _activeSeasons = std::vector<int>();
    std::vector<std::string> activeSeasons;

    std::string currentTime = getCurrentTime().toDateTimeString();

    _database->sql << "select id, name, start_time, end_time from season where "
        "start_time <= ? and (end_time is null or end_time > ?);"
        << currentTime << currentTime >>
        [this, &activeSeasons](int id, std::string name, std::string startTime,
                               std::string endTime) {
          _activeSeasons.push_back(id);
          activeSeasons.push_back(stringFormat("%s (%d)", name, id));
        };

    _logger->info("Active seasons: %s", StringUtil::join(activeSeasons, ", "));
  } catch (const std::exception &e) {
    Printer::LogPrintln(std::string("Unable to initialize timerun database") +
                        e.what());
  }

  _sc->startWorkerThreads(1);
}

void ETJump::TimerunV2::shutdown() {
  _database = nullptr;
  _sc->stopWorkerThreads();
}

void ETJump::TimerunV2::runFrame() { _sc->processCompletedTasks(); }

class ClientConnectResult : public ETJump::SynchronizationContext::ResultBase {
public:
  explicit ClientConnectResult(std::vector<ETJump::TimerunV2::Run> runs) : runs(runs) {}
  std::vector<ETJump::TimerunV2::Run> runs;
};

void ETJump::TimerunV2::clientConnect(int clientNum, int userId) {
  _sc->postTask(
      [this, clientNum, userId] {

        auto parameters = StringUtil::join(Utilities::map(_activeSeasons,
                                             [](int season) {
                                               return std::to_string(season);
                                             }),
                                           ", ");

        std::vector<Run> runs;

        // Above parameters are just season IDs, no risk of SQL injection
        // hence the direct interpolation
        _database->sql << stringFormat(R"(
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
          from run
          where season_id in (%s) and
            map=? and
            user_id=?;
        )", parameters) << _currentMap << userId >> [&runs, this](
            int seasonId, std::string map, std::string runName, int userId,
            int time, std::string checkpointsString, std::string recordDate,
            std::string playerName, std::string metadataString) {
              auto checkpoints =
                  Utilities::map(StringUtil::split(checkpointsString, ","),
                                 [](const std::string &checkpoint) {
                                   try {
                                     return std::stoi(trim(checkpoint));
                                   } catch (const std::runtime_error &e) {
                                     return CheckpointNotSet;
                                   }
                                 });
              Time recordDateTime{};
              try {
                recordDateTime = Time::fromString(recordDate);
              } catch (const std::runtime_error &e) {
                recordDateTime = Time::fromString("1900-01-01 00:00:00");
              }

              std::map<std::string, std::string> metadata;
              for (const auto &kvp :
                   Utilities::map(StringUtil::split(metadataString, ","),
                                  [](const std::string &kvp) {
                                    return StringUtil::split(kvp, "=");
                                  })) {
                if (kvp.size() != 2) {
                  continue;
                }

                metadata[kvp[0]] = kvp[1];
              }

              Run run;
              run.seasonId = seasonId;
              run.map = map;
              run.run = runName;
              run.userId = userId;
              run.time = time;
              run.recordDate = recordDateTime;
              run.checkpoints = checkpoints;
              run.playerName = playerName;
              run.metadata = metadata;
              runs.push_back(std::move(run));
            };

        return std::make_unique<ClientConnectResult>(runs);
      },
      [this, clientNum, userId](
      std::unique_ptr<ETJump::SynchronizationContext::ResultBase> result) {
        auto clientConnectResult =
            static_cast<ClientConnectResult *>(result.get());

        _players[clientNum] = std::make_unique<Player>(
            clientNum, userId, clientConnectResult->runs);
      },
      [this, clientNum, userId](std::runtime_error error) {
        _logger->info("Unable to load player information for clientId: `%d` "
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

class AddSeasonResult : public ETJump::SynchronizationContext::ResultBase {
public:
  AddSeasonResult(std::string message)
    : message(message) {
  }

  std::string message;
};

void ETJump::TimerunV2::addSeason(AddSeasonParams season) {
  _sc->postTask([this, season]() {
                  int count = 0;
                  _database->sql << R"(
                      select count(name) from season where name=? collate nocase;
                    )" << season.name >>
                      count;

                  if (count > 0) {
                    return std::make_unique<AddSeasonResult>(stringFormat(
                        "Cannot add season `%s` as it already exists.",
                        season.name));
                  }

                  if (season.endTime.hasValue()) {
                    if (season.startTime >= season.endTime.value()) {
                      return std::make_unique<AddSeasonResult>(
                          "Start time cannot be after end time");
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

                  if (season.endTime.hasValue())
                    _database->sql << insert << season.name
                        << season.startTime.toDateTimeString()
                        << (*season.endTime).toDateTimeString();
                  else
                    _database->sql << insert << season.name
                        << season.startTime.toDateTimeString()
                        << nullptr;

                  return std::make_unique<AddSeasonResult>(
                      stringFormat("Successfully added season `%s`",
                                   season.name));

                },
                [this, season](
                std::unique_ptr<SynchronizationContext::ResultBase> result) {

                  auto addSeasonResult =
                      static_cast<AddSeasonResult *>(result.get());

                  Printer::SendConsoleMessage(season.clientNum,
                                              addSeasonResult->message + "\n");
                },
                [this, season](std::runtime_error e) {
                  Printer::SendConsoleMessage(
                      season.clientNum,
                      stringFormat("Unable to add season: %s\n", e.what()));
                }
      );
}
