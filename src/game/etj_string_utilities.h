#pragma once
#include <string>
#include <vector>

namespace ETJump
{
	std::string hash(const std::string& input);
	std::string newGuid();
	std::string getBestMatch(const std::vector<std::string>& words, const std::string& current);
	std::string sanitize(const std::string& text, bool toLower = false);
}
