#include <boost/format.hpp>
#include "etj_client_print_utilities.h"
#include "cg_local.h"


void ETJump::ClientPrintUtilities::printToConsole(std::string text)
{
	const auto CG_PRINTF_BUF_LIMIT = 1011;

	while (text.length() > CG_PRINTF_BUF_LIMIT)
	{
		auto current = text.substr(0, CG_PRINTF_BUF_LIMIT);
		text = text.substr(CG_PRINTF_BUF_LIMIT);
		CG_Printf("%s", current.c_str());
	}
}

void ETJump::ClientPrintUtilities::printToConsole(const boost::format& fmt)
{
	ClientPrintUtilities::printToConsole(fmt.str());
}
