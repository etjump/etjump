#pragma once
#include <boost/format/format_fwd.hpp>

namespace ETJump
{
	class Client
	{
	public:
		Client() = delete;
		~Client() = delete;

		// Prints the error message and exits
		static void error(const boost::format& fmt);
		static void error(const std::string& text);
		
		// adds command to the autocomplete list
		static void addCommand(const std::string& command);
	};
}
