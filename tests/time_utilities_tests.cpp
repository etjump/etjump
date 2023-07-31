#include <gtest/gtest.h>
#include "../src/game/etj_time_utilities.h"

using namespace ETJump;

class TimeUtilitiesTests : public testing::Test {
  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(TimeUtilitiesTests, Date_fromString_shouldParseDateCorrectly) {
  Date expected{2021, 3, 5};
  ASSERT_EQ(Date::fromString("2021-03-05"), expected);
}

TEST_F(TimeUtilitiesTests, Date_fromString_shouldThrowIfDateIsNotNumerical) {
  ASSERT_THROW(Date::fromString("foobar"), std::invalid_argument);
}

TEST_F(TimeUtilitiesTests, Date_fromString_shouldThrowIfDayIsMissing) {
  ASSERT_THROW(Date::fromString("2021-03"), std::invalid_argument);
}

