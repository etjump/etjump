#include "../game/etj_deathrun_system.h"
#include <gtest/gtest.h>

using namespace ETJump;

class DeathrunSystemTests : public testing::Test
{
public:
	void SetUp() override {
	}

	void TearDown() override {
	}

	static DeathrunSystem createSystemWithCheckpoints()
	{
		auto system = DeathrunSystem();
		auto location = DeathrunSystem::PrintLocation::Chat;
		auto msg = "";
		auto sound = "";
		system.createCheckpoint(location, msg, sound);
		system.createCheckpoint(location, msg, sound);
		system.createCheckpoint(location, msg, sound);
		return system;
	}
};

TEST_F(DeathrunSystemTests, CreateCheckpoint_Returns_DifferentId_OnMultipleCalls)
{
	auto location = DeathrunSystem::PrintLocation::Chat;
	auto msg = "";
	auto sound = "";
	auto system = DeathrunSystem();
	ASSERT_NE(system.createCheckpoint(location, msg, sound), system.createCheckpoint(location, msg, sound));
}

TEST_F(DeathrunSystemTests, HitCheckpoint_Returns_0_WhenRunHasntStarted)
{
	auto system = createSystemWithCheckpoints();
	ASSERT_EQ(system.hitCheckpoint(0, 0), 0);
}

TEST_F(DeathrunSystemTests, HitCheckpoint_Returns_1_WhenFirstCheckpointIsHit)
{
	auto system = createSystemWithCheckpoints();
	int player = 0;
	system.hitStart(player);
	ASSERT_EQ(system.hitCheckpoint(0, player), 1);
}

TEST_F(DeathrunSystemTests, HitStart_Returns_True_WhenRunWasStarted)
{
	auto system = createSystemWithCheckpoints();
	ASSERT_TRUE(system.hitStart(0));
}

TEST_F(DeathrunSystemTests, HitStart_Returns_False_WhenRunWasAlreadyRunning)
{
	auto system = createSystemWithCheckpoints();

	system.hitStart(0);
	ASSERT_FALSE(system.hitStart(0));
}

TEST_F(DeathrunSystemTests, HitStart_Returns_True_WhenRunWasStartedForAnotherPlayer)
{
	auto system = createSystemWithCheckpoints();
	system.hitStart(0);
	ASSERT_TRUE(system.hitStart(1));
}

TEST_F(DeathrunSystemTests, HitCheckpoint_Returns_False_WhenSameCheckpointIsHit)
{
	auto system = createSystemWithCheckpoints();
	int player = 0;
	int checkpoint = 0;
	system.hitStart(player);
	system.hitCheckpoint(checkpoint, player); 
	ASSERT_FALSE(system.hitCheckpoint(checkpoint, player));
}

TEST_F(DeathrunSystemTests, HitCheckpoint_Returns_True_WhenDiffCheckpointIsHit)
{
	auto system = createSystemWithCheckpoints();
	int player = 0;
	int cp1 = 0;
	int cp2 = 1;
	system.hitStart(player);
	
	system.hitCheckpoint(cp1, player); 
	ASSERT_TRUE(system.hitCheckpoint(cp2, player));
}