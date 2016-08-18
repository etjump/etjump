#include <boost/algorithm/string.hpp>
#include "etj_client_print_utilities.h"
#include "etj_client_commands_handler.h"
#include "etj_trickjump_lines.h"
#include "etj_client.h"
#include "../game/etj_file.h"

ETJump::TrickjumpLines::TrickjumpLines(ClientCommandsHandler* clientCommandsHandler, ClientCommandsHandler* serverCommandsHandler)
	:_clientCommandsHandler(clientCommandsHandler), _serverCommandsHandler(serverCommandsHandler)
{
	if (!_clientCommandsHandler)
	{
		Client::error("TrickjumpLines::clientCommandHandler is null.");
		return;
	}

	if (!_serverCommandsHandler)
	{
		Client::error("TrickjumpLines::serverCommandsHandler is null.");
		return;
	}

	subscribeToClientCommands();
	subscribeToServerCommands();
}

ETJump::TrickjumpLines::~TrickjumpLines()
{
	unsubscribeFromClientCommands();
	unsubscribeFromClientCommands();
}

void ETJump::TrickjumpLines::subscribeToClientCommands()
{
	_clientCommandsHandler->subscribe("listNamespaces", [&](const std::vector<std::string>& args)
	{
		listFiles();
	});

	_clientCommandsHandler->subscribe("listRoutes", [&](const std::vector<std::string>& args)
	{

	});
}

void ETJump::TrickjumpLines::unsubscribeFromClientCommands()
{
}

void ETJump::TrickjumpLines::subscribeToServerCommands()
{
}

void ETJump::TrickjumpLines::unsubscribeFromServerCommands()
{
}

void ETJump::TrickjumpLines::listFiles()
{
	std::vector<std::string> matches;
	try
	{
		matches = File::localFileList("tjlines", ".json");
	}
	catch (File::PathNotFoundException ex)
	{
		ClientPrintUtilities::printToConsole("^3Note: ^7There are currently no trickjump line definition files in etjump/tjlines directory.");
		return;
	}
	catch (std::runtime_error ex)
	{
		ClientPrintUtilities::printToConsole("^1Error: ^7Unknown error occurred while trying to list trickjump lines definition files.");
		return;
	}

	ClientPrintUtilities::printToConsole(boost::join(matches, ", "));

}
