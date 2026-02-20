/*
 * MIT License
 *
 * Copyright (c) 2026 ETJump team <zero@etjump.com>
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
#include <unordered_map>

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

// only contains the platforms we actually support + fallback
inline const std::unordered_map<int32_t, std::string> operatingSystemStrings = {
    {OS_DEFAULT, "Default (unknown)"},   {OS_WIN_X86, "Windows x86"},
    {OS_WIN_X86_64, "Windows x86_64"},   {OS_LINUX_X86, "Linux x86"},
    {OS_LINUX_X86_64, "Linux x86_64"},   {OS_MACOS_X86_64, "macOS x86_64"},
    {OS_MACOS_AARCH64, "macOS AArch64"},
};

namespace Authentication {
#ifdef NEW_AUTH
enum class MigrationType {
  AUTO = 0,
  MANUAL = 1,
  // forces the server to perform migration again even if
  // the user database contains old GUID for the client
  MANUAL_FORCE = 2,
};

inline constexpr char AUTH_REQUEST[] = "auth_request";
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

// template class for a bitset that uses 'enum class' as values for the bits
template <typename EnumT>
class EnumBitset {
  using UnderlyingT = std::underlying_type_t<EnumT>;
  UnderlyingT bits = 0;

public:
  // default constructor, empty bitset
  constexpr EnumBitset() = default;

  // construct from enum value
  // EnumBitset<MyEnum> bitset(MyEnum::VALUE_1)
  constexpr explicit EnumBitset(EnumT flag)
      : bits(static_cast<UnderlyingT>(flag)) {}

  // construct from initializer list
  // EnumBitset<MyEnum> bitset({MyEnum::VALUE_1, MyEnum::VALUE_2 ...})
  constexpr EnumBitset(std::initializer_list<EnumT> flags) {
    for (auto flag : flags) {
      bits |= static_cast<UnderlyingT>(flag);
    }
  }

  // we don't want floating point implicit conversions for constructors
  EnumBitset(float) = delete;
  EnumBitset(double) = delete;
  EnumBitset(long double) = delete;

  constexpr EnumBitset &set(EnumT flag) {
    bits |= static_cast<UnderlyingT>(flag);
    return *this;
  }

  constexpr EnumBitset &operator|=(EnumT flag) {
    bits |= static_cast<UnderlyingT>(flag);
    return *this;
  }

  constexpr EnumBitset &operator|=(EnumBitset other) {
    bits |= other.bits;
    return *this;
  }

  constexpr EnumBitset &clear(EnumT flag) {
    bits &= ~static_cast<UnderlyingT>(flag);
    return *this;
  }

  constexpr bool has(EnumT flag) const {
    return (bits & static_cast<UnderlyingT>(flag)) != 0;
  }

  constexpr bool operator&(EnumT flag) const { return has(flag); }

  constexpr EnumBitset operator&(EnumBitset other) const {
    EnumBitset result;
    result.bits = bits & other.bits;
    return result;
  }

  constexpr EnumBitset operator|(EnumT flag) const {
    EnumBitset result = *this;
    result.set(flag);
    return result;
  }

  constexpr EnumBitset operator|(EnumBitset other) const {
    EnumBitset result;
    result.bits = bits | other.bits;
    return result;
  }

  constexpr bool operator==(EnumBitset other) const {
    return bits == other.bits;
  }

  constexpr explicit operator bool() const { return bits != 0; }
};
} // namespace ETJump
