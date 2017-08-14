#include "etj_string_utilities.h"
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