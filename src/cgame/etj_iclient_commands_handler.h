#pragma once
#include <string>
#include <functional>
#include <vector>

namespace ETJump
{
	class IClientCommandsHandler
	{
	public:
		virtual ~IClientCommandsHandler()
		{
		}

		// returns true if a match was found and function was called
		virtual bool check(const std::string& command, const std::vector<std::string>& arguments) = 0;

		// registers a command handler that will be called if the command was received from the server
		// returns false if handler with the same name already exists
		virtual bool subscribe(const std::string& command, std::function<void(const std::vector<std::string>&)> callback, bool autocomplete = true) = 0;

		// unsubscribes the command handler
		// returns false if it does not exist
		virtual bool unsubcribe(const std::string& command) = 0;
	};
}
