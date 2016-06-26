#include <gtest/gtest.h>
#include "../game/etj_commands_parser.h"

using namespace ETJump;

class CommandsParserTests : public testing::Test
{
public:
	void SetUp() override {
		parser = CommandsParser();
	}

	void TearDown() override {
	}

	CommandsParser parser;
};

TEST_F(CommandsParserTests, TestRegularCommandWithNoOptions)
{
	struct CommandsParser::ParseOptions options;

	std::vector<std::string> args = {
		"!throw",
		"player",
		"far"
	};
	auto parsedCommand = CommandsParser::parse(args, options);
	ASSERT_EQ(parsedCommand.command, "!throw");
	ASSERT_EQ(parsedCommand.arguments.size(), 2);
	ASSERT_EQ(parsedCommand.arguments[0], "player");
	ASSERT_EQ(parsedCommand.arguments[1], "far");
	ASSERT_EQ(parsedCommand.options.size(), 0);
}

TEST_F(CommandParserTests, Test)

