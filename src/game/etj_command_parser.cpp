/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
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

#include <stdexcept>
#include "etj_command_parser.h"

#include "etj_string_utilities.h"

const ETJump::CommandParser::OptionDefinition *
ETJump::CommandParser::getOptionOrNull() {
  if (StringUtil::startsWith(*_current, "--")) {
    const auto optionName = (*_current).substr(2);

    if (_def.options.count(optionName) > 0) {
      return &_def.options[optionName];
    }
  } else if (StringUtil::startsWith(*_current, "-")) {
    const auto optionShortName = (*_current).substr(1);

    for (const auto &op : _def.options) {
      if (op.second.shortName == optionShortName) {
        return &_def.options[op.first];
      }
    }
  }

  return nullptr;
}

void ETJump::CommandParser::expectBoolean(
    const OptionDefinition *optionDefinition) {
  Option option(optionDefinition->name);

  option.boolean = true;

  _cmd.options[option.name] = option;
}

void ETJump::CommandParser::expectToken(
    const OptionDefinition *optionDefinition) {
  ++_current;

  if (_current == end(_args)) {
    _cmd.errors.push_back(
        stringFormat("Missing parameter for `%s`", optionDefinition->name));
    return;
  }

  _cmd.options[optionDefinition->name] =
      createTokenOption(optionDefinition, *_current);
}

ETJump::CommandParser::Option ETJump::CommandParser::createTokenOption(
    const OptionDefinition *optionDefinition, const std::string &text) {
  Option option(optionDefinition->name);
  option.text = text;

  return option;
}

void ETJump::CommandParser::expectMultipleTokens(
    const OptionDefinition *optionDefinition) {
  ++_current;

  if (_current == end(_args)) {
    _cmd.errors.push_back(
        stringFormat("Missing parameter for `%s`", optionDefinition->name));
    return;
  }

  Option option(optionDefinition->name);
  std::vector<std::string> tokens{};

  while (_current != end(_args) && getOptionOrNull() == nullptr) {
    tokens.push_back(*_current);
    ++_current;
  }

  if (_current != end(_args)) {
    // we peeked the next one, and we want the parser to continue from
    // it instead of the one after that
    --_current;
  }

  if (tokens.empty()) {
    _cmd.errors.push_back(
        stringFormat("Missing parameter for `%s`", optionDefinition->name));
    return;
  }

  option.text = StringUtil::join(tokens, " ");

  _cmd.options[optionDefinition->name] = std::move(option);
}

void ETJump::CommandParser::expectInteger(
    const OptionDefinition *optionDefinition) {
  ++_current;

  if (_current == end(_args)) {
    _cmd.errors.push_back(
        stringFormat("Missing parameter for `%s`", optionDefinition->name));
    return;
  }

  _cmd.options[optionDefinition->name] =
      createIntegerOption(optionDefinition, *_current);
}

ETJump::CommandParser::Option ETJump::CommandParser::createIntegerOption(
    const OptionDefinition *optionDefinition, const std::string &text) {
  Option option(optionDefinition->name);
  option.text = text;

  try {
    option.integer = std::stoi(text);
  } catch (const std::invalid_argument &) {
    _cmd.errors.push_back(stringFormat(
        "`%s` is not an integer. Expected an integer for parameter `%s`", text,
        option.name));
  } catch (const std::out_of_range &) {
    _cmd.errors.push_back(stringFormat(
        "`%s` is out of range. Expected an integer for parameter `%s`", text,
        option.name));
  }

  return option;
}

void ETJump::CommandParser::expectDecimal(
    const OptionDefinition *optionDefinition) {
  ++_current;

  if (_current == end(_args)) {
    _cmd.errors.push_back(
        stringFormat("Missing parameter for `%s`", optionDefinition->name));
    return;
  }

  _cmd.options[optionDefinition->name] =
      createDecimalOption(optionDefinition, *_current);
}

ETJump::CommandParser::Option ETJump::CommandParser::createDecimalOption(
    const OptionDefinition *optionDefinition, const std::string &text) {
  Option option(optionDefinition->name);
  option.text = text;

  try {
    option.decimal = std::stod(text);
  } catch (const std::invalid_argument &) {
    _cmd.errors.push_back(stringFormat(
        "`%s` is not a decimal. Expected a decimal for parameter `%s`", text,
        option.name));
  } catch (const std::out_of_range &) {
    _cmd.errors.push_back(stringFormat(
        "`%s` is out of range. Expected a decimal for parameter `%s`", text,
        option.name));
  }

  return option;
}

void ETJump::CommandParser::expectDate(
    const OptionDefinition *optionDefinition) {
  ++_current;

  if (_current == end(_args)) {
    _cmd.errors.push_back(
        stringFormat("Missing parameter for `%s`", optionDefinition->name));
    return;
  }

  _cmd.options[optionDefinition->name] =
      createDateOption(optionDefinition, *_current);
}

ETJump::CommandParser::Option ETJump::CommandParser::createDateOption(
    const OptionDefinition *optionDefinition, const std::string &text) {
  Option option(optionDefinition->name);
  option.text = text;

  try {
    option.date = Date::fromString(text);
  } catch (const std::invalid_argument &) {
    _cmd.errors.push_back(stringFormat(
        "`%s` does not match the expected format of `YYYY-MM-DD`", text));
  }

  return option;
}

void ETJump::CommandParser::expectOption() {
  if (*_current == "--help" || *_current == "-h") {
    _cmd.helpRequested = true;
    return;
  }
  auto optionOrNull = getOptionOrNull();
  if (optionOrNull) {
    switch (optionOrNull->type) {
      case OptionDefinition::Type::Boolean:
        expectBoolean(optionOrNull);
        break;
      case OptionDefinition::Type::Token:
        expectToken(optionOrNull);
        break;
      case OptionDefinition::Type::MultiToken:
        expectMultipleTokens(optionOrNull);
        break;
      case OptionDefinition::Type::Integer:
        expectInteger(optionOrNull);
        break;
      case OptionDefinition::Type::Decimal:
        expectDecimal(optionOrNull);
        break;
      case OptionDefinition::Type::Date:
        expectDate(optionOrNull);
        break;
    }
  } else {
    expectExtraArg();
  }
}

void ETJump::CommandParser::expectExtraArg() {
  _cmd.extraArgs.push_back(*_current);
}

void ETJump::CommandParser::expectOptionOrExtraArgs() {
  if (_current == end(_args)) {
    return;
  }

  if (StringUtil::startsWith(*_current, "-")) {
    expectOption();
  } else {
    expectExtraArg();
  }
}

void ETJump::CommandParser::processPositionalArguments() {
  for (const auto &optDef : _def.options) {
    if (optDef.second.position.hasValue()) {
      if (_cmd.options.count(optDef.second.name) > 0) {
        continue;
      }
      auto pos = optDef.second.position.value();

      if (_cmd.extraArgs.size() >= static_cast<unsigned>(pos) + 1) {
        switch (optDef.second.type) {
          case OptionDefinition::Type::Boolean:
            continue;
          case OptionDefinition::Type::Token:
            _cmd.options[optDef.second.name] =
                createTokenOption(&optDef.second, _cmd.extraArgs[pos]);
            break;
          case OptionDefinition::Type::MultiToken:
            continue;
          case OptionDefinition::Type::Integer:
            _cmd.options[optDef.second.name] =
                createIntegerOption(&optDef.second, _cmd.extraArgs[pos]);
            break;
          case OptionDefinition::Type::Decimal:
            _cmd.options[optDef.second.name] =
                createDecimalOption(&optDef.second, _cmd.extraArgs[pos]);
            break;
          case OptionDefinition::Type::Date:
            _cmd.options[optDef.second.name] =
                createDateOption(&optDef.second, _cmd.extraArgs[pos]);
            break;
        }
      }
    }
  }
}

void ETJump::CommandParser::validateCommand() {
  for (const auto &option : _def.options) {
    if (option.second.required && _cmd.options.count(option.first) == 0) {
      _cmd.errors.push_back(stringFormat(
          "Required option `%s` was not specified.", option.first));
    }
  }
}

ETJump::CommandParser::Command ETJump::CommandParser::parse() {
  try {
    if (_def.options.empty()) {
      _cmd.extraArgs.insert(end(_cmd.extraArgs), begin(_args), end(_args));
      return _cmd;
    }

    _current = begin(_args);

    while (_current != end(_args)) {
      expectOptionOrExtraArgs();

      // option parser might increment the iterator
      if (_current == end(_args)) {
        break;
      }

      ++_current;
    }

    processPositionalArguments();

    validateCommand();

    return _cmd;
  } catch (const std::runtime_error &e) {
    _cmd.errors.push_back(
        stringFormat("Unknown runtime error: `%s`", e.what()));
    return _cmd;
  }
}
std::string ETJump::CommandParser::formatCommandHelpString(
    const std::string &name, const std::string &shortname,
    OptionDefinition::Type optionType, const std::string &description,
    bool required, opt<int> position) {
  // console line width varies a bit depending on engine and resolution,
  // but this should be fine for most scenarios
  const size_t maxLineLen = 120;
  const size_t flagPad = 23 - shortname.length();

  const std::string &opTypeStr = OptionDefinition::typeToString(optionType);
  const std::string &positionStr = stringFormat(
      "%s",
      position.hasValue() ? stringFormat(" (pos: %d) ", position.value()) : "");
  const std::string &requiredStr =
      stringFormat("%s", required ? " [required] " : "");

  const std::string &flagStr =
      stringFormat("    ^7-%s, --%-*s", shortname, flagPad, name);
  const std::string &descStr = stringFormat(
      "^z(%s) ^7%s%s%s", opTypeStr, description, requiredStr, positionStr);

  std::string combinedStr = flagStr + descStr;

  // split from the previous whitespace and indent the new line,
  // if the description text is too long to fit to console max line width
  if (sanitize(combinedStr, false).length() > maxLineLen) {
    const size_t lastWhiteSpace = combinedStr.find_last_of(' ', maxLineLen);

    if (lastWhiteSpace != std::string::npos) {
      combinedStr.replace(
          lastWhiteSpace, 1,
          '\n' + std::string(sanitize(flagStr, false).length(), ' '));
    }
  }

  return combinedStr;
}
