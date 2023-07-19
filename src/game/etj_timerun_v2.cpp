#include "etj_timerun_v2.h"

#include <utility>

#include "etj_log.h"
#include "etj_printer.h"

ETJump::TimerunV2::TimerunV2(
    std::unique_ptr<Log> logger,
    std::unique_ptr<SynchronizationContext> synchronizationContext)
  : _logger(std::move(logger)), _sc(std::move(synchronizationContext)) {
}


void ETJump::TimerunV2::initialize(const Options &options) {
  try {
    _database = std::make_unique<DatabaseV2>("timerunv2", options.path);

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
         "create index idx_map on run(map);",
         "create index idx_map_run on run(map, run);",
         "create index idx_map_run_user_id on run(map, run, user_id);"
        }
        );

    _database->applyMigrations();
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

                  auto t = std::this_thread::get_id();

                  Printer::SendConsoleMessage(season.clientNum,
                                              addSeasonResult->message);
                },
                [this, season](std::runtime_error e) {
                  Printer::SendConsoleMessage(
                      season.clientNum,
                      stringFormat("Unable to add season: %s", e.what()));
                }
      );
}
