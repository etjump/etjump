#pragma once

#include <array>
#include "etj_client.h"
#include "etj_isession_service.h"
#include <memory>
#include "etj_iuser_repository.h"

namespace ETJump
{
	class SessionService : public ISessionService
	{
	public:

		explicit SessionService(const std::shared_ptr<IUserRepository>& userRepository)
			: _userRepository(userRepository)
		{
		}

		void initializeClientSession(int clientNum) override;
		void authenticate(int clientNum, const std::string& name, const std::string& guid, const std::string& hardwareId, const std::string& ip) override;
		void readSession(int clientNum) override;
		void writeSession(int clientNum) override;
		void disconnect(int clientNum) override;
	private:
		std::shared_ptr<IUserRepository> _userRepository;
		static const int MAX_CLIENTS_ON_SERVER = 64;
		std::array<Client, MAX_CLIENTS_ON_SERVER> _clients;
	};
}
