#pragma once
#include <string>
#include <vector>
#include <map>
#include <boost/variant.hpp>

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
				// Duration e.g. 1s 3min 5h 7d 9w 11mon 13y
				Duration
			};
			std::string description;
			std::string name;
			Type type;
		};

		struct Option
		{
			OptionDefinition::Type type;
			// int holds integer
			// long holds duration in ms
			// string holds token and multitoken>
			boost::variant<bool, int, long, std::string> value;
		};

		struct ParsedCommand
		{
			std::string command;
			std::vector<std::string> arguments;
			std::map<std::string, Option> options;
		};

		struct ParseOptions
		{
			
		};

		CommandsParser();
		~CommandsParser();

		static ParsedCommand parse(const std::vector<std::string>& arguments, const ParseOptions& options);
	};
}



