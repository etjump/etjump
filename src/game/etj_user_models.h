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
namespace ETJump::UserModels {
struct User {
  int id;
  std::string name;
  std::string guid;
  std::string ipv4;
  std::string ipv6;
  int level;
  int64_t lastSeen;
  std::string title;
  std::string commands;
  std::string greeting;
};

struct Name {
  std::string name;
  std::string cleanName;
  int userID;
};

struct UserHWID {
  int userID;
  int platform;
  std::string hwid;
};

struct Ban {
  int banID;
  std::string name;
  std::string bannedBy;
  std::string banDate;
  int64_t expires;
  std::string reason;
  int parentBanID;
  std::string guid;
  std::string ipv4;
  std::string ipv6;
  std::string legacyGUID;
  std::string legacyHWID;
};

struct HWIDBan {
  int banID;
  int platform;
  std::string hwid;
};

struct LegacyAuth {
  int userID;
  std::string guid;
  std::string hwid;
};

struct BannedIPAddresses {
  std::string ipv4;
  std::string ipv6;
};

// NOTE: does not contain IP addresses as IP bans can be checked
// without identifying the client first
struct BanData {
  int banID;
  std::string guid;
  std::vector<std::string> hwids;
  std::string legacyGUID;
  std::string legacyHWID;
};

struct EditUserParams {
  int32_t id;
  std::optional<std::string> title;
  std::optional<std::string> commands;
  std::optional<std::string> greeting;
};
} // namespace ETJump::UserModels
#endif
