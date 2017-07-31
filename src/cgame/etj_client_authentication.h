#pragma once
#include <string>
#include <functional>
#include "etj_client_commands_handler.h"
#include "../game/etj_operation_result.h"

namespace ETJump
{
	class ClientAuthentication
	{
	public:
		static const int GUID_LEN = 40;

		explicit ClientAuthentication(
			std::function<void(const std::string&)> sendClientCommand, 
			std::function<void(const std::string&)> print, 
			std::function<std::string()> getHwid,
			std::shared_ptr<Client::ClientCommandsHandler> serverCommandsHandler
		);
		~ClientAuthentication();

		void login();
	private:
		std::function<void(const std::string&)> _sendClientCommand;
		std::function<void(const std::string&)> _print;
		std::function<std::string()> _getHwid;
		std::shared_ptr<Client::ClientCommandsHandler> _serverCommandsHandler;
		const std::string GUID_FILE;
		std::string getGuid();
		std::string getHwid();
		std::string createGuid() const;
		OperationResult saveGuid(const std::string& guid) const;
	};
}



