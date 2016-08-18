#pragma once
#include <string>
#include <boost/format/format_fwd.hpp>

namespace ETJump
{
	class ClientPrintUtilities
	{
	public:
		ClientPrintUtilities() = delete;
		~ClientPrintUtilities() = delete;

		// Prints text to console
		static void printToConsole(std::string text);
		static void printToConsole(const boost::format& fmt);
	};
}



