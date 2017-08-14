#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include "etj_log.h"

namespace ETJump
{
	class SessionService;

	class AdminCommandsHandler
	{
	public:
		// Callback clientNum can be null if called from rcon
		typedef std::function<void(int clientNum, const std::string& command, const std::vector<std::string>& args)> Callback;
		struct Command
		{
			char permission;
			Callback callback;
		};

		explicit AdminCommandsHandler(std::shared_ptr<SessionService> sessionService);
		~AdminCommandsHandler();
		bool checkCommand(int clientNum, const std::vector<std::string>& args);
		bool subscribe(const std::string& command, char permission, Callback callback);
		bool unsubcribe(const std::string& command);
	private:
		std::shared_ptr<SessionService> _sessionService;
		std::map<std::string, Command> _callbacks;
		std::vector<std::string> _subscribedCommands;
		Log _log;
	};
}



