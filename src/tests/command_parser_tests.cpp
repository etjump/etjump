#include <gtest/gtest.h>
#include "../game/etj_command_parser.h"

using namespace ETJump;

class CommandParserTests : public testing::Test
{
public:
	void SetUp() override {
	}

	void TearDown() override {
	}

	CommandParser CreateCommandParser()
	{
		return CommandParser();
	}
};

TEST_F(CommandParserTests, CommandParser_Returns_ParsedText_WhenTypeIsParsedDate)
{
	auto parser = CreateCommandParser();
}