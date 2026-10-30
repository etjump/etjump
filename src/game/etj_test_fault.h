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

/*
 * Injectable test faults for the error handling, to test the fatal error
 * boundary and the engine's error paths in a real build. Only compiled when
 * the USE_TEST_FAULTS CMake option is enabled (off by default, never enabled
 * for CI/CD builds).
 *
 * The vmMain entry points label the call they are dispatching with
 * ETJ_TEST_FAULT_SCOPE, so the fault machinery knows which vm call is active:
 *
 * cgame:
 *   CG_INIT
 *   CG_SHUTDOWN
 *   CG_CONSOLE_COMMAND
 *   CG_DRAW_ACTIVE_FRAME
 *
 * ui:
 *   UI_INIT
 *   UI_SHUTDOWN
 *   UI_CONSOLE_COMMAND
 *   UI_REFRESH
 *
 * game:
 *   GAME_INIT
 *   GAME_SHUTDOWN
 *   GAME_RUN_FRAME
 *   GAME_CLIENT_CONNECT
 *   GAME_CLIENT_THINK
 *   GAME_CLIENT_USERINFO_CHANGED
 *   GAME_CLIENT_DISCONNECT
 *   GAME_CLIENT_BEGIN
 *   GAME_CLIENT_COMMAND
 *   GAME_CONSOLE_COMMAND
 *   GAME_SNAPSHOT_CALLBACK
 *
 * To inject a fault, set the 'etj_testFault' cvar to the vm call under test
 * and place a call to ETJ_TEST_FAULT() at the point where the error should be
 * raised: the fault fires, once, the next time that code runs while the
 * active vm call matches the cvar. The logs name the enclosing function, or a
 * custom string literal if one is given, e.g. ETJ_TEST_FAULT("kick path").
 * Such calls are placed ad hoc for a test run and removed afterwards; only
 * the labels at the vmMain entry points are permanent.
 *
 * Appending ':ex' raises an unexpected C++ exception instead of a fatal
 * error, e.g. 'etj_testFault GAME_RUN_FRAME:ex'. The injected error logs a
 * marker when the stack unwinds past the injection point, which only happens
 * if the error was raised with normal C++ semantics.
 */

#ifdef USE_TEST_FAULTS
  // labels the vm call being dispatched for the duration of the scope, so a
  // fault placed inside it knows which vm call it runs under
  #define ETJ_TEST_FAULT_SCOPE(name)                                           \
    ETJump::TestFault::VmCallScope vmScope { name }

  // raises a fault at this point, if the 'etj_testFault' cvar matches the
  // active vm call; the logs name __func__, or a custom string literal if one
  // is given. Place this ad hoc, remove it after the test
  #define ETJ_TEST_FAULT(...)                                                  \
    ETJump::TestFault::maybeRaise(__func__, "" __VA_ARGS__)
#else
  #define ETJ_TEST_FAULT_SCOPE(name) ((void)0)
  #define ETJ_TEST_FAULT(...) ((void)0)
#endif

namespace ETJump::TestFault {
// marks a vm call as the active one for the duration of the scope; nested
// scopes (the engine calling the module back while it is already running) are
// restored when the inner scope ends. Used on the main thread only.
class VmCallScope {
public:
  explicit VmCallScope(const char *name);
  ~VmCallScope();

  VmCallScope(const VmCallScope &) = delete;
  VmCallScope(VmCallScope &&) = delete;
  VmCallScope &operator=(const VmCallScope &) = delete;
  VmCallScope &operator=(VmCallScope &&) = delete;

private:
  const char *previous;
};

// raises a fault if the 'etj_testFault' cvar matches the active vm call, and
// clears the cvar so the fault only fires once per set; the logs name 'func',
// or 'message' if one was given
void maybeRaise(const char *func, const char *message);
} // namespace ETJump::TestFault
