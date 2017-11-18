#include <gtest/gtest.h>
#include "../cgame/etj_entity_events_handler.h"
#include "../cgame/cg_local.h"

using namespace ETJump;

centity_t ent1;

class EntityEventsHandlerTests : public testing::Test
{
public:
	void SetUp() override
	{
		ent1.currentState.number = 1;

		handler = EntityEventsHandler();
	}

	void TearDown() override
	{
	}

	EntityEventsHandler handler;
};

TEST_F(EntityEventsHandlerTests, SubscribeShouldCreateACallback)
{
	auto eventName = "event";
	auto called = false;

	handler.subscribe(eventName, [&called](centity_t *cent)
	{
		called = true;
	});

	handler.check(eventName, &ent1);
	ASSERT_TRUE(called);
}

TEST_F(EntityEventsHandlerTests, UnsubcribeShouldRemoveCallback)
{
	auto eventName = "event";
	auto called = false;

	handler.subscribe(eventName, [&called](centity_t *cent)
	{
		called = true;
	});

	handler.check(eventName, &ent1);
	ASSERT_TRUE(called);
	called = false;

	handler.unsubcribe(eventName);
	ASSERT_FALSE(called);
}

TEST_F(EntityEventsHandlerTests, DifferentCallbackShouldntBeCalled)
{
	auto eventName = "event";
	auto called = false;

	handler.subscribe(eventName, [&called](centity_t *cent)
	{
		called = true;
	});

	handler.check("otherEvent", &ent1);
	ASSERT_FALSE(called);
}

TEST_F(EntityEventsHandlerTests, HandlerShouldBeCaseInsensitive)
{
	auto eventName = "event";
	auto called = false;

	handler.subscribe(eventName, [&called](centity_t *cent)
	{
		called = true;
	});

	handler.check("EVeNT", &ent1);
	ASSERT_TRUE(called);
}

TEST_F(EntityEventsHandlerTests, SubscribeShouldBeCaseInsensitive)
{
	auto eventName = "EvEnt";
	auto called = false;

	handler.subscribe(eventName, [&called](centity_t *cent)
	{
		called = true;
	});

	handler.check("event", &ent1);
	ASSERT_TRUE(called);
}

TEST_F(EntityEventsHandlerTests, SubscribeShouldPassCorrectEntityInCallback)
{
	auto command = "event";
	auto called  = false;

	handler.subscribe(command, [&called](centity_t *cent)
	{
		if (cent->currentState.number == ent1.currentState.number)
		{
		    called = true;
		}
	});

	handler.check("event", &ent1);
	ASSERT_TRUE(called);
}

TEST_F(EntityEventsHandlerTests, SubscribeShouldAllowMultipleCallbacks)
{
	auto command = "event";
	auto count = 0;

	handler.subscribe(command, [&count](centity_t *cent)
	{
		count++;
	});

	handler.subscribe(command, [&count](centity_t *cent)
	{
		count++;
	});

	handler.check("event", &ent1);
	ASSERT_TRUE(count == 2);
}

TEST_F(EntityEventsHandlerTests, CheckShouldWorkWithIntAsName)
{
	auto called = false;

	handler.subscribe("__event__24", [&called](centity_t *cent)
	{
		called = true;
	});

	handler.check(EV_JUMP, &ent1);
	ASSERT_TRUE(called);
}

TEST_F(EntityEventsHandlerTests, SubscribeShouldWorkWithIntAsName)
{
	auto called = false;

	handler.subscribe(EV_JUMP, [&called](centity_t *cent)
	{
		called = true;
	});

	handler.check("__event__24", &ent1);
	ASSERT_TRUE(called);
}