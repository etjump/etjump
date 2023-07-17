#include "etj_database_v2.h"

namespace ETJump {
DatabaseV2::DatabaseV2(const std::string &fileName)
  : sql(sqlite::database(fileName)) {
  sql << "PRAGMA journal_mode=WAL;";
}

DatabaseV2::~DatabaseV2() {
}

void DatabaseV2::addMigration(const Migration &migration) {
  _migrations.push_back(migration);
}

void DatabaseV2::applyMigrations() {
  sql << R"(
      create table if not exists migrations (
          id integer primary key autoincrement,
          name text not null,
          created_at timestamp default current_timestamp
      );
    )";

  std::vector<std::string> migrations;

  sql << R"(
      select name from migrations order by id asc;
    )" >>
      [&](std::string name) { migrations.push_back(name); };

  for (const auto &migration : _migrations) {
    if (std::find(migrations.begin(), migrations.end(), migration.name) !=
        migrations.end()) {
      continue;
    }

    sql << migration.sql;

    sql << R"(
        insert into migrations (
          name
        ) values (
          ?
        );
      )" << migration.name;
  }
}
}
