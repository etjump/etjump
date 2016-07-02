#pragma once
#include "etj_local.h"
#include <array>
#include "etj_client.h"
#include "etj_isession.h"

namespace ETJump
{
	class IUserRepository;
	class ServerCommandsHandler;

	class Session : public ISession
	{
	public:
		explicit Session(IUserRepository *userRepository, ServerCommandsHandler *commandsHandler, EventAggregator* eventAggregator);
		~Session();

		// if a non empty string is returned, client is dropped
		std::string clientConnect(int clientNum, bool firstTime, const std::string& ipAddr);

		// called whenever a command was received from the client
		void clientThink(int clientNum);

		// returns nullptr if not connected
		const Client* client(int clientNum) override;

	private:
		IUserRepository *_userRepository;
		std::array<Client, MAX_CLIENTS> _clients;
		ServerCommandsHandler* _commandsHandler;
		EventAggregator* _eventAggregator;
	};
}



