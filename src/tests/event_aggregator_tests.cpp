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
		auto connectPayload = reinterpret_cast<const ETJump::EventAggregator::ClientConnectPayload*>(payload);
		ASSERT_NE(connectPayload, nullptr);
		ASSERT_EQ(connectPayload->clientNum, 3);
		ASSERT_EQ(connectPayload->isBot, 1);
		ASSERT_EQ(connectPayload->firstTime, 1);
	});
	const auto clientNum = 3;
	auto payload = ETJump::EventAggregator::ClientConnectPayload{};
	payload.clientNum = 3;
	payload.isBot = 1;
	payload.firstTime = 1;
	eventAggregator.clientEvent(ETJump::EventAggregator::ClientEventType::ClientConnect, &payload);
}

TEST_F(EventAggregatorTests, UnsubscribedClientEventCallbackShouldntBeCalled)
{
	auto calledWith = -1;
	int handle = eventAggregator.subscribe(ETJump::EventAggregator::ClientEventType::ClientConnect, [&](const ETJump::EventAggregator::Payload *payload)
	{
		auto cnPayload = reinterpret_cast<const ETJump::EventAggregator::ClientNumPayload*>(payload);
		ASSERT_NE(cnPayload, nullptr);
		ASSERT_EQ(cnPayload->clientNum, 32);
		calledWith = cnPayload->clientNum;
	});
	auto clientNum = 32;
	auto payload = ETJump::EventAggregator::ClientNumPayload{};
	payload.clientNum = clientNum;
	eventAggregator.clientEvent(ETJump::EventAggregator::ClientEventType::ClientConnect, &payload);
	ASSERT_EQ(calledWith, clientNum);
	eventAggregator.unsubcribe(handle);
	clientNum = 5;
	payload.clientNum = 5;
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
	ETJump::EventAggregator::ClientNumPayload payload{};
	payload.clientNum = 0;
	eventAggregator.clientEvent(ETJump::EventAggregator::ClientEventType::ClientThink, &payload);
	ASSERT_FALSE(firstCalled);
	ASSERT_TRUE(secondCalled);
}