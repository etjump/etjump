#include "etj_commands_parser.h"
#include <algorithm>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>


ETJump::CommandsParser::CommandsParser()
{
}


ETJump::CommandsParser::~CommandsParser()
{
}

ETJump::CommandsParser::ParsedCommand ETJump::CommandsParser::parse(const Arguments& arguments, const ParseOptions& options)
{
	ParsedCommand parsedCommand;
	parsedCommand.command = arguments[0];
	if (options.optionDefinitions.size() == 0)
	{
		parsedCommand.arguments = std::vector<std::string>(begin(arguments) + 1, end(arguments));
		return parsedCommand;
	}

	initializeBooleanOptions(parsedCommand, options);

	parseArguments(Arguments(begin(arguments) + 1, end(arguments)), options, parsedCommand);

	return parsedCommand;
}

ETJump::CommandsParser::OptionDefinitions::const_iterator ETJump::CommandsParser::matchingDefinition(const ParseOptions& parsedOptions, const std::string& argument)
{
	return std::find_if(begin(parsedOptions.optionDefinitions), end(parsedOptions.optionDefinitions), [&argument](const OptionDefinition& definition)
	{
		return definition.name == argument;
	});
}

bool ETJump::CommandsParser::isSingleToken(const OptionDefinition& optionDefinition)
{
	return optionDefinition.type != OptionDefinition::Type::MultiToken;
}

void ETJump::CommandsParser::initializeBooleanOptions(ParsedCommand& parsedCommand, const ParseOptions& options)
{
	// initialize booleans with false
	for (const auto & def : options.optionDefinitions)
	{
		if (def.type == OptionDefinition::Type::Boolean)
		{
			parsedCommand.options[def.name].type = OptionDefinition::Type::Boolean;
			parsedCommand.options[def.name].active = false;
		}
	}
}

bool ETJump::CommandsParser::isPossibleOption(const std::string& argument)
{
	return argument[0] == '-';
}

void ETJump::CommandsParser::parseInteger(Option& option, const std::string& argument)
{
	try
	{
		option.integer = stoi(argument);
	}
	catch (const std::out_of_range&)
	{
		option.errorMessage = (boost::format("%s is out of range. Allowed values are between %d - %d.")
			% argument % std::numeric_limits<int>::min() % std::numeric_limits<int>::max()).str();
	}
	catch (const std::invalid_argument&)
	{
		option.errorMessage = (boost::format("%s is not an integer.") % argument).str();
	}
}

void ETJump::CommandsParser::parseDuration(Option& option, const std::string& argument)
{
	auto idx = 0;
	auto modifierIdx = -1;
	for (const auto& c : argument)
	{
		if (modifierIdx == -1 && (c < '0' || c > '9'))
		{
			modifierIdx = idx;
		}
		++idx;
	}

	std::string modifier;
	std::string coefficient;
	if (modifierIdx != -1)
	{
		modifier = argument.substr(modifierIdx);
	}
	try
	{
		option.duration = boost::lexical_cast<int64_t>(argument.substr(0, modifierIdx));
	}
	catch (const std::out_of_range&)
	{
		option.errorMessage = (boost::format("%s is out of range. Allowed values are between %d - %d.") % argument % std::numeric_limits<long>::min() % std::numeric_limits<long>::max()).str();
	}
	catch (const std::invalid_argument&)
	{
		option.errorMessage = (boost::format("%s is not an integer.") % argument).str();
	}

	if (modifier == "s")
	{
		option.duration *= second;
	}
	else if (modifier == "min")
	{
		option.duration *= minute;
	}
	else if (modifier == "h")
	{
		option.duration *= hour;
	}
	else if (modifier == "d")
	{
		option.duration *= day;
	}
	else if (modifier == "w")
	{
		option.duration *= week;
	}
	else if (modifier == "mon")
	{
		option.duration *= month;
	}
	else if (modifier == "y")
	{
		option.duration *= year;
	}
	else
	{
		option.errorMessage = (boost::format("Unknown duration modifier: %s.") % modifier).str();
	}

	if (option.duration < 0)
	{
		option.errorMessage = (boost::format("Specified duration %s exceeds maximum duration limit.") % argument).str();
	}
}

void ETJump::CommandsParser::parseArguments(const Arguments& arguments, const ParseOptions& options, ParsedCommand& parsedCommand)
{
	auto currentDefinition = end(options.optionDefinitions);
	auto newDefinition = end(options.optionDefinitions);
	Option option;

	for (const auto & argument : arguments)
	{
		if (isPossibleOption(argument))
		{
			// try to find a matching definition
			newDefinition = matchingDefinition(options, argument.substr(1));
			if (newDefinition != end(options.optionDefinitions))
			{
				// make sure definition always has a valid type
				if (newDefinition->type == OptionDefinition::Type::None)
					throw std::runtime_error((boost::format("No type was specified for option: %s.") % newDefinition->name).str());

				// If we're currently parsing an option, finish it and start a new one
				// unless it's a boolean
				if (currentDefinition != end(options.optionDefinitions))
				{
					parsedCommand.options[currentDefinition->name] = option;
				}

				currentDefinition = newDefinition;
				option = {};
				option.type = currentDefinition->type;
				// special case for boolean
				if (option.type == OptionDefinition::Type::Boolean)
				{
					option.active = true;
					parsedCommand.options[currentDefinition->name] = option;
					currentDefinition = end(options.optionDefinitions);
				}

				continue;
			}
			// no matching definition was found so parse it as a regular arg
		}

		// if we're not parsing an option, just append the arg to args
		if (currentDefinition == end(options.optionDefinitions))
		{
			parsedCommand.arguments.push_back(argument);
			continue;
		}

		auto singleToken = true;
		switch (currentDefinition->type)
		{
		case OptionDefinition::Type::None: 
			throw std::runtime_error("OptionDefinition::Type::None is not a valid definition type.");
		case OptionDefinition::Type::Boolean: 
			throw std::runtime_error("OptionDefinition::Type::Boolean should never be parsed here.");
		case OptionDefinition::Type::Token: 
			option.text = argument;
			break;
		case OptionDefinition::Type::MultiToken: 
			singleToken = false;
			if (option.text.length() > 0)
			{
				option.text += " ";
			}
			option.text += argument;
			break;
		case OptionDefinition::Type::Integer: 
			parseInteger(option, argument);
			break;
		case OptionDefinition::Type::Duration: 
			parseDuration(option, argument);
			break;
		}

		if (singleToken)
		{
			parsedCommand.options[currentDefinition->name] = option;
			currentDefinition = end(options.optionDefinitions);
		}
	}

	if (option.type == OptionDefinition::Type::MultiToken)
	{
		parsedCommand.options[currentDefinition->name] = option;
	}
}
