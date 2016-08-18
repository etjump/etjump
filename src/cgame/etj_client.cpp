#include "etj_client.h"
#include "cg_local.h"
#include <boost/format.hpp>
#include <string>


void ETJump::Client::error(const boost::format& fmt)
{
	CG_Error("%s\n", fmt.str().c_str());
}

void ETJump::Client::error(const std::string& text)
{
	CG_Error("%s\n", text.c_str());
}

void ETJump::Client::addCommand(const std::string& command)
{
	trap_AddCommand(command.c_str());
}
