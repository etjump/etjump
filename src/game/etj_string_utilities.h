#pragma once
#include <boost/format.hpp>
#include <string>
#include <vector>

namespace ETJump
{
	std::string hash(const std::string& input);
	std::string newGuid();
	std::string getBestMatch(const std::vector<std::string>& words, const std::string& current);
	std::string sanitize(const std::string& text, bool toLower = false);
	// returns the value if it's specified, else the default value
	std::string getValue(const char *value, const std::string& defaultValue = "");
	std::string getValue(const std::string& value, const std::string& defaultValue = "");
	template<typename T, typename... Targs>
	std::string sprintf(boost::format& format, T&& value, Targs&&... Fargs)
	{
		format % std::forward<T>(value);
		return sprintf(format, std::forward<Targs>(Fargs)...);
	}
	template<typename T, typename... Targs>
	std::string sprintf(const std::string& format, T&& value, Targs&&... Fargs)
	{
		return sprintf(boost::format(format) % value, Fargs...);
	}
	inline std::string sprintf(boost::format& format)
	{
		return format.str();
	}
}
