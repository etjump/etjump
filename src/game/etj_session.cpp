#include "etj_session.h"
#include "etj_server_commands_handler.h"
#include <boost/algorithm/string.hpp>
#include "etj_printer.h"

ETJump::Session::Session(IUserRepository* userRepository, ServerCommandsHandler *commandsHandler): _userRepository(userRepository), _commandsHandler(commandsHandler)
{
	if (!_commandsHandler->subscribe("etguid", [&](int clientNum, const std::vector<std::string>& arguments)
	{
		if (!_clients[clientNum].authenticate(arguments))
		{
			Printer::LogPrintln((boost::format("Possible GUID or Hardware ID spoof attempt from IP address: %s.") % _clients[clientNum].ipAddress()).str());
			_clients[clientNum].drop("Possible GUID or Hardware ID spoof attempt.");
			_clients[clientNum] = Client();
			return;
		}
		_clients[clientNum].authorize(_userRepository);
	}))
	{
		throw std::runtime_error("Tried to subscribe to `etguid` event twice. Contact mod developer.");
	}
}

ETJump::Session::~Session()
{
	_commandsHandler->unsubcribe("etguid");
}

std::string ETJump::Session::clientConnect(int clientNum, bool firstTime, const std::string& ipAddress)
{
	std::vector<std::string> split;
	boost::split(split, ipAddress, boost::is_any_of(":"));

	
	auto client = Client(clientNum, true, split[0]);
	_clients[clientNum] = client;

	client.requestGuid();

	return "";
}

void ETJump::Session::clientThink(int clientNum)
{
	_clients[clientNum].checkPendingAuthorization();
}

const ETJump::Client* ETJump::Session::client(int clientNum)
{
	if (!_clients[clientNum].connected())
	{
		return nullptr;
	}
	return &_clients[clientNum];
}
