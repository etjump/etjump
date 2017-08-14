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

	namespace Duration
	{
		long long asSeconds(std::time_t time);
		long long asMinutes(std::time_t time);
		long long asHours(std::time_t time);
		long long asDays(std::time_t time);
		long long asMonths(std::time_t time);
		long long asYears(std::time_t time);
		std::string fromNow(std::time_t time);
	}
}
