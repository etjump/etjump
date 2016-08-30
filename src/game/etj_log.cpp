#include <boost/format.hpp>
#include "etj_log.h"
#include "g_local.h"

void ETJump::Log::error(const std::string& error)
{
	trap_Error(error.substr(0, MAX_ERROR_LENGTH).c_str());
}

void ETJump::Log::error(const boost::format& fmt)
{
	error(fmt.str());
}
