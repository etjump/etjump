#include <gtest/gtest.h>
#include "../src/game/etj_command_parser.h"
#include "vector_compare.h"

using namespace ETJump;

class CommandParserTests : public testing::Test {
public:
  void SetUp() override {
  }

  void TearDown() override {
  }

  static CommandParser CreateCommandParser(
      CommandParser::CommandDefinition definition,
      const std::vector<std::string> &args) {
    return {definition, args};
  }

  static std::vector<std::string> Args(std::initializer_list<std::string> il) {
    return std::vector<std::string>{il};
  }

  static std::vector<std::string> CreateDefaultArgs() {
    return {"action", "target", "--token", "singleToken", "parabel",
            "--multitoken", "non", "linear", "action", "type"};
  }

  static CommandParser::CommandDefinition CreateCommandDefinition(
      ) {
    auto def = CommandParser::CommandDefinition();
    def.name = "name";
    def.description = "desc";
    def.options = {};
    return def;
  }
};

TEST_F(CommandParserTests, CommandParser_Returns_CommandWithSingleTokenOption) {
  auto args = Args({"random", "parameter", "--token", "aSingleToken", "with",
                    "some", "additional", "stuff"});

  auto cmd =
      CreateCommandParser(
          CreateCommandDefinition().addOption("t", "token", "desc",
                                              CommandParser::OptionDefinition::Type::Token,
                                              true)
          ,
          args)
      .parse();
  ASSERT_EQ(cmd.options["token"].text, "aSingleToken");
}

TEST_F(CommandParserTests,
       CommandParser_Returns_CorrectAdditionalArgsWithSingleToken) {
  auto args = Args({"--token", "aSingleToken", "with", "some", "additional",
                    "stuff"});
  auto cmd =
      CreateCommandParser(
          CreateCommandDefinition().addOption("t", "token", "desc",
                                              CommandParser::OptionDefinition::Type::Token,
                                              true),
          args)
      .parse();
  ASSERT_TRUE(vectorsAreEqual(args, 2, cmd.extraArgs));
}

TEST_F(CommandParserTests,
       CommandParser_Returns_CorrectAdditionalArgsWithSingleTokenInTheMiddle) {
  auto args = Args({"random", "parameter", "--token", "aSingleToken", "with",
                    "some", "additional", "stuff"});
  auto cmd =
      CreateCommandParser(
          CreateCommandDefinition().addOption("t", "token", "desc",
                                              CommandParser::OptionDefinition::Type::Token,
                                              true)
          ,
          args)
      .parse();
  ASSERT_TRUE(
      vectorsAreEqual({ "random", "parameter", "with", "some", "additional",
        "stuff" }, 0, cmd.extraArgs));
}

TEST_F(CommandParserTests, CommandParser_Returns_MultipleCorrectOptions) {
  auto args = Args({"param1", "param2", "--token", "value", "--boolean", "-boo",
                    "--bar"});
  auto cmd =
      CreateCommandParser(
          CreateCommandDefinition().addOption("t", "token", "desc",
                                              CommandParser::OptionDefinition::Type::Token,
                                              true).addOption(
              "b", "boolean", "desc", CommandParser::OptionDefinition::Type::Boolean,
              true)
          ,
          args)
      .parse();
  ASSERT_EQ(cmd.options.size(), 2);
  ASSERT_NE(cmd.options.find("token"), end(cmd.options));
  ASSERT_NE(cmd.options.find("boolean"), end(cmd.options));
}

TEST_F(CommandParserTests,
       CommandParser_Returns_CorrectArgs_WithMultipleOptions) {
  auto args = Args({"param1", "param2", "--token", "value", "--boolean", "-boo",
                    "--bar"});
  auto cmd =
      CreateCommandParser(
          CreateCommandDefinition().addOption("t", "token", "desc",
                                              CommandParser::OptionDefinition::Type::Token,
                                              true)
                                   .addOption("b", "boolean", "desc",
                                              CommandParser::OptionDefinition::Type::Boolean,
                                              true),
          args)
      .parse();
  ASSERT_TRUE(
      vectorsAreEqual(cmd.extraArgs, 0, { "param1", "param2", "-boo", "--bar" }
      ));
}

TEST_F(CommandParserTests,
       CommandParser_Returns_CorrectOptionValues_WithMultipleOptions) {
  auto args = Args({"param1", "param2", "--token", "value", "--boolean", "-boo",
                    "--bar"});
  auto cmd =
      CreateCommandParser(
          CreateCommandDefinition().addOption("t", "token", "desc",
                                              CommandParser::OptionDefinition::Type::Token,
                                              true)
                                   .addOption(
                                       "b", "boolean", "desc",
                                       CommandParser::OptionDefinition::Type::Boolean,
                                       true),
          args)
      .parse();
  ASSERT_EQ(cmd.options["token"].text, "value");
  ASSERT_EQ(cmd.options["boolean"].boolean, true);
}

TEST_F(CommandParserTests, CommandParser_Returns_CorrectArgs_WithMultiToken) {
  auto args = Args({"param1", "param2", "param3", "--multi", "param4", "param5",
                    "param6", "--token", "param7"});
  auto cmd =
      CreateCommandParser(
          CreateCommandDefinition().addOption("m", "multi", "desc",
                                              CommandParser::OptionDefinition::Type::MultiToken,
                                              true)
                                   .addOption("t", "token", "desc",
                                              CommandParser::OptionDefinition::Type::Token,
                                              true),
          args)
      .parse();
  ASSERT_EQ(cmd.options["multi"].text, "param4 param5 param6");
}

TEST_F(CommandParserTests,
       CommandParser_Returns_CorrectArgsForToken_WithMultiToken) {
  auto args = Args({"param1", "param2", "param3", "--multi", "param4", "param5",
                    "param6", "--token", "param7", "param8"});
  auto cmd =
      CreateCommandParser(
          CreateCommandDefinition().addOption("m", "multi", "desc",
                                              CommandParser::OptionDefinition::Type::MultiToken,
                                              true)
                                   .addOption("t", "token", "desc",
                                              CommandParser::OptionDefinition::Type::Token,
                                              true)
          ,
          args)
      .parse();
  ASSERT_EQ(cmd.options["token"].text, "param7");
}

TEST_F(CommandParserTests,
       CommandParser_Returns_CorrectArgs_WhenMultiTokenHasBeenParsed) {
  auto args = Args({"param1", "param2", "param3", "--multi", "param4", "param5",
                    "param6", "--token", "param7", "param8"});
  auto cmd =
      CreateCommandParser(
          CreateCommandDefinition()
          .addOption("m", "multi", "desc",
                     CommandParser::OptionDefinition::Type::MultiToken, true)
          .addOption("t", "token", "desc",
                     CommandParser::OptionDefinition::Type::Token, true),
          args)
      .parse();
  ASSERT_TRUE(
      vectorsAreEqual(cmd.extraArgs, 0, { "param1", "param2", "param3", "param8"
        }));
}

TEST_F(CommandParserTests, CommandParser_Returns_Error_WhenNoParamForToken) {
  auto args = Args({"--token1"});
  auto cmd = CreateCommandParser(
          CreateCommandDefinition().addOption("t1", "token1", "desc",
                                              CommandParser::OptionDefinition::Type::Token,
                                              true)
          ,
          args)
      .parse();
  ASSERT_EQ(cmd.errors[0], "Missing parameter for `token1`");
}


TEST_F(CommandParserTests,
       CommandParser_ReturnsAnError_IfRequiredOptionIsMissing) {
  auto args = CreateDefaultArgs();
  auto def = CreateCommandDefinition().addOption(
      "o", "opt", "desc", CommandParser::OptionDefinition::Type::Boolean, true);
  auto parser = CreateCommandParser(def, args);
  ASSERT_EQ(parser.parse().errors.size(), 1);
}

TEST_F(CommandParserTests,
       CommandParser_ConsidersOptionsWithoutDefinitionExtraArgs) {
  auto args = std::vector<std::string>{"--extra-arg"};
  auto def = CreateCommandDefinition().addOption(
      "o", "opt", "desc", CommandParser::OptionDefinition::Type::Boolean, true);
  auto parser = CreateCommandParser(def, args);

  ASSERT_EQ(parser.parse().extraArgs[0], "--extra-arg");
}

TEST_F(CommandParserTests, CommandParser_ParsesIntegersCorrectly) {
  auto args = std::vector<std::string>{"--integer", "1"};
  auto def = CreateCommandDefinition().addOption(
      "i", "integer", "desc", CommandParser::OptionDefinition::Type::Integer, true);
  auto parser = CreateCommandParser(def, args);

  ASSERT_EQ(parser.parse().options["integer"].integer, 1);
}

TEST_F(CommandParserTests, CommandParser_ReturnsError_IfIntegerIsTooLarge) {
  auto args = std::vector<std::string>{"--integer", "999999999999999999999999"};
  auto def = CreateCommandDefinition().addOption(
      "i", "integer", "desc", CommandParser::OptionDefinition::Type::Integer, true);
  auto parser = CreateCommandParser(def, args);

  ASSERT_TRUE(
      ETJump::StringUtil::contains(parser.parse().errors[0], "is out of range"
      ));
}

TEST_F(CommandParserTests, CommandParser_ReturnsError_IfParamIsNotInteger) {
  auto args = std::vector<std::string>{"--integer", "foobar"};
  auto def = CreateCommandDefinition().addOption(
      "i", "integer", "desc", CommandParser::OptionDefinition::Type::Integer, true);
  auto parser = CreateCommandParser(def, args);

  ASSERT_TRUE(ETJump::StringUtil::contains(parser.parse().errors[0],
    "is not an integer"));
}

TEST_F(CommandParserTests, CommandParser_ParsesDecimalsCorrectly) {
  auto args = std::vector<std::string>{"--decimal", "1.23"};
  auto def = CreateCommandDefinition().addOption(
      "d", "decimal", "desc", CommandParser::OptionDefinition::Type::Decimal, true);
  auto parser = CreateCommandParser(def, args);

  ASSERT_DOUBLE_EQ(parser.parse().options["decimal"].decimal, 1.23);
}

TEST_F(CommandParserTests, CommandParser_ReturnsError_IfDecimalIsTooLarge) {
  auto args = std::vector<std::string>{"--decimal", "1.79769e+309"};
  auto def = CreateCommandDefinition().addOption(
      "d", "decimal", "desc", CommandParser::OptionDefinition::Type::Decimal, true);
  auto parser = CreateCommandParser(def, args);

  ASSERT_TRUE(ETJump::StringUtil::contains(parser.parse().errors[0],
    "is out of range"));
}

TEST_F(CommandParserTests, CommandParser_ReturnsError_IfParamIsNotDecimal) {
  auto args = std::vector<std::string>{"--decimal", "foobar"};
  auto def = CreateCommandDefinition().addOption(
      "d", "decimal", "desc", CommandParser::OptionDefinition::Type::Decimal, true);
  auto parser = CreateCommandParser(def, args);

  ASSERT_TRUE(ETJump::StringUtil::contains(parser.parse().errors[0],
    "is not a decimal"));
}

TEST_F(CommandParserTests, CommandParser_ParsesDatesCorrectly) {
  auto args = std::vector<std::string>{"--date", "2023-01-05"};
  auto def = CreateCommandDefinition().addOption(
      "d", "date", "desc", CommandParser::OptionDefinition::Type::Date, true);
  auto parser = CreateCommandParser(def, args);

  auto expectedDate = Date{2023, 1, 5};
  ASSERT_TRUE(parser.parse().options["date"].date == expectedDate);
}

TEST_F(CommandParserTests, CommandParser_ReturnsError_IfDateIsNotValid) {
  auto args = std::vector<std::string>{"--date", "foobar"};
  auto def = CreateCommandDefinition().addOption(
      "d", "date", "desc", CommandParser::OptionDefinition::Type::Date, true);
  auto parser = CreateCommandParser(def, args);

  ASSERT_TRUE(
      StringUtil::contains(parser.parse().errors[0],
        "does not match the expected format"));
}

TEST_F(CommandParserTests, CommandParser_HandlesPositionalArgumentsCorrectly) {
  auto args = std::vector<std::string>{"a", "b"};
  auto def =
      CreateCommandDefinition()
          .addOption("fa", "field-a", "desc",
                     CommandParser::OptionDefinition::Type::Token, true, 0)
          .addOption("fb", "field-b", "desc",
                     CommandParser::OptionDefinition::Type::Token, true, 1);

  auto parser = CreateCommandParser(def, args);

  ASSERT_EQ(parser.parse().options["field-a"].text, "a");
  ASSERT_EQ(parser.parse().options["field-b"].text, "b");
}

TEST_F(CommandParserTests, CommandParser_HandlesPositionalArgumentsCorrectly_IfSomeArePassedExplicitly) {
  auto args = std::vector<std::string>{"--field-b", "b", "a"};
  auto def =
      CreateCommandDefinition()
          .addOption("fa", "field-a", "desc",
                     CommandParser::OptionDefinition::Type::Token, true, 0)
          .addOption("fb", "field-b", "desc",
                     CommandParser::OptionDefinition::Type::Token, true, 1);

  auto parser = CreateCommandParser(def, args);

  ASSERT_EQ(parser.parse().options["field-a"].text, "a");
  ASSERT_EQ(parser.parse().options["field-b"].text, "b");
}

TEST_F(CommandParserTests, CommandParser_ShouldNotCrash_IfNoParametersArePassed) {
  auto args = std::vector<std::string>{};
  auto def = CreateCommandDefinition().addOption(
      "fa", "field-a", "desc", CommandParser::OptionDefinition::Type::Token, false, 0);

  auto parser = CreateCommandParser(def, args);

  ASSERT_NO_THROW(parser.parse());
}