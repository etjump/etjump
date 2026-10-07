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
#include <array>
#include <string>
#include <string_view>
#include <stdexcept>

namespace ETJump {
namespace Constants {
namespace Authentication {
const std::string GUID_REQUEST = "guid_request";
const std::string AUTHENTICATE = "authenticate";
} // namespace Authentication

namespace SpectatorHudSync {
inline constexpr int32_t UploadMinIntervalMs = 300;
inline constexpr int32_t RequestMinIntervalMs = 300;
inline constexpr int32_t ServerForwardMinIntervalMs = 200;
inline constexpr char FieldSeparator = ',';
inline constexpr char ValueSeparator = ';';
inline constexpr char KvSeparator = ':';

inline constexpr std::array<const char *, 24> CgazCvarNames = {
    "etj_drawCGaz",
    "etj_CGazY",
    "etj_CGaz2Y",
    "etj_CGazHeight",
    "etj_CGaz2Color1",
    "etj_CGaz2Color2",
    "etj_CGaz1Color1",
    "etj_CGaz1Color2",
    "etj_CGaz1Color3",
    "etj_CGaz1Color4",
    "etj_CGazFov",
    "etj_CGazTrueness",
    "etj_CGazOnTop",
    "etj_CGaz2FixedSpeed",
    "etj_CGaz2NoVelocityDir",
    "etj_CGaz1DrawSnapZone",
    "etj_CGaz2WishDirFixedSpeed",
    "etj_CGaz2WishDirUniformLength",
    "etj_CGaz1DrawMidLine",
    "etj_CGaz1MidlineColor",
    "etj_CGaz2HighRes",
    "etj_CGaz2Thickness1",
    "etj_CGaz2Thickness2",
    "etj_stretchCgaz",
};

inline constexpr std::array<const char *, 14> SnaphudCvarNames = {
    "etj_drawSnapHUD",
    "etj_snapHUDOffsetY",
    "etj_snapHUDHeight",
    "etj_snapHUDColor1",
    "etj_snapHUDColor2",
    "etj_snapHUDHLColor1",
    "etj_snapHUDHLColor2",
    "etj_snapHUDFov",
    "etj_snapHUDHLActive",
    "etj_snapHUDTrueness",
    "etj_snapHUDEdgeThickness",
    "etj_snapHUDBorderThickness",
    "etj_snapHUDActiveIsPrimary",
    "etj_snapHUDCrop",
};

// handled separately because it is a multi-token parser-style cvar value
inline constexpr const char *SnaphudCropOffsetCvarName =
    "etj_snapHUDCropOffsets";

inline constexpr std::string_view UploadCommand = "hudsync_upload";
inline constexpr std::string_view RequestCommand = "hudsync_request";
inline constexpr std::string_view ServerCommand = "hudsync";
} // namespace SpectatorHudSync
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

  // construct from underlying integer value
  // EnumBitset<MyEnum> bitset(int)
  constexpr explicit EnumBitset(UnderlyingT flags) : bits(flags) {}

  // we don't want floating point implicit conversions for constructors
  EnumBitset(float) = delete;
  EnumBitset(double) = delete;
  EnumBitset(long double) = delete;

  constexpr EnumBitset &set(EnumT flag) {
    bits |= static_cast<UnderlyingT>(flag);
    return *this;
  }

  constexpr EnumBitset &reset() {
    bits = 0;
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

  constexpr explicit operator UnderlyingT() const { return bits; }
};
} // namespace ETJump
