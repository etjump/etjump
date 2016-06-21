#include "etj_commands_handler.h"



ETJump::CommandsHandler::CommandsHandler()
{
	_callbacks.clear();
}


ETJump::CommandsHandler::~CommandsHandler()
{
}

bool ETJump::CommandsHandler::check(const std::string& command, const std::vector<std::string>& arguments)
{
	auto match = _callbacks.find(command);
	if (match != end(_callbacks))
	{
		match->second(arguments);
		return true;
	}
	return false;
}

bool ETJump::CommandsHandler::subscribe(const std::string& command, std::function<void(const std::vector<std::string>&)> callback)
{
	if (_callbacks.find(command) != end(_callbacks))
	{
		return false;
	}

	_callbacks[command] = callback;
	return true;
}

bool ETJump::CommandsHandler::unsubcribe(const std::string& command)
{
	auto callback = _callbacks.find(command);
	if (callback != end(_callbacks))
	{
		return false;
	}

	_callbacks.erase(callback);
	return true;
}
