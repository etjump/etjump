#include <gtest/gtest.h>
#include "../game/etj_default_vote_strategy.h"

using namespace ETJump;

class DefaultVoteStrategyTests : public testing::Test
{
public:
	DefaultVoteStrategyTests()
	{
	}

	void SetUp() override
	{
		strategy = ETJump::DefaultVoteStrategy();
	}

	static IVoteStrategy::Input createValidInput()
	{
		return IVoteStrategy::Input(50, 1, 0, 1, 1, 5000, 30000, 3000);
	}

	ETJump::DefaultVoteStrategy strategy;
};

TEST_F(DefaultVoteStrategyTests, VoteShouldNotPassOrFailBeforeMinDuration)
{
	auto input = createValidInput();
	input.minWait = 3000;
	input.duration = 2500;
	auto result = strategy.getVoteResult(input);
	ASSERT_EQ(result.status, IVoteStrategy::Status::Wait);
	input.noVotes = 1;
	input.yesVotes = 0;
	result = strategy.getVoteResult(input);
	ASSERT_EQ(result.status, IVoteStrategy::Status::Wait);
}

TEST_F(DefaultVoteStrategyTests, VoteShouldFailIfDurationHasExpired)
{
	auto input = createValidInput();
	input.maxDuration = 30000;
	input.duration = 30000;
	input.yesVotes = 0;
	input.noVotes = 0;
	auto result = strategy.getVoteResult(input);
	ASSERT_EQ(result.status, IVoteStrategy::Status::Fail);
}

TEST_F(DefaultVoteStrategyTests, VoteShouldFailIfNoVotesExceedLimit)
{
	auto input = createValidInput();
	input.yesVotes = 0;
	input.noVotes = 1;
	auto result = strategy.getVoteResult(input);
	ASSERT_EQ(result.status, IVoteStrategy::Status::Fail);
	input.votePercent = 80;
	input.noVotes = 3;
	input.yesVotes = 7;
	input.numPlayers = 10;
	result = strategy.getVoteResult(input);
	ASSERT_EQ(result.status, IVoteStrategy::Status::Fail);
}