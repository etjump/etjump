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
#include <stdexcept>

namespace ETJump {
namespace Constants {
namespace Authentication {
const std::string GUID_REQUEST = "guid_request";
const std::string AUTHENTICATE = "authenticate";
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
