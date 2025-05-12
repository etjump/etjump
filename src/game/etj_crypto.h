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

extern "C" {
#include "sha1.h"
}

#include "picosha2.h"
#include "etj_string_utilities.h"

namespace ETJump {
class Crypto {
public:
  // returned hash is always uppercased
  static std::string sha1(const std::string &str) {
    SHA1Context sha{};

    SHA1Reset(&sha);
    SHA1Input(&sha, reinterpret_cast<const unsigned char *>(str.c_str()),
              str.length());

    if (!SHA1Result(&sha)) {
      return "";
    }

    return stringFormat("%08X%08X%08X%08X%08X", sha.Message_Digest[0],
                        sha.Message_Digest[1], sha.Message_Digest[2],
                        sha.Message_Digest[3], sha.Message_Digest[4]);
  }

  // returned hash is always uppercased
  static std::string sha2(const std::string &str) {
    std::string hash_hex_str;
    picosha2::hash256_hex_string(str, hash_hex_str);
    return StringUtil::toUpperCase(hash_hex_str);
  }

  static bool isValidSHA1(const std::string &hash) {
    return isValidHash(hash, SHA1_LEN);
  }

  static bool isValidSHA2(const std::string &hash) {
    return isValidHash(hash, SHA2_LEN);
  }

private:
  static constexpr size_t SHA1_LEN = 40;
  static constexpr size_t SHA2_LEN = 64;

  static bool isValidHash(const std::string &hash, const size_t len) {
    if (hash.length() != len) {
      return false;
    }

    for (const auto &c : hash) {
      if (c < '0' || c > 'F') {
        return false;
      }
    }

    return true;
  }
};
} // namespace ETJump
