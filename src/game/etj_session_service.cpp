#include "etj_session_service.h"
#include "etj_session_repository.h"
#include "etj_user_service.h"
#include "etj_string_utilities.h"
#include "etj_log.h"
#include "etj_time_utilities.h"
#include "etj_session_keys.h"
#include "etj_client_commands_handler.h"
#include "etj_local.h"
#include "etj_level_service.h"
#include "etj_printer.h"
#include <boost/algorithm/string/replace.hpp>
#include "etj_task.h"

std::bitset<ETJump::SessionService::CachedUserData::MAX_PERMISSIONS> parsePermissions(const std::string& levelCommands, const std::string& userCommands)
{
	bool exclude = false;
	std::bitset<ETJump::SessionService::CachedUserData::MAX_PERMISSIONS> permissions;
	permissions.reset();
	for (const char c : levelCommands)
	{
		switch (c)
		{
		case '*':
			for (int i = 0; i < ETJump::SessionService::CachedUserData::MAX_PERMISSIONS; ++i)
			{
				permissions[i] = true;
			}
			break;
		case '-':
			exclude = true;
			break;
		default:
			permissions.set(static_cast<int>(c), !exclude);
			break;
		}
	}

	exclude = false;
	for (const char c : userCommands)
	{
		switch (c)
		{
		case '*':
			for (int i = 0; i < ETJump::SessionService::CachedUserData::MAX_PERMISSIONS; ++i)
			{
				permissions[i] = true;
			}
			break;
		case '-':
			exclude = true;
			break;
		default:
			permissions.set(static_cast<int>(c), !exclude);
			break;
		}
	}
	return permissions;
}


const std::string ETJump::SessionService::INVALID_AUTH_ATTEMPT = "Invalid authentication attempt";

ETJump::SessionService::SessionService(
	std::shared_ptr<UserService> userService, 
	std::shared_ptr<SessionRepository> sessionRepository, 
	std::shared_ptr<ETJump::Server::ClientCommandsHandler> clientCommandsHandler,
	std::shared_ptr<LevelService> levelService,
	std::function<void(int clientNum, const char* reason, int timeout)> dropClient,
	std::function<void(int clientNum, const char *text)> sendServerCommand
)
	: _userService(userService), _sessionRepository(sessionRepository), _log(Log("SessionService")), _dropClient(dropClient), _sendServerCommand(sendServerCommand), _clientCommandsHandler(clientCommandsHandler), _levelService(levelService)
{
	for (int i = 0, len = _users.size(); i < len; ++i)
	{
		_users[i] = User();
		_cachedUserData[i] = CachedUserData();
	}

	_clientCommandsHandler->subscribe(Constants::Authentication::AUTHENTICATE, [&](int clientNum, const std::string& command, const std::vector<std::string>& args)
	{
		// TODO/FIXME?: could just add callbacks for changing certain 
		// session parameters in client struct but I guess accessing them directly 
		// won't be too bad..
		auto entity = g_entities + clientNum;
		char userinfo[MAX_INFO_STRING] = "";

		trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));
		auto value = Info_ValueForKey(userinfo, "ip");
		std::string ip = "";
		if (value != nullptr)
		{
			ip = value;
		}

		this->authenticate(clientNum, entity->client->pers.netname, ip, args);
	});
}

ETJump::SessionService::~SessionService()
{
	_clientCommandsHandler->unsubcribe(Constants::Authentication::AUTHENTICATE);
}

void ETJump::SessionService::connect(int clientNum, bool firstTime)
{
	_users[clientNum] = User();
	_cachedUserData[clientNum] = CachedUserData();

	removeClientTasks(clientNum);
	
	if (firstTime)
	{
		clearSession(clientNum);
		_sendServerCommand(clientNum, Constants::Authentication::GUID_REQUEST.c_str());
	}
}

void ETJump::SessionService::disconnect(int clientNum)
{
	_userService->updateLastSeen(_users[clientNum].id, DateTime::now());
	_sessions[clientNum].values = std::map<std::string, std::string>();

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
				_log.infoLn("User \"" + user.name + "\" logged in from ip \"" + _getUserTasks[i].ipAddress + "\" with hardware id \"" + _getUserTasks[i].hardwareId + "\"");
				_users[_getUserTasks[i].clientNum] = user;
				setSessionValue(_getUserTasks[i].clientNum, KEY_GUID, user.guid);
				setSessionValue(_getUserTasks[i].clientNum, KEY_HARDWARE_ID, _getUserTasks[i].hardwareId);
				cacheUserData(clientNum);
				if (getSessionValue(_getUserTasks[i].clientNum, KEY_GREETING_DISPLAYED).length() == 0)
				{
					auto greeting = user.greeting;
					if (greeting.length() == 0)
					{
						auto level = _levelService->get(user.level);
						greeting = level != nullptr ? level->greeting : "";
					}
					boost::replace_all(greeting, "[n]", (g_entities + clientNum)->client->pers.netname);
					boost::replace_all(greeting, "[d]", Duration::fromNow(user.lastSeen));
					boost::replace_all(greeting, "[t]", DateTime::toLocalTime(user.lastSeen));
					Printer::broadcastChatMessage(greeting);
					setSessionValue(clientNum, KEY_GREETING_DISPLAYED, "1");
				}
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

void ETJump::SessionService::cacheUserData(int clientNum)
{
	const auto user = &_users[clientNum];
	CachedUserData userData;
	userData.level = _levelService->get(user->level);
	userData.permissions = parsePermissions(userData.level->commands, user->commands);
	_cachedUserData[clientNum] = userData;
}

void ETJump::SessionService::authenticate(int clientNum, const std::string& name, const std::string& ipAddress, const std::vector<std::string>& arguments)
{
	if (arguments.size() != 2)
	{
		dropClient(clientNum, INVALID_AUTH_ATTEMPT);
		return;
	}

	if (arguments[0].size() != Constants::Authentication::GUID_LEN || arguments[1].size() != Constants::Authentication::GUID_LEN)
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

	auto guid = ETJump::hash(arguments[0]);
	auto hardwareId = ETJump::hash(arguments[1]);
	setSessionValue(clientNum, KEY_GUID, guid);
	setSessionValue(clientNum, KEY_HARDWARE_ID, hardwareId);
	addGetUserTaskAsync(clientNum, name, ipAddress, guid, hardwareId);
}

void ETJump::SessionService::handleAsyncTasks()
{
	std::vector<int> deletedTask;
	for (int i = 0, len = _tasks.size(); i < len; ++i)
	{
		if (_tasks[i]->isReady())
		{
			_tasks[i]->execute();
			deletedTask.push_back(i);
		}
	}

	std::vector<std::unique_ptr<AbstractTask>> temp;
	for (int i = 0, len = _tasks.size(); i < len; ++i)
	{
		if (find(begin(deletedTask), end(deletedTask), i) == end(deletedTask))
		{
			temp.push_back(std::move(_tasks[i]));
		}
	}
	_tasks = std::move(temp);
}

void ETJump::SessionService::checkOnceASecondTasks()
{
	if (_nextOnceASecondCheck < DateTime::now())
	{
		std::vector<int> deletedTaskIndices;
		for (int i = 0, len = _onceASecondTasks.size(); i < len; ++i)
		{
			if (_onceASecondTasks[i]())
			{
				deletedTaskIndices.push_back(i);
			}
		}

		std::vector<std::function<bool()>> temp;
		for (int i = 0, len = _onceASecondTasks.size(); i < len; ++i)
		{
			if (find(begin(deletedTaskIndices), end(deletedTaskIndices), i) == end(deletedTaskIndices))
			{
				temp.push_back(std::move(_onceASecondTasks[i]));
			}
		}

		_nextOnceASecondCheck = DateTime::now();
		_onceASecondTasks = std::move(temp);
	}
}

void ETJump::SessionService::runFrame()
{
	handleGetUserTasks();
	handleAsyncTasks();
	checkOnceASecondTasks();
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

const ETJump::User& ETJump::SessionService::getUser(int clientNum)
{
	if (clientNum < 0 || clientNum >= Constants::Common::MAX_CONNECTED_CLIENTS)
	{
		_log.fatalLn("out of bounds client number when getting user \"" + std::to_string(clientNum) + "\"");
		throw std::runtime_error("user index out of bounds");
	}

	return _users[clientNum];
}

bool ETJump::SessionService::hasPermission(int clientNum, char permission)
{
	if (clientNum < 0)
	{
		return true;
	}

	return _cachedUserData[clientNum].permissions.test(static_cast<int>(permission));
}

void ETJump::SessionService::setLevelIfHasLevel(int clientNum, int level, int newLevel)
{
	for (auto & u : _users)
	{
		if (u.id != User::NO_USER_ID)
		{
			if (u.level == level)
			{
				u.level = newLevel;
			}
		}
	}

	auto future = _userService->setLevelIfHasLevel(level, newLevel);
	auto task = new Task<int>(std::move(future), [=](int numAffectedPlayers)
	{
		Printer::sendChatMessage(clientNum, "^2ETJump: ^7set the level of " + std::to_string(numAffectedPlayers) + " players from " + std::to_string(level) + " to " + std::to_string(newLevel));
	});
	_tasks.push_back(std::unique_ptr<Task<int>>(task));
}

std::string ETJump::SessionService::getName(int clientNum)
{
	if (clientNum < 0)
	{
		return "Console";
	}
	return (g_entities + clientNum)->client->pers.netname;
}

void ETJump::SessionService::updateUser(int updaterClientNum, int userId, const MutableUserFields& changes, int changedFields)
{
	// update the connected client if there's one
	for (int i = 0; i < Constants::Common::MAX_CONNECTED_CLIENTS; ++i)
	{
		auto &u = _users[i];
		if (u.id == userId)
		{
			if (changedFields & static_cast<int>(UserFields::Commands))
			{
				u.commands = changes.commands;
				_cachedUserData[i].permissions = parsePermissions(_levelService->get(u.level)->commands, u.commands);
			}
			if (changedFields & static_cast<int>(UserFields::Greeting))
			{
				u.greeting = changes.greeting;
			}
			if (changedFields & static_cast<int>(UserFields::Title))
			{
				u.title = changes.title;
			}
			break;
		}
	}

	auto task = new Task<int>(_userService->updateUser(userId, changes, changedFields), [updaterClientNum, userId](int _)
	{
		Printer::sendChatMessage(updaterClientNum, "^2ETJump: ^7updated user " + std::to_string(userId) + ".");
	});
	_tasks.push_back(std::unique_ptr<Task<int>>(task));
}

bool ETJump::SessionService::isEqualOrHigherLevel(int clientNum, int target)
{
	return _users[clientNum].level >= _users[target].level;
}

bool ETJump::SessionService::isHigherLevel(int clientNum, int target)
{
	return _users[clientNum].level > _users[target].level;
}

void ETJump::SessionService::mute(int target, long long duration)
{
	(g_entities + target)->client->sess.muted = qtrue;
	Printer::sendChatMessage(target, "^5You've been muted.");
	if (duration > 0)
	{
		auto mutedUntil = DateTime::now() + duration;
		_onceASecondTasks.push_back([target, mutedUntil]()
		{
			if (DateTime::now() > mutedUntil)
			{
				(g_entities + target)->client->sess.muted = qfalse;
				Printer::sendChatMessage(target, "^5You've been unmuted.");
				return true;
			}
			return false;
		});
	}
}

std::vector<int> ETJump::SessionService::findUsersByName(const std::string& partial)
{
	try
	{
		std::locale locale = std::locale::classic();
		bool isNumeric = true; 
		for (const auto & c : partial)
		{
			if (!std::isdigit(c, locale))
			{
				isNumeric = false;
				break;
			}
		}
		if (isNumeric)
		{
			auto clientNum = std::stoi(partial);
			if (clientNum >= 0 && clientNum < Constants::Common::MAX_CONNECTED_CLIENTS)
			{
				return std::vector<int>{clientNum};
			}
		}
	} catch (const std::out_of_range&)
	{
	} catch (const std::invalid_argument&)
	{
	}

	std::vector<int> matches;
	for (int i = 0; i < level.numConnectedClients; ++i)
	{
		auto clientNum = level.sortedClients[i];

		auto text = ETJump::sanitize(partial, true);
		auto name = ETJump::sanitize(partial, true);
		if (name.find(text) == std::string::npos)
		{
			continue;
		}
		matches.push_back(clientNum);
	}
	return matches;
}
