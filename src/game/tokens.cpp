#include "tokens.hpp"
#include <fstream>
#include "Utilities.h"
#include "../json/json.h"


Tokens::Tokens()
{
}


Tokens::~Tokens()
{
}


std::pair<bool, std::string> Tokens::createToken(Difficulty difficulty, std::array<float, 3> coordinates)
{
	std::array<Token, TOKENS_PER_DIFFICULTY> *tokens = nullptr;
	switch (difficulty)
	{
	case Easy:
		tokens = &_easyTokens;
		break;
	case Medium:
		tokens = &_mediumTokens;
		break;
	case Hard:
		tokens = &_hardTokens;
		break;
	}

	Token *nextFreeToken = nullptr;
	for (auto & token : *tokens)
	{
		if (!token.isActive)
		{
			nextFreeToken = &token;
		}
	}

	if (nextFreeToken == nullptr)
	{
		return std::make_pair(false, "no free tokens left for the difficulty.");
	}

	nextFreeToken->isActive = true;
	nextFreeToken->name = "";
	nextFreeToken->coordinates = coordinates;

	if (!saveTokens(_filepath))
	{
		return std::make_pair(false, "Could not save tokens to a file. Check logs for more information.");
	}

	return std::make_pair(true, "");
}

bool Tokens::loadTokens(const std::string& filepath)
{
	_filepath = filepath;
	std::string content;
	try
	{
		content = Utilities::ReadFile(filepath);
	} catch (std::runtime_error& e)
	{
		Utilities::Logln(std::string("Tokens: Could not read file: ") + e.what());
		return false;
	}
	
	Json::Value root;
	Json::Reader reader;

	if (!reader.parse(content, root))
	{
		Utilities::Logln("Tokens: Could not parse file \"" + filepath + "\".");
		Utilities::Logln("Tokens: " + reader.getFormattedErrorMessages());
		return false;
	}

	try
	{
		auto easyTokens = root["easyTokens"];
		auto mediumTokens = root["mediumTokens"];
		auto hardTokens = root["hardTokens"];

		auto idx = 0;
		for (auto&easyToken:easyTokens)
		{
			_easyTokens[idx].fromJson(easyToken);
			++idx;
		}

		idx = 0;
		for (auto&mediumToken:mediumTokens)
		{
			_mediumTokens[idx].fromJson(mediumToken);
			++idx;
		}

		idx = 0;
		for (auto&hardToken:hardTokens)
		{
			_hardTokens[idx].fromJson(hardToken);
			++idx;
		}
	} catch (std::runtime_error& e)
	{
		Utilities::Logln(std::string("Tokens: Could not parse configuration from file \"" + filepath + "\": ") + e.what());
		return false;
	}

	Utilities::Logln("Tokens: Successfully loaded all tokens from \"" + filepath + "\" for current map.");

	return false;
}

bool Tokens::saveTokens(const std::string& filepath)
{
	Json::Value root;
	root["easyTokens"] = Json::arrayValue;
	root["mediumTokens"] = Json::arrayValue;
	root["hardTokens"] = Json::arrayValue;
	for (auto&token:_easyTokens)
	{
		if (token.isActive)
		{
			root["easyTokens"].append(token.toJson());
		}
	}

	for (auto&token:_mediumTokens)
	{
		if (token.isActive)
		{
			root["mediumTokens"].append(token.toJson());
		}
	}

	for (auto&token:_hardTokens)
	{
		if (token.isActive)
		{
			root["hardTokens"].append(token.toJson());
		}
	}

	Json::StyledWriter writer;
	auto output = writer.write(root);
	try
	{
		Utilities::WriteFile(filepath, output);
	} catch (std::runtime_error& e)
	{
		Utilities::Logln(std::string("Tokens: Could not save tokens to file: ") + e.what());
		return false;
	}

	Utilities::Logln("Tokens: Saved all tokens to \"" + filepath + "\"");

	return true;
}

void Tokens::Token::fromJson(const Json::Value& json)
{	
	if (json["coordinates"].size() != 3)
	{
		throw std::runtime_error("Coordinates array should have 3 items.");
	}
	coordinates[0] = json["coordinates"][0].asFloat();
	coordinates[1] = json["coordinates"][1].asFloat();
	coordinates[2] = json["coordinates"][2].asFloat();
	name = json["name"].asString();
	isActive = true;
}

Json::Value Tokens::Token::toJson() const
{
	Json::Value jsonToken;
	jsonToken["coordinates"] = Json::arrayValue;
	jsonToken["coordinates"].append(coordinates[0]);
	jsonToken["coordinates"].append(coordinates[1]);
	jsonToken["coordinates"].append(coordinates[2]);

	jsonToken["name"] = name;

	return jsonToken;
}