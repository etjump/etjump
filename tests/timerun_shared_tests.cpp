#include <gtest/gtest.h>
#include "../src/game/etj_timerun_shared.h"

using namespace ETJump;

class TimerunSharedTests : public testing::Test {
public:
  void SetUp() override {
  }

  void TearDown() override {
  }

  std::array<int, MAX_TIMERUN_CHECKPOINTS> CreateDefaultCheckpoints() {
    return {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, -1, -1};
  }
};

TEST_F(TimerunSharedTests, Start_ShouldSerialize) {
  auto start = TimerunCommands::Start(1, 2, "3", 4, CreateDefaultCheckpoints());

  ASSERT_EQ(start.serialize(),
            "timerun start 1 2 \"3\" 4 \"1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1,-1\"");
}

TEST_F(TimerunSharedTests, Start_ShouldDeserialize) {
  auto args =
      std::vector<std::string>{"timerun", "start", "1", "2", "3", "4",
                               "1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1,-1"};
  auto deserialized = TimerunCommands::Start::deserialize(args);

  ASSERT_EQ(deserialized.value().clientNum, 1);
  ASSERT_EQ(deserialized.value().startTime, 2);
  ASSERT_EQ(deserialized.value().runName, "3");
  ASSERT_EQ(deserialized.value().previousRecord.value(), 4);
  ASSERT_EQ(deserialized.value().checkpoints[10], 11);
}

TEST_F(TimerunSharedTests, Start_ShouldSerialize_IfNoPreviousRecord) {
  auto start = TimerunCommands::Start(1, 2, "3", opt<int>(),
                                      CreateDefaultCheckpoints());

  ASSERT_EQ(start.serialize(),
            "timerun start 1 2 \"3\" -1 \"1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1,-1\"");
}

TEST_F(TimerunSharedTests, Start_ShouldDeserialize_IfNoPreviousRecord) {
  auto args = std::vector<std::string>{"timerun", "start", "1", "2", "3", "-1",
                                       "1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1,-1"};
  auto deserialized = TimerunCommands::Start::deserialize(args);

  ASSERT_FALSE(deserialized.value().previousRecord.hasValue());
}


TEST_F(TimerunSharedTests, Checkpoint_ShouldSerialize) {
  auto checkpoint = TimerunCommands::Checkpoint(1, 2, 3, "4");

  ASSERT_EQ(checkpoint.serialize(), "timerun checkpoint 1 2 3 \"4\"");
}

TEST_F(TimerunSharedTests, Checkpoint_ShouldDeserialize) {
  auto args =
      std::vector<std::string>{"timerun", "checkpoint", "1", "2", "3", "4"};
  auto checkpoint = TimerunCommands::Checkpoint::deserialize(args);

  ASSERT_EQ(checkpoint.value().runName, "4");
}

TEST_F(TimerunSharedTests, Stop_ShouldSerialize) {
  auto stop = TimerunCommands::Stop(1, 2, "run");

  ASSERT_EQ(stop.serialize(), "timerun stop 1 2 \"run\"");
}

TEST_F(TimerunSharedTests, Stop_ShouldDeserialize) {
  auto args = std::vector<std::string>{"timerun", "stop", "1", "2", "run"};
  auto stop = TimerunCommands::Stop::deserialize(args);

  ASSERT_EQ(stop.value().runName, "run");
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
  auto args = std::vector<std::string>{"timerun", "record", "1", "2", "3", "run"};
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
  auto args = std::vector<std::string>{"timerun", "completion", "1", "2", "3", "run"};
  auto interrupt = TimerunCommands::Completion::deserialize(args);

  ASSERT_EQ(interrupt.value().clientNum, 1);
  ASSERT_EQ(interrupt.value().completionTime, 2);
  ASSERT_EQ(interrupt.value().previousRecordTime.value(), 3);
  ASSERT_EQ(interrupt.value().runName, "run");
}