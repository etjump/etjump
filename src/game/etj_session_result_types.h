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
  #include <optional>

  #include "etj_synchronization_context.h"
  #include "etj_user_models.h"

namespace ETJump {
// a successful result will have userID > 0
class AuthenticationResult final : public SynchronizationContext::ResultBase {
public:
  AuthenticationResult(const int userID, const bool isBanned)
      : userID(userID), isBanned(isBanned) {}

  int userID;
  bool isBanned;
};

class GuidMigrationResult final : public SynchronizationContext::ResultBase {
public:
  GuidMigrationResult(const int userID, const bool isBanned,
                      std::string message)
      : userID(userID), isBanned(isBanned), message(std::move(message)) {}

  int userID;
  bool isBanned;
  std::string message;
};

class AddUserResult final : public SynchronizationContext::ResultBase {
public:
  AddUserResult(const int userID, std::string message)
      : userID(userID), message(std::move(message)) {}

  int userID;
  std::string message;
};

class IPBanResult final : public SynchronizationContext::ResultBase {
public:
  IPBanResult(const bool isBanned, std::string ip)
      : isBanned(isBanned), ip(std::move(ip)) {}

  bool isBanned;
  std::string ip;
};

class GetUserDataResult final : public SynchronizationContext::ResultBase {
public:
  explicit GetUserDataResult(
      UserModels::User user,
      std::optional<UserModels::LegacyAuth> legacyAuth = std::nullopt)
      : user(std::move(user)), legacyAuth(std::move(legacyAuth)) {}

  UserModels::User user;
  std::optional<UserModels::LegacyAuth> legacyAuth;
};

class UpdateLastSeenResult final : public SynchronizationContext::ResultBase {
public:
  explicit UpdateLastSeenResult(std::string message)
      : message(std::move(message)) {}

  std::string message;
};

class AddNewNameResult final : public SynchronizationContext::ResultBase {
public:
  explicit AddNewNameResult(std::string message)
      : message(std::move(message)) {}

  std::string message;
};

class GetUsersResult final : public SynchronizationContext::ResultBase {
public:
  explicit GetUsersResult(std::vector<UserModels::User> users)
      : users(std::move(users)) {}

  std::vector<UserModels::User> users;
};

class GetUserNamesResult final : public SynchronizationContext::ResultBase {
public:
  explicit GetUserNamesResult(std::vector<std::string> usernames)
      : usernames(std::move(usernames)) {}

  std::vector<std::string> usernames;
};

class GetUsersByNameResult final : public SynchronizationContext::ResultBase {
public:
  explicit GetUsersByNameResult(
      std::vector<std::pair<int32_t, std::string>> users)
      : users(std::move(users)) {}

  std::vector<std::pair<int32_t, std::string>> users;
};

class EditUserResult final : public SynchronizationContext::ResultBase {
public:
  EditUserResult(std::string message, int32_t clientNum)
      : message(std::move(message)), clientNum(clientNum) {}

  static constexpr int32_t CLIENT_NOT_CONNECTED = -1;

  std::string message;
  // clientNum of the edited user, if they are currently connected.
  // This is so that we can update the in-memory user data
  int32_t clientNum;
};

class GetBansResult final : public SynchronizationContext::ResultBase {
public:
  explicit GetBansResult(std::vector<UserModels::Ban> bans)
      : bans(std::move(bans)) {}

  std::vector<UserModels::Ban> bans;
};
} // namespace ETJump

#endif
