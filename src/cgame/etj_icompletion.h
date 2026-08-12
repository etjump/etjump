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

#include <functional>
#include <string>
#include <vector>

#include "../game/etj_syscall_ext_shared.h"

namespace ETJump {
class ICompletion {
public:
  struct ArgCompletion {
    explicit ArgCompletion(std::string arg) : arg(std::move(arg)) {};

    ArgCompletion(std::string arg,
                  std::function<bool(const std::vector<std::string> &)> fn)
        : arg(std::move(arg)), fn(std::move(fn)) {}

    ArgCompletion(std::string arg, std::vector<std::string> aliases)
        : arg(std::move(arg)), aliases(std::move(aliases)) {}

    ArgCompletion(std::string arg, std::vector<std::string> aliases,
                  std::function<bool(const std::vector<std::string> &)> fn)
        : arg(std::move(arg)), aliases(std::move(aliases)), fn(std::move(fn)) {}

    std::string arg;
    std::vector<std::string> aliases;
    std::function<bool(const std::vector<std::string> &args)> fn;
  };

  static void completeArg(const std::string &arg) {
    SyscallExt::trap_CommandComplete(arg.c_str());
  }

  static bool defaultCompletion(const std::vector<ArgCompletion> &completions) {
    for (const auto &completion : completions) {
      completeArg(completion.arg);
    }

    return true;
  }

  virtual ~ICompletion() = default;

  // 'args' should always be lowercased!
  [[nodiscard]] virtual bool
  complete(const std::vector<std::string> &args) const = 0;
};
} // namespace ETJump
