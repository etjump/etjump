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

template <typename T>
class opt {
public:
  opt() : _hasValue(false) {}

  // we want this to be implicit
  // e.g. opt<int> x = 5;
  opt(T val) : _hasValue(true), _value(std::move(val)) {}

  bool hasValue() const { return _hasValue; }

  const T &valueOr(const T& defaultValue) const {
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
