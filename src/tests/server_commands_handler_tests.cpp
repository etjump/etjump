#include <gtest/gtest.h>
#include "../game/etj_server_commands_handler.h"

using namespace ETJump;

class ServerCommandsHandlerTests : public testing::Test
{
public:
	void SetUp() override {
		handler = ServerCommandsHandler();
	}

	void TearDown() override {
	}

	ServerCommandsHandler handler;
};

TEST_F(ServerCommandsHandlerTests, SubscribeShouldCreateACallback)
{
	auto command = "command";
	auto called = false;
	handler.subscribe(command, [&called](int clientNum, const std::vector<std::string>& args)
	{
		called = true;
	});
	handler.check(command, 0, std::vector<std::string>());
	ASSERT_TRUE(called);
}

TEST_F(ServerCommandsHandlerTests, UnsubcribeShouldRemoveCallback)
{
	auto command = "command";
	auto called = false;
	handler.subscribe(command, [&called](int clientNum, const std::vector<std::string>& args)
	{
		called = true;
	});
	handler.check(command, 0, std::vector<std::string>());
	ASSERT_TRUE(called);
	called = false;
	handler.unsubcribe(command);
	ASSERT_FALSE(called);
}

TEST_F(ServerCommandsHandlerTests, DifferentCallbackShouldntBeCalled)
{
	auto command = "command";
	auto called = false;
	handler.subscribe(command, [&called](int clientNum, const std::vector<std::string>& args)
	{
		called = true;
	});
	handler.check("secondCommand", 0, std::vector<std::string>());
	ASSERT_FALSE(called);
}

TEST_F(ServerCommandsHandlerTests, HandlerShouldBeCaseInsensitive)
{
	auto command = "command";
	auto called = false;
	handler.subscribe(command, [&called](int clientNum, const std::vector<std::string>& args)
	{
		called = true;
	});
	handler.check("ComMand", 0, std::vector<std::string>());
	ASSERT_TRUE(called);
}

TEST_F(ServerCommandsHandlerTests, SubscribeShouldBeCaseInsensitive)
{
	auto command = "ComMand";
	auto called = false;
	handler.subscribe(command, [&called](int clientNum, const std::vector<std::string>& args)
	{
		called = true;
	});
	handler.check("command", 0, std::vector<std::string>());
	ASSERT_TRUE(called);
}