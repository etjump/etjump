/*
 * MIT License
 *
 * Copyright (c) 2024 ETJump team <zero@etjump.com>
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

#include "q_shared.h"

#if defined(__linux__) || defined(__APPLE__)
  #define FN_PUBLIC __attribute__((visibility("default")))
#elif defined(_WIN32)
  // https://msdn.microsoft.com/en-us/library/wf2w9f6x.aspx
  #define FN_PUBLIC __declspec(dllexport)
#else
  #error "Unsupported compiler"
#endif

static constexpr intptr_t VM_CALL_END = -1337;
#define SystemCall(...) ExpandSyscall(__VA_ARGS__, VM_CALL_END)

template <typename T, typename... Types>
static intptr_t ExpandSyscall(T syscallArg, Types... args) {
  // we have to do C-style casting here to support all types
  // of arguments passed onto syscalls
  return syscall((intptr_t)syscallArg, (intptr_t)args...);
}
