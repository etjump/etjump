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

		static constexpr const int64_t second = 1000;
		static constexpr const int64_t minute = 60 * second;
		static constexpr const int64_t hour = 60 * minute;
		static constexpr const int64_t day = 24 * hour;
		static constexpr const int64_t week = 7 * day;
		static constexpr const int64_t month = 30 * day;
		static constexpr const int64_t year = 365 * day;

		typedef std::vector<std::string> Arguments;
		typedef std::map<std::string, Option> ParsedCommandOptions;
		struct ParsedCommand
		{
			// name of the command
			std::string command;
			// list of arguments
			Arguments arguments;
			// parsed options
			ParsedCommandOptions options;
		};

		typedef std::vector<OptionDefinition> OptionDefinitions;
		struct ParseOptions
		{
			// command description
			std::string description;
			// command options
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
	
		// initializes the bool option flags
		static void initializeBooleanOptions(ParsedCommand& parsedCommand, const ParseOptions& options);

		// checks whether argument is possibly a new option
		static bool isPossibleOption(const std::string& argument);

		// parses an integer from the argument and updates the option
		static void parseInteger(Option& option, const std::string& argument);

		// parses a duration from the argument and updates the option
		static void parseDuration(Option& option, const std::string& argument);

		// parses the arguments and updates the parsed command
		static void parseArguments(const Arguments& arguments, const ParseOptions& options, ParsedCommand& parsedCommand);
	};
}



