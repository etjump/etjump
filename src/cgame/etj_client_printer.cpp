#include <boost/format.hpp>
#include "etj_client_printer.h"
#include "cg_local.h"

void ETJump::ClientPrinter::write(const std::string& message)
{
	CG_Printf(message.c_str());
}

void ETJump::ClientPrinter::write(const boost::format& fmt)
{
	CG_Printf(fmt.str().c_str());
}

void ETJump::ClientPrinter::writeLine(const std::string& message)
{
	CG_Printf("%s\n", message.c_str());
}

void ETJump::ClientPrinter::writeLine(const boost::format& fmt)
{
	CG_Printf("%s\n", fmt.str().c_str());
}
