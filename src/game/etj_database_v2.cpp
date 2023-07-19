#include "etj_database_v2.h"

#include "etj_log.h"
#include "etj_printer.h"
#include "etj_string_utilities.h"

ETJump::Log logger("databasev2");

namespace ETJump {
DatabaseV2::DatabaseV2(const std::string& name, const std::string &fileName)
  : sql(sqlite::database(fileName)), _name(name) {
  logger.info(
      stringFormat("Initializing `%s` at `%s`", name, fileName));
  sql << "PRAGMA journal_mode=WAL;";
}

DatabaseV2::~DatabaseV2() = default;

void DatabaseV2::addMigration(const Migration &migration) {
  _migrations.push_back(migration);
}

void DatabaseV2::addMigration(const std::string &name,
                              const std::vector<std::string> &statements) {
  _migrations.push_back(Migration{name, statements});
}

void DatabaseV2::applyMigrations() {
  logger.info(stringFormat("Applying migrations to `%s`", _name));

  sql << R"(
      create table if not exists migrations (
          name text primary key not null,
          created_at timestamp default current_timestamp
      );
    )";

  std::vector<std::string> migrations;

  sql << "begin;";

  sql << R"(
      select name from migrations;
    )" >>
      [&](std::string name) { migrations.push_back(name); };

  bool appliedMigration = false;

  for (const auto &migration : _migrations) {
    if (std::find(migrations.begin(), migrations.end(), migration.name) !=
        migrations.end()) {
      continue;
    }

    appliedMigration = true;

    logger.info("Applying migration `%s` to `%s`", migration.name, _name);

    for (const auto& statement : migration.statements) {
      sql << statement;  
    }

    sql << R"(
        insert into migrations (
          name
        ) values (
          ?
        );
      )" << migration.name;
  }

  sql << "commit;";

  if (!appliedMigration) {
    std::string latestMigration = "no migration available";
    if (!_migrations.empty()) {
      latestMigration = _migrations[_migrations.size() - 1].name;
    }
    logger.info(
        stringFormat("`%s` is up to date. Latest migration `%s`. No migrations applied.", _name, latestMigration));
  }
}
}
