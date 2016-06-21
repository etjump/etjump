#include "etj_server_commands_handler.h"
#include <algorithm>
#include <boost/algorithm/string/case_conv.hpp>

ETJump::ServerCommandsHandler::ServerCommandsHandler()
{
	_callbacks.clear();
}


ETJump::ServerCommandsHandler::~ServerCommandsHandler()
{
}

bool ETJump::ServerCommandsHandler::subscribe(std::string command, std::function<void(int, const std::vector<std::string>&)> callback)
{
	boost::algorithm::to_lower(command);
	if (_callbacks.find(command) != end(_callbacks))
	{
		return false;
	}

	_callbacks[command] = callback;
	return true;
}

void ETJump::ServerCommandsHandler::unsubcribe(std::string command)
{
	boost::algorithm::to_lower(command);
	auto callback = _callbacks.find(command);
	if (callback == end(_callbacks))
	{
		return;
	}

	_callbacks.erase(callback);
}

bool ETJump::ServerCommandsHandler::check(std::string command, int clientNum, const std::vector<std::string>& arguments) const
{
	boost::algorithm::to_lower(command);
	auto callback = _callbacks.find(command);
	if (callback == end(_callbacks))
	{
		return false;
	}

	callback->second(clientNum, arguments);
	return true;
}
