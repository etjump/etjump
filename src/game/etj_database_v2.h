/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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
#include <string>
#include <sqlite_modern_cpp.h>

#include "etj_container_utilities.h"
#include "etj_string_utilities.h"

namespace ETJump {
class DatabaseV2 {
public:
  struct Migration {
    std::string name;
    std::vector<std::string> statements;
  };

  explicit DatabaseV2(const std::string &name, const std::string &fileName);
  ~DatabaseV2();

  void addMigration(const Migration &migration);
  void addMigration(const std::string &name,
                    const std::vector<std::string> &statements);

  void applyMigrations();

  template <typename T>
  static std::string createPlaceholderString(const std::vector<T> &input) {
    return StringUtil::join(Container::map(input, [](const auto &e) {
      return "?";
    }), ",");
  }

  // expose the database object directly
  // as it provides a reasonable interface
  // to database
  sqlite::database sql;

private:
  std::string _name;
  std::vector<Migration> _migrations;
};
}
