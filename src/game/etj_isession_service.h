#pragma once
#include <string>

namespace ETJump
{
	class ISessionService
	{
	public:
		virtual ~ISessionService()
		{
		}

		virtual void initializeClientSession(int clientNum) = 0;
		virtual void authenticate(int clientNum, const std::string& name, const std::string& guid, const std::string& hardwareId, const std::string& ip) = 0;
		virtual void readSession(int clientNum) = 0;
		virtual void writeSession(int clientNum) = 0;
		virtual void disconnect(int clientNum) = 0;
	};
}
