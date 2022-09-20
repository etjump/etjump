/*
 * MIT License
 *
 * Copyright (c) 2022 ETJump team <zero@etjump.com>
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
#include <map>
#include <vector>

namespace ETJump {
class CommandParser {
public:
  struct OptionDefinition {
    enum class Type {
      Boolean,
      Token,
      MultiToken,
      Integer,
      Decimal,
      Date,
      Duration
    };

    std::string name;
    std::string description;
    Type type;
    bool required;
  };

  struct CommandDefinition {
    std::string name;
    std::string description;
    std::map<std::string, OptionDefinition> options;
  };

  struct Option {
    std::string name;
    bool boolean;
    std::string text;
    int integer;
    double decimal;
    long date;
    long duration;
  };

  struct Command {
    std::vector<std::string> errors;
    std::map<std::string, Option> options;
    std::vector<std::string> extraArgs;
  };

  Command parse(CommandDefinition definition, std::vector<std::string> args);

private:
  /**
   * Tries to parse an option. Returns true if successful and updates
   * option Returns false if failed
   * @param arg
   * @param option
   * @returns boolean
   */
  bool parseOption(const std::string &arg, std::string &option);
};
} // namespace ETJump
