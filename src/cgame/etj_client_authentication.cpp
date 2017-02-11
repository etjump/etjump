#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "etj_client_authentication.h"
#include "../game/etj_file.h"
#include "../game/etj_shared.h"

ETJump::ClientAuthentication::ClientAuthentication(
	std::function<void(const std::string&)> sendClientCommand, 
	std::function<void(const std::string&)> print,
	std::function<std::string()> getHwid,
	std::shared_ptr<ClientCommandsHandler> serverCommandsHandler
		)
	: _sendClientCommand(sendClientCommand), _print(print), _getHwid(getHwid), _serverCommandsHandler(serverCommandsHandler), GUID_FILE("etguid.dat")
{
	_serverCommandsHandler->subscribe(Constants::Authentication::GUID_REQUEST, [&](const std::vector<std::string>& args)
	{
		login();
	});
}

ETJump::ClientAuthentication::~ClientAuthentication()
{
	_serverCommandsHandler->unsubcribe(Constants::Authentication::GUID_REQUEST);
}

void ETJump::ClientAuthentication::login()
{
	auto guid = getGuid();
	auto result = saveGuid(guid);
	if (!result.success)
	{
		_print("^1Error: " + result.message);
	}
	auto hwid = getHwid();
	auto authenticate = (boost::format("%s %s %s") % Constants::Authentication::AUTHENTICATE % guid % hwid).str();
	_sendClientCommand(authenticate);
}

std::string ETJump::ClientAuthentication::getGuid()
{
	try
	{
		File guidFile(GUID_FILE);
		auto guid = guidFile.read();
		return std::string(begin(guid), end(guid));
	} catch (File::FileNotFoundException)
	{
		return createGuid();
	}
}

std::string ETJump::ClientAuthentication::getHwid()
{
	return _getHwid();
}

std::string ETJump::ClientAuthentication::createGuid() const
{
	boost::uuids::random_generator gen;
	auto u = gen();
	return boost::to_upper_copy(boost::lexical_cast<std::string>(u));
}

ETJump::ClientAuthentication::OperationResult ETJump::ClientAuthentication::saveGuid(const std::string& guid) const
{
	File guidFile(GUID_FILE, File::Mode::Write);
	try
	{
		guidFile.write(guid);
		return{ true, "" };
	} catch (File::WriteFailedException wfe)
	{
		return{ false, std::string("saving guid failed: ") + wfe.what() + " Using temporary guid." };
	}
}
