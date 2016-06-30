#include <algorithm>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>
#include "etj_client_authentication.h"
#include "etj_client_commands_handler.h"
#include "cg_local.h"
#include "etj_platform_specific.h"
#include "../game/etj_file.h"
#include <boost/format.hpp>

const char* ETJump::ClientAuthentication::guidFile = "etguid.dat";

ETJump::ClientAuthentication::ClientAuthentication()
{
	initialize();
}


ETJump::ClientAuthentication::~ClientAuthentication()
{
	serverCommandsHandler->unsubcribe(Authentication::GUID_REQUEST);
}

void ETJump::ClientAuthentication::initialize()
{
	if (!readGuid())
	{
		_guid = createGuid();
		CG_Printf("^2Successfully created a new GUID.\n");
		if (!saveGuid())
		{
			CG_Printf("^1Error: ^7Could not save guid to file.\n");
		}
	} else
	{
		CG_Printf("^2Successfully loaded existing GUID.\n");
	}

	_hardwareId = PlatformSpecific::hardwareId();

	serverCommandsHandler->subscribe(Authentication::GUID_REQUEST, [&](const std::vector<std::string>& arguments)
	{
		authenticate();
	});
}

void ETJump::ClientAuthentication::authenticate() const
{
	trap_SendClientCommand(va("etguid %s %s", G_SHA1(_guid.c_str()), _hardwareId.c_str()));
}

bool ETJump::ClientAuthentication::readGuid()
{
	try
	{
		File file(guidFile);
		auto guid = file.read();
		if (std::any_of(begin(guid), end(guid), [](char c)
		{
			return c < '0' || c > 'F';
		}))
		{
			return false;
		}
		_guid = std::string(guid.data(), guid.size());
		return true;
	} catch (const File::FileNotFoundException&)
	{
		return false;
	}
}

std::string ETJump::ClientAuthentication::createGuid() const
{
	boost::uuids::random_generator gen;

	std::string guid = G_SHA1(to_string(gen()).c_str());
	boost::to_upper(guid);

	return guid;
}

bool ETJump::ClientAuthentication::saveGuid() const
{
	try
	{
		File file(guidFile, File::Mode::Write);
		file.write(_guid);
		return true;
	} catch (const File::WriteFailedException& exception)
	{
		CG_Printf((boost::format("%s\n") % exception.what()).str().c_str());
		return false;
	}
}

