#include <gtest/gtest.h>
#include "../src/game/etj_shared.h"

namespace ETJump {
class EnumBitsetTests : public testing::Test {
  void SetUp() override {}
  void TearDown() override {}

public:
  enum class TestEnum : uint8_t {
    VALUE_NONE = 0,
    VALUE_1 = 1 << 0,
    VALUE_2 = 1 << 1,
    VALUE_3 = 1 << 2,
    VALUE_4 = 1 << 3,
  };
};

TEST_F(EnumBitsetTests, ConstructsFromEnumValue) {
  EnumBitset<TestEnum> bitset(TestEnum::VALUE_2);

  EXPECT_TRUE(bitset.has(TestEnum::VALUE_2));
  EXPECT_FALSE(bitset.has(TestEnum::VALUE_1));
}

TEST_F(EnumBitsetTests, ConstructsFromInitializerList) {
  EnumBitset<TestEnum> bitset{TestEnum::VALUE_1, TestEnum::VALUE_3};

  EXPECT_TRUE(bitset.has(TestEnum::VALUE_1));
  EXPECT_FALSE(bitset.has(TestEnum::VALUE_2));
  EXPECT_TRUE(bitset.has(TestEnum::VALUE_3));
}

TEST_F(EnumBitsetTests, DefaultConstructorIsEmpty) {
  EnumBitset<TestEnum> bitset;

  EXPECT_FALSE(bitset.has(TestEnum::VALUE_1));
  EXPECT_FALSE(bitset.has(TestEnum::VALUE_2));
  EXPECT_FALSE(static_cast<bool>(bitset));
}

TEST_F(EnumBitsetTests, SetAddsFlag) {
  EnumBitset<TestEnum> bitset;
  bitset.set(TestEnum::VALUE_1);

  EXPECT_TRUE(bitset.has(TestEnum::VALUE_1));
  EXPECT_FALSE(bitset.has(TestEnum::VALUE_2));
}

TEST_F(EnumBitsetTests, SetMultipleFlags) {
  EnumBitset<TestEnum> bitset;
  bitset.set(TestEnum::VALUE_1).set(TestEnum::VALUE_3);

  EXPECT_TRUE(bitset.has(TestEnum::VALUE_1));
  EXPECT_FALSE(bitset.has(TestEnum::VALUE_2));
  EXPECT_TRUE(bitset.has(TestEnum::VALUE_3));
}

TEST_F(EnumBitsetTests, ClearRemovesFlag) {
  EnumBitset<TestEnum> bitset(TestEnum::VALUE_2);
  bitset.clear(TestEnum::VALUE_2);

  EXPECT_FALSE(bitset.has(TestEnum::VALUE_2));
}

TEST_F(EnumBitsetTests, ClearDoesNotAffectOtherFlags) {
  EnumBitset<TestEnum> bitset;
  bitset.set(TestEnum::VALUE_1).set(TestEnum::VALUE_2);
  bitset.clear(TestEnum::VALUE_1);

  EXPECT_FALSE(bitset.has(TestEnum::VALUE_1));
  EXPECT_TRUE(bitset.has(TestEnum::VALUE_2));
}

TEST_F(EnumBitsetTests, OrOperatorWithFlag) {
  EnumBitset<TestEnum> bitset(TestEnum::VALUE_1);
  auto result = bitset | TestEnum::VALUE_2;
  EXPECT_TRUE(result.has(TestEnum::VALUE_1));
  EXPECT_TRUE(result.has(TestEnum::VALUE_2));
  // original remains unchanged
  EXPECT_FALSE(bitset.has(TestEnum::VALUE_2));
}

TEST_F(EnumBitsetTests, OrOperatorWithBitset) {
  EnumBitset<TestEnum> bitset1(TestEnum::VALUE_1);
  EnumBitset<TestEnum> bitset2(TestEnum::VALUE_2);

  const auto result = bitset1 | bitset2;

  EXPECT_TRUE(result.has(TestEnum::VALUE_1));
  EXPECT_TRUE(result.has(TestEnum::VALUE_2));
}

TEST_F(EnumBitsetTests, AndOperatorWithFlag) {
  EnumBitset<TestEnum> bitset;
  bitset.set(TestEnum::VALUE_1).set(TestEnum::VALUE_2);

  EXPECT_TRUE(bitset & TestEnum::VALUE_1);
  EXPECT_TRUE(bitset & TestEnum::VALUE_2);
  EXPECT_FALSE(bitset & TestEnum::VALUE_3);
}

TEST_F(EnumBitsetTests, AndOperatorWithBitset) {
  EnumBitset<TestEnum> bitset1;
  bitset1.set(TestEnum::VALUE_1).set(TestEnum::VALUE_2);

  EnumBitset<TestEnum> bitset2;
  bitset2.set(TestEnum::VALUE_2).set(TestEnum::VALUE_3);

  const auto result = bitset1 & bitset2;

  EXPECT_FALSE(result.has(TestEnum::VALUE_1));
  EXPECT_TRUE(result.has(TestEnum::VALUE_2));
  EXPECT_FALSE(result.has(TestEnum::VALUE_3));
}

TEST_F(EnumBitsetTests, EqualityOperator) {
  EnumBitset<TestEnum> bitset1(TestEnum::VALUE_1);
  EnumBitset<TestEnum> bitset2(TestEnum::VALUE_1);
  EnumBitset<TestEnum> bitset3(TestEnum::VALUE_2);

  EXPECT_TRUE(bitset1 == bitset2);
  EXPECT_FALSE(bitset1 == bitset3);
}

TEST_F(EnumBitsetTests, BoolConversion) {
  EnumBitset<TestEnum> empty;
  EnumBitset<TestEnum> nonEmpty(TestEnum::VALUE_1);

  EXPECT_FALSE(static_cast<bool>(empty));
  EXPECT_TRUE(static_cast<bool>(nonEmpty));
}

TEST_F(EnumBitsetTests, ChainedOperations) {
  EnumBitset<TestEnum> bitset;

  auto result = bitset | TestEnum::VALUE_1 | TestEnum::VALUE_2;
  result.set(TestEnum::VALUE_3).clear(TestEnum::VALUE_1);

  EXPECT_FALSE(result.has(TestEnum::VALUE_1));
  EXPECT_TRUE(result.has(TestEnum::VALUE_2));
  EXPECT_TRUE(result.has(TestEnum::VALUE_3));
}

TEST_F(EnumBitsetTests, UseInIfStatement) {
  EnumBitset<TestEnum> bitset;
  bitset.set(TestEnum::VALUE_1).set(TestEnum::VALUE_2);

  if (bitset & TestEnum::VALUE_1) {
    SUCCEED();
  } else {
    FAIL() << "Should have VALUE_1";
  }

  if (bitset.has(TestEnum::VALUE_3)) {
    FAIL() << "Should not have VALUE_3";
  }

  SUCCEED();
}

} // namespace ETJump
