#include "etj_time_utilities.h"
#include <chrono>
#include <time.h>
#include <ctime>

long long ETJump::getCurrentTimestamp()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

ETJump::Clock ETJump::getCurrentClock()
{
	time_t now = std::time(nullptr);
	tm tstruct = *std::localtime(&now);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) % 1000;
	return{ tstruct.tm_hour, tstruct.tm_min, tstruct.tm_sec, (int)ms.count() };
}

ETJump::Clock ETJump::toClock(long long timestamp)
{
	int hours = timestamp / 6000;
	int minutes = timestamp / 60000;
	timestamp -= minutes * 60000;
	int seconds = timestamp / 1000;
	timestamp -= seconds * 1000;
	int millis = timestamp;

	return{ hours, minutes, seconds, millis };
}

ETJump::Date ETJump::getCurrentDate()
{

	time_t now = std::time(nullptr);
	tm tstruct = *std::localtime(&now);
	return{ tstruct.tm_year + 1900, tstruct.tm_mon + 1, tstruct.tm_mday, tstruct.tm_yday };
}

ETJump::Time ETJump::getCurrentTime()
{
	return{ getCurrentClock(), getCurrentDate() };
}
