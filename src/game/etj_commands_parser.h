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
				// default value, throws if this is set on parse
				None,
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
				// Duration e.g. 3min 5h 7d 9w 11mon 13y
				Duration
			};
			OptionDefinition(std::string name, Type type, std::string description = ""): name(name), type(type), description(description) {};
			std::string name;
			Type type;
			std::string description;
		};

		struct Option
		{
			Option(): type(OptionDefinition::Type::None), integer(0), duration(0), text(""), active(false) {}
			OptionDefinition::Type type;
			// int holds integer
			int integer;
			// int64 holds duration in ms
			int64_t duration;
			// string holds token and multitoken>
			std::string text;
			bool active;
			// if there was an error while parsing the option, this will be updated
			std::string errorMessage;
		};

		typedef std::vector<std::string> Arguments;
		typedef std::map<std::string, Option> ParsedCommandOptions;
		struct ParsedCommand
		{
			std::string command;
			Arguments arguments;
			ParsedCommandOptions options;
		};

		typedef std::vector<OptionDefinition> OptionDefinitions;
		struct ParseOptions
		{
			OptionDefinitions optionDefinitions;
		};

		CommandsParser();
		~CommandsParser();

		static ParsedCommand parse(const Arguments& arguments, const ParseOptions& options);

	private:
		// finds a matching option definition from the options object
		static OptionDefinitions::const_iterator matchingDefinition(const ParseOptions& options, const std::string& argument);

		// is option a single token option or multitoken
		static bool isSingleToken(const OptionDefinition& optionDefinition);
	};
}



