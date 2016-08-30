#pragma once
#include <string>
#include <boost/format/format_fwd.hpp>

namespace ETJump
{
	class Log
	{
	public:
		static const int MAX_ERROR_LENGTH = 1020;

		static void error(const std::string& error);
		static void error(const boost::format& fmt);
	};
}

