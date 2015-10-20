#ifndef TOKENS_HH
#define TOKENS_HH
#include <string>
#include <array>
#include "../json/json-forwards.h"
#include <memory>

typedef struct gentity_s gentity_t;
typedef struct TokenInformation_s TokenInformation;

class Tokens
{
public:
	enum Difficulty
	{
		Easy,
		Medium,
		Hard
	};

	static const int TOKENS_PER_DIFFICULTY = 6;
	struct Token
	{
		Token();
		std::array<float, 3> coordinates;
		std::string name;
		bool isActive;
		gentity_t *entity;
		// Because we cannot capture values for the entity think lambda
		// we must pass the data as a gentity pointer in gentity
		// Because there is so much data stored storing same data for
		// every entity would be pretty pointles
		// Only tokens have the data
		std::unique_ptr<TokenInformation> data;
		Json::Value toJson() const;
		void fromJson(const Json::Value& json);
	};
	Tokens();
	~Tokens();


	std::pair<bool, std::string> createToken(Difficulty difficulty, std::array<float, 3> coordinates);
	struct NearestToken
	{
		int number;
		Tokens::Token *token;
		float distance;
		Difficulty difficulty;
	};
	NearestToken findNearestToken(std::array<float, 3> coordinates);
	std::pair<bool, std::string> moveNearestToken(std::array<float, 3> coordinates);
	std::pair<bool, std::string> deleteNearestToken(std::array<float, 3> coordinates);
	std::pair<bool, std::string> deleteToken(Difficulty difficulty, int index);

	bool loadTokens(const std::string& filepath);
	bool saveTokens(const std::string& filepath);
	void createEntity(Token& token, Difficulty difficulty);
	void createEntities();
	void reset();
	std::array<int, 3> getTokenCounts() const;
private:
	std::string _filepath;
	std::array<Token, TOKENS_PER_DIFFICULTY> _easyTokens;
	std::array<Token, TOKENS_PER_DIFFICULTY> _mediumTokens;
	std::array<Token, TOKENS_PER_DIFFICULTY> _hardTokens;
};
#endif