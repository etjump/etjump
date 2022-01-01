#include <gtest/gtest.h>
#include "../src/game/etj_string_utilities.h"

using namespace ETJump;

class StringUtilitiesTests : public testing::Test
{
    void SetUp() override
    {
        
    }

    void TearDown() override
    {
        
    }
};

TEST_F(StringUtilitiesTests, trimStart_ShouldRemoveWhitespaceFromTheBeginingOfString)
{
    std::string inputStr{ "  foo bar" };
    std::string expectedStr{ "foo bar" };

    auto outputStr = trimStart(inputStr);
   	
    EXPECT_EQ(outputStr, expectedStr);
}

TEST_F(StringUtilitiesTests, trimEnd_ShouldRemoveWhitespaceFromTheEndOfString)
{
    std::string inputStr{ "foo bar  " };
    std::string expectedStr{ "foo bar" };

    auto outputStr = trimEnd(inputStr);
   	
    EXPECT_EQ(outputStr, expectedStr);
}

TEST_F(StringUtilitiesTests, trim_ShouldRemoveWhitespaceFromBothStartAndEndOfString)
{
    std::string inputStr{ "   foo bar  " };
    std::string expectedStr{ "foo bar" };

    auto outputStr = trim(inputStr);
   	
    EXPECT_EQ(outputStr, expectedStr);
}

TEST_F(StringUtilitiesTests, splitString_ShouldEffectivelySplitStringOnSeparatorEncounter)
{
    std::string input = "Lorem ipsum \ndolor sit amet, \nconsectetur \nadipisicing elit. \nTenetur, fuga!";
    std::vector<std::string> expectedSplits { 
    	"Lorem ipsum \ndolor sit amet, \n", "consectetur \nadipisicing elit. \n", "Tenetur, fuga!" };
	auto splits = splitString(input, '\n', 40);
   	EXPECT_EQ(splits.size(), expectedSplits.size());
   	for (int i = 0; i < static_cast<int>(splits.size()); i++) 
	{
	    EXPECT_EQ(splits[i], expectedSplits[i]);
	}
}

TEST_F(StringUtilitiesTests, splitString_ShouldEffectivelySplitStringOnMaxWidthExceed)
{
    std::string input = "Lorem ipsum dolor sit amet, consectetur adipisicing elit. Tenetur, fuga!";
    std::vector<std::string> expectedSplits { 
		"Lorem ipsum dolor sit amet, consectetur ", "adipisicing elit. Tenetur, fuga!"};
	auto splits = splitString(input, '\n', 40);
   	EXPECT_EQ(splits.size(), expectedSplits.size());
   	for (int i = 0; i < static_cast<int>(splits.size()); i++) 
	{
	    EXPECT_EQ(splits[i], expectedSplits[i]);
	}
}

TEST_F(StringUtilitiesTests, toLowerCase_ShouldConvertStringIntoLowercasedCopy)
{
    std::string input = "HELLO WORLD";
    auto fixedString = StringUtil::toLowerCase(input);
    EXPECT_EQ(fixedString, "hello world");
}

TEST_F(StringUtilitiesTests, toLowerCase_ShouldConvertStringIntoUppercasedCopy)
{
    std::string input = "hello world";
    auto fixedString = StringUtil::toUpperCase(input);
    EXPECT_EQ(fixedString, "HELLO WORLD");
}

TEST_F(StringUtilitiesTests, eraseLast_ShouldEraseLastSubstringOccurenceFromInputStringCopy)
{
    std::string input = "hello world, hello world";
    auto fixedString = StringUtil::eraseLast(input, "hello");
    EXPECT_EQ(fixedString, "hello world,  world");
}

TEST_F(StringUtilitiesTests, join_ShouldConcatenateStringChunksIntoOneDelimitedPiece)
{
    std::vector<std::string> input = { "hello world", "hello world" };
    auto fixedString = StringUtil::join(input, ", ");
    EXPECT_EQ(fixedString, "hello world, hello world");
}
