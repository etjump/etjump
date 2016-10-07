#pragma once
#include <string>

namespace ETJump
{
	class ISyscallsFacade
	{
	public:
		virtual ~ISyscallsFacade()
		{
		}

		// paremeters for command buffer stuffing
		enum class CbufExec
		{
			Now, // don't return until completed, a VM should NEVER use this,
				 // because some commands might cause the VM to be unloaded...
				 Insert, // insert at current position, but don't run yet
				 Append  // add to end of the command buffer (normal case)
		};

		// execute console command on server console
		// e.g. map <mapname>
		virtual void execConsoleCommand(CbufExec execWhen, const std::string& command) const = 0;
	};
}
