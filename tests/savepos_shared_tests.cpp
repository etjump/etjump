#include <gtest/gtest.h>
#include "../src/game/etj_savepos_shared.h"

namespace ETJump {
class SavePosSharedTests : public testing::Test {
public:
  void SetUp() override {}
  void TearDown() override {}

  static std::array<int, MAX_TIMERUN_CHECKPOINTS> createNullCheckpoints() {
    std::array<int, MAX_TIMERUN_CHECKPOINTS> checkpoints{};
    checkpoints.fill(-1);
    return checkpoints;
  }

  static std::array<int, MAX_TIMERUN_CHECKPOINTS> createDefaultCheckpoints() {
    return {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, -1, -1};
  }

  static std::array<int, MAX_TIMERUN_CHECKPOINTS>
  createDefaultPreviousCheckpoints() {
    return {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, -1, -1, -1, -1, -1, -1};
  }

  static std::vector<std::string> createSampleArgs() {
    return {"1",
            "1",
            "1",
            "2",
            "2",
            "2",
            "3",
            "3",
            "3",
            "0",
            "samplerun",
            "100",
            "-1",
            "1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1,-1",
            "1,2,3,4,5,6,7,8,9,10,-1,-1,-1,-1,-1,-1"};
  }

  static SavePosData createSampleData() {
    SavePosData data{};

    data.name = "sample";
    data.mapname = "oasis";

    for (int i = 0; i < 3; i++) {
      data.pos.origin[i] = 1;
      data.pos.angles[i] = 2;
      data.pos.velocity[i] = 3;
    }

    data.pos.stance = PlayerStance::Stand;

    data.timerunInfo.runName = "samplerun";
    data.timerunInfo.currentRunTimer = 100;
    data.timerunInfo.previousRecord = -1;
    data.timerunInfo.checkpoints = createDefaultCheckpoints();
    data.timerunInfo.previousRecordCheckpoints =
        createDefaultPreviousCheckpoints();

    return data;
  }
};

TEST_F(SavePosSharedTests, serialize_ShouldSerialize) {
  const SavePosData data = createSampleData();

  ASSERT_EQ(SavePosData::serialize(data),
            "loadpos 1 1 1 2 2 2 3 3 3 0 \"samplerun\" 100 -1 "
            "1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1,-1 "
            "1,2,3,4,5,6,7,8,9,10,-1,-1,-1,-1,-1,-1");
}

TEST_F(SavePosSharedTests, serialize_ShouldSerializeWithMultipartRunName) {
  SavePosData data = createSampleData();
  data.timerunInfo.runName = "foo bar";

  ASSERT_EQ(SavePosData::serialize(data),
            "loadpos 1 1 1 2 2 2 3 3 3 0 \"foo bar\" 100 -1 "
            "1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1,-1 "
            "1,2,3,4,5,6,7,8,9,10,-1,-1,-1,-1,-1,-1");
}

TEST_F(SavePosSharedTests, serialize_ShouldSerializeWithoutTimerunData) {
  SavePosData data = createSampleData();
  data.timerunInfo.runName.clear();

  ASSERT_EQ(SavePosData::serialize(data), "loadpos 1 1 1 2 2 2 3 3 3 0");
}

TEST_F(SavePosSharedTests, deserialize_ShouldDeserialize) {
  const auto args = createSampleArgs();
  const SavePosData data = SavePosData::deserialize(args);

  ASSERT_EQ(data.pos.origin[0], 1);
  ASSERT_EQ(data.pos.origin[1], 1);
  ASSERT_EQ(data.pos.origin[2], 1);

  ASSERT_EQ(data.pos.angles[0], 2);
  ASSERT_EQ(data.pos.angles[1], 2);
  ASSERT_EQ(data.pos.angles[2], 2);

  ASSERT_EQ(data.pos.velocity[0], 3);
  ASSERT_EQ(data.pos.velocity[1], 3);
  ASSERT_EQ(data.pos.velocity[2], 3);

  ASSERT_EQ(data.pos.stance, PlayerStance::Stand);

  ASSERT_EQ(data.timerunInfo.runName, "samplerun");
  ASSERT_EQ(data.timerunInfo.currentRunTimer, 100);
  ASSERT_EQ(data.timerunInfo.previousRecord, -1);

  ASSERT_EQ(data.timerunInfo.checkpoints, createDefaultCheckpoints());
  ASSERT_EQ(data.timerunInfo.previousRecordCheckpoints,
            createDefaultPreviousCheckpoints());
}

TEST_F(SavePosSharedTests, deserialize_ShouldDeserializeWithMultipartRunName) {
  auto args = createSampleArgs();
  args[10] = "foo bar";
  const SavePosData data = SavePosData::deserialize(args);

  ASSERT_EQ(data.pos.origin[0], 1);
  ASSERT_EQ(data.pos.origin[1], 1);
  ASSERT_EQ(data.pos.origin[2], 1);

  ASSERT_EQ(data.pos.angles[0], 2);
  ASSERT_EQ(data.pos.angles[1], 2);
  ASSERT_EQ(data.pos.angles[2], 2);

  ASSERT_EQ(data.pos.velocity[0], 3);
  ASSERT_EQ(data.pos.velocity[1], 3);
  ASSERT_EQ(data.pos.velocity[2], 3);

  ASSERT_EQ(data.pos.stance, PlayerStance::Stand);

  ASSERT_EQ(data.timerunInfo.runName, "foo bar");
  ASSERT_EQ(data.timerunInfo.currentRunTimer, 100);
  ASSERT_EQ(data.timerunInfo.previousRecord, -1);

  ASSERT_EQ(data.timerunInfo.checkpoints, createDefaultCheckpoints());
  ASSERT_EQ(data.timerunInfo.previousRecordCheckpoints,
            createDefaultPreviousCheckpoints());
}

TEST_F(SavePosSharedTests, deserialize_ShouldDeserializeWithoutTimerunData) {
  auto args = createSampleArgs();
  args[10].clear();
  const SavePosData data = SavePosData::deserialize(args);

  ASSERT_EQ(data.pos.origin[0], 1);
  ASSERT_EQ(data.pos.origin[1], 1);
  ASSERT_EQ(data.pos.origin[2], 1);

  ASSERT_EQ(data.pos.angles[0], 2);
  ASSERT_EQ(data.pos.angles[1], 2);
  ASSERT_EQ(data.pos.angles[2], 2);

  ASSERT_EQ(data.pos.velocity[0], 3);
  ASSERT_EQ(data.pos.velocity[1], 3);
  ASSERT_EQ(data.pos.velocity[2], 3);

  ASSERT_EQ(data.pos.stance, PlayerStance::Stand);

  ASSERT_EQ(data.timerunInfo.runName, "");
  ASSERT_EQ(data.timerunInfo.currentRunTimer, 0);
  ASSERT_EQ(data.timerunInfo.previousRecord, -1);

  ASSERT_EQ(data.timerunInfo.checkpoints, createNullCheckpoints());
  ASSERT_EQ(data.timerunInfo.previousRecordCheckpoints,
            createNullCheckpoints());
}

TEST_F(SavePosSharedTests,
       deserialize_ReturnsNullCheckpointsIfTooFewCheckpointArgs) {
  auto args = createSampleArgs();
  args[13] = "1,2,3,4,5";
  SavePosData data = SavePosData::deserialize(args);

  ASSERT_EQ(data.timerunInfo.checkpoints, createNullCheckpoints());

  args = createSampleArgs();
  args[14] = "1,2,3,4,5";
  data = SavePosData::deserialize(args);

  ASSERT_EQ(data.timerunInfo.previousRecordCheckpoints,
            createNullCheckpoints());
}

TEST_F(SavePosSharedTests, deserialize_ShouldThrowError) {
  auto args = createSampleArgs();
  args[2] = "foo";
  const SavePosData data = SavePosData::deserialize(args);

  ASSERT_FALSE(data.error.empty());
}
} // namespace ETJump
