#include "etj_client_commands_handler.h"
#include <boost/algorithm/string/case_conv.hpp>

ETJump::Server::ClientCommandsHandler::ClientCommandsHandler()
{
}

ETJump::Server::ClientCommandsHandler::~ClientCommandsHandler()
{
}

bool ETJump::Server::ClientCommandsHandler::check(int clientNum, const std::string& command, const std::vector<std::string>& arguments)
{
	auto lowercaseCommand = boost::algorithm::to_lower_copy(command);
	auto match = _callbacks.find(lowercaseCommand);
	if (match != end(_callbacks))
	{
		match->second(clientNum, command, arguments);
		return true;
	}
	return false;
}

bool ETJump::Server::ClientCommandsHandler::subscribe(const std::string& command, Callback callback, bool autocomplete)
{
	auto lowercaseCommand = boost::algorithm::to_lower_copy(command);
	if (_callbacks.find(lowercaseCommand) != end(_callbacks))
	{
		return false;
	}

	_callbacks[lowercaseCommand] = callback;
	return true;
}

bool ETJump::Server::ClientCommandsHandler::unsubcribe(const std::string& command)
{
	auto lowercaseCommand = boost::algorithm::to_lower_copy(command);
	auto callback = _callbacks.find(lowercaseCommand);
	if (callback != end(_callbacks))
	{
		return false;
	}

	_callbacks.erase(callback);
	return true;
}
