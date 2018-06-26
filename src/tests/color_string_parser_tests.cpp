#include <gtest/gtest.h>
#include "../cgame/etj_utilities.h"
#include <string>

using namespace ETJump;

class ColorStringParsingTests : public testing::Test
{
public:
	void SetUp() override {
	}

	void TearDown() override {
	}

	static bool Vector4Compare(const vec4_t &v1, const vec4_t &v2)
	{
		if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2] || v1[3] != v2[3])
		{
			return false;
		}
		return true;
	}
};

TEST_F(ColorStringParsingTests, parseColorString_ShouldParseNamedColorString)
{
	const std::string colorString{ "yellow" };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, colorYellow));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParseNamedColorStringAndIgnoreWhiteSpace)
{
	const std::string colorString{ "   yellow   " };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, colorYellow));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParseWrongNamedColorStringAndSetBlack)
{
	const std::string colorString{ "yellowish" };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, colorBlack));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParseUppserCasedNamedColorString)
{
	const std::string colorString{ "YELLOW" };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, colorYellow));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_RGBUpperCasedHexColorString)
{
	const std::string colorString{ "0XAABBCC" };
	vec4_t expectedColor{ 0xaa / 255.f, 0xbb / 255.f, 0xcc / 255.f, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_RGBLowerCasedHexColorString)
{
	const std::string colorString{ "0xaabbcc" };
	vec4_t expectedColor{ 0xaa / 255.f, 0xbb / 255.f, 0xcc / 255.f, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_RGBLowerCasedHexColorStringAndIgnoreWhiteSpace)
{
	const std::string colorString{ "  0xaabbcc  " };
	vec4_t expectedColor{ 0xaa / 255.f, 0xbb / 255.f, 0xcc / 255.f, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_RGBALowerCasedHexColorString)
{
	const std::string colorString{ "0xaabbccdd" };
	vec4_t expectedColor{ 0xaa / 255.f, 0xbb / 255.f, 0xcc / 255.f, 0xdd / 255.f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_InvalidRGBALowerCasedHexColorStringAndSetBlack)
{
	const std::string colorString{ "0xaabbccddeeffgghhiijj" };
	vec4_t expectedColor{0.f, 0.f, 0.f, 1.f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_LongRGBALowerCasedHexColorStringAndIgnoreExtra)
{
	const std::string colorString{ "0xaabbccddeeff9988" };
	vec4_t expectedColor{ 0xaa / 255.f, 0xbb / 255.f, 0xcc / 255.f, 0xdd / 255.f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_RGBLowerCasedHexColorStringWithInvalidHexCharactersAndSetBlack)
{
	const std::string colorString{ "0xaagghh" };
	vec4_t expectedColor{ 0.f, 0.f, 0.f, 1.f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_RGBUpperCasedHashPrefixedColorString)
{
	const std::string colorString{ "#AABBCC" };
	vec4_t expectedColor{ 0xaa / 255.f, 0xbb / 255.f, 0xcc / 255.f, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_RGBLowerCasedHashPrefixedColorString)
{
	const std::string colorString{ "#aabbcc" };
	vec4_t expectedColor{ 0xaa / 255.f, 0xbb / 255.f, 0xcc / 255.f, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_RGBLowerCasedHashPrefixedColorStringAndIgnoreWhiteSpace)
{
	const std::string colorString{ "  #aabbcc  " };
	vec4_t expectedColor{ 0xaa / 255.f, 0xbb / 255.f, 0xcc / 255.f, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_RGBALowerCasedHashPrefixedColorString)
{
	const std::string colorString{ "#aabbccdd" };
	vec4_t expectedColor{ 0xaa / 255.f, 0xbb / 255.f, 0xcc / 255.f, 0xdd / 255.f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_InvalidRGBALowerCasedHashPrefixedColorStringAndSetBlack)
{
	const std::string colorString{ "#aabbccddeeffgghhiijj" };
	vec4_t expectedColor{ 0.f, 0.f, 0.f, 1.f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_LongRGBALowerCasedHashPrefixedColorStringAndIgnoreExtra)
{
	const std::string colorString{ "#aabbccddeeff9988" };
	vec4_t expectedColor{ 0xaa / 255.f, 0xbb / 255.f, 0xcc / 255.f, 0xdd / 255.f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}


TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_LowerCasedHashPrefixedColorStringWithOnlyRedValueSet)
{
	const std::string colorString{ "#aa" };
	vec4_t expectedColor{ 0xaa / 255.f, 0.f, 0.f, 1.f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_LowerCasedHashPrefixedColorStringWithArbitraryNumberOfValuesSet)
{
	const std::string colorString{ "#aff" };
	vec4_t expectedColor{ 0x0a / 255.f, 0xff / 255.f, 0.f, 1.f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_RGBLowerCasedHashPrefixedColorStringWithInvalidHexCharactersAndSetBlack)
{
	const std::string colorString{ "#aappgg" };
	vec4_t expectedColor{ 0.f, 0.f, 0.f, 1.f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_ColorStringWithNormalizedRGBValues)
{
	const std::string colorString{ "0.5 0.2 0.1" };
	vec4_t expectedColor{ 0.5f, 0.2f, 0.1f, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_ColorStringWithNormalizedRGBValuesAndIgnoreWhiteSpace)
{
	const std::string colorString{ "  0.5  0.2   0.1  " };
	vec4_t expectedColor{ 0.5f, 0.2f, 0.1f, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_ColorStringWithNonNormalizedRGBValues)
{
	const std::string colorString{ "127 192 204" };
	vec4_t expectedColor{ 127.f / 255.f, 192.f / 255.f, 204.f / 255.f, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_ColorStringWithNormalizedRGBAValues)
{
	const std::string colorString{ "0.8 0.5 0.2 0.7" };
	vec4_t expectedColor{ 0.8f, 0.5f, 0.2f, 0.7f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_ColorStringWithNonNormalizedRGBAValues)
{
	const std::string colorString{ "127 192 204 156" };
	vec4_t expectedColor{ 127.f / 255.f, 192.f / 255.f, 204.f / 255.f, 156.f / 255.f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_ColorStringRGBAValuesAndIgnoreExtraValues)
{
	const std::string colorString{ "127 192 204 156 207 206 107" };
	vec4_t expectedColor{ 127.f / 255.f, 192.f / 255.f, 204.f / 255.f, 156.f / 255.f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParseAndValidate_ColorStringWithNormalizedRGBValues)
{
	const std::string colorString{ "-1.0 -0.5 0.5" };
	vec4_t expectedColor{ 0.0f, 0.0f, 0.5f, 1.0f};
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParseAndValidate_ColorStringWithNonNormalizedRGBValues)
{
	const std::string colorString{ "-20 300 204" };
	vec4_t expectedColor{ 0.0f, 1.0f, 204.f / 255.f, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}


TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_ColorStringWithMixedRGBValuesAndConsiderItNonNormalized)
{
	const std::string colorString{ "0.51 1.0 192" };
	vec4_t expectedColor{ 0.51f / 255.f, 1.0f / 255.f, 192.f / 255.f, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_ColorStringWithOnlyRedValueSet)
{
	const std::string colorString{ "0.75" };
	vec4_t expectedColor{ 0.75f, 0.f, 0.f, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_ColorStringWithMixedValuesAndSetBlack)
{
	const std::string colorString{ "0.75 red #ffffff" };
	vec4_t expectedColor{ 0.f, 0.f, 0.f, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_MaliciousColorStringAndSetBlack)
{
	const std::string colorString{ "0.75 .- .." };
	vec4_t expectedColor{ 0.f, 0.f, 0.f, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_GreenHexValue)
{
	const std::string colorString{ "0x00ff00" };
	vec4_t expectedColor{ 0.0, 1.f, 0.0, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_BlueHexValue)
{
	const std::string colorString{ "0x0000ff" };
	vec4_t expectedColor{ 0.0, 0.f, 1.0, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}

TEST_F(ColorStringParsingTests, parseColorString_ShouldParse_EmptyHexValue)
{
	const std::string colorString{ "0x" };
	vec4_t expectedColor{ 0.0, 0.f, 0.0, 1.0f };
	vec4_t outColor;
	parseColorString(colorString, outColor);
	ASSERT_TRUE(Vector4Compare(outColor, expectedColor));
}