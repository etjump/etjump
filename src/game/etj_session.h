#pragma once
#include "etj_local.h"
#include <array>
#include "etj_client.h"

namespace ETJump
{
	class Session
	{
	public:
		Session();
		~Session();

		// if a non empty string is returned, client is dropped
		std::string clientConnect(int clientNum, bool firstTime, const std::string& ipAddr);

	private:
		std::array<Client, MAX_CLIENTS> _clients;
	};
}



