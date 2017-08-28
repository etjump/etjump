#pragma once

#include <string>
#include <map>
#include <vector>
#include <ostream>

namespace ETJump
{
	class CommandParser
	{
	public:
		struct OptionDefinition
		{
			enum class Type
			{
				Boolean,
				Token,
				MultiToken,
				Integer,
				Decimal,
				//Date,
				Duration
			};

			std::string name;
			std::string description;
			Type type;
			bool required;
		};

		struct CommandDefinition
		{
			std::string name;
			std::string description;
			std::map<std::string, OptionDefinition> options;
			// only single token options
			std::vector<OptionDefinition> positionalOptions;
		};

		struct Option
		{
			Option(): name(""), boolean(false), text(""), integer(0), decimal(0), date(0), duration(0) {}
			std::string name;
			bool boolean;
			std::string text;
			int integer;
			double decimal;
			long long date;
			long long duration;

			std::string toString() const
			{
				return "name: " + name +
					" boolean: " + std::to_string(boolean) +
					" text: " + text +
					" integer: " + std::to_string(integer) +
					" decimal: " + std::to_string(decimal) +
					" date: " + std::to_string(date) +
					" duration: " + std::to_string(duration);
			}
			friend std::ostream& operator<<(std::ostream& os, const Option& obj)
			{
				return os
					<< "name: " << obj.name
					<< " boolean: " << obj.boolean
					<< " text: " << obj.text
					<< " integer: " << obj.integer
					<< " decimal: " << obj.decimal
					<< " date: " << obj.date
					<< " duration: " << obj.duration;
			}
		};

		struct Command
		{
			std::vector<std::string> errors;
			std::map<std::string, Option> options;
			std::vector<std::string> extraArgs;
		};

		static std::string toString(OptionDefinition::Type type);

		CommandParser();
		~CommandParser();

		Command parse(CommandDefinition definition, const std::vector<std::string>& args);

	private:
		/**
		 * Tries to parse an option. Returns true if successful and updates option
		 * Returns false if failed
		 * @param arg 
		 * @param option 
		 * @returns boolean
		 */
		bool parseOption(const std::string& arg, std::string& option);

	};
}



