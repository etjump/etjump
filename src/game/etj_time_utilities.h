#pragma once
#include <string>
#include <ctime>

namespace ETJump
{
	namespace DateTime
	{
		std::string getCurrentUtcTimestamp();
		std::time_t now();
		std::string toLocalTime(std::time_t time);
	}
}
