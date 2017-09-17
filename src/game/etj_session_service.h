#pragma once
#include <array>
#include "etj_log.h"
#include <future>
#include "etj_user.h"
#include "etj_session_repository.h"
#include "etj_shared.h"
#include "etj_level.h"
#include <bitset>
#include "etj_task.h"

namespace ETJump
{
	namespace Server {
		class ClientCommandsHandler;
	}

	class UserService;
	class SessionRepository;
	class LevelService;

	class SessionService
	{
	public:
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

		struct CachedUserData
		{
			static const int MAX_PERMISSIONS = 256;
			CachedUserData() : level(nullptr)
			{
				for (int i = 0; i < MAX_PERMISSIONS; ++i)
				{
					permissions[i] = false;
				}
			}
			const Level* level;
			std::bitset<MAX_PERMISSIONS> permissions;
		};

		explicit SessionService(std::shared_ptr<UserService> userService, std::shared_ptr<SessionRepository> sessionRepository, std::shared_ptr<ETJump::Server::ClientCommandsHandler> clientCommandsHandler, std::shared_ptr<LevelService>, std::function<void(int clientNum, const char* reason, int timeout)> dropClient, std::function<void(int clientNum, const char *text)> sendServerCommand);
		~SessionService();

		void connect(int clientNum, bool firstTime);
		void disconnect(int clientNum);
		void authenticate(int clientNum, const std::string& name, const std::string& ipAddress, const std::vector<std::string>& arguments);
		void handleAsyncTasks();
		void checkOnceASecondTasks();
		void runFrame();
		void readSession(int levelTime);
		void writeSession();
		void setSessionValue(int clientNum, const std::string& key, const std::string& value);
		std::string getSessionValue(int clientNum, const std::string& key);
		void readClientSession(int clientNum, const std::string& alias, const std::string& ipAddress);
		const User& getUser(int clientNum);
		bool hasPermission(int clientNum, char permission);
		/**
		 * Finds all users (online and offline) with the specified level and sets
		 * them to the new level
		 */
		void setLevelIfHasLevel(int clientNum, int level, int newLevel);
		void updateUser(int updaterClientNum, int userId, const MutableUserFields& changes, int changedFields);
		bool isEqualOrHigherLevel(int clientNum, int target);
		bool isHigherLevel(int clientNum, int target);
		void mute(int target, long long duration);
		bool unmute(int target);
		void updateUserInfoValue(int target, const std::string& key, const std::string& value);
		static std::string getName(int clientNum);
		static std::vector<std::string> getNames(const std::vector<int>& clientNums);
		std::vector<int> findUsersByName(const std::string& name);
		void dropClient(int clientNum, const std::string& reason, int seconds = 180);
	private:
		void removeClientTasks(int clientNum);
		void removeGetUserTasks(std::function<bool(const GetUserTask&)> predicate);
		void addGetUserTaskAsync(int clientNum, const std::string& name, const std::string& ipAddress, const std::string& guid, const std::string& hardwareId);
		void addGetUserTaskAsync(int clientNum, const std::string& name, const std::string& ipAddress, const std::string& guid, const std::string& hardwareId, std::future<User> task);
		void handleGetUserTasks();
		void clearSession(int clientNum);
		void cacheUserData(int clientNum);

		std::shared_ptr<UserService> _userService;
		std::shared_ptr<SessionRepository> _sessionRepository;

		std::vector<GetUserTask> _getUserTasks;
		std::array<User, Constants::Common::MAX_CONNECTED_CLIENTS> _users;
		std::array<CachedUserData, Constants::Common::MAX_CONNECTED_CLIENTS> _cachedUserData;
		Log _log;
		std::function<void(int, const char*, int)> _dropClient;
		std::map<int, SessionRepository::Session> _sessions;
		std::function<void(int, const char*)> _sendServerCommand;
		std::shared_ptr<ETJump::Server::ClientCommandsHandler> _clientCommandsHandler;
		std::shared_ptr<LevelService> _levelService;
		std::vector<std::unique_ptr<AbstractTask>> _tasks;
		time_t _nextOnceASecondCheck;
		std::vector<std::function<bool()>> _onceASecondTasks;
	};
}



