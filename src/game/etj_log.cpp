#include "g_local.h"
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include "etj_log.h"
#include <chrono>

ETJump::Log::Log(const std::string& name)
	: _name(name)
{
}

ETJump::Log::~Log()
{
}

ETJump::Log::LogLevel ETJump::Log::_level = ETJump::Log::LogLevel::Debug;


void ETJump::Log::debug(const std::string& text) const
{
	if (Log::_level <= LogLevel::Debug)
	{
		log(text, LogLevel::Debug);
	}
}

void ETJump::Log::debugLn(const std::string& text) const
{
	debug(text + "\n");
}

void ETJump::Log::infoLn(const std::string& text) const
{
	info(text + "\n");
}

void ETJump::Log::warnLn(const std::string& text) const
{
	warn(text + "\n");
}

void ETJump::Log::errorLn(const std::string& text) const
{
	error(text + "\n");
}

void ETJump::Log::fatalLn(const std::string& text) const
{
	fatal(text + "\n");
}

void ETJump::Log::info(const std::string& text) const
{
	if (Log::_level <= LogLevel::Info)
	{
		log(text, LogLevel::Info);
	}
}

void ETJump::Log::warn(const std::string& text) const
{
	if (Log::_level <= LogLevel::Warn)
	{
		log(text, LogLevel::Warn);
	}
}

void ETJump::Log::error(const std::string& text) const
{
	if (Log::_level <= LogLevel::Error)
	{
		log(text, LogLevel::Error);
	}
}

void ETJump::Log::fatal(const std::string& text) const
{
	if (Log::_level <= LogLevel::Fatal)
	{
		log(text, LogLevel::Fatal);
	}
}

void ETJump::Log::setLogLevel(LogLevel level)
{
	Log::_level = level;
}

ETJump::Log::LogLevel ETJump::Log::getLogLevel(LogLevel level)
{
	return Log::_level;
}

void ETJump::Log::log(const std::string& text, LogLevel level) const
{
	auto printer = level == LogLevel::Fatal ? G_Error : G_LogPrintf;
	auto toBePrinted = "[" + toString(level) + "] " + _name + ": " + text;
	while (toBePrinted.length() > MAX_SERVER_PRINT_LEN)
	{
		printer("%s", toBePrinted.substr(0, MAX_SERVER_PRINT_LEN).c_str());
		toBePrinted = toBePrinted.substr(MAX_SERVER_PRINT_LEN);
	}
	if (toBePrinted.length() > 0)
	{
		printer("%s", toBePrinted.substr(0, MAX_SERVER_PRINT_LEN).c_str());
	}
}

std::string ETJump::Log::toString(LogLevel level)
{
	switch (level)
	{
	case LogLevel::Debug: 
		return "Debug";
	case LogLevel::Info: 
		return "Info";
	case LogLevel::Warn:
		return "Warn";
	case LogLevel::Error: 
		return "Error";
	case LogLevel::Fatal: 
		return "Fatal";
	default: 
		return "Unknown";
	}
}
