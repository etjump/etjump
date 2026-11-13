#include <cstring>
#include <string>

#include <gtest/gtest.h>
#include "../src/game/etj_game_failure.h"

namespace ETJump {
TEST(GameFailureTests, IsInactiveByDefault) {
  GameFailure failure;

  EXPECT_FALSE(failure.isActive());
  EXPECT_FALSE(failure.takeClientDrops(100));
  EXPECT_FALSE(failure.takeShutdown(GAME_FAILURE_SHUTDOWN_DELAY * 10));
}

TEST(GameFailureTests, GetDenialMessage_IsGenericBeforeFailure) {
  // e.g. for a failing connect nested inside a syscall
  const GameFailure failure;

  EXPECT_STREQ(failure.getDenialMessage(), GAME_FAILURE_GENERIC_DENIAL);
}

TEST(GameFailureTests, Activate_DeniesClientsWithMessage) {
  GameFailure failure;
  failure.activate("Tracker error", false, false, 100);

  EXPECT_TRUE(failure.isActive());
  EXPECT_STREQ(failure.getDenialMessage(), "Tracker error");
}

TEST(GameFailureTests, Activate_HidesUnexpectedErrorText) {
  GameFailure failure;
  failure.activate("Unhandled exception: C:\\server\\db", true, false, 100);

  EXPECT_STREQ(failure.getDenialMessage(), GAME_FAILURE_GENERIC_DENIAL);
}

TEST(GameFailureTests, Activate_SanitizesDenialMessage) {
  GameFailure failure;
  failure.activate("Tracker error on line: \"1\"\nat 100%s", false, false, 0);

  EXPECT_STREQ(failure.getDenialMessage(),
               "Tracker error on line: '1' at 100 s");
}

TEST(GameFailureTests, Activate_TruncatesDenialMessage) {
  GameFailure failure;
  const std::string message(GAME_FAILURE_MAX_DENIAL_LENGTH * 4, 'a');
  failure.activate(message.c_str(), false, false, 100);

  EXPECT_EQ(std::strlen(failure.getDenialMessage()),
            GAME_FAILURE_MAX_DENIAL_LENGTH);
}

TEST(GameFailureTests, GetDenialMessage_KeepsItsAddress) {
  GameFailure failure;
  const char *denial = failure.getDenialMessage();

  failure.activate("Tracker error", false, false, 100);

  // handed to the engine before the failure happens
  EXPECT_EQ(failure.getDenialMessage(), denial);
  EXPECT_STREQ(denial, "Tracker error");
}

TEST(GameFailureTests, TakeClientDrops_OnlyWhenRequested) {
  GameFailure failure;
  failure.activate("Tracker error", false, false, 100);

  EXPECT_FALSE(failure.takeClientDrops(116));
}

TEST(GameFailureTests, TakeClientDrops_ReturnsTrueOnlyOnce) {
  GameFailure failure;
  failure.activate("Tracker error", false, true, 100);

  EXPECT_TRUE(failure.takeClientDrops(116));
  EXPECT_FALSE(failure.takeClientDrops(132));
}

TEST(GameFailureTests, TakeShutdown_WaitsForDelayAfterActivate) {
  GameFailure failure;
  failure.activate("Tracker error", false, false, 100);

  EXPECT_FALSE(failure.takeShutdown(100));
  EXPECT_FALSE(failure.takeShutdown(100 + GAME_FAILURE_SHUTDOWN_DELAY - 1));
  EXPECT_TRUE(failure.takeShutdown(100 + GAME_FAILURE_SHUTDOWN_DELAY));
}

TEST(GameFailureTests, TakeShutdown_WaitsForDelayAfterDrops) {
  GameFailure failure;
  failure.activate("Tracker error", false, true, 100);

  // the clients haven't been dropped yet
  EXPECT_FALSE(failure.takeShutdown(100 + GAME_FAILURE_SHUTDOWN_DELAY * 2));

  EXPECT_TRUE(failure.takeClientDrops(5000));
  EXPECT_FALSE(failure.takeShutdown(5000 + GAME_FAILURE_SHUTDOWN_DELAY - 1));
  EXPECT_TRUE(failure.takeShutdown(5000 + GAME_FAILURE_SHUTDOWN_DELAY));
}

TEST(GameFailureTests, TakeShutdown_ReturnsTrueOnlyOnce) {
  GameFailure failure;
  failure.activate("Tracker error", false, false, 100);

  EXPECT_TRUE(failure.takeShutdown(100 + GAME_FAILURE_SHUTDOWN_DELAY));
  EXPECT_FALSE(failure.takeShutdown(100 + GAME_FAILURE_SHUTDOWN_DELAY * 2));
}

TEST(GameFailureTests, Reset_DeactivatesAndRestoresGenericDenial) {
  GameFailure failure;
  failure.activate("Tracker error", false, true, 100);
  failure.reset();

  EXPECT_FALSE(failure.isActive());
  EXPECT_FALSE(failure.takeClientDrops(116));
  EXPECT_FALSE(failure.takeShutdown(100 + GAME_FAILURE_SHUTDOWN_DELAY));
  EXPECT_STREQ(failure.getDenialMessage(), GAME_FAILURE_GENERIC_DENIAL);
}

TEST(GameFailureTests, Activate_StartsNewShutdownAfterReset) {
  GameFailure failure;
  failure.activate("first", false, false, 100);
  EXPECT_TRUE(failure.takeShutdown(100 + GAME_FAILURE_SHUTDOWN_DELAY));
  failure.reset();

  failure.activate("second", false, false, 5000);

  EXPECT_STREQ(failure.getDenialMessage(), "second");
  EXPECT_FALSE(failure.takeShutdown(5000 + GAME_FAILURE_SHUTDOWN_DELAY - 1));
  EXPECT_TRUE(failure.takeShutdown(5000 + GAME_FAILURE_SHUTDOWN_DELAY));
}

TEST(GameFailureTests, GetMessage_KeepsFullErrorMessage) {
  const std::string message =
      "Unhandled exception: \"quoted\" 100%\n" +
      std::string(GAME_FAILURE_MAX_DENIAL_LENGTH * 2, 'a');

  GameFailure failure;
  failure.activate(message.c_str(), true, false, 0);

  EXPECT_EQ(failure.getMessage(), message);
}

TEST(GameFailureTests, ShutsDownThroughError_OnlyOnListenServer) {
  GameFailure failure;
  failure.activate("Tracker error", false, false, 100);

  EXPECT_FALSE(failure.shutsDownThroughError(true));
  EXPECT_TRUE(failure.shutsDownThroughError(false));
}

TEST(GameFailureTests, ShutsDownThroughError_NotAfterLocalClientDropped) {
  GameFailure failure;
  failure.activate("Tracker error", false, true, 100);
  EXPECT_TRUE(failure.takeClientDrops(116));
  failure.setLocalClientDropped();

  EXPECT_FALSE(failure.shutsDownThroughError(false));
}

TEST(GameFailureTests, Activate_ForgetsLocalClientDroppedBefore) {
  GameFailure failure;
  failure.activate("first", false, true, 100);
  failure.setLocalClientDropped();
  failure.reset();

  failure.activate("second", false, false, 5000);

  EXPECT_TRUE(failure.shutsDownThroughError(false));
}
} // namespace ETJump
