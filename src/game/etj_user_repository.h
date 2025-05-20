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
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#ifdef NEW_AUTH
  #include "etj_database_v2.h"
  #include "etj_user_models.h"

  #include <memory>

namespace ETJump {
class UserRepository {
public:
  UserRepository(std::unique_ptr<DatabaseV2> database,
                 std::unique_ptr<DatabaseV2> oldDatabase);
  ~UserRepository();

  void insertUser(const UserModels::User &user) const;
  void insertName(const UserModels::Name &name) const;
  void insertBan(const UserModels::Ban &ban) const;

  void addNewUser(const UserModels::User &userParams) const;

  UserModels::User getUserData(const std::string &guid) const;
  UserModels::User getUserData(int userID) const;
  int getUserID(const std::string &guid) const;
  UserModels::LegacyAuth getLegacyAuthData(const std::string &oldGuid) const;
  void migrateGuid(int oldID, const std::string &newGUID) const;

  std::vector<UserModels::UserHWID> getHWIDsForUser(int userID) const;
  std::vector<UserModels::UserHWID>
  getHWIDsForUsersPlatform(int userID, int userPlatform) const;
  void addHwid(const UserModels::UserHWID &params) const;

  void updateIPv4(int userID, const std::string &ip) const;
  void updateIPv6(int userID, const std::string &ip) const;

  std::vector<UserModels::BannedIPAddresses> getBannedIPAddresses() const;

private:
  void migrate() const;
  void migrateUsers() const;
  void migrateNames() const;
  void migrateBans() const;

  void insertLegacyAuth(const UserModels::LegacyAuth &auth) const;

  bool oldTableHasData(const std::string &table) const;
  static std::vector<std::string> createInitialMigration();

  std::unique_ptr<DatabaseV2> db;
  std::unique_ptr<DatabaseV2> oldDb;
};
} // namespace ETJump
#endif
