#pragma once
#include <string>
#include "etj_user.h"
#include "etj_iuser_repository.h"

namespace ETJump
{
	class Client
	{
	public:
		static const auto INVALID_SLOT = -1;
		static const auto NOT_AUTHORIZED = 0;
		explicit Client(int clientNum, bool connected, const std::string& ipAddress);
		Client();
		~Client();
		Client(const Client& client);
		Client& operator=(const Client& client);

		// sends a guid request to client
		void requestGuid() const;
		// Parses the guid & hardware id response to retrieve
		// guid & hardware id for the client
		bool authenticate(const std::vector<std::string>& arguments);
		// drops the client from the server
		void drop(const std::string& message) const;
		// authorizes the client => either fetches an existing user 
		// or creates a new one.
		// Note: the operation is asyncronous and the result will be
		// parsed later unless the user repository has it cached and
		// it's available instantly
		void authorize(IUserRepository* userRepository);
		// checks if there are any pending results and parses them
		// if there are some
		void checkPendingAuthorization();

		int slot() const
		{
			return _slot;
		}

		bool connected() const
		{
			return _connected;
		}

		std::string ipAddress() const
		{
			return _ipAddress;
		}

		std::string guid() const
		{
			return _guid;
		}

		std::string hardwareId() const
		{
			return _hardwareId;
		}

		const User* user() const
		{
			return &_user;
		}

	private:
		int _slot;
		bool _connected;
		std::string _ipAddress;
		std::string _guid;
		std::string _hardwareId;
		// Note: this is a local copy and changing this will not
		// change the database copy
		User _user;
		bool _authorizationIsPending;
		std::future<IUserRepository::Result> _pendingAuthorization;
	};
}



