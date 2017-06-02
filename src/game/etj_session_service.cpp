#include "etj_session_service.h"

template<typename R>
bool is_ready(std::future<R> const& f)
{
	return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

const std::string ETJump::SessionService::INVALID_AUTH_ATTEMPT = "Invalid authentication attempt";

ETJump::SessionService::SessionService(std::shared_ptr<UserService> userService)
	: _userService(userService)
{
}

ETJump::SessionService::~SessionService()
{
}

void ETJump::SessionService::connect(int clientNum)
{
	_users[clientNum] = User();

	removeClientTasks(clientNum);
}

void ETJump::SessionService::disconnect(int clientNum)
{
	_users[clientNum] = User();

	removeClientTasks(clientNum);
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

	auto task = GetUserTask();
	task.user = _userService->getUser(arguments[1]);
	task.guid = arguments[1];
	task.clientNum = clientNum;
	task.alias = name;
	task.ipAddress = ipAddress;
	task.hardwareId = arguments[2];
	_getUserTasks.push_back(std::move(task));
}

void ETJump::SessionService::runFrame()
{
	for (int i = 0, len = _getUserTasks.size(); i < len; ++i)
	{
		if (is_ready(_getUserTasks[i].user))
		{
			int clientNum = _getUserTasks[i].clientNum;
			auto user  = _getUserTasks[i].user.get();
			if (user.id == ETJump::UserRepository::NEW_USER_ID)
			{
				_userService->insertUser(_getUserTasks[i].guid, _getUserTasks[i].alias, _getUserTasks[i].ipAddress, _getUserTasks[i].hardwareId);
				removeGetUserTasks([clientNum](const GetUserTask& t)
				{
					return t.clientNum == clientNum;
				});
			} else
			{
				if (std::find(begin(user.hardwareIds), end(user.hardwareIds), _getUserTasks[i].hardwareId) == end(user.hardwareIds))
				{
					user.hardwareIds.push_back(_getUserTasks[i].hardwareId);
					_userService->addHardwareId(user.id, _getUserTasks[i].hardwareId);
				}
				if (std::find(begin(user.aliases), end(user.aliases), _getUserTasks[i].alias) == end(user.aliases))
				{
					user.aliases.push_back(_getUserTasks[i].alias);
					_userService->addAlias(user.id, _getUserTasks[i].alias);
				}
				if (std::find(begin(user.ipAddresses), end(user.ipAddresses), _getUserTasks[i].ipAddress) == end(user.ipAddresses))
				{
					user.ipAddresses.push_back(_getUserTasks[i].ipAddress);
					_userService->addIpAddress(user.id, _getUserTasks[i].ipAddress);
				}
				removeGetUserTasks([clientNum](const GetUserTask& t)
				{
					return t.clientNum == clientNum;
				});
			}
		}
	}
}

void ETJump::SessionService::dropClient(int clientNum, const std::string& reason, int seconds)
{
	
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


