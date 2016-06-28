#pragma once
#include <string>
#include "etj_user.h"

namespace ETJump
{
	class Client
	{
	public:
		explicit Client(int clientNum, bool connected, const std::string& ipAddress);
		Client();
		~Client();

		// sends a guid request to client
		void requestGuid();
		// Parses the guid & hardware id response to retrieve
		// guid & hardware id for the client
		bool parseIdsResponse(const std::vector<std::string>& arguments);
		// drops the client from the server
		void drop(const std::string& message);

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
			return _user;
		}
	private:
		int _slot;
		bool _connected;
		std::string _ipAddress;
		std::string _guid;
		std::string _hardwareId;
		const User *_user;
	};
}



