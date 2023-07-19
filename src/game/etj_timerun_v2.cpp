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

void ETJump::TimerunV2::addSeason(AddSeasonParams season) {
  _sc->postTask([this]() {
                  return std::make_unique<SynchronizationContext::ResultBase>();
                },
                [this](
                std::unique_ptr<SynchronizationContext::ResultBase> result) {
                  std::string name;

                  _logger->info("success");
                },
                [this](std::runtime_error e) {
                  _logger->info("error: %s", e.what());
                }
      );

  //auto addSeasonTask = [this](AddSeasonParams params) -> AsyncResult {
  //  int count = 0;
  //  _database->sql << R"(
  //    select count(name) from season where name=? collate nocase;
  //  )" << params.name >>
  //      count;

  //  if (count > 0) {
  //    return AsyncResult{
  //        stringFormat("Cannot add season `%s` as it already exists.",
  //                     params.name),
  //        ""};
  //  }

  //  if (params.endTime.hasValue()) {
  //    if (params.startTime >= *params.endTime) {
  //      return {"Start time cannot be after end time", ""};
  //    }
  //  }

  //  std::string insert = R"(
  //    insert into season (
  //      name,
  //      start_time,
  //      end_time
  //    ) values (
  //      ?,
  //      ?,
  //      ?
  //    );
  //  )";

  //  if (params.endTime.hasValue())
  //    _database->sql << insert << params.name
  //        << params.startTime.time_since_epoch().count()
  //        << (*params.endTime).time_since_epoch().count();
  //  else
  //    _database->sql << insert << params.name
  //        << params.startTime.time_since_epoch().count() << nullptr;

  //  return {"", ""};
  //};

  //auto res = std::async(std::launch::async, addSeasonTask, season);

  //auto status = res.wait_for(std::chrono::seconds(0));

  //try {
  //  if (status == std::future_status::ready) {
  //    _logger->info("ready");
  //  } else {
  //    res.wait();
  //    _logger->info("done");
  //  }
  //  _logger->info(res.get().error);
  //} catch (std::runtime_error &e) {
  //  _logger->info(e.what());
  //}

}
