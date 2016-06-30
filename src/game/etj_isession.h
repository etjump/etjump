#pragma once
namespace ETJump
{
	class Client;

	class ISession
	{
	public:
		// returns the client pointer
		// returns nullptr if client is not connected
		virtual const Client *client(int clientNum) = 0;
	};
}