#pragma once
#include "etj_isyscalls_facade.h"

namespace ETJump
{
	class SyscallsFacade : public ISyscallsFacade
	{
	public:
		void execConsoleCommand(CbufExec execWhen, const std::string& command) const override;
	};
}

