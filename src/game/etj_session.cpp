#include "etj_session.h"
#include "etj_server_commands_handler.h"
#include <boost/algorithm/string.hpp>
#include "etj_printer.h"

ETJump::Session::Session()
{
	if (!commandsHandler->subscribe("etguid", [&](int clientNum, const std::vector<std::string>& arguments)
	{
		if (!_clients[clientNum].parseIdsResponse(arguments))
		{
			Printer::LogPrintln((boost::format("Possible GUID or Hardware ID spoof attempt from IP address: %s.") % _clients[clientNum].ipAddress()).str());
			_clients[clientNum].drop("Possible GUID or Hardware ID spoof attempt.");
			_clients[clientNum] = Client();
		}
	}))
	{
		throw std::runtime_error("Tried to subscribe to `etguid` event twice. Contact mod developer.");
	}
}


ETJump::Session::~Session()
{
	commandsHandler->unsubcribe("etguid");
}

std::string ETJump::Session::clientConnect(int clientNum, bool firstTime, const std::string& ipAddress)
{
	std::vector<std::string> split;
	boost::split(split, ipAddress, boost::is_any_of(":"));

	auto client = _clients[clientNum] = Client(clientNum, true, split[0]);

	client.requestGuid();

	return "";
}

