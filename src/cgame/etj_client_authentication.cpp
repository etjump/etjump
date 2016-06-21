#include <algorithm>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>

#include "etj_client_authentication.h"
#include "etj_client_commands_handler.h"
#include "cg_local.h"
#include "etj_platform_specific.h"

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
		CG_Printf("^2Successfully created a new GUID.");
		if (!saveGuid())
		{
			CG_Printf("^1Error: ^7Could not save guid to file.\n");
		}
		return;
	}
	CG_Printf("^2Successfully loaded existing GUID.");

	_hardwareId = PlatformSpecific::hardwareId();

	serverCommandsHandler->subscribe(Authentication::GUID_REQUEST, [=](const std::vector<std::string>& arguments)
	{
		authenticate();
	});
}

void ETJump::ClientAuthentication::authenticate()
{
	trap_SendClientCommand(va("etguid %s %s", G_SHA1(_guid.c_str()), _hardwareId.c_str()));
}

bool ETJump::ClientAuthentication::readGuid()
{
	fileHandle_t f = -1;

	auto len = trap_FS_FOpenFile(guidFile, &f, FS_READ);

	// don't bother reading anything if it's not exactly 40 characters
	if (len != GUID_LEN)
	{
		trap_FS_FCloseFile(f);
		return false;
	}

	char buffer[GUID_LEN + 1] = "";
	trap_FS_Read(buffer, len, f);
	buffer[GUID_LEN] = 0;
	trap_FS_FCloseFile(f);


	if (std::any_of(buffer, buffer + len, [](char c)
	{
		return c < '0' || c > 'F';
	}))
	{
		return false;
	}

	_guid = buffer;

	return true;
}

std::string ETJump::ClientAuthentication::createGuid() const
{
	boost::uuids::random_generator gen;

	std::string guid = G_SHA1(to_string(gen()).c_str());
	boost::to_upper(guid);

	return guid;
}

bool ETJump::ClientAuthentication::saveGuid()
{
	auto f = -1;

	if (trap_FS_FOpenFile(guidFile, &f, FS_WRITE) < 0)
	{
		return false;
	}

	trap_FS_Write(_guid.c_str(), _guid.length(), f);
	trap_FS_FCloseFile(f);
	return true;
}

