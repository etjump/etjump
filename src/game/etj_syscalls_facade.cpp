#include <boost/format.hpp>
#include "etj_syscalls_facade.h"
#include "g_local.h"
#include "etj_log.h"

void ETJump::SyscallsFacade::execConsoleCommand(CbufExec execWhen, const std::string& command) const
{
	cbufExec_t exec = EXEC_APPEND;
	switch (execWhen)
	{
	case CbufExec::Now: 
		exec = EXEC_NOW;
	case CbufExec::Insert: 
		exec = EXEC_INSERT;
	case CbufExec::Append: 
		exec = EXEC_APPEND;
	default: 
		Log::error(boost::format("Unknown execWhen: %d") % static_cast<int>(execWhen));
	}

	trap_SendConsoleCommand(exec, command.c_str());
}
