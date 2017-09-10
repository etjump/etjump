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
	
	inline std::string stringFormat(boost::format& fmt)
	{
		return fmt.str();
	}

	template<typename T, typename... Targs>
	std::string stringFormat(boost::format& fmt, T&& value, Targs&&... Fargs)
	{
		fmt % std::forward<T>(value);
		return stringFormat(fmt, std::forward<Targs>(Fargs)...);
	}

	template<typename T, typename... Targs>
	std::string stringFormat(const std::string& fmt, T&& value, Targs&&... Fargs)
	{
		return stringFormat(boost::format(fmt) % value, Fargs...);
	}
}
