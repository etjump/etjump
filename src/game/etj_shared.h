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
#include <string>
#include <stdexcept>

namespace ETJump {
namespace Constants {
// these match the values that 'g_oss' uses
inline constexpr int OS_DEFAULT = 0;              // unknown
inline constexpr int OS_WIN_X86 = 1 << 0;         // Windows x86
inline constexpr int OS_LINUX_X86 = 1 << 1;       // Linux x86
inline constexpr int OS_LINUX_X86_64 = 1 << 2;    // Linux x86_64
inline constexpr int OS_MACOS_X86_64 = 1 << 3;    // macOS x86_64
inline constexpr int OS_ANDROID_AARCH64 = 1 << 4; // Android aarch64
inline constexpr int OS_LINUX_ARMV7 = 1 << 5;     // Raspberry Pi arm
inline constexpr int OS_LINUX_ARMV8_64 = 1 << 6;  // Raspberry Pi aarch64
inline constexpr int OS_MACOS_AARCH64 = 1 << 7;   // macOS aarch64
inline constexpr int OS_WIN_X86_64 = 1 << 8;      // Windows x86_64
inline constexpr int OS_ANDROID_X86 = 1 << 9;     // Android x86
inline constexpr int OS_ANDROID_X86_64 = 1 << 10; // Android x86_64

namespace Authentication {
#ifdef NEW_AUTH
enum class MigrationType {
  AUTO = 0,
  MANUAL = 1,
  // forces the server to perform migration again even if
  // the user database contains old GUID for the client
  MANUAL_FORCE = 2,
};

inline constexpr char GUID_REQUEST[] = "guid_request";
inline constexpr char GUID_MIGRATE_REQUEST[] = "guid_migrate_request";

inline constexpr char GUID_MIGRATE[] = "guid_migrate";
inline constexpr char GUID_MIGRATE_FAIL[] = "guid_migrate_fail";

inline constexpr char AUTHENTICATE[] = "auth";

// not a big fan of this tbh but whatever, works
inline constexpr size_t HWID_SIZE_WIN = 6;
inline constexpr size_t HWID_SIZE_LINUX = 4;
inline constexpr size_t HWID_SIZE_MAC = 1;
#else
const std::string GUID_REQUEST = "guid_request";
const std::string AUTHENTICATE = "authenticate";
#endif
} // namespace Authentication
} // namespace Constants

template <typename T>
class opt {
public:
  opt() : _hasValue(false) {}

  // we want this to be implicit
  // e.g. opt<int> x = 5;
  // NOLINTNEXTLINE(google-explicit-constructor)
  opt(T val) : _hasValue(true), _value(std::move(val)) {}

  bool hasValue() const { return _hasValue; }

  const T &valueOr(const T &defaultValue) const {
    if (!_hasValue) {
      return defaultValue;
    }
    return _value;
  }

  T valueOr(T defaultValue) {
    if (!_hasValue) {
      return defaultValue;
    }
    return _value;
  }

  T &operator*() {
    if (!_hasValue) {
      throw std::runtime_error("Optional value is not set");
    }
    return _value;
  }

  const T &operator*() const {
    if (!_hasValue) {
      throw std::runtime_error("Optional value is not set");
    }
    return _value;
  }

  T &value() {
    if (!_hasValue) {
      throw std::runtime_error("Optional value is not set");
    }
    return _value;
  }

  const T &value() const {
    if (!_hasValue) {
      throw std::runtime_error("Optional value is not set");
    }
    return _value;
  }

private:
  bool _hasValue;
  T _value;
};
} // namespace ETJump
