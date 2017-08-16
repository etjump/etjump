#pragma once

#include <memory>

namespace ETJump
{
	class AdminCommandsHandler;
	class LevelService;
	void registerAdminCommands(std::shared_ptr<AdminCommandsHandler> adminCommandsHandler, std::shared_ptr<LevelService> levelService);
}
