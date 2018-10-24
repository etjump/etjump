#pragma once

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace ETJump
{
	struct Clock
	{
		int hours;
		int min;
		int sec;
		int ms;
	};

	struct Date
	{
		int year; // year
		int mon; // month
		int day; // day of the month		
		int days;  // days passed since January 1
	};

	struct Time
	{
		Clock clock;
		Date date;
	};

	long long getCurrentTimestamp();
	Clock getCurrentClock();
	Clock toClock(long long timestamp);
	Date getCurrentDate();
	Time getCurrentTime();
}