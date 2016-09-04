#include <gtest/gtest.h>
#include "../game/etj_event_aggregator.h"

class EventAggregatorTests : public testing::Test
{
public:
	void SetUp() override {
		eventAggregator = ETJump::EventAggregator();
	}

	void TearDown() override {

	}

	ETJump::EventAggregator eventAggregator;
};

TEST_F(EventAggregatorTests, ClientEventCallbackShouldBeCalled)
{
	auto calledWith = -1;
	eventAggregator.subscribe(ETJump::EventAggregator::ClientEventType::ClientConnect, [&](const ETJump::EventAggregator::Payload *payload)
	{
		ASSERT_NE(payload, nullptr);
		ASSERT_EQ(payload->integers.size(), 1);
		calledWith = payload->integers[0];
	});
	const auto clientNum = 3;
	auto payload = ETJump::EventAggregator::Payload{ std::vector<int>{clientNum} };
	eventAggregator.clientEvent(ETJump::EventAggregator::ClientEventType::ClientConnect, &payload);
	ASSERT_EQ(calledWith, clientNum);
}

TEST_F(EventAggregatorTests, UnsubscribedClientEventCallbackShouldntBeCalled)
{
	auto calledWith = -1;
	int handle = eventAggregator.subscribe(ETJump::EventAggregator::ClientEventType::ClientConnect, [&](const ETJump::EventAggregator::Payload *payload)
	{
		ASSERT_NE(payload, nullptr);
		ASSERT_EQ(payload->integers.size(), 1);
		calledWith = payload->integers[0];
	});
	auto clientNum = 3;
	auto payload = ETJump::EventAggregator::Payload{ std::vector<int>{clientNum} };
	eventAggregator.clientEvent(ETJump::EventAggregator::ClientEventType::ClientConnect, &payload);
	ASSERT_EQ(calledWith, clientNum);
	eventAggregator.unsubcribe(handle);
	clientNum = 5;
	payload.integers[0] = 5;
	eventAggregator.clientEvent(ETJump::EventAggregator::ClientEventType::ClientConnect, &payload);
	ASSERT_NE(calledWith, clientNum);
}

TEST_F(EventAggregatorTests, SameClientEventHandleShouldntBeReturnedTwice)
{
	int firstHandle = eventAggregator.subscribe(ETJump::EventAggregator::ClientEventType::ClientBegin, [](const ETJump::EventAggregator::Payload *) {});
	int secondHandle = eventAggregator.subscribe(ETJump::EventAggregator::ClientEventType::ClientBegin, [](const ETJump::EventAggregator::Payload *) {});
	ASSERT_NE(firstHandle, secondHandle);
}

TEST_F(EventAggregatorTests, ServerEventCallbackShouldBeCalled)
{
	auto called = false;
	eventAggregator.subscribe(ETJump::EventAggregator::ServerEventType::RunFrame, [&](const ETJump::EventAggregator::Payload *payload)
	{
		called = true;
	});
	eventAggregator.serverEvent(ETJump::EventAggregator::ServerEventType::RunFrame, nullptr);
	ASSERT_TRUE(called);
}

TEST_F(EventAggregatorTests, UnsubscribedServerEventCallbackShouldntBeCalled)
{
	bool called = false;
	int handle = eventAggregator.subscribe(ETJump::EventAggregator::ServerEventType::RunFrame, [&](const ETJump::EventAggregator::Payload *)
	{
		called = true;
	});
	eventAggregator.serverEvent(ETJump::EventAggregator::ServerEventType::RunFrame, nullptr);
	ASSERT_TRUE(called);
	called = false;
	eventAggregator.unsubcribe(handle);
	eventAggregator.serverEvent(ETJump::EventAggregator::ServerEventType::RunFrame, nullptr);
	ASSERT_FALSE(called);
}

TEST_F(EventAggregatorTests, NotAllEventsShouldBeCalled)
{
	auto firstCalled = false;
	auto secondCalled = false;
	eventAggregator.subscribe(ETJump::EventAggregator::ClientEventType::ClientBegin, [&](const ETJump::EventAggregator::Payload *payload)
	{
		firstCalled = true;
	});
	eventAggregator.subscribe(ETJump::EventAggregator::ClientEventType::ClientThink, [&](const ETJump::EventAggregator::Payload *payload)
	{
		secondCalled = true;
	});
	const ETJump::EventAggregator::Payload payload{ std::vector<int>{0} };
	eventAggregator.clientEvent(ETJump::EventAggregator::ClientEventType::ClientThink, &payload);
	ASSERT_FALSE(firstCalled);
	ASSERT_TRUE(secondCalled);
}