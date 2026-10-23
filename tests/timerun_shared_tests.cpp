#include <gtest/gtest.h>
#include "../src/game/etj_timerun_shared.h"

using namespace ETJump;

class TimerunSharedTests : public testing::Test {
public:
  void SetUp() override {}

  void TearDown() override {}

  std::array<int, MAX_TIMERUN_CHECKPOINTS> CreateDefaultCheckpoints() {
    return {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, -1, -1};
  }

  std::array<int, MAX_TIMERUN_CHECKPOINTS> CreateDefaultCurrentCheckpoints() {
    return {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, -1, -1, -1, -1, -1, -1};
  }

  std::array<bool, MAX_TIMERUN_CHECKPOINTS> CreateDefaultRunBestPersonal() {
    return {true,  true,  false, false, false, false, false, false,
            false, false, false, false, false, false, false, false};
  }

  std::array<bool, MAX_TIMERUN_CHECKPOINTS> CreateDefaultRunBestGlobal() {
    return {true,  false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false};
  }
};

TEST_F(TimerunSharedTests, Start_ShouldSerialize) {
  auto start = TimerunCommands::Start(
      1, 2, "3", 4, 1, CreateDefaultCheckpoints(),
      CreateDefaultCurrentCheckpoints(), CreateDefaultRunBestPersonal(),
      CreateDefaultRunBestGlobal());

  ASSERT_EQ(
      start.serialize(),
      "timerun start 1 2 \"3\" 4 1 \"1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1,-1\" "
      "\"1,2,3,4,5,6,7,8,9,10,-1,-1,-1,-1,-1,-1\" "
      "\"1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0\" "
      "\"1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\"");
}

TEST_F(TimerunSharedTests, Start_ShouldDeserialize) {
  auto args =
      std::vector<std::string>{"timerun",
                               "start",
                               "1",
                               "2",
                               "3",
                               "4",
                               "1",
                               "1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1,-1",
                               "1,2,3,4,5,6,7,8,9,10,-1,-1,-1,-1,-1,-1"};
  auto deserialized = TimerunCommands::Start::deserialize(args);

  ASSERT_EQ(deserialized.value().clientNum, 1);
  ASSERT_EQ(deserialized.value().startTime, 2);
  ASSERT_EQ(deserialized.value().runName, "3");
  ASSERT_EQ(deserialized.value().previousRecord.value(), 4);
  ASSERT_EQ(deserialized.value().runHasCheckpoints, 1);
  ASSERT_EQ(deserialized.value().checkpoints[10], 11);
  ASSERT_EQ(deserialized.value().currentRunCheckpoints[10], -1);
  // this stream predates the run's verdicts, so both arrays stay false
  ASSERT_FALSE(deserialized.value().currentRunBestPersonalSegments[0]);
  ASSERT_FALSE(deserialized.value().currentRunBestGlobalSegments[0]);
}

TEST_F(TimerunSharedTests, Start_ShouldDeserialize_BestSegments) {
  auto args = std::vector<std::string>{"timerun",
                                       "start",
                                       "1",
                                       "2",
                                       "3",
                                       "4",
                                       "1",
                                       "1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1,-1",
                                       "1,2,3,4,5,6,7,8,9,10,-1,-1,-1,-1,-1,-1",
                                       "1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0",
                                       "1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"};
  auto deserialized = TimerunCommands::Start::deserialize(args);

  ASSERT_TRUE(deserialized.value().currentRunBestPersonalSegments[0]);
  ASSERT_TRUE(deserialized.value().currentRunBestPersonalSegments[1]);
  ASSERT_FALSE(deserialized.value().currentRunBestPersonalSegments[2]);
  ASSERT_TRUE(deserialized.value().currentRunBestGlobalSegments[0]);
  ASSERT_FALSE(deserialized.value().currentRunBestGlobalSegments[1]);
}

TEST_F(TimerunSharedTests, Start_ShouldSerialize_IfNoPreviousRecord) {
  auto start = TimerunCommands::Start(
      1, 2, "3", std::nullopt, true, CreateDefaultCheckpoints(),
      CreateDefaultCurrentCheckpoints(), CreateDefaultRunBestPersonal(),
      CreateDefaultRunBestGlobal());

  ASSERT_EQ(
      start.serialize(),
      "timerun start 1 2 \"3\" -1 1 \"1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1,-1\" "
      "\"1,2,3,4,5,6,7,8,9,10,-1,-1,-1,-1,-1,-1\" "
      "\"1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0\" "
      "\"1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\"");
}

TEST_F(TimerunSharedTests, Start_ShouldDeserialize_IfNoPreviousRecord) {
  auto args =
      std::vector<std::string>{"timerun",
                               "start",
                               "1",
                               "2",
                               "3",
                               "-1",
                               "1",
                               "1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1,-1",
                               "1,2,3,4,5,6,7,8,9,10,-1,-1,-1,-1,-1,-1"};
  auto deserialized = TimerunCommands::Start::deserialize(args);

  ASSERT_FALSE(deserialized.value().previousRecord.has_value());
}

TEST_F(TimerunSharedTests, Checkpoint_ShouldSerialize) {
  auto checkpoint = TimerunCommands::Checkpoint(1, 2, 3, "4", 5, true, false);

  ASSERT_EQ(checkpoint.serialize(), "timerun checkpoint 1 2 3 \"4\" 5 1 0");
}

TEST_F(TimerunSharedTests, Checkpoint_ShouldDeserialize) {
  auto args = std::vector<std::string>{"timerun", "checkpoint", "1", "2",
                                       "3",       "4",          "5"};
  auto checkpoint = TimerunCommands::Checkpoint::deserialize(args);

  ASSERT_EQ(checkpoint.value().checkpointIndex, 5);
  // this stream predates the best segment verdicts, so they stay false
  ASSERT_FALSE(checkpoint.value().bestPersonalSegment);
  ASSERT_FALSE(checkpoint.value().bestGlobalSegment);
}

TEST_F(TimerunSharedTests, Checkpoint_ShouldDeserialize_BestSegments) {
  auto args = std::vector<std::string>{"timerun", "checkpoint", "1", "2", "3",
                                       "4",       "5",          "1", "1"};
  auto checkpoint = TimerunCommands::Checkpoint::deserialize(args);

  ASSERT_EQ(checkpoint.value().checkpointIndex, 5);
  ASSERT_TRUE(checkpoint.value().bestPersonalSegment);
  ASSERT_TRUE(checkpoint.value().bestGlobalSegment);
}

TEST_F(TimerunSharedTests, Checkpoint_ShouldDeserializeWithoutCheckpointIndex) {
  auto args =
      std::vector<std::string>{"timerun", "checkpoint", "1", "2", "3", "4"};
  auto checkpoint = TimerunCommands::Checkpoint::deserialize(args);

  ASSERT_EQ(checkpoint.value().runName, "4");
}

TEST_F(TimerunSharedTests, Stop_ShouldSerialize) {
  auto stop = TimerunCommands::Stop(1, 2, "run", false, true);

  ASSERT_EQ(stop.serialize(), "timerun stop 1 2 \"run\" 0 1");
}

TEST_F(TimerunSharedTests, Stop_ShouldDeserialize) {
  auto args = std::vector<std::string>{"timerun", "stop", "1", "2", "run"};
  auto stop = TimerunCommands::Stop::deserialize(args);

  ASSERT_EQ(stop.value().runName, "run");
  // this stream predates the best segment verdicts, so they stay false
  ASSERT_FALSE(stop.value().beatPersonal);
  ASSERT_FALSE(stop.value().beatGlobal);
}

TEST_F(TimerunSharedTests, Stop_ShouldDeserialize_BestSegments) {
  auto args =
      std::vector<std::string>{"timerun", "stop", "1", "2", "run", "1", "0"};
  auto stop = TimerunCommands::Stop::deserialize(args);

  ASSERT_EQ(stop.value().completionTime, 2);
  ASSERT_TRUE(stop.value().beatPersonal);
  ASSERT_FALSE(stop.value().beatGlobal);
}

TEST_F(TimerunSharedTests, Interrupt_ShouldSerialize) {
  auto interrupt = TimerunCommands::Interrupt(1);

  ASSERT_EQ(interrupt.serialize(), "timerun interrupt 1");
}

TEST_F(TimerunSharedTests, Interrupt_ShouldDeserialize) {
  auto args = std::vector<std::string>{"timerun", "interrupt", "1"};
  auto interrupt = TimerunCommands::Interrupt::deserialize(args);

  ASSERT_EQ(interrupt.value().clientNum, 1);
}

TEST_F(TimerunSharedTests, Record_ShouldSerialize) {
  auto record = TimerunCommands::Record(1, 2, 3, "run");

  ASSERT_EQ(record.serialize(), "timerun record 1 2 3 \"run\"");
}

TEST_F(TimerunSharedTests, Record_ShouldDeserialize) {
  auto args =
      std::vector<std::string>{"timerun", "record", "1", "2", "3", "run"};
  auto record = TimerunCommands::Record::deserialize(args);

  ASSERT_EQ(record.value().clientNum, 1);
  ASSERT_EQ(record.value().completionTime, 2);
  ASSERT_EQ(record.value().previousRecordTime.value(), 3);
  ASSERT_EQ(record.value().runName, "run");
}

TEST_F(TimerunSharedTests, Completion_ShouldSerialize) {
  auto completion = TimerunCommands::Completion(1, 2, 3, "run");

  ASSERT_EQ(completion.serialize(), "timerun completion 1 2 3 \"run\"");
}

TEST_F(TimerunSharedTests, Completion_ShouldDeserialize) {
  auto args =
      std::vector<std::string>{"timerun", "completion", "1", "2", "3", "run"};
  auto interrupt = TimerunCommands::Completion::deserialize(args);

  ASSERT_EQ(interrupt.value().clientNum, 1);
  ASSERT_EQ(interrupt.value().completionTime, 2);
  ASSERT_EQ(interrupt.value().previousRecordTime.value(), 3);
  ASSERT_EQ(interrupt.value().runName, "run");
}

TEST_F(TimerunSharedTests, ComputeSegmentTimes_NoCheckpointsYieldsNothing) {
  EXPECT_TRUE(TimerunSegments::computeSegmentTimes({}, 5000).empty());
}

TEST_F(TimerunSharedTests, ComputeSegmentTimes_IncludesFinishSegment) {
  const auto segments =
      TimerunSegments::computeSegmentTimes({100, 250, 400}, 600);

  EXPECT_EQ(segments, (std::vector<int32_t>{100, 150, 150, 200}));
}

TEST_F(TimerunSharedTests, ComputeSegmentTimes_FinishAtLastCheckpoint) {
  const auto segments = TimerunSegments::computeSegmentTimes({100, 200}, 200);

  EXPECT_EQ(segments, (std::vector<int32_t>{100, 100}));
}

TEST_F(TimerunSharedTests, ComputeSegmentTimes_CheckpointGapDropsTail) {
  const auto segments = TimerunSegments::computeSegmentTimes(
      {100, TIMERUN_CHECKPOINT_NOT_SET, 300}, 400);

  // malformed input that the writers cannot produce: the positions after the
  // gap cannot be trusted, so no finish is appended
  EXPECT_EQ(segments, (std::vector<int32_t>{100}));
}

TEST_F(TimerunSharedTests,
       ComputeSegmentTimes_TrailingUnsetStillAppendsFinish) {
  const auto segments = TimerunSegments::computeSegmentTimes(
      {100, TIMERUN_CHECKPOINT_NOT_SET, TIMERUN_CHECKPOINT_NOT_SET}, 250);

  // this is the normal shape of a run with fewer than the maximum checkpoints
  EXPECT_EQ(segments, (std::vector<int32_t>{100, 150}));
}

TEST_F(TimerunSharedTests,
       ComputeSegmentTimes_TrailingUnsetAfterGapStillAppendsFinish) {
  const auto segments = TimerunSegments::computeSegmentTimes(
      {100, 200, TIMERUN_CHECKPOINT_NOT_SET, TIMERUN_CHECKPOINT_NOT_SET}, 400);

  // with two checkpoints before the gap, the gap scan must start after the
  // gap itself - scanning from the front finds an earlier checkpoint and
  // drops the finish segment for every run with 2-15 checkpoints
  EXPECT_EQ(segments, (std::vector<int32_t>{100, 100, 200}));
}

TEST_F(TimerunSharedTests,
       ComputeSegmentTimes_GapAfterTwoCheckpointsDropsTail) {
  const auto segments = TimerunSegments::computeSegmentTimes(
      {100, 200, TIMERUN_CHECKPOINT_NOT_SET, 400}, 500);

  // a set value after the gap means the tail cannot be trusted, so the
  // prefix is kept and no finish is appended
  EXPECT_EQ(segments, (std::vector<int32_t>{100, 100}));
}

TEST_F(TimerunSharedTests,
       ComputeSegmentTimes_ZeroLengthCheckpointKeepsNumbering) {
  const auto segments =
      TimerunSegments::computeSegmentTimes({100, 100, 300}, 400);

  // the second checkpoint was hit on the same command time as the first, so
  // its leg is unmeasurable - it stays as a 0 placeholder so the following
  // segments keep their numbers
  EXPECT_EQ(segments, (std::vector<int32_t>{100, 0, 200, 100}));
}

TEST_F(TimerunSharedTests, ComputeSegmentTimes_CheckpointsOnSameTick) {
  // a run where checkpoints 1 and 2 triggered simultaneously, then 3, then the
  // finish - the merged tail must not be attributed to segment 2
  const auto segments =
      TimerunSegments::computeSegmentTimes({1280, 1280, 2888}, 4064);

  EXPECT_EQ(segments, (std::vector<int32_t>{1280, 0, 1608, 1176}));
}

TEST_F(TimerunSharedTests, BestSegmentBeaten_TiesCountAsGold) {
  const std::map<std::string, std::vector<int32_t>> personal{
      {"run", {100, 200}}};
  const std::map<std::string, std::vector<int32_t>> global{{"run", {90, 210}}};

  EXPECT_EQ(TimerunSegments::bestSegmentBeaten(personal, global, "run", 0, 100),
            (std::pair<bool, bool>{true, false}));
  EXPECT_EQ(TimerunSegments::bestSegmentBeaten(personal, global, "run", 0, 90),
            (std::pair<bool, bool>{true, true}));
}

TEST_F(TimerunSharedTests, BestSegmentBeaten_SlowerThanBestIsNotGold) {
  const std::map<std::string, std::vector<int32_t>> personal{
      {"run", {100, 200}}};
  const std::map<std::string, std::vector<int32_t>> global{{"run", {90, 210}}};

  EXPECT_EQ(TimerunSegments::bestSegmentBeaten(personal, global, "run", 0, 101),
            (std::pair<bool, bool>{false, false}));
  EXPECT_EQ(TimerunSegments::bestSegmentBeaten(personal, global, "run", 1, 211),
            (std::pair<bool, bool>{false, false}));
}

TEST_F(TimerunSharedTests, BestSegmentBeaten_MissingValuesAreNotGold) {
  const std::map<std::string, std::vector<int32_t>> personal{{"run", {100}}};
  const std::map<std::string, std::vector<int32_t>> global{};

  // zero duration never counts
  EXPECT_EQ(TimerunSegments::bestSegmentBeaten(personal, global, "run", 0, 0),
            (std::pair<bool, bool>{false, false}));
  // segment out of range for the personal cache, absent from the global cache
  EXPECT_EQ(TimerunSegments::bestSegmentBeaten(personal, global, "run", 1, 50),
            (std::pair<bool, bool>{false, false}));
  // unknown run
  EXPECT_EQ(
      TimerunSegments::bestSegmentBeaten(personal, global, "other", 0, 50),
      (std::pair<bool, bool>{false, false}));
}
