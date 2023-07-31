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
#include <map>
#include <ostream>
#include <vector>

#include "etj_container_utilities.h"
#include "etj_synchronization_context.h"
#include "etj_time_utilities.h"

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
    };

    static std::string typeToString(Type t) {
      switch (t) {
        case Type::Boolean:
          return "BOOLEAN";
        case Type::Token:
          return "TEXT";
        case Type::MultiToken:
          return "MULTI-PART TEXT";
        case Type::Integer:
          return "INTEGER";
        case Type::Decimal:
          return "DECIMAL";
        case Type::Date:
          return "DATE";
      }
      return "";
    }

    std::string name{};
    std::string description{};
    Type type{Type::Token};
    bool required{true};

    static OptionDefinition create(const std::string &name,
                                   const std::string &description, Type type,
                                   bool required) {
      auto def = OptionDefinition{};

      def.name = name;
      def.description = description;
      def.type = type;
      def.required = required;

      return def;
    }
  };

  struct CommandDefinition {
    std::string name{};
    std::string description{};
    std::map<std::string, OptionDefinition> options{};

    std::string help() const {
      auto optionsToStrings = Container::map(options, [](const auto &pair) {
        return stringFormat("    --%s (%s) %s", pair.second.name,
                            OptionDefinition::typeToString(pair.second.type),
                            pair.second.description);
      });

      return stringFormat("Usage: %s\n\n    %s\n\nOptions:\n%s", name,
                          description,
                          StringUtil::join(optionsToStrings, "\n"));
    }

    static CommandDefinition
    create(const std::string name, const std::string description) {
      CommandDefinition def;

      def.name = name;
      def.description = description;
      def.options = {};

      return def;
    }

    CommandDefinition &addOption(const std::string &name,
                                 const std::string &description,
                                 OptionDefinition::Type type, bool required) {
      auto opt = OptionDefinition{};
      opt.name = name;
      opt.description = description;
      opt.type = type;
      opt.required = required;

      this->options[name] = opt;

      return *this;
    }
  };

  struct Option {
    Option() = default;

    explicit Option(const std::string &name)
      : name(name) {
    }

    std::string name{};
    bool boolean{};
    std::string text{};
    int integer{};
    double decimal{};
    Date date{};
  };

  struct Command {
    std::vector<std::string> errors;
    std::map<std::string, Option> options;
    std::vector<std::string> extraArgs;
    bool helpRequested{};

    opt<Option> getOptional(const std::string &name) {
      if (options.count(name) == 0) {
        return ETJump::opt<Option>();
      }
      return ETJump::opt<Option>(options[name]);
    };
  };

  CommandParser(CommandDefinition definition, std::vector<std::string> args)
    : _args(args),
      _current(begin(args)), _def(definition), _cmd(Command()) {
  }

  const OptionDefinition *getOptionOrNull();
  void expectBoolean(const OptionDefinition *optionDefinition);
  void expectToken(const OptionDefinition *optionDefinition);
  void expectMultipleTokens(const OptionDefinition *optionDefinition);
  void expectInteger(const OptionDefinition *optionDefinition);
  void expectDecimal(const OptionDefinition *optionDefinition);
  void expectDate(const OptionDefinition *optionDefinition);
  void expectOption();
  void expectExtraArg();
  void expectOptionOrExtraArgs();
  void validateCommand();
  Command parse();

private:
  std::vector<std::string> _args;
  std::vector<std::string>::iterator _current;
  CommandDefinition _def;
  Command _cmd;
};
} // namespace ETJump
