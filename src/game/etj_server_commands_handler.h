#pragma once
#include <string>
#include <functional>
#include <vector>
#include <map>

namespace ETJump
{
	class ServerCommandsHandler
	{
	public:
		static const int ConsoleClientNum = -1;

		ServerCommandsHandler();
		~ServerCommandsHandler();

		// subscribes to a command. 
		// returns false if command has been subscribed to already
		bool subscribe(std::string command, std::function<void(int, const std::vector<std::string>&)> callback);

		// unsubcribes from a command
		// does nothing if command is not recognized
		void unsubcribe(std::string command);

		// checks whether a callback has been registered for the command
		bool check(std::string command, int clientNum, const std::vector<std::string>& arguments) const;
	private:
		std::map<std::string, std::function<void(int, const std::vector<std::string>&)>> _callbacks;
	};
}



