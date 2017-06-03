#pragma once
#include <string>
#include <ctime>

namespace ETJump
{
	namespace DateTime
	{
		std::string getCurrentUtcTimestamp();
		std::time_t now();
	}
}
