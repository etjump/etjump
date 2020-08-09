/*
 * MIT License
 * 
 * Copyright (c) 2020 ETJump team <zero@etjump.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

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
	std::string stringFormat(boost::format& fmt, const T& value, const Targs&... Fargs)
	{
		fmt % value;
		return stringFormat(fmt, Fargs...);
	}

	template<typename T, typename... Targs>
	std::string stringFormat(const std::string& fmt, const T& value, const Targs&... Fargs)
	{
		return stringFormat(boost::format(fmt) % value, Fargs...);
	}

    std::string trimStart(std::string input);
    std::string trimEnd(std::string input);
    std::string trim(const std::string& input);

    std::vector<std::string> splitString(std::string &input, char separator, size_t maxLength);
}
