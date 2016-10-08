#pragma once
#include <string>
#include <boost/format/format_fwd.hpp>

namespace ETJump
{
	class ClientPrinter
	{
	public:
		// prints the message to console
		static void write(const std::string& message);
		static void write(const boost::format& fmt);

		// prints the message to console with newline
		static void writeLine(const std::string& message);
		static void writeLine(const boost::format& fmt);
	};
}



