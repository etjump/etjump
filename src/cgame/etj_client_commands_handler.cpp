#include "etj_client_commands_handler.h"
#include <boost/algorithm/string/case_conv.hpp>


ETJump::Client::ClientCommandsHandler::ClientCommandsHandler(void (*addToAutocompleteList)(const char *)):
	_addToAutocompleteList{addToAutocompleteList}
{
	_callbacks.clear();
}


ETJump::Client::ClientCommandsHandler::~ClientCommandsHandler()
{
}

bool ETJump::Client::ClientCommandsHandler::check(const std::string& command, const std::vector<std::string>& arguments)
{
	auto lowercaseCommand = boost::algorithm::to_lower_copy(command);
	auto match = _callbacks.find(lowercaseCommand);
	if (match != end(_callbacks))
	{
		match->second(arguments);
		return true;
	}
	return false;
}

bool ETJump::Client::ClientCommandsHandler::subscribe(const std::string& command, std::function<void(const std::vector<std::string>&)> callback, bool autocomplete)
{
	auto lowercaseCommand = boost::algorithm::to_lower_copy(command);
	if (_callbacks.find(lowercaseCommand) != end(_callbacks))
	{
		return false;
	}

	_callbacks[lowercaseCommand] = callback;
	if (_addToAutocompleteList != nullptr && autocomplete)
	{
		_addToAutocompleteList(command.c_str());
	}
	return true;
}

bool ETJump::Client::ClientCommandsHandler::unsubcribe(const std::string& command)
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
