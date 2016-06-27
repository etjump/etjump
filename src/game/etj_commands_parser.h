#pragma once
#include <string>
#include <vector>
#include <map>

namespace ETJump
{
	class CommandsParser
	{
	public:
		// Command definition that is used for parsing
		struct OptionDefinition
		{
			enum class Type
			{
				// Just a single -option. If no option is specified in the args
				// it's set to false
				// e.g. "!create -random" 
				Boolean,
				// A single token 
				// e.g. Token type => "!create -type cute animals" => "cute"
				Token,
				// Multiple tokens. Parses until the next option
				// e.g. MultiToken type => "!create -type cute animals" => "cute animals"
				MultiToken,
				// A single token that has to fit to an integer
				Integer,
				// Only positive integers
				PositiveInteger,
				// Only negative integers
				NegativeInteger,
				// Duration e.g. 1s 3min 5h 7d 9w 11mon 13y
				Duration
			};
			OptionDefinition(std::string name, Type type, std::string description = ""): name(name), type(type), description(description) {};
			std::string name;
			Type type;
			std::string description;
		};

		struct Option
		{
			Option(): type(OptionDefinition::Type::Token), integer(0), duration(0), text(""), active(false) {}
			OptionDefinition::Type type;
			// int holds integer
			int integer;
			// long holds duration in ms
			long duration;
			// string holds token and multitoken>
			std::string text;
			bool active;
			// if there was an error while parsing the option, this will be updated
			std::string errorMessage;
		};

		struct ParsedCommand
		{
			std::string command;
			std::vector<std::string> arguments;
			std::map<std::string, Option> options;
		};

		struct ParseOptions
		{
			std::vector<OptionDefinition> optionDefinitions;
		};

		CommandsParser();
		~CommandsParser();

		static ParsedCommand parse(const std::vector<std::string>& arguments, const ParseOptions& options);
	};
}



