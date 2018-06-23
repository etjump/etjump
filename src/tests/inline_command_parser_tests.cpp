#include <gtest/gtest.h>
#include "../cgame/etj_inline_command_parser.h"

using namespace ETJump;

class InlineCommandParserTests : public testing::Test
{
    void SetUp() override
    {
        
    }

    void TearDown() override
    {
        
    }
};

std::string inQuotes(const std::string& input)
{
    return "\"" + input + "\"";
}

TEST_F(InlineCommandParserTests, Parse_HandlesSingleParamCorrectly)
{
    std::vector<std::string> args{ "say foo" };

    InlineCommandParser parser;
    auto output = parser.parse(args);

    ASSERT_EQ(output.at(0), inQuotes("say foo"));
}

TEST_F(InlineCommandParserTests, Parse_HandlesSeparatorCorrectly)
{
    std::vector<std::string> args{ "say foo", "|", "say bar" };

    InlineCommandParser parser;
    auto output = parser.parse(args);

    ASSERT_EQ(output.at(0), inQuotes("say foo"));
    ASSERT_EQ(output.at(1), inQuotes("say bar"));
}

TEST_F(InlineCommandParserTests, Parse_IgnoresEmptyCommandAfterSeparator)
{
    std::vector<std::string> args{ "say foo", "|" };

    InlineCommandParser parser;
    auto output = parser.parse(args);

    ASSERT_EQ(output.size(), 1);
    ASSERT_EQ(output.at(0), inQuotes("say foo"));
}

TEST_F(InlineCommandParserTests, Parse_IgnoresWhitespaceAroundSeparator)
{
    std::vector<std::string> args{ "say foo", "    |    ", "say bar" };

    InlineCommandParser parser;
    auto output = parser.parse(args);

    ASSERT_EQ(output.at(0), inQuotes("say foo"));
    ASSERT_EQ(output.at(1), inQuotes("say bar"));
}

TEST_F(InlineCommandParserTests, Parse_SupportsMultipleCommands)
{
    std::vector<std::string> args{ "say foo", "|", "say bar", "|", "say kek" };

    InlineCommandParser parser;
    auto output = parser.parse(args);

    ASSERT_EQ(output.at(0), inQuotes("say foo"));
    ASSERT_EQ(output.at(1), inQuotes("say bar"));
    ASSERT_EQ(output.at(2), inQuotes("say kek"));
}