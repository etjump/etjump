#include <gtest/gtest.h>
#include "../game/etj_parser.h"

class ParserTests : testing::Test
{
public:
	void SetUp() override {
	}

	void TearDown() override {
	}

};

const std::string commonTestString =
"// comment"
"[level]"
"level = 0"
"name = Visitor"
"cmds = a"
"greeting = Welcome Visitor [n]^7! Your last visit was on [t]!"
""
"/* comment */"
"[level]"
"level = 1"
"name = Friend"
"cmds = a"
"greeting = Welcome Friend [n]^7! Your last visit was [d] ago!"
""
"//"
"[level]"
"level = 2"
"name = Moderator"
"cmds = *-As"
"greeting = Welcome Moderator [n]^7!"
""
"[level]"
"level = 3"
"name = Administrator"
"cmds = *"
"greeting = Welcome Administrator [n]^7!";

TEST_F(ParserTests, ParserIgnoresDoubleSlashComments)
{
	ETJump::Tokenizer parser(commonTestString);
	auto firstToken = parser.getToken();
	ASSERT_EQ(firstToken, "[level]");
}

TEST_F(ParserTests, ParserIgnoresSlashStarComments)
{
	ETJump::Tokenizer parser(commonTestString);
	std::vector<std::string> tokens{ parser.getToken(), parser.getToken() };
	ASSERT_EQ(firstToken, "[level]");
}