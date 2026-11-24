#include <ctime>
#include <gtest/gtest.h>
#include <memory>

#include "../src/game/etj_shared.h"
#include "../src/game/etj_user_repository.h"

using namespace ETJump;

class UserRepositoryTests : public testing::Test {
protected:
  void SetUp() override {
    repository = std::make_unique<UserRepository>(std::make_unique<DatabaseV2>("tests", ":memory:"),
                                                  std::make_unique<DatabaseV2>("tests-legacy", ":memory:"));
  }

  int addUser(const std::string &guid, const std::string &ipv4 = "") const {
    UserModels::User user{};
    user.name = "player";
    user.guid = guid;
    user.ipv4 = ipv4;
    repository->addNewUser(user);
    return repository->getUserID(guid);
  }

  int32_t addBan(const std::string &guid, const int64_t expires, const std::string &ipv4 = "",
                 const std::vector<std::string> &hwids = {}) const {
    UserModels::BanUserParams ban{};
    ban.name     = "player";
    ban.bannedBy = "admin";
    ban.banDate  = "01/01/26 00:00:00";
    ban.expires  = expires;
    ban.guid     = guid;
    ban.ipv4     = ipv4;

    for (const auto &hwid : hwids) {
      ban.hwidBan.push_back({Constants::OS_WIN_X86_64, hwid});
    }

    return repository->banUser(ban);
  }

  static int64_t now() {
    return std::time(nullptr);
  }

  std::unique_ptr<UserRepository> repository;
};

TEST_F(UserRepositoryTests, addNewUser_CanBeFetchedByGuidAndId) {
  const int id = addUser("GUID1", "1.2.3.4");
  ASSERT_GT(id, 0);

  const auto byGuid = repository->getUserData("GUID1");
  EXPECT_EQ(byGuid.id, id);
  EXPECT_EQ(byGuid.ipv4, "1.2.3.4");

  const auto byId = repository->getUserData(id);
  EXPECT_EQ(byId.guid, "GUID1");
}

TEST_F(UserRepositoryTests, getUserID_ReturnsZeroForUnknownGuid) {
  EXPECT_EQ(repository->getUserID("UNKNOWN"), 0);
}

TEST_F(UserRepositoryTests, getHWIDsForUsersPlatform_OnlyReturnsThatPlatform) {
  const int id = addUser("GUID1");
  repository->addHwid({id, Constants::OS_WIN_X86_64, "WIN"});
  repository->addHwid({id, Constants::OS_LINUX_X86_64, "LINUX"});

  const auto win = repository->getHWIDsForUsersPlatform(id, Constants::OS_WIN_X86_64);
  ASSERT_EQ(win.size(), 1);
  EXPECT_EQ(win[0].hwid, "WIN");
  EXPECT_EQ(repository->getHWIDsForUser(id).size(), 2);
}

TEST_F(UserRepositoryTests, banUser_StoresHwidsInBanData) {
  const int32_t banId = addBan("GUID1", 0, "", {"HWID_A", "HWID_B"});

  const auto bans = repository->getBanData();
  ASSERT_EQ(bans.size(), 1);
  EXPECT_EQ(bans[0].banID, banId);
  EXPECT_EQ(bans[0].guid, "GUID1");
  EXPECT_EQ(bans[0].hwids, (std::vector<std::string>{"HWID_A", "HWID_B"}));
}

TEST_F(UserRepositoryTests, unbanUser_RemovesBan) {
  const int32_t banId = addBan("GUID1", 0, "1.2.3.4", {"HWID_A"});

  EXPECT_EQ(repository->unbanUser(banId), banId);
  EXPECT_TRUE(repository->getBanData().empty());
  EXPECT_TRUE(repository->getBannedIPAddresses().empty());
  EXPECT_EQ(repository->unbanUser(banId), 0);
}

TEST_F(UserRepositoryTests, getBanData_IncludesPermanentAndActiveTemporaryBans) {
  addBan("PERMANENT", 0);
  addBan("ACTIVE", now() + 3600);

  EXPECT_EQ(repository->getBanData().size(), 2);
}

// a temporary ban that expired while the server was down must not be enforced
TEST_F(UserRepositoryTests, getBanData_ExcludesExpiredTemporaryBans) {
  addBan("EXPIRED", now() - 3600, "", {"HWID_A"});

  EXPECT_TRUE(repository->getBanData().empty());
}

TEST_F(UserRepositoryTests, getBannedIPAddresses_ExcludesExpiredTemporaryBans) {
  addBan("EXPIRED", now() - 3600, "1.1.1.1");
  addBan("ACTIVE", now() + 3600, "2.2.2.2");
  addBan("PERMANENT", 0, "3.3.3.3");

  const auto ips = repository->getBannedIPAddresses();
  ASSERT_EQ(ips.size(), 2);
  for (const auto &ip : ips) {
    EXPECT_NE(ip.ipv4, "1.1.1.1");
  }
}

// expired bans must stay visible here, so the session can find and remove them
TEST_F(UserRepositoryTests, getBans_StillListsExpiredBans) {
  addBan("EXPIRED", now() - 3600);

  EXPECT_EQ(repository->getBans().size(), 1);
}
