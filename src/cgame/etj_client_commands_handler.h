#pragma once
#include <string>
#include <functional>
#include <map>
#include <vector>
#include <memory>
#include "etj_iclient_commands_handler.h"

namespace ETJump
{
	class ClientCommandsHandler : public IClientCommandsHandler
	{
	public:
		explicit ClientCommandsHandler(void (const char *));
		~ClientCommandsHandler();

		// returns true if a match was found and function was called
		bool check(const std::string& command, const std::vector<std::string>& arguments) override;

		// registers a command handler that will be called if the command was received from the server
		// returns false if handler with the same name already exists
		bool subscribe(const std::string& command, std::function<void(const std::vector<std::string>&)> callback, bool autocomplete = true) override;

		// unsubscribes the command handler
		// returns false if it does not exist
		bool unsubcribe(const std::string& command) override;
	private:
		void(*_addToAutocompleteList)(const char *command);

		std::map<std::string, std::function<void(std::vector<std::string>)>> _callbacks;
	};
}



