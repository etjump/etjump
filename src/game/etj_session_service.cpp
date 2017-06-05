#include "etj_session_service.h"
#include "etj_session_repository.h"
#include "etj_user_service.h"
#include "etj_string_utilities.h"
#include "etj_log.h"
#include "etj_time_utilities.h"
#include "etj_session_keys.h"

template<typename R>
bool is_ready(std::future<R> const& f)
{
	return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

const std::string ETJump::SessionService::INVALID_AUTH_ATTEMPT = "Invalid authentication attempt";

ETJump::SessionService::SessionService(std::shared_ptr<UserService> userService, std::shared_ptr<SessionRepository> sessionRepository, std::function<void(int clientNum, const char* reason, int timeout)> dropClient)
	: _userService(userService), _sessionRepository(sessionRepository), _log(Log("SessionService")), _dropClient(dropClient)
{
	for (int i = 0, len = _users.size(); i < len; ++i)
	{
		_users[i] = User();
	}
}

ETJump::SessionService::~SessionService()
{
}

void ETJump::SessionService::connect(int clientNum, bool firstTime)
{
	_users[clientNum] = User();

	if (firstTime)
	{
		clearSession(clientNum);
	}

	removeClientTasks(clientNum);
}

void ETJump::SessionService::disconnect(int clientNum)
{
	_userService->updateLastSeen(_users[clientNum].id, DateTime::now());

	_users[clientNum] = User();

	removeClientTasks(clientNum);
}

void ETJump::SessionService::addGetUserTaskAsync(int clientNum, const std::string& name, const std::string& ipAddress, const std::string& guid, const std::string& hardwareId)
{
	auto task = GetUserTask();
	task.user = _userService->getUser(guid);
	task.guid = guid;
	task.clientNum = clientNum;
	task.alias = name;
	task.ipAddress = ipAddress;
	task.hardwareId = hardwareId;
	_getUserTasks.push_back(std::move(task));
}

void ETJump::SessionService::addGetUserTaskAsync(int clientNum, const std::string& name, const std::string& ipAddress, const std::string& guid, const std::string& hardwareId, std::future<User> getUserTask)
{
	auto task = GetUserTask();
	task.user = std::move(getUserTask);
	task.guid = guid;
	task.clientNum = clientNum;
	task.alias = name;
	task.ipAddress = ipAddress;
	task.hardwareId = hardwareId;
	_getUserTasks.push_back(std::move(task));
}

void ETJump::SessionService::handleGetUserTasks()
{
	std::vector<int> deletedTask;
	for (int i = 0, len = _getUserTasks.size(); i < len; ++i)
	{
		if (is_ready(_getUserTasks[i].user))
		{
			int clientNum = _getUserTasks[i].clientNum;
			auto user = _getUserTasks[i].user.get();
			if (user.id == ETJump::UserRepository::NEW_USER_ID)
			{
				_log.infoLn("Added a new user " + _getUserTasks[i].alias + " with guid " + _getUserTasks[i].guid);
				// returns the inserted user
				auto getUserTask = _userService->insertUser(_getUserTasks[i].guid, _getUserTasks[i].alias, _getUserTasks[i].ipAddress, _getUserTasks[i].hardwareId);
				deletedTask.push_back(i);
				addGetUserTaskAsync(clientNum, _getUserTasks[i].alias, _getUserTasks[i].ipAddress, _getUserTasks[i].guid, _getUserTasks[i].hardwareId, std::move(getUserTask));
			}
			else
			{
				if (find(begin(user.hardwareIds), end(user.hardwareIds), _getUserTasks[i].hardwareId) == end(user.hardwareIds))
				{
					user.hardwareIds.push_back(_getUserTasks[i].hardwareId);
					_userService->addHardwareId(user.id, _getUserTasks[i].hardwareId);
				}
				if (find(begin(user.aliases), end(user.aliases), _getUserTasks[i].alias) == end(user.aliases))
				{
					user.aliases.push_back(_getUserTasks[i].alias);
					_userService->addAlias(user.id, _getUserTasks[i].alias);
				}
				if (find(begin(user.ipAddresses), end(user.ipAddresses), _getUserTasks[i].ipAddress) == end(user.ipAddresses))
				{
					user.ipAddresses.push_back(_getUserTasks[i].ipAddress);
					_userService->addIpAddress(user.id, _getUserTasks[i].ipAddress);
				}
				deletedTask.push_back(i);
				user.lastSeen = DateTime::now();
				_userService->updateLastSeen(user.id, user.lastSeen);
				_log.infoLn("User \"" + user.name + "\" logged in from ip \"" + _getUserTasks[i].ipAddress + "\" with hardware id \"" + _getUserTasks[i].hardwareId + "\"");
				_users[_getUserTasks[i].clientNum] = user;
				if (getSessionValue(_getUserTasks[i].clientNum, KEY_GUID).length() == 0)
				{
					// print greeting
				}
				setSessionValue(_getUserTasks[i].clientNum, KEY_GUID, user.guid);
				setSessionValue(_getUserTasks[i].clientNum, KEY_HARDWARE_ID, _getUserTasks[i].hardwareId);
				
			}
		}
	}

	std::vector<GetUserTask> temp;
	for (int i = 0, len = _getUserTasks.size(); i < len; ++i)
	{
		if (find(begin(deletedTask), end(deletedTask), i) == end(deletedTask))
		{
			temp.push_back(std::move(_getUserTasks[i]));
		}
	}
	_getUserTasks = std::move(temp);
}

void ETJump::SessionService::clearSession(int clientNum)
{
	_sessionRepository->clearSession(clientNum);
}

void ETJump::SessionService::authenticate(int clientNum, const std::string& name, const std::string& ipAddress, const std::vector<std::string>& arguments)
{
	if (arguments.size() != 3)
	{
		dropClient(clientNum, INVALID_AUTH_ATTEMPT);
		return;
	}

	if (arguments[1].size() != GUID_LEN || arguments[2].size() != GUID_LEN)
	{
		dropClient(clientNum, INVALID_AUTH_ATTEMPT);
		return;
	}

	if (std::any_of(begin(arguments[1]), end(arguments[1]), [](const char& c)
	{
		if (c >= '0' && c <= '9')
		{
			return false;
		}
		if (c >= 'A' && c <= 'F')
		{
			return false;
		}
		return true;
	}))
	{
		dropClient(clientNum, INVALID_AUTH_ATTEMPT);
		return;
	}

	auto guid = ETJump::hash(arguments[1]);
	auto hardwareId = ETJump::hash(arguments[2]);
	setSessionValue(clientNum, KEY_GUID, guid);
	setSessionValue(clientNum, KEY_HARDWARE_ID, hardwareId);
	addGetUserTaskAsync(clientNum, name, ipAddress, guid, hardwareId);
}

void ETJump::SessionService::runFrame()
{
	handleGetUserTasks();
}

void ETJump::SessionService::readSession(int levelTime)
{
	// if level time is 0, the server just started 
	// --> clear the session data
	if (levelTime == 0)
	{
		_sessionRepository->clearSessions();
		return;
	}

	_sessions = _sessionRepository->loadSessions();
}

void ETJump::SessionService::writeSession()
{
	std::vector<SessionRepository::Session> sessions;

	for (auto & s : _sessions)
	{
		sessions.push_back(std::move(s.second));
	}

	_sessionRepository->clearSessions();
	_sessionRepository->writeSessions(sessions);
}

void ETJump::SessionService::setSessionValue(int clientNum, const std::string& key, const std::string& value)
{
	auto previous = _sessions.find(clientNum);
	if (previous == end(_sessions))
	{
		auto session = SessionRepository::Session();
		session.clientNum = clientNum;
		_sessions[clientNum] = session;
		previous = _sessions.find(clientNum);
	}
	
	previous->second.values[key] = value;
}

std::string ETJump::SessionService::getSessionValue(int clientNum, const std::string& key)
{
	auto userSession = _sessions.find(clientNum);
	if (userSession == end(_sessions))
	{
		return "";
	}
	auto value = userSession->second.values.find(key);
	if (value == end(userSession->second.values))
	{
		return "";
	}
	return value->second;
}

void ETJump::SessionService::readClientSession(int clientNum, const std::string& alias, const std::string& ipAddress)
{
	auto clientSession = _sessions.find(clientNum);
	if (clientSession == end(_sessions))
	{
		return;
	}

	auto guid = clientSession->second.values[KEY_GUID];
	auto hardwareId = clientSession->second.values[KEY_HARDWARE_ID];

	addGetUserTaskAsync(clientNum, alias, ipAddress, guid, hardwareId);
}

void ETJump::SessionService::dropClient(int clientNum, const std::string& reason, int seconds)
{
	seconds = std::max(0, seconds);
	_log.infoLn("Dropping client " + std::to_string(clientNum) + " for: " + reason + " Duration: " + std::to_string(seconds) + "s");
	_dropClient(clientNum, reason.c_str(), seconds);
}

void ETJump::SessionService::removeClientTasks(int clientNum)
{
	removeGetUserTasks([clientNum](const GetUserTask& t)
	{
		return t.clientNum == clientNum;
	});
}

void ETJump::SessionService::removeGetUserTasks(std::function<bool(const GetUserTask&)> predicate)
{
	std::vector<GetUserTask> tasks;

	for (auto & t : _getUserTasks)
	{
		if (predicate(t))
		{
			tasks.push_back(std::move(t));
		}
	}

	_getUserTasks = std::move(tasks);
}


