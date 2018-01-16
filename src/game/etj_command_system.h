#pragma once
#include "etj_command_parser.h"
#include <functional>

namespace ETJump
{
	class CommandSystem
	{
	public:
		CommandSystem();
		~CommandSystem();

		int subscribe(const std::string& command, const CommandParser::CommandDefinition& commandDefinition, std::function<void(const CommandParser::Command&)> callback);
		void unsubscribe(int handle);
	private:
		CommandParser _parser;
	};
}



