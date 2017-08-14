#include "etj_time_utilities.h"
#include <chrono>
#include <ctime>
#include <map>
#include <boost/format.hpp>

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

const std::map<std::string, int> thresholds{
	{ "ss", 44 },
	{ "s", 45 },
	{ "m", 45 },
	{ "h", 22 },
	{ "d", 26 },
	{ "M", 11 }
};

const std::map<std::string, std::string> relativeTime {
	{ "future", "in %s" },
	{ "past", "%s ago" },
	{ "s", "a few seconds" },
	{ "ss", "%d seconds" },
	{ "m", "a minute" },
	{ "mm", "%d minutes" },
	{ "h", "an hour" },
	{ "hh", "%d hours" },
	{ "d", "a day" },
	{ "dd", "%d days" },
	{ "M", "a month" },
	{ "MM", "%d months" },
	{ "y", "a year" },
	{ "yy", "%d years" },
};

long long daysToMonths(long long days)
{
	// 400 years have 146097 days (taking into account leap year rules)
	// 400 years have 12 months === 4800
	return days * 4800 / 146097;
}

long long ETJump::Duration::asSeconds(std::time_t time)
{
	return time;
}

long long ETJump::Duration::asMinutes(std::time_t time)
{
	return time / 60;
}

long long ETJump::Duration::asHours(std::time_t time)
{
	return time / 3600;
}

long long ETJump::Duration::asDays(std::time_t time)
{
	return time / 86400;
}

long long ETJump::Duration::asMonths(std::time_t time)
{
	return daysToMonths(asDays(time));
}

long long ETJump::Duration::asYears(std::time_t time)
{
	return daysToMonths(asDays(time)) / 12;
}

std::string ETJump::Duration::fromNow(std::time_t time)
{
	auto currentTime = ETJump::DateTime::now();
	auto duration = currentTime - time;
	auto isFuture = duration >= 0;
	duration = std::abs(duration);
	auto seconds = round(asSeconds(time));
	auto minutes = round(asMinutes(time));
	auto hours = round(asHours(time));
	auto days = round(asDays(time));
	auto months = round(asMonths(time));
	auto years = round(asYears(time));

	std::string output;
	if (seconds <= thresholds.at("ss"))
	{
		output = relativeTime.at("s");
	} else if (seconds < thresholds.at("s"))
	{
		output = (boost::format(relativeTime.at("ss")) % seconds).str();
	} else if (minutes <= 1)
	{
		output = relativeTime.at("m");
	} else if (minutes < thresholds.at("m"))
	{
		output = (boost::format(relativeTime.at("mm")) % minutes).str();
	} else if (hours <= 1)
	{
		output = relativeTime.at("h");
	} else if (hours < thresholds.at("h"))
	{
		output = (boost::format(relativeTime.at("hh")) % hours).str();
	} else if (days <= 1)
	{
		output = relativeTime.at("d");
	} else if (days < thresholds.at("d"))
	{
		output = (boost::format(relativeTime.at("dd")) % days).str();
	} else if (months <= 1)
	{
		output = relativeTime.at("M");
	} else if (months < thresholds.at("M"))
	{
		output = (boost::format(relativeTime.at("MM")) % months).str();
	} else if (years <= 1)
	{
		output = relativeTime.at("y");
	} else
	{
		output = (boost::format(relativeTime.at("yy")) % years).str();
	}
	
	return (boost::format(isFuture ? relativeTime.at("future") : relativeTime.at("past")) % output).str();
}
