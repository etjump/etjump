#include <gtest/gtest.h>
#include "../src/game/etj_command_parser.h"
#include "vector_compare.h"

using namespace ETJump;

class CommandParserTests : public testing::Test
{
public:
	void SetUp() override
	{
	}

	void TearDown() override
	{
	}

	static CommandParser CreateCommandParser()
	{
		return CommandParser();
	}

	static std::vector<std::string> Args(std::initializer_list<std::string> il)
	{
		return std::vector<std::string>{il};
	}

	static std::vector<std::string> CreateDefaultArgs()
	{
		return {"action", "target", "--token", "singleToken", "parabel", "--multitoken", "non", "linear", "action", "type"};
	}

	static CommandParser::CommandDefinition CreateCommandDefinition(std::string name, std::string desc, std::map<std::string, CommandParser::OptionDefinition> opts)
	{
		auto def = CommandParser::CommandDefinition();
		def.name = name;
		def.description = desc;
		def.options = opts;
		return def;
	}

	static CommandParser::OptionDefinition CreateOptionDefinition(std::string name, std::string desc, CommandParser::OptionDefinition::Type type, bool required)
	{
		auto opt = CommandParser::OptionDefinition();
		opt.name = name;
		opt.description = desc;
		opt.required = required;
		opt.type = type;
		return opt;
	}
};

TEST_F(CommandParserTests, CommandParser_Returns_CommandWithSingleTokenOption)
{
	auto args = Args({"random", "parameter", "--token", "aSingleToken", "with", "some", "additional", "stuff"});
	auto cmd = CreateCommandParser().parse(CreateCommandDefinition("cmd", "desc", {
		{"token", CreateOptionDefinition("token", "desc", CommandParser::OptionDefinition::Type::Token, true)}
	}), args);
	ASSERT_EQ(cmd.options["token"].text, "aSingleToken");
}

TEST_F(CommandParserTests, CommandParser_Returns_CorrectAdditionalArgsWithSingleToken)
{
	auto args = Args({"--token", "aSingleToken", "with", "some", "additional", "stuff"});
	auto cmd = CreateCommandParser().parse(CreateCommandDefinition("cmd", "desc", {
		{"token", CreateOptionDefinition("token", "desc", CommandParser::OptionDefinition::Type::Token, true)}
	}), args);
	ASSERT_TRUE(vectorsAreEqual(args, 2, cmd.extraArgs));
}

TEST_F(CommandParserTests, CommandParser_Returns_CorrectAdditionalArgsWithSingleTokenInTheMiddle)
{
	auto args = Args({"random", "parameter", "--token", "aSingleToken", "with", "some", "additional", "stuff"});
	auto cmd = CreateCommandParser().parse(CreateCommandDefinition("cmd", "desc", {
		{"token", CreateOptionDefinition("token", "desc", CommandParser::OptionDefinition::Type::Token, true)}
	}), args);
	ASSERT_TRUE(vectorsAreEqual({ "random", "parameter", "with", "some", "additional", "stuff" }, 0, cmd.extraArgs));
}

TEST_F(CommandParserTests, CommandParser_Returns_MultipleCorrectOptions)
{
	auto args = Args({"param1", "param2", "--token", "value", "--boolean", "-boo", "--bar"});
	auto cmd = CreateCommandParser().parse(CreateCommandDefinition("cmd", "desc", {
		{"token", CreateOptionDefinition("token", "desc", CommandParser::OptionDefinition::Type::Token, true)},
		{"boolean", CreateOptionDefinition("boolean", "desc", CommandParser::OptionDefinition::Type::Boolean, true)}
	}), args);
	ASSERT_EQ(cmd.options.size(), 2);
	ASSERT_NE(cmd.options.find("token"), end(cmd.options));
	ASSERT_NE(cmd.options.find("boolean"), end(cmd.options));
}

TEST_F(CommandParserTests, CommandParser_Returns_CorrectArgs_WithMultipleOptions)
{
	auto args = Args({"param1", "param2", "--token", "value", "--boolean", "-boo", "--bar"});
	auto cmd = CreateCommandParser().parse(CreateCommandDefinition("cmd", "desc", {
		{"token", CreateOptionDefinition("token", "desc", CommandParser::OptionDefinition::Type::Token, true)},
		{"boolean", CreateOptionDefinition("boolean", "desc", CommandParser::OptionDefinition::Type::Boolean, true)}
	}), args);
	ASSERT_TRUE(vectorsAreEqual(cmd.extraArgs, 0, { "param1", "param2", "-boo", "--bar" }));
}

TEST_F(CommandParserTests, CommandParser_Returns_CorrectOptionValues_WithMultipleOptions)
{
	auto args = Args({"param1", "param2", "--token", "value", "--boolean", "-boo", "--bar"});
	auto cmd = CreateCommandParser().parse(CreateCommandDefinition("cmd", "desc", {
		{"token", CreateOptionDefinition("token", "desc", CommandParser::OptionDefinition::Type::Token, true)},
		{"boolean", CreateOptionDefinition("boolean", "desc", CommandParser::OptionDefinition::Type::Boolean, true)}
	}), args);
	ASSERT_EQ(cmd.options["token"].text, "value");
	ASSERT_EQ(cmd.options["boolean"].boolean, true);
}

TEST_F(CommandParserTests, CommandParser_Returns_CorrectArgs_WithMultiToken)
{
	auto args = Args({"param1", "param2", "param3", "--multi", "param4", "param5", "param6", "--token", "param7"});
	auto cmd = CreateCommandParser().parse(CreateCommandDefinition("cmd", "desc", {
		{"multi", CreateOptionDefinition("multi", "desc", CommandParser::OptionDefinition::Type::MultiToken, true)},
		{"token", CreateOptionDefinition("token", "desc", CommandParser::OptionDefinition::Type::Token, true)}
	}), args);
	ASSERT_EQ(cmd.options["multi"].text, "param4 param5 param6");
}

TEST_F(CommandParserTests, CommandParser_Returns_CorrectArgsForToken_WithMultiToken)
{
	auto args = Args({"param1", "param2", "param3", "--multi", "param4", "param5", "param6", "--token", "param7", "param8"});
	auto cmd = CreateCommandParser().parse(CreateCommandDefinition("cmd", "desc", {
		{"multi", CreateOptionDefinition("multi", "desc", CommandParser::OptionDefinition::Type::MultiToken, true)},
		{"token", CreateOptionDefinition("token", "desc", CommandParser::OptionDefinition::Type::Token, true)}
	}), args);
	ASSERT_EQ(cmd.options["token"].text, "param7");
}

TEST_F(CommandParserTests, CommandParser_Returns_CorrectArgs_WhenMultiTokenHasBeenParsed)
{
	auto args = Args({"param1", "param2", "param3", "--multi", "param4", "param5", "param6", "--token", "param7", "param8"});
	auto cmd = CreateCommandParser().parse(CreateCommandDefinition("cmd", "desc", {
		{"multi", CreateOptionDefinition("multi", "desc", CommandParser::OptionDefinition::Type::MultiToken, true)},
		{"token", CreateOptionDefinition("token", "desc", CommandParser::OptionDefinition::Type::Token, true)}
	}), args);
	ASSERT_TRUE(vectorsAreEqual(cmd.extraArgs, 0, { "param1", "param2", "param3", "param8" }));
}

TEST_F(CommandParserTests, CommandParser_Returns_EmptyText_WhenConsequentTokens)
{
	auto args = Args({ "param1", "--token1", "--token2", "param2", "param3", "--multi", "param4", "param5", "param6", "--token", "param7", "param8" });
	auto cmd = CreateCommandParser().parse(CreateCommandDefinition("cmd", "desc", {
		{ "token1", CreateOptionDefinition("token1", "desc", CommandParser::OptionDefinition::Type::Token, true) },
		{ "token2", CreateOptionDefinition("token2", "desc", CommandParser::OptionDefinition::Type::Token, true) }
	}), args);
	ASSERT_EQ(cmd.options["token1"].text, "");
}

TEST_F(CommandParserTests, CommandParser_Returns_EmptyText_WhenConsequentMultiTokens)
{
	auto args = Args({ "param1", "--token1", "--token2", "param2", "param3", "--multi", "param4", "param5", "param6", "--token", "param7", "param8" });
	auto cmd = CreateCommandParser().parse(CreateCommandDefinition("cmd", "desc", {
		{ "token1", CreateOptionDefinition("token1", "desc", CommandParser::OptionDefinition::Type::MultiToken, true) },
		{ "token2", CreateOptionDefinition("token2", "desc", CommandParser::OptionDefinition::Type::MultiToken, true) }
	}), args);
	ASSERT_EQ(cmd.options["token1"].text, "");
}


TEST_F(CommandParserTests, CommandParser_Throws_IfRequiredOptionIsMissing)
{
	auto args = CreateDefaultArgs();
	auto command = CreateCommandDefinition("cmd", "desc", {{"opt", CreateOptionDefinition("opt", "desc", CommandParser::OptionDefinition::Type::Boolean, true)}});
	auto parser = CreateCommandParser();
	ASSERT_THROW(parser.parse(command, args), std::runtime_error);
}

