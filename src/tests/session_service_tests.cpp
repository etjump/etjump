#include <gtest/gtest.h>
#include "../game/etj_session_service.h"

class SessionServiceTests : public testing::Test
{
public:
	void SetUp() override {
	}
	void TearDown() override {
	}
};

TEST_F(SessionServiceTests, ParsePermissionsShouldIncludeAllPermissions)
{
	auto permissions = ETJump::parsePermissions("*", "");
	for (int i = 0; i < ETJump::SessionService::CachedUserData::MAX_PERMISSIONS; ++i)
	{
		ASSERT_TRUE(permissions[i]);
	}
}

TEST_F(SessionServiceTests, ParsePermissionsShouldIncludeAllPermissionsEvenIfStarIsntFirst)
{
	auto permissions = ETJump::parsePermissions("abcdef*", "");
	for (int i = 0; i < ETJump::SessionService::CachedUserData::MAX_PERMISSIONS; ++i)
	{
		ASSERT_TRUE(permissions[i]);
	}
}

TEST_F(SessionServiceTests, ParsePermissionsShouldIncludeAllBut3Commands)
{
	auto permissions = ETJump::parsePermissions("*-adf", "");
	for (int i = 0; i < ETJump::SessionService::CachedUserData::MAX_PERMISSIONS; ++i)
	{
		if (i == static_cast<int>('a') || i == static_cast<int>('d') || i == static_cast<int>('f'))
		{
			ASSERT_FALSE(permissions[i]);
		} else
		{
			ASSERT_TRUE(permissions[i]);
		}
	}
}

TEST_F(SessionServiceTests, ParsePermissionsShouldIncludeAllButA)
{
	auto permissions = ETJump::parsePermissions("*", "-a");
	for (int i = 0; i < ETJump::SessionService::CachedUserData::MAX_PERMISSIONS; ++i)
	{
		if (i == static_cast<int>('a'))
		{
			ASSERT_FALSE(permissions[i]);
		}
		else
		{
			ASSERT_TRUE(permissions[i]);
		}
	}
}