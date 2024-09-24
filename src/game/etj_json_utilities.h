/*
 * MIT License
 *
 * Copyright (c) 2024 ETJump team <zero@etjump.com>
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

#include "json/json.h"
#include "etj_log.h"

namespace ETJump {
class JsonUtils {
  static Log logger;

public:
  // returns true on successful read
  static bool readFile(const std::string &file, Json::Value &root);

  // returns true on successful write
  static bool writeFile(const std::string &file, const Json::Value &root);

  // tries to parse a JSON value
  // returns true if parse was successful, otherwise false
  // if provided, any errors are written to 'errors'
  // 'field' can be provided as the key for more descriptive error messages
  template <typename T>
  static bool parseValue(T &value, const Json::Value &jsonValue,
                         std::string *errors = nullptr,
                         const std::string &field = "") {
    try {
      value = jsonValue.as<T>();
      return true;
    } catch (const Json::LogicError &e) {
      if (errors) {
        *errors = stringFormat(
            "Failed to parse JSON value%s: %s",
            field.empty() ? "" : stringFormat(" for field '%s'", field),
            e.what());
      }

      return false;
    }
  }
};
} // namespace ETJump
