/*
 * MIT License
 *
 * Copyright (c) 2026 ETJump team <zero@etjump.com>
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

#include <algorithm>
#include <cctype>
#include <iomanip>

#include "etj_string_utilities.h"

namespace StringUtils {
// https://en.wikipedia.org/wiki/Levenshtein_distance
// https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C.2B.2B
unsigned int levenshteinDistance(const std::string &s1, const std::string &s2) {
  const std::size_t len1 = s1.size(), len2 = s2.size();
  std::vector<std::vector<unsigned int>> d(len1 + 1,
                                           std::vector<unsigned int>(len2 + 1));

  d[0][0] = 0;
  for (unsigned int i = 1; i <= len1; ++i) d[i][0] = i;
  for (unsigned int i = 1; i <= len2; ++i) d[0][i] = i;

  for (unsigned int i = 1; i <= len1; ++i)
    for (unsigned int j = 1; j <= len2; ++j)
      d[i][j] = std::min({d[i - 1][j] + 1, d[i][j - 1] + 1,
                          d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1)});
  return d[len1][len2];
}

std::string getBestMatch(const std::vector<std::string> &words,
                         const std::string &current) {
  std::vector<std::pair<std::string, int>> distances;

  for (const auto &word : words) {
    distances.push_back(
        std::make_pair(word, levenshteinDistance(word, current)));
  }

  auto smallest = std::min_element(begin(distances), end(distances),
                                   [](const std::pair<std::string, int> &lhs,
                                      const std::pair<std::string, int> &rhs) {
                                     return lhs.second < rhs.second;
                                   });

  return smallest->first;
}

std::string sanitize(const std::string_view text, const bool toLower,
                     const bool removeControlChars) {
  std::string out;

  for (size_t i = 0; i < text.length(); ++i) {
    if (isColorString(text, i)) {
      i++;
      continue;
    }

    if (removeControlChars && text[i] < 32) {
      continue;
    }

    out += toLower ? static_cast<char>(std::tolower(text[i])) : text[i];
  }

  return out;
}

std::string getValue(const char *value, const std::string &defaultValue) {
  return strlen(value) > 0 ? value : defaultValue;
}

std::string getValue(const std::string &value,
                     const std::string &defaultValue) {
  return value.length() > 0 ? value : defaultValue;
}

std::string trimStart(const std::string &input) {
  std::string str = input;
  str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
              return !std::isspace(ch);
            }));
  return str;
}

std::string trimEnd(const std::string &input) {
  std::string str = input;
  str.erase(std::find_if(str.rbegin(), str.rend(),
                         [](int ch) { return !std::isspace(ch); })
                .base(),
            str.end());
  return str;
}

std::string trim(const std::string &input) { return trimEnd(trimStart(input)); }

// word-wrapper
std::vector<std::string> wrapWords(const std::string &input, char separator,
                                   size_t maxLength) {
  std::vector<std::string> output;
  size_t lastPos = 0;

  if (input.size() <= maxLength) {
    output.push_back(input);
    return output;
  }

  while (true) {
    auto pos = input.rfind(separator, lastPos + maxLength);

    /* separator not found */
    if (pos == std::string::npos) {
      /* split by length; */
      size_t numSplits = input.size() / maxLength;
      for (size_t i = 1; i <= numSplits; ++i) {
        output.push_back(input.substr(lastPos, maxLength));
        lastPos = (maxLength * i);
      }
      break;
    }

    // if we landed on a separator char, back off one char and re-search,
    // otherwise we'll exceed maxLength as pos is incremented
    if (input[pos] == separator) {
      pos = input.rfind(separator, lastPos + maxLength - 1);
    }

    pos += 1;
    /* no new separators were found */
    if (pos == lastPos) {
      break;
    }
    output.push_back(input.substr(lastPos, pos - lastPos));
    lastPos = pos;
  }
  /* add last bit if any */
  if (lastPos < input.size()) {
    output.push_back(input.substr(lastPos));
  }
  return output;
}

std::string getSecondsString(const int &seconds) {
  return getPluralizedString(seconds, "second");
}

std::string getMinutesString(const int &minutes) {
  return getPluralizedString(minutes, "minute");
}

std::string getHoursString(const int &hours) {
  return getPluralizedString(hours, "hour");
}

std::string getDaysString(const int &days) {
  return getPluralizedString(days, "day");
}

std::string getWeeksString(const int &weeks) {
  return getPluralizedString(weeks, "week");
}

std::string getMonthsString(const int &months) {
  return getPluralizedString(months, "month");
}

std::string getYearsString(const int &years) {
  return getPluralizedString(years, "year");
}

std::string toLowerCase(const std::string &input) {
  std::string str = input;
  std::transform(str.begin(), str.end(), str.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return str;
}

std::string toUpperCase(const std::string &input) {
  std::string str = input;
  std::transform(str.begin(), str.end(), str.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return str;
}

std::string eraseLast(const std::string &input, const std::string &substring) {
  std::string str = input;
  auto pos = str.rfind(substring);
  if (pos != std::string::npos) {
    str.erase(pos, substring.size());
  }
  return str;
}

std::vector<std::string> split(const std::string &input,
                               const std::string &delimiter) {
  size_t posStart = 0, posEnd, delimLen = delimiter.length();
  std::string token;
  std::vector<std::string> splits;

  while ((posEnd = input.find(delimiter, posStart)) != std::string::npos) {
    token = input.substr(posStart, posEnd - posStart);
    posStart = posEnd + delimLen;
    splits.push_back(token);
  }

  splits.push_back(input.substr(posStart));

  return splits;
}

void replaceAll(std::string &input, const std::string &from,
                const std::string &to) {
  size_t startPost = 0;
  while ((startPost = input.find(from, startPost)) != std::string::npos) {
    input.replace(startPost, from.length(), to);
    startPost += to.length();
  }
}

void replaceAll(std::wstring &input, const std::wstring &from,
                const std::wstring &to) {
  size_t startPost = 0;
  while ((startPost = input.find(from, startPost)) != std::wstring::npos) {
    input.replace(startPost, from.length(), to);
    startPost += to.length();
  }
}

void stringSubstitute(std::string &input, char character,
                      const std::string &replacement, size_t numChars) {
  size_t startPos = 0;

  while ((startPos = input.find(character, startPos)) != std::string::npos) {
    size_t charsToReplace = std::min(numChars, input.length() - startPos - 1);
    input.replace(startPos, charsToReplace + 1, replacement);
    startPos += replacement.length();
  }
}

bool startsWith(const std::string &str, const std::string &prefix) {
  if (prefix.length() > str.length())
    return false;

  return str.compare(0, prefix.length(), prefix) == 0;
}

bool endsWith(const std::string &str, const std::string &suffix) {
  if (suffix.length() > str.length())
    return false;

  return str.compare(str.length() - suffix.length(), suffix.length(), suffix) ==
         0;
}

bool iEqual(const std::string_view str1, const std::string_view str2,
            bool sanitized) {
  if (sanitized) {
    return sanitize(str1, true) == sanitize(str2, true);
  }

  return std::equal(str1.begin(), str1.end(), str2.begin(), str2.end(),
                    [](unsigned char a, unsigned char b) {
                      return std::tolower(a) == std::tolower(b);
                    });
}

int32_t countExtraPadding(const std::string &input,
                          const int32_t targetPadding) {
  return targetPadding +
         static_cast<int32_t>(input.length() - sanitize(input).length());
}

std::string normalizeNumberString(const std::string &input) {
  if (input.empty()) {
    return "";
  }

  double number;

  try {
    number = std::stod(input);
  } catch (...) {
    return "";
  }

  std::ostringstream oss;
  oss << std::setprecision(10) << std::fixed << number;
  std::string result = oss.str();

  if (result.find('.') != std::string::npos) {
    removeTrailingChars(result, '0');

    // remove trailing dot if the result is just an integer
    if (result.back() == '.') {
      result.pop_back();
    }
  }

  return result;
}

void removeTrailingChars(std::string &str, const char charToRemove) {
  if (str.empty()) {
    return;
  }

  const size_t pos = str.find_last_not_of(charToRemove);

  if (pos == std::string::npos) {
    str.clear();
  } else {
    str.erase(pos + 1);
  }
}

void removeLeadingChars(std::string &str, const char charToRemove) {
  if (str.empty()) {
    return;
  }

  const size_t pos = str.find_first_not_of(charToRemove);

  if (pos == std::string::npos) {
    str.clear();
  } else {
    str.erase(0, pos);
  }
}

bool isColorString(const std::string_view str, const size_t idx) {
  return str[idx] == '^' && str.length() > idx + 1 && str[idx + 1] != '^';
}

std::string truncate(const std::string &str, const size_t len) {
  if (str.empty() || str.length() <= len || sanitize(str).length() <= len) {
    return str;
  }

  size_t outLen = len;
  size_t idx = 0;

  for (size_t i = 0; i < outLen && outLen < str.length(); i++) {
    if (isColorString(str, idx)) {
      outLen += 2;
      idx += 2;
      i++;
      continue;
    }

    idx++;
  }

  std::string out = str.substr(0, outLen);
  return out;
}

void stripExtension(std::string &str) {
  const auto pos = str.rfind('.');

  if (pos != std::string::npos) {
    str.erase(pos);
  }
}

void stripLocalizationMarkers(std::string &str) {
  replaceAll(str, "[lon]", "");
  replaceAll(str, "[lof]", "");
}

void escapeColorCodes(std::string &str, char escapeColor) {
  for (size_t i = 0; i < str.length(); i++) {
    if (isColorString(str, i) && str.length() > i + 2) {
      str.insert(i + 1, "^");
      str.insert(i + 2, 1, escapeColor);
      i += 2;
    }
  }
}
} // namespace StringUtils
