#include <gtest/gtest.h>
#include "../src/ui/etj_cgame_keep_alive.h"

namespace ETJump {
TEST(CgameKeepAliveTrackerTests, Update_IgnoresUnsetFlag) {
  CgameKeepAliveTracker tracker;

  EXPECT_FALSE(tracker.update(false, 100));
  EXPECT_FALSE(tracker.update(false, 116));
}

TEST(CgameKeepAliveTrackerTests, Update_ReleasesAfterRealtimeChanged) {
  CgameKeepAliveTracker tracker;

  // refreshed during the error handling, and possibly more than once
  EXPECT_FALSE(tracker.update(true, 100));
  EXPECT_FALSE(tracker.update(true, 100));

  // the next frame
  EXPECT_TRUE(tracker.update(true, 116));
}

TEST(CgameKeepAliveTrackerTests, Update_ReleasesOnlyOnce) {
  CgameKeepAliveTracker tracker;

  EXPECT_FALSE(tracker.update(true, 100));
  EXPECT_TRUE(tracker.update(true, 116));

  // the flag is cleared a moment later, a new observation starts over
  EXPECT_FALSE(tracker.update(true, 132));
  EXPECT_FALSE(tracker.update(false, 148));
}

TEST(CgameKeepAliveTrackerTests, Update_StartsOverWhenFlagCleared) {
  CgameKeepAliveTracker tracker;

  EXPECT_FALSE(tracker.update(true, 100));
  EXPECT_FALSE(tracker.update(false, 116));
  EXPECT_FALSE(tracker.update(true, 132));
  EXPECT_TRUE(tracker.update(true, 148));
}
} // namespace ETJump
