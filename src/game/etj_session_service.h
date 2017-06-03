#pragma once
#include <array>
#include "etj_user_service.h"
#include "etj_commands.h"
#include "etj_log.h"

namespace ETJump
{
	class SessionService
	{
	public:
		static const int GUID_LEN = 40;
		static const std::string INVALID_AUTH_ATTEMPT;

		struct GetUserTask
		{
			int clientNum;
			std::future<User> user;
			std::string guid;
			std::string alias;
			std::string ipAddress;
			std::string hardwareId;
		};

		explicit SessionService(std::shared_ptr<UserService> userService);
		~SessionService();

		void connect(int clientNum);
		void disconnect(int clientNum);
		void authenticate(int clientNum, const std::string& name, const std::string& ipAddress, const std::vector<std::string>& arguments);
		void runFrame();
	private:
		void dropClient(int clientNum, const std::string& reason, int seconds = 180);
		void removeClientTasks(int clientNum);
		void removeGetUserTasks(std::function<bool(const GetUserTask&)> predicate);
		void addGetUserTaskAsync(int clientNum, const std::string& name, const std::string& ipAddress, const std::string& guid, const std::string& hardwareId);

		std::shared_ptr<UserService> _userService;

		std::vector<GetUserTask> _getUserTasks;
		std::array<User, 64> _users;
		Log _log;
	};
}



