#include <string>

#include <gtest/gtest.h>
#include "../src/game/etj_game_init_failure.h"

namespace ETJump {
TEST(GameInitFailureTests, IsInactiveByDefault) {
  const GameInitFailure failure;

  EXPECT_FALSE(failure.isActive());
  EXPECT_FALSE(failure.shouldRaise(GameInitFailure::RAISE_DELAY * 10));
}

TEST(GameInitFailureTests, Activate_DeniesClientsWithMessage) {
  GameInitFailure failure;
  failure.activate("Tracker error", false, 100);

  EXPECT_TRUE(failure.isActive());
  EXPECT_STREQ(failure.getMessage(), "Tracker error");
  EXPECT_STREQ(failure.getDenialMessage(), "Tracker error");
}

TEST(GameInitFailureTests, Activate_HidesUnexpectedErrorText) {
  GameInitFailure failure;
  failure.activate("Unhandled exception: C:\\server\\db", true, 100);

  EXPECT_STREQ(failure.getMessage(), "Unhandled exception: C:\\server\\db");
  EXPECT_STREQ(failure.getDenialMessage(),
               GameInitFailure::UNEXPECTED_ERROR_DENIAL);
}

TEST(GameInitFailureTests, ShouldRaise_WaitsForRaiseDelay) {
  GameInitFailure failure;
  failure.activate("Tracker error", false, 100);

  EXPECT_FALSE(failure.shouldRaise(100));
  EXPECT_FALSE(failure.shouldRaise(100 + GameInitFailure::RAISE_DELAY - 1));
  EXPECT_TRUE(failure.shouldRaise(100 + GameInitFailure::RAISE_DELAY));
}

TEST(GameInitFailureTests, Reset_DeactivatesButKeepsMessage) {
  GameInitFailure failure;
  failure.activate("Tracker error", false, 100);
  failure.reset();

  EXPECT_FALSE(failure.isActive());
  EXPECT_FALSE(failure.shouldRaise(100 + GameInitFailure::RAISE_DELAY));

  // still needed to raise the error after resetting
  EXPECT_STREQ(failure.getMessage(), "Tracker error");
}

TEST(GameInitFailureTests, Activate_ReplacesPreviousFailure) {
  GameInitFailure failure;
  failure.activate("first", false, 100);
  failure.reset();
  failure.activate("second", false, 5000);

  EXPECT_STREQ(failure.getMessage(), "second");
  EXPECT_FALSE(failure.shouldRaise(5000 + GameInitFailure::RAISE_DELAY - 1));
  EXPECT_TRUE(failure.shouldRaise(5000 + GameInitFailure::RAISE_DELAY));
}
} // namespace ETJump
