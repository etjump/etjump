#pragma once
#include "etj_local.h"
#include <array>
#include "etj_client.h"

namespace ETJump
{
	class IUserRepository;

	class Session
	{
	public:
		Session(IUserRepository *userRepository);
		~Session();

		// if a non empty string is returned, client is dropped
		std::string clientConnect(int clientNum, bool firstTime, const std::string& ipAddr);

		// called whenever a command was received from the client
		void clientThink(int clientNum);
	private:
		IUserRepository *_userRepository;
		std::array<Client, MAX_CLIENTS> _clients;
	};
}



