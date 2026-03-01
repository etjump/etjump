#include <gtest/gtest.h>
#include "../src/game/etj_string_utilities.h"

using namespace ETJump;

class StringUtilitiesTests : public testing::Test {
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(StringUtilitiesTests,
       trimStart_ShouldRemoveWhitespaceFromTheBeginingOfString) {
  std::string inputStr{"  foo bar"};
  std::string expectedStr{"foo bar"};

  auto outputStr = trimStart(inputStr);

  EXPECT_EQ(outputStr, expectedStr);
}

TEST_F(StringUtilitiesTests, trimEnd_ShouldRemoveWhitespaceFromTheEndOfString) {
  std::string inputStr{"foo bar  "};
  std::string expectedStr{"foo bar"};

  auto outputStr = trimEnd(inputStr);

  EXPECT_EQ(outputStr, expectedStr);
}

TEST_F(StringUtilitiesTests,
       trim_ShouldRemoveWhitespaceFromBothStartAndEndOfString) {
  std::string inputStr{"   foo bar  "};
  std::string expectedStr{"foo bar"};

  auto outputStr = trim(inputStr);

  EXPECT_EQ(outputStr, expectedStr);
}

TEST_F(StringUtilitiesTests,
       wrapWords_ShouldEffectivelySplitStringOnSeparatorEncounter) {
  std::string input = "Lorem ipsum \ndolor sit amet, \nconsectetur "
                      "\nadipisicing elit. \nTenetur, fuga!";
  std::vector<std::string> expectedSplits{"Lorem ipsum \ndolor sit amet, \n",
                                          "consectetur \nadipisicing elit. \n",
                                          "Tenetur, fuga!"};
  auto splits = wrapWords(input, '\n', 40);
  EXPECT_EQ(splits.size(), expectedSplits.size());
  for (int i = 0; i < static_cast<int>(splits.size()); i++) {
    EXPECT_EQ(splits[i], expectedSplits[i]);
  }
}

TEST_F(StringUtilitiesTests,
       wrapWords_ShouldEffectivelySplitStringOnMaxWidthExceed) {
  std::string input = "Lorem ipsum dolor sit amet, consectetur adipisicing "
                      "elit. Tenetur, fuga!";
  std::vector<std::string> expectedSplits{
      "Lorem ipsum dolor sit amet, consectetur ",
      "adipisicing elit. Tenetur, fuga!"};
  auto splits = wrapWords(input, '\n', 40);
  EXPECT_EQ(splits.size(), expectedSplits.size());
  for (int i = 0; i < static_cast<int>(splits.size()); i++) {
    EXPECT_EQ(splits[i], expectedSplits[i]);
  }
}

TEST_F(StringUtilitiesTests,
       wrapWords_SplitShouldNotExceedMaxLengthIfLandingOnSeparator) {
  std::string input = "Lorem ipsum \ndolor sit amet, \nconsectetur "
                      "\nadipisicing elit. \nTenetur, fuga!";
  std::vector<std::string> expectedSplits{
      "Lorem ipsum \ndolor sit amet, \n", "consectetur \n",
      "adipisicing elit. \n", "Tenetur, fuga!"};
  auto splits = wrapWords(input, '\n', 31);
  EXPECT_EQ(splits.size(), expectedSplits.size());
  for (int i = 0; i < static_cast<int>(splits.size()); i++) {
    EXPECT_EQ(splits[i], expectedSplits[i]);
  }
}

TEST_F(StringUtilitiesTests,
       toLowerCase_ShouldConvertStringIntoLowercasedCopy) {
  std::string input = "HELLO WORLD";
  auto fixedString = StringUtil::toLowerCase(input);
  EXPECT_EQ(fixedString, "hello world");
}

TEST_F(StringUtilitiesTests,
       toUpperCase_ShouldConvertStringIntoUppercasedCopy) {
  std::string input = "hello world";
  auto fixedString = StringUtil::toUpperCase(input);
  EXPECT_EQ(fixedString, "HELLO WORLD");
}

TEST_F(StringUtilitiesTests,
       eraseLast_ShouldEraseLastSubstringOccurenceFromInputStringCopy) {
  std::string input = "hello world, hello world";
  auto fixedString = StringUtil::eraseLast(input, "hello");
  EXPECT_EQ(fixedString, "hello world,  world");
}

TEST_F(StringUtilitiesTests,
       join_ShouldConcatenateStringChunksIntoOneDelimitedPiece) {
  std::vector<std::string> input = {"hello world", "hello world"};
  auto fixedString = StringUtil::join(input, ", ");
  EXPECT_EQ(fixedString, "hello world, hello world");
}

TEST_F(StringUtilitiesTests, countExtraPadding_ShouldWorkCorrectly) {
  EXPECT_EQ(StringUtil::countExtraPadding("123", 10), 10);
  EXPECT_EQ(StringUtil::countExtraPadding("^1123", 10), 12);
  EXPECT_EQ(StringUtil::countExtraPadding("^^1123", 10), 12);
  EXPECT_EQ(StringUtil::countExtraPadding("^1t^2e^3s^4t", 10), 18);
}

TEST_F(StringUtilitiesTests, iEqual_ShouldWorkCorrectly) {
  // basic case-insensitive comparison
  EXPECT_EQ(StringUtil::iEqual("FOO", "foo"), true);
  EXPECT_EQ(StringUtil::iEqual("FOO", "bar"), false);

  // case-insensitive comparison with sanitization
  EXPECT_EQ(StringUtil::iEqual("FOO", "^1foo", true), true);
  EXPECT_EQ(StringUtil::iEqual("^2FOO", "^1foo", true), true);
  EXPECT_EQ(StringUtil::iEqual("^2FOO", "^1bar", true), false);

  // case-insensitive comparison without sanitization
  EXPECT_EQ(StringUtil::iEqual("FOO", "^1foo"), false);

  // empty strings comparison
  EXPECT_EQ(StringUtil::iEqual("", ""), true);
  EXPECT_EQ(StringUtil::iEqual("", "foo"), false);
  EXPECT_EQ(StringUtil::iEqual("foo", ""), false);

  // different lengths
  EXPECT_EQ(StringUtil::iEqual("FOO", "FOOBAR"), false);
  EXPECT_EQ(StringUtil::iEqual("FOOBAR", "FOO"), false);

  // mixed case with color codes
  EXPECT_EQ(StringUtil::iEqual("^1FoO", "^2fOo", true), true);
  EXPECT_EQ(StringUtil::iEqual("^1FoO", "^2BaR", true), false);

  // no sanitization parameter specified
  // (default behavior should be case-insensitive)
  EXPECT_EQ(StringUtil::iEqual("FoO", "foO"), true);
  EXPECT_EQ(StringUtil::iEqual("FoO", "BaR"), false);
}

TEST_F(StringUtilitiesTests, normalizeNumberString_ValidInput) {
  EXPECT_EQ(StringUtil::normalizeNumberString("123"), "123");
  EXPECT_EQ(StringUtil::normalizeNumberString("123.456"), "123.456");
  EXPECT_EQ(StringUtil::normalizeNumberString("-123"), "-123");
  EXPECT_EQ(StringUtil::normalizeNumberString("-123.456"), "-123.456");
  EXPECT_EQ(StringUtil::normalizeNumberString("1e10"), "10000000000");
  EXPECT_EQ(StringUtil::normalizeNumberString("1e-10"), "0.0000000001");
  EXPECT_EQ(StringUtil::normalizeNumberString("123.4560000"), "123.456");
}

TEST_F(StringUtilitiesTests, normalizeNumberString_EdgeCases) {
  EXPECT_EQ(StringUtil::normalizeNumberString(""), "");
  EXPECT_EQ(StringUtil::normalizeNumberString("0"), "0");
  EXPECT_EQ(StringUtil::normalizeNumberString("0.0"), "0");
  EXPECT_EQ(StringUtil::normalizeNumberString("-0"), "-0");
  EXPECT_EQ(StringUtil::normalizeNumberString("0.0000000001"), "0.0000000001");
  EXPECT_EQ(StringUtil::normalizeNumberString("-0.0000000001"),
            "-0.0000000001");
}

TEST_F(StringUtilitiesTests, normalizeNumberString_InvalidInput) {
  EXPECT_EQ(StringUtil::normalizeNumberString("abc"), "");
  EXPECT_EQ(StringUtil::normalizeNumberString("123abc"), "123");
  EXPECT_EQ(StringUtil::normalizeNumberString("123.456.789"), "123.456");
  EXPECT_EQ(StringUtil::normalizeNumberString("."), "");
}

TEST_F(StringUtilitiesTests, normalizeNumberString_PrecisionAndRounding) {
  EXPECT_EQ(StringUtil::normalizeNumberString("123.4567890123456789"),
            "123.4567890123");
  // rounds up
  EXPECT_EQ(StringUtil::normalizeNumberString("123.45678901235"),
            "123.4567890124");
}

TEST_F(StringUtilitiesTests,
       normalizeNumberString_LeadingZerosAndDecimalPoint) {
  EXPECT_EQ(StringUtil::normalizeNumberString("0123.4500"), "123.45");
  EXPECT_EQ(StringUtil::normalizeNumberString("0123.0"), "123");
}

TEST_F(StringUtilitiesTests,
       normalizeNumberString_TrailingZerosAndDecimalPoint) {
  EXPECT_EQ(StringUtil::normalizeNumberString("123.4500"), "123.45");
  EXPECT_EQ(StringUtil::normalizeNumberString("123.0"), "123");
}

TEST_F(StringUtilitiesTests, removeTrailingChars_NoTrailingChars) {
  std::string input = "aaa";
  ETJump::StringUtil::removeTrailingChars(input, ' ');
  EXPECT_EQ(input, "aaa");

  input = "aaa aaa";
  ETJump::StringUtil::removeTrailingChars(input, ' ');
  EXPECT_EQ(input, "aaa aaa");
}

TEST_F(StringUtilitiesTests, removeTrailingChars_WithTrailingChars) {
  std::string input = "aaa   ";
  ETJump::StringUtil::removeTrailingChars(input, ' ');
  EXPECT_EQ(input, "aaa");
}

TEST_F(StringUtilitiesTests, removeTrailingChars_AllCharsToRemove) {
  std::string input = "   ";
  ETJump::StringUtil::removeTrailingChars(input, ' ');
  EXPECT_TRUE(input.empty());
}

TEST_F(StringUtilitiesTests, removeTrailingChars_EmptyString) {
  std::string input;
  ETJump::StringUtil::removeTrailingChars(input, ' ');
  EXPECT_TRUE(input.empty());
}

TEST_F(StringUtilitiesTests, removeLeadingChars_NoLeadingChars) {
  std::string input = "aaa";
  ETJump::StringUtil::removeLeadingChars(input, ' ');
  EXPECT_EQ(input, "aaa");

  input = "aaa aaa";
  ETJump::StringUtil::removeLeadingChars(input, ' ');
  EXPECT_EQ(input, "aaa aaa");
}

TEST_F(StringUtilitiesTests, removeLeadingChars_WithLeadingChars) {
  std::string input = "   aaa";
  ETJump::StringUtil::removeLeadingChars(input, ' ');
  EXPECT_EQ(input, "aaa");
}

TEST_F(StringUtilitiesTests, removeLeadingChars_AllCharsToRemove) {
  std::string input = "   ";
  ETJump::StringUtil::removeLeadingChars(input, ' ');
  EXPECT_TRUE(input.empty());
}

TEST_F(StringUtilitiesTests, removeLeadingChars_EmptyString) {
  std::string input{};
  ETJump::StringUtil::removeLeadingChars(input, ' ');
  EXPECT_TRUE(input.empty());
}

TEST_F(StringUtilitiesTests, sortStrings_SortNoCase) {
  std::vector<std::string> vec = {"A", "c", "d", "B"};
  ETJump::StringUtil::sortStrings(vec, true);

  ASSERT_EQ(vec[0], "A");
  ASSERT_EQ(vec[1], "B");
  ASSERT_EQ(vec[2], "c");
  ASSERT_EQ(vec[3], "d");
}

TEST_F(StringUtilitiesTests, sortStrings_SortNoCase_DuplicatesKeepOrder) {
  std::vector<std::string> vec = {"a", "B", "A", "b"};
  ETJump::StringUtil::sortStrings(vec, true);

  ASSERT_EQ(vec[0], "a");
  ASSERT_EQ(vec[1], "A");
  ASSERT_EQ(vec[2], "B");
  ASSERT_EQ(vec[3], "b");
}

TEST_F(StringUtilitiesTests, sortStrings_SortWithCase) {
  std::vector<std::string> vec = {"AB", "Ab", "aA", "aC", "AA", "Ba"};
  ETJump::StringUtil::sortStrings(vec, false);

  ASSERT_EQ(vec[0], "AA");
  ASSERT_EQ(vec[1], "AB");
  ASSERT_EQ(vec[2], "Ab");
  ASSERT_EQ(vec[3], "Ba");
  ASSERT_EQ(vec[4], "aA");
  ASSERT_EQ(vec[5], "aC");
}

TEST_F(StringUtilitiesTests, truncate_BasicTruncate) {
  const std::string in = "^1test^2test";

  ASSERT_EQ(StringUtil::truncate(in, 4), "^1test");
  ASSERT_EQ(StringUtil::truncate(in, 2), "^1te");
  ASSERT_EQ(StringUtil::truncate(in, 8), "^1test^2test");
  ASSERT_EQ(StringUtil::truncate(in, 10), "^1test^2test");
  ASSERT_EQ(StringUtil::truncate(in, 11), "^1test^2test");
  ASSERT_EQ(StringUtil::truncate(in, 12), "^1test^2test");
  ASSERT_EQ(StringUtil::truncate(in, 13), "^1test^2test");
}

TEST_F(StringUtilitiesTests, truncate_InvalidInput) {
  const std::string in = "^1test^2test";

  ASSERT_EQ(StringUtil::truncate(in, -1), "^1test^2test");
  ASSERT_EQ(StringUtil::truncate("", 10), "");
}

TEST_F(StringUtilitiesTests, truncate_LenIsInColorCode) {
  const std::string in = "^1test^2test";

  ASSERT_EQ(StringUtil::truncate(in, 5), "^1test^2t");
  ASSERT_EQ(StringUtil::truncate(in, 6), "^1test^2te");
}

TEST_F(StringUtilitiesTests, truncate_MultipleCarets) {
  const std::string in = "^^1test^^^2test";

  ASSERT_EQ(StringUtil::truncate(in, 1), "^");
  ASSERT_EQ(StringUtil::truncate(in, 3), "^^1te");
  ASSERT_EQ(StringUtil::truncate(in, 6), "^^1test^");
  ASSERT_EQ(StringUtil::truncate(in, 7), "^^1test^^");
  ASSERT_EQ(StringUtil::truncate(in, 9), "^^1test^^^2te");
}

TEST_F(StringUtilitiesTests, sanitize_basicSanitize) {
  ASSERT_EQ(sanitize("^1test ^2test"), "test test");
  ASSERT_EQ(sanitize("test TEST"), "test TEST");
  ASSERT_EQ(sanitize("test^1 TEST"), "test TEST");
  ASSERT_EQ(sanitize("test test^1"), "test test");
}

TEST_F(StringUtilitiesTests, sanitize_lowercasesCorrectly) {
  ASSERT_EQ(sanitize("^1TEST ^2test", true), "test test");
  ASSERT_EQ(sanitize("Test TEST", true), "test test");
  ASSERT_EQ(sanitize("TEST^1 teST", true), "test test");
  ASSERT_EQ(sanitize("TEST TeSt^1", true), "test test");
}

TEST_F(StringUtilitiesTests, sanitize_handlesControlCharacters) {
  const std::string in = "^1te\x19=st ^2TEST";

  ASSERT_EQ(sanitize(in), "te=st TEST");
  ASSERT_EQ(sanitize(in, true, false), "te\x19=st test");
}

TEST_F(StringUtilitiesTests, sanitize_multipleCarets) {
  const std::string in = "^^1test ^^^2TEST";

  ASSERT_EQ(sanitize(in), "^test ^^TEST");
  ASSERT_EQ(sanitize(in, true), "^test ^^test");
}

TEST_F(StringUtilitiesTests, sanitize_handlesEndingCarets) {
  ASSERT_EQ(sanitize("test test^"), "test test^");
  ASSERT_EQ(sanitize("test test^^"), "test test^^");
}

TEST_F(StringUtilitiesTests, stripExtension_stripsCorrectly) {
  std::string in = "test.dat";
  StringUtil::stripExtension(in);

  ASSERT_EQ(in, "test");
}

TEST_F(StringUtilitiesTests, stripExtension_handlesEmptyString) {
  std::string in = "";
  StringUtil::stripExtension(in);

  ASSERT_EQ(in, "");
}

TEST_F(StringUtilitiesTests, stripExtension_stripsOnlyLastExt) {
  std::string in = "test.dat.dat";
  StringUtil::stripExtension(in);

  ASSERT_EQ(in, "test.dat");
}

TEST_F(StringUtilitiesTests, stripExtension_noExtIsUnmodified) {
  std::string in = "test";
  StringUtil::stripExtension(in);

  ASSERT_EQ(in, "test");
}
