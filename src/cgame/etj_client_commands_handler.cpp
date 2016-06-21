#include "etj_client_commands_handler.h"



ETJump::ClientCommandsHandler::ClientCommandsHandler()
{
	_callbacks.clear();
}


ETJump::ClientCommandsHandler::~ClientCommandsHandler()
{
}

bool ETJump::ClientCommandsHandler::check(const std::string& command, const std::vector<std::string>& arguments)
{
	auto match = _callbacks.find(command);
	if (match != end(_callbacks))
	{
		match->second(arguments);
		return true;
	}
	return false;
}

bool ETJump::ClientCommandsHandler::subscribe(const std::string& command, std::function<void(const std::vector<std::string>&)> callback)
{
	if (_callbacks.find(command) != end(_callbacks))
	{
		return false;
	}

	_callbacks[command] = callback;
	return true;
}

bool ETJump::ClientCommandsHandler::unsubcribe(const std::string& command)
{
	auto callback = _callbacks.find(command);
	if (callback != end(_callbacks))
	{
		return false;
	}

	_callbacks.erase(callback);
	return true;
}
