/*
 * MIT License
 *
 * Copyright (c) 2023 ETJump team <zero@etjump.com>
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

#ifndef IAUTHENTICATION_HH
#define IAUTHENTICATION_HH
#include <string>
#include "etj_user.h"

/**
 * Session interface for database
 */

class IAuthentication {
public:
  virtual ~IAuthentication(){};
  virtual const std::string GetMessage() const = 0;
  virtual bool AddUser(const std::string &guid, const std::string &hardwareId,
                       const std::string &name) = 0;
  virtual bool AddNewHardwareId(int id, const std::string &hardwareId) = 0;
  virtual bool BanUser(std::string const &name, std::string const &guid,
                       std::string const &hardwareId, std::string const &ip,
                       std::string const &bannedBy, std::string const &banDate,
                       unsigned expires, std::string const &reason) = 0;
  virtual bool IsBanned(const std::string &guid,
                        const std::string &hardwareId) = 0;
  virtual bool IsIpBanned(const std::string &ip) = 0;
  virtual bool UserExists(unsigned id) = 0;
  virtual bool UserExists(const std::string &guid) = 0;
  virtual const User_s *GetUserData(const std::string &guid) const = 0;
  virtual const User_s *GetUserData(int id) const = 0;
  virtual bool SetLevel(int id, int level) = 0;
  virtual void NewName(int id, const std::string &name) = 0;
  virtual bool UpdateLastSeen(int id, int lastSeen) = 0;
  virtual int ResetUsersWithLevel(int level) = 0;
};

#endif
