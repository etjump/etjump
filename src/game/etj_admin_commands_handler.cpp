#include "etj_admin_commands_handler.h"
#include "etj_printer.h"
#include "etj_session_service.h"
#include <boost/algorithm/string/case_conv.hpp>
#include "etj_string_utilities.h"
#include <boost/algorithm/string.hpp>
#include "etj_result_set_formatter.h"


ETJump::AdminCommandsHandler::AdminCommandsHandler(std::shared_ptr<SessionService> sessionService): _sessionService(sessionService), _log(Log("AdminCommandsHandler"))
{
}

ETJump::AdminCommandsHandler::~AdminCommandsHandler()
{
}

bool ETJump::AdminCommandsHandler::checkCommand(int clientNum, const std::vector<std::string>& args)
{
	int start = 0;
	if (args[start] == "say" || args[start] == "enc_say")
	{
		start = 1;
	} else
	{
		if (!_sessionService->hasPermission(clientNum, '/'))
		{
			Printer::sendConsoleMessage(clientNum, "^3Unauthorized: ^7you are not authorized to use silent commands.");
		}
	}

	auto command = args[start].substr(1);
	if (args[start][0] != '!' || command.length() == 0)
	{
		return false;
	}

	std::vector<std::string> additionalArgs;
	std::copy(begin(args) + 1, end(args), std::back_inserter(additionalArgs));

	boost::to_lower(command);

	auto iter = _callbacks.lower_bound(command);
	std::vector<std::map<std::string, Command>::iterator> matchingCommands;
	for (; iter != end(_callbacks) && iter->first.compare(0, command.length(), command) == 0; ++iter)
	{
		if (_sessionService->hasPermission(clientNum, iter->second.permission))
		{
			if (iter->first == command)
			{
				matchingCommands.clear();
				matchingCommands.push_back(iter);
				break;
			}
			matchingCommands.push_back(iter);
		}
	}

	if (matchingCommands.size() == 0)
	{
		auto match = ETJump::getBestMatch(_subscribedCommands, command);
		Printer::sendConsoleMessage(clientNum, "^3Unknown command: ^7could not find command " + command + ". Did you mean " + match + "?\n");
		return true;
	}

	if (matchingCommands.size() == 1)
	{
		auto name = _sessionService->getName(clientNum);
		_log.infoLn(name + " executed command !" + command + " " + boost::join(additionalArgs, " "));
		matchingCommands[0]->second.callback(clientNum, command, additionalArgs);
		return true;
	}

	if (matchingCommands.size() > 1)
	{
		Utilities::ResultSetFormatter rsf;
		std::vector<std::string> headers{ "Command" };
		std::vector<std::map<std::string, std::string>> rows;
		for (const auto & c : matchingCommands)
		{
			rows.push_back({ {"Command", c->first} });
		}
		Printer::sendConsoleMessage(clientNum, "^3Multiple matching commands^7\n" + rsf.toString(headers, rows, matchingCommands.size(), 0));
		return true;
	}

	return false;
}

bool ETJump::AdminCommandsHandler::subscribe(const std::string& command, char permission, Callback callback)
{
	auto lowercaseCommand = boost::algorithm::to_lower_copy(command);
	if (_callbacks.find(lowercaseCommand) != end(_callbacks))
	{
		return false;
	}

	_callbacks[lowercaseCommand] = { permission, callback };
	_subscribedCommands.push_back(lowercaseCommand);
	return true;
}

bool ETJump::AdminCommandsHandler::unsubcribe(const std::string& command)
{
	auto lowercaseCommand = boost::algorithm::to_lower_copy(command);
	auto callback = _callbacks.find(lowercaseCommand);
	if (callback != end(_callbacks))
	{
		return false;
	}

	_callbacks.erase(callback);
	_subscribedCommands.erase(std::remove(begin(_subscribedCommands), end(_subscribedCommands), command), end(_subscribedCommands));
	return true;
}