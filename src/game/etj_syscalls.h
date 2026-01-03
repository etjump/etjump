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

#ifdef GAMEDLL
  #include "q_shared.h"
#else
  #include "../game/q_shared.h"
#endif

#if defined(__linux__) || defined(__APPLE__)
  #define FN_PUBLIC __attribute__((visibility("default")))
#elif defined(_WIN32)
  #define FN_PUBLIC __declspec(dllexport)
#else
  #error "Unsupported compiler"
#endif

inline constexpr intptr_t VM_CALL_END = -1337;
#define SystemCall(...) ExpandSyscall(__VA_ARGS__, VM_CALL_END)

extern intptr_t(QDECL *vmSyscall)(intptr_t arg, ...);
extern "C" FN_PUBLIC void dllEntry(intptr_t(QDECL *syscallptr)(intptr_t arg,
                                                               ...));

template <typename T, typename... Types>
intptr_t ExpandSyscall(T syscallArg, Types... args) {
  // C-style casts here for simplicity, to handle all types of arguments
  return vmSyscall((intptr_t)syscallArg, (intptr_t)args...);
}
