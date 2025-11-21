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

  [[nodiscard]] bool userExists(int32_t userID) const;

  [[nodiscard]] UserModels::User getUserData(const std::string &guid) const;
  [[nodiscard]] UserModels::User getUserData(int userID) const;
  [[nodiscard]] int getUserID(const std::string &guid) const;
  [[nodiscard]] UserModels::LegacyAuth
  getLegacyAuthData(const std::string &oldGuid) const;
  [[nodiscard]] UserModels::LegacyAuth getLegacyAuthData(int32_t userID) const;
  void migrateGuid(int oldID, const std::string &newGUID) const;

  [[nodiscard]] std::vector<UserModels::UserHWID>
  getHWIDsForUser(int userID) const;
  [[nodiscard]] std::vector<UserModels::UserHWID>
  getHWIDsForUsersPlatform(int userID, int userPlatform) const;
  void addHwid(const UserModels::UserHWID &params) const;

  void updateIPv4(int userID, const std::string &ip) const;
  void updateIPv6(int userID, const std::string &ip) const;

  void updateLastSeen(int32_t userID, int64_t time) const;

  // false if the name already stored for the user
  [[nodiscard]] bool addNewName(const UserModels::Name &name) const;

  // NOTE: does not report errors for invalid user ID!
  void editUser(const UserModels::EditUserParams &params) const;

  void setLevel(int32_t userID, int32_t level) const;
  // returns the number of users who's level was deleted
  [[nodiscard]] int32_t deleteLevel(int32_t level) const;

  [[nodiscard]] std::vector<UserModels::User> getUsers() const;
  [[nodiscard]] std::vector<std::pair<int32_t, std::string>>
  getUsersByName(const std::string &name) const;
  [[nodiscard]] std::vector<std::string> getUserNames(int32_t userID) const;

  [[nodiscard]] std::vector<UserModels::BannedIPAddresses>
  getBannedIPAddresses() const;
  [[nodiscard]] std::vector<UserModels::BanData> getBanData() const;
  [[nodiscard]] std::vector<UserModels::Ban> getBans() const;

private:
  void migrate() const;
  void migrateUsers() const;
  void migrateNames() const;
  void migrateBans() const;

  void insertLegacyAuth(const UserModels::LegacyAuth &auth) const;

  [[nodiscard]] bool oldTableHasData(const std::string &table) const;
  static std::vector<std::string> createInitialMigration();

  std::unique_ptr<DatabaseV2> db;
  std::unique_ptr<DatabaseV2> oldDb;
};
} // namespace ETJump
#endif
