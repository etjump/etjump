#ifndef TOKENS_HH
#define TOKENS_HH
#include <string>
#include <array>
#include "../json/json-forwards.h"

typedef struct gentity_s gentity_t;

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
		Token() : coordinates{0,0,0}, name(""), isActive(false) {}
		std::array<float, 3> coordinates;
		std::string name;
		bool isActive;
		gentity_t *entity;
		Json::Value toJson() const;
		void fromJson(const Json::Value& json);
	};
	Tokens();
	~Tokens();


	std::pair<bool, std::string> createToken(Difficulty difficulty, std::array<float, 3> coordinates);

	bool loadTokens(const std::string& filepath);
	bool saveTokens(const std::string& filepath);
	bool createEntity(Token& token, Difficulty difficulty);
	bool createEntities();
private:
	std::string _filepath;
	std::array<Token, TOKENS_PER_DIFFICULTY> _easyTokens;
	std::array<Token, TOKENS_PER_DIFFICULTY> _mediumTokens;
	std::array<Token, TOKENS_PER_DIFFICULTY> _hardTokens;
};
#endif