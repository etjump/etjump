#include "etj_command_parser.h"
#include <boost/algorithm/string/case_conv.hpp>
#include "etj_time_utilities.h"


std::string ETJump::CommandParser::toString(OptionDefinition::Type type)
{
	switch (type)
	{
	case OptionDefinition::Type::Boolean: return "boolean";
	case OptionDefinition::Type::Token: return "token";
	case OptionDefinition::Type::MultiToken: return "multi token";
	case OptionDefinition::Type::Integer: return "integer";
	case OptionDefinition::Type::Decimal: return "decimal";
	case OptionDefinition::Type::Date: return "date";
	case OptionDefinition::Type::Duration: return "duration";
	default: return "unknown type";
	}
}

ETJump::CommandParser::CommandParser()
{
}

ETJump::CommandParser::~CommandParser()
{
}

ETJump::CommandParser::Command ETJump::CommandParser::parse(CommandDefinition definition, const std::vector<std::string>& args)
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
			case OptionDefinition::Type::Integer:
				try
				{
					currentOption.name = optionDefinition->second.name;
					currentOption.integer = std::stoi(arg);
					optionDefinition = end;
					command.options[currentOption.name] = currentOption;
				} catch (const std::exception&)
				{
					command.errors.push_back("Option `" + optionDefinition->second.name + "` is of type " + toString(optionDefinition->second.type) + ". Cannot convert `" + arg + "` to the specified type.");
				}
				break;
			case OptionDefinition::Type::Decimal:
				try
				{
					currentOption.name = optionDefinition->second.name;
					currentOption.decimal = std::stod(arg);
					optionDefinition = end;
					command.options[currentOption.name] = currentOption;
				}
				catch (const std::exception&)
				{
					command.errors.push_back("Option `" + optionDefinition->second.name + "` is of type " + toString(optionDefinition->second.type) + ". Cannot convert `" + arg + "` to the specified type.");
				}
				break;
			case OptionDefinition::Type::Date:
			case OptionDefinition::Type::Duration:
				currentOption.name = optionDefinition->second.name;
				currentOption.duration = ETJump::Duration::parseDuration(arg);
				optionDefinition = end;
				command.options[currentOption.name] = currentOption;
				break;
			case OptionDefinition::Type::Token:
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
				command.errors.push_back("Unknown option type: `" + std::to_string(static_cast<int>(optionDefinition->second.type)) + "`");
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
			command.errors.push_back("Required option `" + opt.first + "` was not specified.");
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

