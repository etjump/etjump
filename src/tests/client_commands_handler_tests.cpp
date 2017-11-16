#include <gtest/gtest.h>
#include <memory>
#include "../cgame/etj_client_commands_handler.h"

using namespace ETJump;

class ClientCommandsHandlerTests : public testing::Test
{
public:
	void SetUp() override
	{
		handler = std::unique_ptr<ClientCommandsHandler>(new ClientCommandsHandler(nullptr));
	}

	void TearDown() override
	{
	}

	std::unique_ptr<ClientCommandsHandler> handler;
};

TEST_F(ClientCommandsHandlerTests, SubscribeShouldCreateACallback)
{
	auto command = "command";
	auto called  = false;

	handler->subscribe(command, [&called](const std::vector<std::string>& args)
	{
		called = true;
	});
	handler->check(command, std::vector<std::string>());
	ASSERT_TRUE(called);
}

TEST_F(ClientCommandsHandlerTests, UnsubcribeShouldRemoveCallback)
{
	auto command = "command";
	auto called  = false;

	handler->subscribe(command, [&called](const std::vector<std::string>& args)
	{
		called = true;
	});
	handler->check(command, std::vector<std::string>());
	ASSERT_TRUE(called);
	called = false;
	handler->unsubcribe(command);
	ASSERT_FALSE(called);
}

TEST_F(ClientCommandsHandlerTests, DifferentCallbackShouldntBeCalled)
{
	auto command = "command";
	auto called  = false;

	handler->subscribe(command, [&called](const std::vector<std::string>& args)
	{
		called = true;
	});
	handler->check("secondCommand", std::vector<std::string>());
	ASSERT_FALSE(called);
}

TEST_F(ClientCommandsHandlerTests, HandlerShouldBeCaseInsensitive)
{
	auto command = "command";
	auto called  = false;

	handler->subscribe(command, [&called](const std::vector<std::string>& args)
	{
		called = true;
	});
	handler->check("ComMand", std::vector<std::string>());
	ASSERT_TRUE(called);
}

TEST_F(ClientCommandsHandlerTests, SubscribeShouldBeCaseInsensitive)
{
	auto command = "ComMand";
	auto called  = false;

	handler->subscribe(command, [&called](const std::vector<std::string>& args)
	{
		called = true;
	});
	handler->check("command", std::vector<std::string>());
	ASSERT_TRUE(called);
}
