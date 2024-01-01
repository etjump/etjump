/*
 * MIT License
 *
 * Copyright (c) 2024 ETJump team <zero@etjump.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#include <string>
#include <sstream>
#include <vector>
#include <fmt/printf.h>

namespace ETJump {
std::string hash(const std::string &input);
std::string getBestMatch(const std::vector<std::string> &words,
                         const std::string &current);
std::string sanitize(const std::string &text, bool toLower = false);
// returns the value if it's specified, else the default value
std::string getValue(const char *value, const std::string &defaultValue = "");
std::string getValue(const std::string &value,
                     const std::string &defaultValue = "");

template <typename... Targs>
std::string stringFormat(const std::string &format, const Targs &...Fargs) {
  return fmt::sprintf(format, Fargs...);
}

std::string trimStart(const std::string &input);
std::string trimEnd(const std::string &input);
std::string trim(const std::string &input);

std::vector<std::string> wrapWords(std::string &input, char separator,
                                   size_t maxLength);

template <typename T>
std::string getPluralizedString(const T &val, const std::string &str) {
  return std::to_string(val) + " " + str + (val == 1 ? "" : "s");
}

std::string getSecondsString(const int &seconds);
std::string getMinutesString(const int &minutes);
std::string getHoursString(const int &hours);
std::string getDaysString(const int &days);
std::string getWeeksString(const int &weeks);
std::string getMonthsString(const int &months);
std::string getYearsString(const int &years);

namespace StringUtil {
std::string toLowerCase(const std::string &input);
std::string toUpperCase(const std::string &input);
std::string eraseLast(const std::string &input, const std::string &substring);
template <typename T>
std::string join(const T &v, const std::string &delim) {
  std::ostringstream s;
  for (const auto &i : v) {
    if (&i != &v[0]) {
      s << delim;
    }
    s << i;
  }
  return s.str();
}
std::vector<std::string> split(const std::string &input,
                               const std::string &delimiter);
void replaceAll(std::string &input, const std::string &from,
                const std::string &to);

// replaces all occurrences of specified character
// and the following N characters in the string
// substitution will "eat" the substitution character, so it does need to
// be accounted for when defining how many characters to replace
void stringSubstitute(std::string &input, char character,
                      const std::string &replacement, size_t numChars);

bool startsWith(const std::string &str, const std::string &prefix);
bool endsWith(const std::string &str, const std::string &suffix);
bool contains(const std::string &str, const std::string &text);
bool matches(const std::string &str, const std::string &text);
// Counts the extra padding needed when using format specifiers like
// %-20s with text that contains ET color codes
unsigned countExtraPadding(const std::string &input);
} // namespace StringUtil
} // namespace ETJump
