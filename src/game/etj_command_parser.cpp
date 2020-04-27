/*
 * MIT License
 * 
 * Copyright (c) 2020 ETJump team <haapanen.jussi@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "etj_command_parser.h"
#include <boost/algorithm/string/case_conv.hpp>


ETJump::CommandParser::CommandParser()
{
}

ETJump::CommandParser::~CommandParser()
{
}

ETJump::CommandParser::Command ETJump::CommandParser::parse(CommandDefinition definition, std::vector<std::string> args)
{
	Command command;

	if (definition.options.size() == 0)
	{
		command.extraArgs.insert(end(command.extraArgs), begin(args), end(args));
		return command;
	}

	std::string option;
	auto optionDefinition = std::end(definition.options);
	auto newOptionDefinition = optionDefinition;
	auto end = std::end(definition.options);
	Option currentOption;

	/**
	 * Parse each argument from the args vector
	 * If current arg is an option and it can be found in the options definition
	 * save currently parsed option if there's any and start a new option
	 */

	for (const auto & arg : args)
	{
		newOptionDefinition = end;
		if (parseOption(arg, option))
		{
			newOptionDefinition = definition.options.find(option);
		}

		if (optionDefinition == end && newOptionDefinition == end)
		{
			command.extraArgs.push_back(arg);
			continue;
		}

		if (newOptionDefinition == end)
		{
			switch (optionDefinition->second.type)
			{
			case OptionDefinition::Type::Boolean:
				throw std::runtime_error("Trying to add args for OptionDefinition::Type::Boolean.");
				// Single token
			case OptionDefinition::Type::Token:
			case OptionDefinition::Type::Integer:
			case OptionDefinition::Type::Decimal:
			case OptionDefinition::Type::Date:
			case OptionDefinition::Type::Duration:
				currentOption.name = optionDefinition->second.name;
				currentOption.text = arg;
				optionDefinition = end;
				command.options[currentOption.name] = currentOption;
				break;
				// Multi token
			case OptionDefinition::Type::MultiToken:
				currentOption.name = optionDefinition->second.name;
				currentOption.text += currentOption.text.length() > 0 ? " " + arg : arg;
				break;
			default:
				throw std::runtime_error("Unknown option type: `" + std::to_string(static_cast<int>(optionDefinition->second.type)) + "`");
			}

			continue;
		}

		if (optionDefinition != end)
		{
			command.options[currentOption.name] = currentOption;
		}

		// special case for boolean
		if (newOptionDefinition->second.type == OptionDefinition::Type::Boolean)
		{
			command.options[newOptionDefinition->second.name].boolean = true;
			command.options[newOptionDefinition->second.name].name = newOptionDefinition->second.name;
		}
		else
		{
			optionDefinition = newOptionDefinition;
			currentOption = Option{};
			currentOption.name = newOptionDefinition->second.name;
		}
	}

	if (optionDefinition != end)
	{
		command.options[currentOption.name] = currentOption;
	}

	for (const auto & opt : definition.options)
	{
		if (opt.second.required && command.options.find(opt.first) == std::end(command.options))
		{
			throw std::runtime_error("Required option `" + opt.first + "` was not specified.");
		}
	}

	return command;
}

bool ETJump::CommandParser::parseOption(const std::string& arg, std::string& option)
{
	if (arg.length() <= 2)
	{
		return false;
	}

	if (arg[0] != '-' || arg[1] != '-')
	{
		return false;
	}

	option = arg.substr(2);
	return true;
}

