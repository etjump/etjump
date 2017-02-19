#include "etj_session_service.h"

void ETJump::SessionService::initializeClientSession(int clientNum)
{
	_clients[clientNum] = Client();
}

void ETJump::SessionService::authenticate(int clientNum, const std::string& name, const std::string& guid, const std::string& hardwareId, const std::string& ip)
{
	_userRepository->createOrUpdateAsync(name, guid, hardwareId, ip, [=](const std::shared_ptr<IUserRepository::TaskResult<User>> result)
	{
		if (result->error.length() == 0)
		{
			_clients[clientNum] = Client(result->result);
		}
	});
}

void ETJump::SessionService::readSession(int clientNum)
{
}

void ETJump::SessionService::writeSession(int clientNum)
{
}

void ETJump::SessionService::disconnect(int clientNum)
{
	_clients[clientNum] = Client();
}
