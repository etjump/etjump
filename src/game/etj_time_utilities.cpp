#include "etj_time_utilities.h"
#include <chrono>
#include <ctime>

std::string ETJump::DateTime::getCurrentUtcTimestamp()
{
	time_t now;
	time(&now);
	char buf[64];
	strftime(buf, sizeof(buf), "%FT%TZ", gmtime(&now));
	return buf;
}

std::time_t ETJump::DateTime::now()
{
	time_t now;
	time(&now);
	return now;
}

std::string ETJump::DateTime::toLocalTime(std::time_t time)
{
	auto res = localtime(&time);
	char buf[100] = "";
	strftime(buf, sizeof(buf), "%c", res);
	return buf;
}

