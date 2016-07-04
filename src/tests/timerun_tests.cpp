#include <gtest/gtest.h>
#include "../game/etj_timerun_session.h"

class TimerunSessionTests : public testing::Test
{
public:

	TimerunSessionTests()
	{
	}

	ETJump::TimerunSession session;
};

TEST_F(TimerunSessionTests, StartTimerShouldStartTimerForPlayer)
{
	const auto clientNum = 0;
	const auto levelTime = 1234;
	ASSERT_TRUE(session.startTimer(clientNum, levelTime));
	ASSERT_TRUE(session.isRunning(clientNum));
	ASSERT_EQ(session.runStartTime(clientNum), levelTime);
}

TEST_F(TimerunSessionTests, StartTimerShouldNotStartTimerIfPlayerIsRunning)
{
	const auto clientNum = 0;
	const auto speed = 500;
	const auto levelTime = 1234;
	const auto secondLevelTime = 4567;
	ASSERT_TRUE(session.startTimer(clientNum, levelTime, speed));
	ASSERT_TRUE(session.isRunning(clientNum));
	ASSERT_EQ(session.runStartTime(clientNum), levelTime);
	ASSERT_FALSE(session.startTime(clientNum, secondLevelTime, speed));
	ASSERT_TRUE(session.isRunning(clientNum));
	ASSERT_EQ(session.runStartTime(clientNum), levelTime);
}

TEST_F(TimerunSessionTests, StartTimerShouldNotStartTimerIfPlayerIsMovingTooFast)
{
	const auto clientNum = 0;
	const auto speed = 1000;
	const auto levelTime = 1234;
	ASSERT_FALSE(session.startTimer(clientNum, levelTime, speed));
	ASSERT_FALSE(session.isRunning(clientNum));
	ASSERT_NE(session.runStartTime(clientNum), levelTime);
}

TEST_F(TimerunSessionTests, )