#include "etj_utilities.h"
#include <regex>
#include <unordered_map>
#include <sstream>
#include <boost/algorithm/string.hpp>

std::string ETJump::composeShader(const char *name, ShaderStage general, ShaderStages stages)
{
	std::string shader;
	// shader name
	shader = shader + name + " {\n";
	// General Directives
	for (auto &dir : general)
	{
		shader = shader + dir + "\n";
	}
	// Stage Directives
	for (auto &stage : stages)
	{
		shader += "{\n";
		for (auto &dir : stage)
		{
			shader = shader + dir + "\n";
		}
		shader += "}\n";
	}
	shader += "}\n";

	return shader;
}

std::string ETJump::composeShader(const char *name, ShaderStages stages)
{
	return composeShader(name, { "" }, stages);
}

static std::unordered_map<std::string, vec4_t*> validColorNames =
{
	{ "white",    &colorWhite },
	{ "red",      &colorRed },
	{ "green",    &colorGreen },
	{ "blue",     &colorBlue },
	{ "yellow",   &colorYellow },
	{ "magenta",  &colorMagenta },
	{ "cyan",     &colorCyan },
	{ "orange",   &colorOrange },
	{ "mdred",    &colorMdRed },
	{ "mdgreen",  &colorMdGreen },
	{ "dkgreen",  &colorDkGreen },
	{ "mdcyan",   &colorMdCyan },
	{ "mdyellow", &colorMdYellow },
	{ "mdorange", &colorMdOrange },
	{ "mdblue",   &colorMdBlue },
	{ "gray",     &colorMdGrey },
	{ "grey",     &colorMdGrey },
	{ "ltgrey",   &colorLtGrey },
	{ "mdgrey",   &colorMdGrey },
	{ "dkgrey",   &colorDkGrey },
	{ "black",    &colorBlack },
};

static void parseNamedColorString(const std::string &token, vec4_t &color)
{
	auto validColor = validColorNames[token];
	if (validColor)
	{
		color[0] = (*validColor)[0];
		color[1] = (*validColor)[1];
		color[2] = (*validColor)[2];
	}
}

static void parseRGBAValuedColorString(const std::string colorString, vec4_t &color)
{
	std::istringstream tokenStream{ colorString };
	std::string token;
	auto colorChannel = 0;
	while (tokenStream >> token)
	{
		if (colorChannel > 3)
		{
			break;
		}
		auto value = std::min(std::max(std::stof(token), 0.f), 255.f);
		color[colorChannel] = value;
		colorChannel++;
	}
}

static void parseHexValuedColorString(const std::string &token, vec4_t &color)
{
	auto colorValue = std::stoll(token, nullptr, 16);
	auto channelCount = (colorValue >> 24 ? 1 : 0) + (colorValue >> 16 ? 1 : 0) + (colorValue >> 8 ? 1 : 0) + 1;
	auto maxShift = 8 * channelCount;
	for (auto i = 0; i < channelCount; i++)
	{
		color[i] = (colorValue >> (maxShift - 8 * (i + 1))) & 0xff;
	}
}

static void normalizeColorIfRequired(vec4_t &v)
{
	float max = 0.f;
	for (auto i = 0; i < 3; i++)
	{
		max = std::max(v[i], max);
	}

	// non-normalized color
	if (max > 1.0f)
	{
		for (auto i = 0; i < 3; i++)
		{
			v[i] /= 255.f;
		}
	}

	// handle alpha separately
	if (v[3] > 1.0)
	{
		v[3] /= 255.f;
	}
}

const std::string alphaRegex{ "^[a-zA-Z]+" }; // white, black, etc
const std::string digitRegex{ "^([-+]?[0-9]*\\.?[0-9]+\\s*)+" }; // 255 0 0, 1.0 0 0
const std::string hexedRegex{ "^0[xX][a-fA-F0-9]+" }; // 0xff0000
const std::string hashdRegex{ "^#[a-fA-F0-9]+" }; // #ff0000

void ETJump::parseColorString(const std::string &colorString, vec4_t &color)
{	
	Vector4Set(color, 0.0f, 0.0f, 0.0f, 1.0); // set defaults
	std::string token{ boost::algorithm::trim_copy(colorString) };

	if (std::regex_match(token, std::regex(alphaRegex)))
	{
		parseNamedColorString(token, color);
		return;
	}
	else if (std::regex_match(token, std::regex(digitRegex)))
	{
		parseRGBAValuedColorString(token, color);
	}
	else if (std::regex_match(token, std::regex(hexedRegex)))
	{
		parseHexValuedColorString(token.substr(2, 8), color);
	}
	else if (std::regex_match(token, std::regex(hashdRegex)))
	{
		parseHexValuedColorString(token.substr(1, 8), color);
	}

	normalizeColorIfRequired(color);
}