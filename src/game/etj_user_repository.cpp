/*
 * MIT License
 *
 * Copyright (c) 2025 ETJump team <zero@etjump.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALING0S IN
 * THE SOFTWARE.
 */

#ifdef NEW_AUTH

  #include "etj_user_repository.h"
  #include "etj_crypto.h"

namespace ETJump {
UserRepository::UserRepository(std::unique_ptr<DatabaseV2> database,
                               std::unique_ptr<DatabaseV2> oldDatabase)
    : db(std::move(database)), oldDb(std::move(oldDatabase)) {
  migrate();
}

UserRepository::~UserRepository() {
  db = nullptr;
  oldDb = nullptr;
}

void UserRepository::insertUser(const UserModels::User &user) const {
  // when we're performing the initial migration from old database,
  // there are no GUIDs on the new format, so we must make them null
  // in order to not violate uniqueness constrain
  // realistically this is the only scenario where guid can be empty
  const std::optional<std::string> guid =
      user.guid.empty() ? std::nullopt : std::optional(user.guid);

  db->sql << R"(
    insert into users (
      id,
      name,
      guid,
      ipv4,
      ipv6,
      level,
      last_seen,
      title,
      commands,
      greeting
    ) values (
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?
    );
  )" << user.id
          << user.name << guid << user.ipv4 << user.ipv6 << user.level
          << user.lastSeen << user.title << user.commands << user.greeting;
}

void UserRepository::insertName(const UserModels::Name &name) const {
  db->sql << R"(
    insert into names (
      name,
      clean_name,
      user_id
    ) values (
      ?,
      ?,
      ?
    );
  )" << name.name
          << name.cleanName << name.userID;
}

void UserRepository::insertBan(const UserModels::Ban &ban) const {
  db->sql << R"(
    insert into bans (
      id,
      name,
      banned_by,
      ban_date,
      expires,
      reason,
      parent_ban,
      guid,
      ipv4,
      ipv6,
      legacy_guid,
      legacy_hwid
    ) values (
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?
    );
  )" << ban.banID
          << ban.name << ban.bannedBy << ban.banDate << ban.expires
          << ban.reason << ban.parentBanID << ban.guid << ban.ipv4 << ban.ipv6
          << ban.legacyGUID << ban.legacyHWID;
}

void UserRepository::addNewUser(const UserModels::User &userParams) const {
  db->sql << R"(
    insert into users (
      name,
      guid,
      ipv4,
      ipv6,
      level,
      last_seen,
      title,
      commands,
      greeting
    ) values (
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?,
      ?
    );
  )" << userParams.name
          << userParams.guid << userParams.ipv4 << userParams.ipv6
          << userParams.level << userParams.lastSeen << userParams.title
          << userParams.commands << userParams.greeting;
}

// returns empty user if the GUID is not found in the database
UserModels::User UserRepository::getUserData(const std::string &guid) const {
  UserModels::User user{};

  db->sql << R"(
    select
      *
    from users
    where
      guid=?;
  )" << guid >>
      [&user](const int id, const std::string &name, const std::string &dbGuid,
              const std::string &ipv4, const std::string &ipv6, const int level,
              const int64_t lastSeen, const std::string &title,
              const std::string &commands, const std::string &greeting) {
        // since we're fetching data by GUID, ensure it's not empty,
        // which is true while user is migrating from old GUID
        if (!dbGuid.empty()) {
          user.id = id;
          user.name = name;
          user.guid = dbGuid;
          user.ipv4 = ipv4;
          user.ipv6 = ipv6;
          user.level = level;
          user.lastSeen = lastSeen;
          user.title = title;
          user.commands = commands;
          user.greeting = greeting;
        }
      };

  return user;
}

UserModels::User UserRepository::getUserData(const int userID) const {
  UserModels::User user{};

  db->sql << R"(
    select
      *
    from users
    where
      id=?;
  )" << userID >>
      [&user](const int id, const std::string &name, const std::string &dbGuid,
              const std::string &ipv4, const std::string &ipv6, const int level,
              const int64_t lastSeen, const std::string &title,
              const std::string &commands, const std::string &greeting) {
        user.id = id;
        user.name = name;
        user.guid = dbGuid;
        user.ipv4 = ipv4;
        user.ipv6 = ipv6;
        user.level = level;
        user.lastSeen = lastSeen;
        user.title = title;
        user.commands = commands;
        user.greeting = greeting;
      };

  return user;
}

// returns 0 if no user is found with the given GUID
int UserRepository::getUserID(const std::string &guid) const {
  int userID = 0;

  db->sql << R"(
    select
      id
    from users where
      guid=?;
  )" << guid >>
      userID;

  return userID;
}

UserModels::LegacyAuth
UserRepository::getLegacyAuthData(const std::string &oldGuid) const {
  UserModels::LegacyAuth legacyAuth{};

  db->sql << R"(
    select
      *
    from legacy_auth
    where
      guid=?;
  )" << oldGuid >>
      [&legacyAuth](const int user_id, const std::string &guid,
                    const std::string &hwid) {
        legacyAuth.userID = user_id;
        legacyAuth.guid = guid;
        legacyAuth.hwid = hwid;
      };

  return legacyAuth;
}

void UserRepository::migrateGuid(const int oldID,
                                 const std::string &newGUID) const {
  int count = 0;
  db->sql << R"(select count(*) from users where guid=?;)" << newGUID >> count;

  // if we're migrating manually, we need to delete the existing entry
  if (count != 0) {
    int userID = 0;

    db->sql << R"(
      select
        id
      from users
      where
        guid=?;
    )" << newGUID >>
        userID;

    if (userID == oldID) {
      throw std::runtime_error(
          "Current GUID is already associated with your current user ID");
    }

    // TODO: this should probably also erase timerun records for the user?
    db->sql << "delete from users where id=?;" << userID;
    db->sql << "delete from users_hwid where user_id=?;" << userID;
  }

  db->sql << R"(
    update
      users
    set
      guid=?
    where
      id=?;
    )" << newGUID
          << oldID;
}

std::vector<UserModels::UserHWID>
UserRepository::getHWIDsForUser(const int userID) const {
  std::vector<UserModels::UserHWID> userHwids{};

  db->sql << R"(
    select
      user_id,
      platform,
      hwid
    from users_hwid
    where
      user_id=?;
  )" << userID >>
      [&userHwids](const int id, const int platform, const std::string &hwid) {
        UserModels::UserHWID userHwid{};
        userHwid.userID = id;
        userHwid.platform = platform;
        userHwid.hwid = hwid;

        userHwids.emplace_back(userHwid);
      };

  return userHwids;
}

std::vector<UserModels::UserHWID>
UserRepository::getHWIDsForUsersPlatform(const int userID,
                                         const int userPlatform) const {
  std::vector<UserModels::UserHWID> userHwids{};

  db->sql << R"(
    select
      user_id,
      platform,
      hwid
    from users_hwid
    where
      user_id=? and
      platform=?;
  )" << userID
          << userPlatform >>
      [&userHwids](const int id, const int platform, const std::string &hwid) {
        UserModels::UserHWID userHwid{};
        userHwid.userID = id;
        userHwid.platform = platform;
        userHwid.hwid = hwid;

        userHwids.emplace_back(userHwid);
      };

  return userHwids;
}

void UserRepository::addHwid(const UserModels::UserHWID &params) const {
  db->sql << R"(
    insert into users_hwid (
      user_id,
      platform,
      hwid
    ) values (
      ?,
      ?,
      ?
    );
  )" << params.userID
          << params.platform << params.hwid;
}

void UserRepository::updateIPv4(const int userID, const std::string &ip) const {
  db->sql << R"(
    update
      users
    set
      ipv4=?
    where
      id=?;
  )" << ip << userID;
}

void UserRepository::updateIPv6(const int userID, const std::string &ip) const {
  db->sql << R"(
    update
      users
    set
      ipv6=?
    where
      id=?;
  )" << ip << userID;
}

std::vector<UserModels::BannedIPAddresses>
UserRepository::getBannedIPAddresses() const {
  std::vector<UserModels::BannedIPAddresses> bannedIPAddresses{};

  db->sql << R"(
    select
      ipv4,
      ipv6
    from bans;
  )" >>
      [&bannedIPAddresses](const std::string &ipv4, const std::string &ipv6) {
        UserModels::BannedIPAddresses bannedIP{};
        bannedIP.ipv4 = ipv4;
        bannedIP.ipv6 = ipv6;

        bannedIPAddresses.emplace_back(bannedIP);
      };

  return bannedIPAddresses;
}

void UserRepository::migrate() const {
  db->addMigration("initial", createInitialMigration());
  db->applyMigrations();

  int count = 0;
  db->sql << "select count(*) from users" >> count;

  if (count == 0) {
    migrateUsers();
  }

  count = 0;
  db->sql << "select count(*) from names" >> count;

  if (count == 0) {
    migrateNames();
  }

  count = 0;
  db->sql << "select count(*) from bans" >> count;

  if (count == 0) {
    migrateBans();
  }
}

void UserRepository::migrateUsers() const {
  if (!oldTableHasData("users")) {
    return;
  }

  std::vector<UserModels::User> oldUsers{};
  std::vector<UserModels::LegacyAuth> oldAuth{};

  // the old system has had some bugs throughout the years, and it's possible
  // that there are entries with malformed info
  // we don't want these, but because user IDs are tied to timerun database,
  // we just mark these as invalid rather than removing them
  // FIXME: TODO: I'd really like to just get rid of these entries,
  //  but we cannot change user IDs without having a logic to update
  //  the timerun database entries to reflect the updates user IDs.
  oldDb->sql << "select * from users;" >>
      [&oldUsers,
       &oldAuth](const int id, const std::string &guid, const int level,
                 const int64_t lastSeen, const std::string &name,
                 const std::string &hwid, const std::string &title,
                 const std::string &commands, const std::string &greeting) {
        // it's possible that there are entries with an invalid id
        if (id > 0) {
          UserModels::User user{};
          UserModels::LegacyAuth legacyAuth{};

          if (!Crypto::isValidSHA1(guid)) {
            user.id = id;
            user.name = "MALFORMED_ENTRY";

            legacyAuth.userID = id;
            legacyAuth.guid = "MALFORMED_ENTRY";
            legacyAuth.hwid = "MALFORMED_ENTRY";
          } else {
            user.id = id;
            user.name = name;
            user.level = level;
            user.lastSeen = lastSeen;
            user.title = title;
            user.commands = commands;
            user.greeting = greeting;

            legacyAuth.userID = id;
            legacyAuth.guid = guid;
            legacyAuth.hwid = hwid;
          }

          oldUsers.emplace_back(user);
          oldAuth.emplace_back(legacyAuth);
        }
      };

  db->sql << "begin;";

  for (const auto &user : oldUsers) {
    insertUser(user);
  }

  for (const auto &auth : oldAuth) {
    insertLegacyAuth(auth);
  }

  db->sql << "commit;";
}

void UserRepository::migrateNames() const {
  if (!oldTableHasData("name")) {
    return;
  }

  std::vector<UserModels::Name> oldNames{};

  oldDb->sql << R"(
    select
      clean_name,
      name,
      user_id
    from name;
  )" >>
      [&oldNames](const std::string &cleanName, const std::string &name,
                  const int id) {
        UserModels::Name names{};

        names.name = name;
        names.cleanName = cleanName;
        names.userID = id;

        oldNames.emplace_back(names);
      };

  db->sql << "begin;";

  for (const auto &name : oldNames) {
    insertName(name);
  }

  db->sql << "commit;";
}

void UserRepository::migrateBans() const {
  if (!oldTableHasData("bans")) {
    return;
  }

  std::vector<UserModels::Ban> oldBans{};

  oldDb->sql << "select * from bans;" >>
      [&oldBans](const int id, const std::string &name, const std::string &guid,
                 const std::string &hwid, const std::string &ip,
                 const std::string &bannedBy, const std::string &banDate,
                 const int64_t expires, const std::string &reason) {
        UserModels::Ban ban{};

        ban.banID = id;
        ban.name = name;
        ban.bannedBy = bannedBy;
        ban.banDate = banDate;
        ban.expires = expires;
        ban.reason = reason;
        ban.ipv4 = ip;
        ban.legacyGUID = guid;
        ban.legacyHWID = hwid;

        oldBans.emplace_back(ban);
      };

  db->sql << "begin;";

  for (const auto &ban : oldBans) {
    insertBan(ban);
  }

  db->sql << "commit;";
}

void UserRepository::insertLegacyAuth(
    const UserModels::LegacyAuth &auth) const {
  db->sql << R"(
    insert into legacy_auth (
      user_id,
      guid,
      hwid
    ) values (
      ?,
      ?,
      ?
    );
  )" << auth.userID
          << auth.guid << auth.hwid;
}

bool UserRepository::oldTableHasData(const std::string &table) const {
  const std::string stmt =
      "select count(*) from sqlite_master where tbl_name='" + table + "'";
  int count = 0;
  oldDb->sql << stmt >> count;

  return count != 0;
}

std::vector<std::string> UserRepository::createInitialMigration() {
  const std::string usersStmt = R"(
    create table users (
      id integer primary key autoincrement,
      name text not null,
      guid text unique,
      ipv4 text,
      ipv6 text,
      level integer not null,
      last_seen integer,
      title text,
      commands text,
      greeting text
    );
  )";

  const std::string namesStmt = R"(
    create table names (
      id integer primary key autoincrement,
      name text not null,
      clean_name text not null,
      user_id integer not null,
      foreign key (user_id) references users(id)
    );
  )";

  const std::string hwidStmt = R"(
    create table users_hwid (
      id integer primary key autoincrement,
      user_id integer not null,
      platform integer not null,
      hwid text not null,
      foreign key (user_id) references users(id)
    );
  )";

  // FIXME: get rid of legacy_hwid? it's probably not useful
  const std::string bansStmt = R"(
    create table bans (
      id integer primary key autoincrement,
      name text not null,
      banned_by text not null,
      ban_date text not null,
      expires integer,
      reason text,
      parent_ban integer,
      guid text not null,
      ipv4 text,
      ipv6 text,
      legacy_guid,
      legacy_hwid
    );
  )";

  const std::string hwidBansStmt = R"(
    create table hwid_bans (
      ban_id integer not null,
      platform integer not null,
      hwid text not null,
      foreign key (ban_id) references bans(id)
    );
  )";

  // FIXME: get rid of hwid? it's probably not useful
  const std::string legacyAuthStmt = R"(
    create table legacy_auth (
      user_id integer primary key,
      guid text not null,
      hwid text not null,
      foreign key (user_id) references users(id)
    );
  )";

  return {usersStmt, namesStmt,    hwidStmt,
          bansStmt,  hwidBansStmt, legacyAuthStmt};
}
} // namespace ETJump
#endif
