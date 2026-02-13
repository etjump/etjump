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

#pragma once

#include <toml.hpp>

#include "etj_file.h"
#include "etj_filesystem.h"
#include "etj_string_utilities.h"

namespace ETJump {
class TOMLUtils {
public:
  template <typename T>
  static bool readFile(const std::string &file, T &table,
                       std::string *err = nullptr) {
    const auto filePath = FileSystem::Path::getPath(file);

    try {
      if constexpr (std::is_same_v<T, toml::value>) {
        table = toml::parse(filePath);
      } else if constexpr (std::is_same_v<T, toml::ordered_value>) {
        table = toml::parse<toml::ordered_type_config>(filePath);
      } else {
        static_assert(sizeof(T) == 0, "Unsupported TOML value type");
      }
    } catch (const toml::syntax_error &e) {
      if (err) {
        *err =
            stringFormat("Failed to parse TOML file '%s': %s", file, e.what());
      }

      return false;
    } catch (toml::file_io_error &e) {
      if (err) {
        *err = stringFormat("Failed to open file '%s' for reading: %s", file,
                            e.what());
      }

      return false;
    }

    return true;
  }

  template <typename T>
  static bool writeFile(const std::string &file, const T &table,
                        std::string *err = nullptr) {
    if (file.empty()) {
      if (err) {
        *err = "Failed to write TOML file: empty filename";
      }

      return false;
    }

    try {
      const File fOut(file, File::Mode::Write);
      fOut.write(toml::format(table));
    } catch (const File::FileIOException &e) {
      if (err) {
        *err = stringFormat("Failed to write TOML file: %s", e.what());
      }

      return false;
    }

    return true;
  }
};
} // namespace ETJump
