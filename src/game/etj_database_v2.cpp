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

#include "etj_database_v2.h"

#include "etj_log.h"
#include "etj_printer.h"
#include "etj_string_utilities.h"

ETJump::Log logger("databasev2");

namespace ETJump {
DatabaseV2::DatabaseV2(const std::string &name, const std::string &fileName)
    : sql(sqlite::database(fileName)), _name(name) {
  logger.info(stringFormat("Initializing `%s` at `%s`", name, fileName));
  sql << "PRAGMA journal_mode=WAL;";

  sql.define("lsanitize",
             [](std::string s) { return ETJump::sanitize(s, true); });
  sql.define("sanitize",
             [](std::string s) { return ETJump::sanitize(s, false); });
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

    for (const auto &statement : migration.statements) {
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
    logger.info(stringFormat(
        "`%s` is up to date. Latest migration `%s`. No migrations applied.",
        _name, latestMigration));
  }
}
} // namespace ETJump
