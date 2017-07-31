#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>

namespace ETJump
{
	namespace Server
	{
		class ClientCommandsHandler
		{
		public:
			explicit ClientCommandsHandler();
			~ClientCommandsHandler();

			typedef std::function<void(int clientNum, const std::string& command, const std::vector<std::string>&)> Callback;

			bool check(int clientNum, const std::string& command, const std::vector<std::string>& arguments);
			bool subscribe(const std::string& command, Callback callback, bool autocomplete = true);
			bool unsubcribe(const std::string& command);
		private:

			std::map<std::string, Callback> _callbacks;
		};
	}
}
