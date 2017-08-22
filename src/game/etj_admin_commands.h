#pragma once

#include <memory>

namespace ETJump
{
	class AdminCommandsHandler;
	class LevelService;
	class SessionService;
	class UserService;
	void registerAdminCommands(std::shared_ptr<AdminCommandsHandler> adminCommandsHandler, std::shared_ptr<LevelService> levelService, std::shared_ptr<SessionService> sessionService, std::shared_ptr<UserService> injectedUserService);
}
