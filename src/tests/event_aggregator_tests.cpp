#include <gtest/gtest.h>
#include "../game/etj_event_aggregator.h"

class EventAggregatorTests : public testing::Test
{
public:
	void SetUp() override {
		eventAggregator = ETJump::EventAggregator();
	}

	void TearDown() override{

	}

	ETJump::EventAggregator eventAggregator;
};

TEST_F(EventAggregatorTests, ClientEventCallbackShouldBeCalled)
{
	auto calledWith = -1;
	eventAggregator.subscribe(ETJump::EventAggregator::ClientEventType::ClientConnect, [&](int clientNum)
	{
		calledWith = clientNum;
	});
	const auto clientNum = 3;
	eventAggregator.clientEvent(ETJump::EventAggregator::ClientEventType::ClientConnect, clientNum);
	ASSERT_EQ(calledWith, clientNum);
}

TEST_F(EventAggregatorTests, UnsubscribedClientEventCallbackShouldntBeCalled)
{
	auto calledWith = -1;
	int handle = eventAggregator.subscribe(ETJump::EventAggregator::ClientEventType::ClientConnect, [&](int clientNum)
	{
		calledWith = clientNum;
	});
	auto clientNum = 3;
	eventAggregator.clientEvent(ETJump::EventAggregator::ClientEventType::ClientConnect, clientNum);
	ASSERT_EQ(calledWith, clientNum);
	eventAggregator.unsubcribe(handle);
	clientNum = 5;
	eventAggregator.clientEvent(ETJump::EventAggregator::ClientEventType::ClientConnect, clientNum);
	ASSERT_NE(calledWith, clientNum);
}

TEST_F(EventAggregatorTests, SameClientEventHandleShouldntBeReturnedTwice)
{
	int firstHandle = eventAggregator.subscribe(ETJump::EventAggregator::ClientEventType::ClientBegin, [](int clientNum) {});
	int secondHandle = eventAggregator.subscribe(ETJump::EventAggregator::ClientEventType::ClientBegin, [](int clientNum) {});
	ASSERT_NE(firstHandle, secondHandle);
}

TEST_F(EventAggregatorTests, ServerEventCallbackShouldBeCalled)
{
	auto called = false;
	eventAggregator.subscribe(ETJump::EventAggregator::ServerEventType::RunFrame, [&]()
	{
		called = true;
	});
	eventAggregator.serverEvent(ETJump::EventAggregator::ServerEventType::RunFrame);
	ASSERT_TRUE(called);
}

TEST_F(EventAggregatorTests, UnsubscribedServerEventCallbackShouldntBeCalled)
{
	bool called = false;
	int handle = eventAggregator.subscribe(ETJump::EventAggregator::ServerEventType::RunFrame, [&]()
	{
		called = true;
	});
	eventAggregator.serverEvent(ETJump::EventAggregator::ServerEventType::RunFrame);
	ASSERT_TRUE(called);
	called = false;
	eventAggregator.unsubcribe(handle);
	eventAggregator.serverEvent(ETJump::EventAggregator::ServerEventType::RunFrame);
	ASSERT_FALSE(called);
}

TEST_F(EventAggregatorTests, NotAllEventsShouldBeCalled)
{
	auto firstCalled = false;
	auto secondCalled = false;
	eventAggregator.subscribe(ETJump::EventAggregator::ClientEventType::ClientBegin, [&](int clientNum)
	{
		firstCalled = true;
	});
	eventAggregator.subscribe(ETJump::EventAggregator::ClientEventType::ClientThink, [&](int clientNum)
	{
		secondCalled = true;
	});
	eventAggregator.clientEvent(ETJump::EventAggregator::ClientEventType::ClientThink, 0);
	ASSERT_FALSE(firstCalled);
	ASSERT_TRUE(secondCalled);
}