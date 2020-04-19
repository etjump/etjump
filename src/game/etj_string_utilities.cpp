#include "etj_string_utilities.h"
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid_io.hpp>

extern "C" {
#include "../sha-1/sha1.h"
}


std::string ETJump::hash(const std::string& input)
{
	SHA1Context sha;

	SHA1Reset(&sha);
	SHA1Input(&sha, reinterpret_cast<const unsigned char *>(input.c_str()), input.length());

	if (!SHA1Result(&sha))
	{
		return "";
	}

	char buffer[60] = "";
	snprintf(buffer, sizeof(buffer), "%08X%08X%08X%08X%08X", sha.Message_Digest[0],
		sha.Message_Digest[1],
		sha.Message_Digest[2],
		sha.Message_Digest[3],
		sha.Message_Digest[4]);
	return buffer;
}

std::string ETJump::newGuid()
{
	boost::uuids::random_generator gen;
	auto u = gen();
	return boost::uuids::to_string(u);
}

// https://en.wikipedia.org/wiki/Levenshtein_distance
// https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C.2B.2B
unsigned int levenshteinDistance(const std::string& s1, const std::string& s2)
{
	const std::size_t len1 = s1.size(), len2 = s2.size();
	std::vector<std::vector<unsigned int>> d(len1 + 1, std::vector<unsigned int>(len2 + 1));

	d[0][0] = 0;
	for (unsigned int i = 1; i <= len1; ++i) d[i][0] = i;
	for (unsigned int i = 1; i <= len2; ++i) d[0][i] = i;

	for (unsigned int i = 1; i <= len1; ++i)
		for (unsigned int j = 1; j <= len2; ++j)
			d[i][j] = std::min({ d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1) });
	return d[len1][len2];
}

std::string ETJump::getBestMatch(const std::vector<std::string>& words, const std::string& current)
{
	std::vector<std::pair<std::string, int>> distances;

	for (const auto & word : words)
	{
		distances.push_back(std::make_pair(word, levenshteinDistance(word, current)));
	}

	auto smallest = std::min_element(begin(distances), end(distances), [](const std::pair<std::string, int>& lhs, const std::pair<std::string, int>& rhs)
	{
		return lhs.second < rhs.second;
	});

	return smallest->first;
}

static void SanitizeConstString(const char *in, char *out, bool toLower)
{
	while (*in)
	{
		if (*in == 27 || *in == '^')
		{
			in++;       // skip color code
			if (*in)
			{
				in++;
			}
			continue;
		}

		if (*in < 32)
		{
			in++;
			continue;
		}

		*out++ = (toLower) ? tolower(*in++) : *in++;
	}

	*out = 0;
}

std::string ETJump::sanitize(const std::string& text, bool toLower)
{
	auto len = text.length();
	std::vector<char> out(len + 1);
	SanitizeConstString(text.c_str(), out.data(), toLower ? true : false);
	return std::string(out.data());
}

std::string ETJump::getValue(const char* value, const std::string& defaultValue)
{
	return strlen(value) > 0 ? value : defaultValue;
}

std::string ETJump::getValue(const std::string& value, const std::string& defaultValue)
{
	return value.length() > 0
		? value
		: defaultValue;
}

std::string ETJump::trimStart(std::string input)
{
    auto locale = std::locale("");

    input.erase(input.begin(), std::find_if(input.begin(), input.end(), [&](int ch) {
        return !std::isspace(ch, locale);
    }));

    return input;
}

std::string ETJump::trimEnd(std::string input)
{
    auto locale = std::locale("");
  
    input.erase(std::find_if(input.rbegin(), input.rend(), [&](int ch) {
        return !std::isspace(ch, locale);
    }).base(), input.end());

    return input;
}

std::string ETJump::trim(const std::string& input)
{
    return trimEnd(trimStart(input));
}

std::vector<std::string> ETJump::splitString(std::string &input, char separator, size_t maxLength)
{
	std::vector<std::string> output;
	size_t lastPos = 0;

	if (input.size() <= maxLength)
	{
		output.push_back(input);
		return output;
	}

	while (true) 
	{
		auto pos = input.rfind(separator, lastPos + maxLength);
		/* separator not found */
		if (pos == std::string::npos)
		{
			/* split by length; */
			size_t numSplits = input.size() / maxLength;
			for (size_t i = 1; i <= numSplits; ++i)
			{
				output.push_back(input.substr(lastPos, maxLength));
				lastPos = (maxLength * i);
			}
			break;
		}
		/* no new separators were found */
		if ((pos + 1) == lastPos)
		{
			break;
		}
		output.push_back(input.substr(lastPos, pos - lastPos));
		lastPos = pos + 1;
	}
	/* add last bit if any */
	if (lastPos < input.size())
	{
		output.push_back(input.substr(lastPos));
	}
	return output;
}
