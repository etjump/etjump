#include "etj_server_commands_handler.h"



ETJump::ServerCommandsHandler::ServerCommandsHandler()
{
	_callbacks.clear();
}


ETJump::ServerCommandsHandler::~ServerCommandsHandler()
{
}

bool ETJump::ServerCommandsHandler::check(const std::string& command, const std::vector<std::string>& arguments)
{
	auto match = _callbacks.find(command);
	if (match != end(_callbacks))
	{
		match->second(arguments);
		return true;
	}
	return false;
}

bool ETJump::ServerCommandsHandler::subscribe(const std::string& command, std::function<void(const std::vector<std::string>&)> callback)
{
	if (_callbacks.find(command) != end(_callbacks))
	{
		return false;
	}

	_callbacks[command] = callback;
	return true;
}

bool ETJump::ServerCommandsHandler::unsubcribe(const std::string& command)
{
	auto callback = _callbacks.find(command);
	if (callback != end(_callbacks))
	{
		return false;
	}

	_callbacks.erase(callback);
	return true;
}
