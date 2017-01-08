#pragma once

#include <string>
#include <map>
#include <vector>

namespace ETJump
{
	class CommandParser
	{
	public:
		struct Option
		{
			enum class Type
			{
				Boolean,
				Text,
				Integer,
				Decimal,
				Date,
				Duration
			};

			std::string name;
			Type type;
			bool required;
		};

		struct Command
		{
			std::string name;
			std::string description;
			std::map<std::string, Option> options;
		};

		struct ParseResult
		{
			std::vector<std::string> errors;
			std::map<std::string, Option> options;
			std::vector<std::string> extraArgs;
		};

		CommandParser();
		~CommandParser();
	};
}



