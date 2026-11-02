#include <csetjmp>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include "../src/game/etj_fatal_error_shared.h"

#ifdef _MSC_VER
  #define TEST_NOINLINE __declspec(noinline)
#else
  #define TEST_NOINLINE __attribute__((noinline))
#endif

namespace ETJump {
// simulates the engine longjmp'ing over an active vmMain call deeper in the
// stack, without loading the module again, the recursion adds depth even
// when ASan moves 'padding' off the stack. The padding is indexed at runtime,
// as clang otherwise shrinks the array to the elements it sees accessed,
// despite it being volatile, and the levels add less depth than a single
// run() frame holding a CaughtError takes.
TEST_NOINLINE static void enterWithoutLeaving(const int levels) {
  volatile char padding[256];
  padding[levels & 0xff] = 0;

  if (levels > 0) {
    enterWithoutLeaving(levels - 1);
  } else {
    FatalErrorBoundary::detail::enter();
  }

  // keeps the recursion from being turned into a tail call
  padding[1] = padding[0];
}

// the engine calling into the module during a syscall, from frames of its
// own, which is what makes the call nested (a nested run() inside the outer
// call's lambda could be inlined into the same frame)
template <typename Dispatch>
TEST_NOINLINE static intptr_t
callFromSyscall(Dispatch &&dispatch, const intptr_t failureResult = 0,
                const FatalErrorBoundary::DeferFunction defer = nullptr) {
  return FatalErrorBoundary::run(std::forward<Dispatch>(dispatch),
                                 failureResult, defer);
}

// raises an error outside of vmMain, further down the stack than the calls
// made from the test itself
TEST_NOINLINE static void raiseOutsideBoundary(const int levels) {
  volatile char padding[256];
  padding[levels & 0xff] = 0;

  if (levels > 0) {
    raiseOutsideBoundary(levels - 1);
  } else if (padding[0] == 0) {
    // always true, but the compiler can't tell (raiseError never returns,
    // which would make it take the recursion as infinite)
    FatalErrorBoundary::raiseError("outside");
  }

  padding[1] = padding[0];
}

static std::jmp_buf engineLoop;

// a vmMain call the engine makes through the same path every time,
// 'jumpOut' longjmps out of it like the engine does on errors
TEST_NOINLINE static intptr_t callThroughEngine(const bool jumpOut) {
  return FatalErrorBoundary::run([jumpOut]() -> intptr_t {
    if (jumpOut) {
      std::longjmp(engineLoop, 1);
    }

    FatalErrorBoundary::throwFatal("boom");
  });
}

static_assert(!std::is_base_of_v<std::exception, FatalError>,
              "FatalError must not be caught by std::exception handlers");

class FatalErrorBoundaryTests : public testing::Test {
public:
  // thrown by the fake trap_Error, so the test continues after raising
  struct RaiseCalled {};

  struct Guard {
    bool &destroyed;
    ~Guard() { destroyed = true; }
  };

  inline static std::vector<std::string> raised;
  inline static std::vector<std::string> printed;
  inline static std::string deferred;
  inline static bool deferredUnexpected = false;
  inline static std::vector<std::string> deferredMessages;

  static void fakeRaise(const char *message) {
    raised.emplace_back(message);
    throw RaiseCalled{};
  }

  // like the engine, calls back into the module (its shutdown) while
  // handling the error, which fails as well
  static void raiseWithShutdown(const char *message) {
    raised.emplace_back(message);

    EXPECT_EQ(callFromSyscall([]() -> intptr_t {
                FatalErrorBoundary::throwFatal("shutdown");
              }),
              0);

    throw RaiseCalled{};
  }

  static void fakePrint(const char *message) { printed.emplace_back(message); }

  static bool fakeDefer(const char *message, const bool unexpected) {
    deferred = message;
    deferredUnexpected = unexpected;
    return true;
  }

  // for the first error, makes a syscall and calls into the module again,
  // like a defer function that drops clients would, which fails as well
  static bool reenteringDefer(const char *message, const bool) {
    deferredMessages.emplace_back(message);

    if (deferredMessages.size() == 1) {
      FatalErrorBoundary::rethrowPending();
      callFromSyscall(
          []() -> intptr_t { FatalErrorBoundary::throwFatal("second"); }, 0,
          reenteringDefer);

      EXPECT_STREQ(message, "first");
    }

    return true;
  }

  void SetUp() override {
    raised.clear();
    printed.clear();
    deferred.clear();
    deferredUnexpected = false;
    deferredMessages.clear();
    FatalErrorBoundary::initialize(fakeRaise, fakePrint);
  }

  void TearDown() override {}
};

TEST_F(FatalErrorBoundaryTests, Run_ReturnsDispatchResult) {
  EXPECT_EQ(FatalErrorBoundary::run([] { return intptr_t{42}; }), 42);
  EXPECT_TRUE(raised.empty());
}

TEST_F(FatalErrorBoundaryTests, Run_UnwindsStackAndRaisesOnceOnTopLevelError) {
  bool destroyed = false;

  EXPECT_THROW(FatalErrorBoundary::run([&]() -> intptr_t {
                 Guard guard{destroyed};
                 FatalErrorBoundary::throwFatal("boom");
               }),
               RaiseCalled);

  EXPECT_TRUE(destroyed);
  EXPECT_EQ(raised, std::vector<std::string>{"boom"});
}

TEST_F(FatalErrorBoundaryTests, Run_RethrowsNestedErrorAfterSyscallReturns) {
  bool destroyed = false;
  bool reachedAfterSyscall = false;

  EXPECT_THROW(FatalErrorBoundary::run([&]() -> intptr_t {
                 Guard guard{destroyed};

                 // the engine calls back into the module during a syscall
                 const intptr_t result = callFromSyscall(
                     []() -> intptr_t {
                       FatalErrorBoundary::throwFatal("nested");
                     });

                 EXPECT_EQ(result, 0);
                 EXPECT_TRUE(raised.empty());

                 // the syscall returns
                 FatalErrorBoundary::rethrowPending();
                 reachedAfterSyscall = true;
                 return 1;
               }),
               RaiseCalled);

  EXPECT_FALSE(reachedAfterSyscall);
  EXPECT_TRUE(destroyed);
  EXPECT_EQ(raised, std::vector<std::string>{"nested"});
}

TEST_F(FatalErrorBoundaryTests, Run_PrintsNestedErrorRightAway) {
  EXPECT_THROW(FatalErrorBoundary::run([]() -> intptr_t {
                 callFromSyscall([]() -> intptr_t {
                   FatalErrorBoundary::throwFatal("nested");
                 });

                 // the engine might never return from the syscall
                 EXPECT_EQ(printed.size(), 1U);

                 FatalErrorBoundary::rethrowPending();
                 return 1;
               }),
               RaiseCalled);

  ASSERT_EQ(printed.size(), 1U);
  EXPECT_NE(printed[0].find("nested"), std::string::npos);
  EXPECT_EQ(raised, std::vector<std::string>{"nested"});
}

TEST_F(FatalErrorBoundaryTests, Run_PrintsDeeplyNestedErrorOnce) {
  EXPECT_THROW(FatalErrorBoundary::run([]() -> intptr_t {
                 callFromSyscall([]() -> intptr_t {
                   callFromSyscall([]() -> intptr_t {
                     FatalErrorBoundary::throwFatal("deep");
                   });

                   FatalErrorBoundary::rethrowPending();
                   return 1;
                 });

                 FatalErrorBoundary::rethrowPending();
                 return 1;
               }),
               RaiseCalled);

  EXPECT_EQ(printed.size(), 1U);
  EXPECT_EQ(raised, std::vector<std::string>{"deep"});
}

TEST_F(FatalErrorBoundaryTests, Run_ReturnsFailureResultForLeftoverError) {
  struct FailOnDestroy {
    ~FailOnDestroy() {
      // the engine calls back into the module from a syscall made here,
      // and the error can't be rethrown while unwinding
      callFromSyscall([]() -> intptr_t { throw std::runtime_error("nested"); });
      FatalErrorBoundary::rethrowPending();
    }
  };

  EXPECT_EQ(FatalErrorBoundary::run(
                []() -> intptr_t {
                  try {
                    FailOnDestroy fail;
                    throw std::runtime_error("swallowed");
                  } catch (const std::runtime_error &) {
                  }

                  // e.g. a connect that would be accepted otherwise
                  return 1;
                },
                9, fakeDefer),
            9);

  EXPECT_EQ(deferred, "Unhandled exception: nested");
  EXPECT_TRUE(deferredUnexpected);
}

TEST_F(FatalErrorBoundaryTests, RethrowPending_SkipsOtherNestedCalls) {
  intptr_t otherResult = 0;

  EXPECT_THROW(FatalErrorBoundary::run([&]() -> intptr_t {
                 callFromSyscall([]() -> intptr_t {
                   FatalErrorBoundary::throwFatal("nested");
                 });

                 // before the syscall returns, the engine calls into the
                 // module again (e.g. its shutdown), which must run normally
                 otherResult = callFromSyscall([] {
                   FatalErrorBoundary::rethrowPending();
                   return intptr_t{7};
                 });

                 // the syscall returns
                 FatalErrorBoundary::rethrowPending();
                 return 1;
               }),
               RaiseCalled);

  EXPECT_EQ(otherResult, 7);
  EXPECT_EQ(raised, std::vector<std::string>{"nested"});
}

TEST_F(FatalErrorBoundaryTests, RethrowPending_DoesNotThrowWhileUnwinding) {
  struct RethrowOnDestroy {
    ~RethrowOnDestroy() { FatalErrorBoundary::rethrowPending(); }
  };

  EXPECT_THROW(FatalErrorBoundary::run([]() -> intptr_t {
                 callFromSyscall([]() -> intptr_t {
                   FatalErrorBoundary::throwFatal("nested");
                 });

                 RethrowOnDestroy rethrow;
                 throw std::runtime_error("unwinding");
               }),
               RaiseCalled);

  // the first error is kept
  EXPECT_EQ(raised, std::vector<std::string>{"nested"});
}

TEST_F(FatalErrorBoundaryTests, RethrowPending_DoesNothingOutsideBoundary) {
  EXPECT_NO_THROW(FatalErrorBoundary::rethrowPending());
}

TEST_F(FatalErrorBoundaryTests, RethrowPending_DoesNotThrowLeftoverError) {
  struct FailOnDestroy {
    ~FailOnDestroy() {
      callFromSyscall([]() -> intptr_t { throw std::runtime_error("nested"); });
      FatalErrorBoundary::rethrowPending();
    }
  };

  intptr_t laterResult = 0;

  // not passed to EXPECT_EQ directly, MSVC gives every __LINE__ inside a
  // multi-line macro argument the same value, so the EXPECT_NO_THROW labels
  // in the lambda would clash
  const auto dispatch = [&]() -> intptr_t {
    try {
      FailOnDestroy fail;
      throw std::runtime_error("swallowed");
    } catch (const std::runtime_error &) {
    }

    // a later syscall that didn't call into the module, e.g.
    // one made by a destructor on a regular scope exit
    EXPECT_NO_THROW(FatalErrorBoundary::rethrowPending());

    // a later call nested inside another syscall runs normally
    laterResult = callFromSyscall([] {
      FatalErrorBoundary::rethrowPending();
      return intptr_t{7};
    });

    EXPECT_NO_THROW(FatalErrorBoundary::rethrowPending());
    return 1;
  };

  EXPECT_EQ(FatalErrorBoundary::run(dispatch, 9, fakeDefer), 9);

  EXPECT_EQ(laterResult, 7);
  EXPECT_EQ(deferred, "Unhandled exception: nested");
}

TEST_F(FatalErrorBoundaryTests, ThrowFatal_ThrowsInCallMadeWhileUnwinding) {
  struct FailOnDestroy {
    intptr_t &result;

    ~FailOnDestroy() {
      // the nested call catches its own exceptions,
      // even while the outer call unwinds the stack
      result = callFromSyscall(
          []() -> intptr_t { FatalErrorBoundary::throwFatal("nested"); }, 4);
    }
  };

  intptr_t nestedResult = 0;

  EXPECT_THROW(FatalErrorBoundary::run([&]() -> intptr_t {
                 FailOnDestroy fail{nestedResult};
                 throw std::runtime_error("outer");
               }),
               RaiseCalled);

  EXPECT_EQ(nestedResult, 4);

  // raised once the outer call ended, the fatal error was recorded when it
  // was thrown, and the outer exception only once it was caught, which is
  // still printed
  EXPECT_EQ(raised, std::vector<std::string>{"nested"});
  ASSERT_EQ(printed.size(), 2U);
  EXPECT_NE(printed[1].find("Additional fatal error"), std::string::npos);
  EXPECT_NE(printed[1].find("outer"), std::string::npos);
}

TEST_F(FatalErrorBoundaryTests, Run_PassesOnErrorOfCallThatFailedUnwinding) {
  struct FailOnDestroy {
    ~FailOnDestroy() {
      // can't be rethrown here, as the call below unwinds the stack
      callFromSyscall([]() -> intptr_t { throw std::runtime_error("deep"); });
      FatalErrorBoundary::rethrowPending();
    }
  };

  bool reachedAfterSyscall = false;

  EXPECT_THROW(FatalErrorBoundary::run([&]() -> intptr_t {
                 EXPECT_EQ(callFromSyscall(
                               []() -> intptr_t {
                                 try {
                                   FailOnDestroy fail;
                                   throw std::runtime_error("handled");
                                 } catch (const std::runtime_error &) {
                                 }

                                 // e.g. a connect that would be accepted
                                 return 1;
                               },
                               5),
                           5);

                 // the syscall returns
                 FatalErrorBoundary::rethrowPending();
                 reachedAfterSyscall = true;
                 return 1;
               }),
               RaiseCalled);

  EXPECT_FALSE(reachedAfterSyscall);
  EXPECT_EQ(raised, std::vector<std::string>{"Unhandled exception: deep"});
}

TEST_F(FatalErrorBoundaryTests, Run_KeepsFatalErrorThatStartedUnwinding) {
  struct FailOnDestroy {
    ~FailOnDestroy() {
      callFromSyscall(
          []() -> intptr_t { throw std::runtime_error("consequence"); });
      FatalErrorBoundary::rethrowPending();
    }
  };

  EXPECT_THROW(FatalErrorBoundary::run([]() -> intptr_t {
                 callFromSyscall([]() -> intptr_t {
                   FailOnDestroy fail;
                   FatalErrorBoundary::throwFatal("cause");
                 });

                 FatalErrorBoundary::rethrowPending();
                 return 1;
               }),
               RaiseCalled);

  EXPECT_EQ(raised, std::vector<std::string>{"cause"});
}

TEST_F(FatalErrorBoundaryTests, Run_FailsWhenHandlerSwallowsFatalError) {
  EXPECT_EQ(FatalErrorBoundary::run(
                []() -> intptr_t {
                  try {
                    FatalErrorBoundary::throwFatal("swallowed");
                  } catch (...) {
                  }

                  return 1;
                },
                3, fakeDefer),
            3);

  EXPECT_EQ(deferred, "swallowed");
  EXPECT_FALSE(deferredUnexpected);
}

TEST_F(FatalErrorBoundaryTests, Run_PassesOnSwallowedNestedError) {
  bool reachedAfterSyscall = false;

  EXPECT_THROW(FatalErrorBoundary::run([&]() -> intptr_t {
                 EXPECT_EQ(callFromSyscall(
                               []() -> intptr_t {
                                 try {
                                   FatalErrorBoundary::throwFatal("nested");
                                 } catch (...) {
                                 }

                                 return 1;
                               },
                               5),
                           5);

                 FatalErrorBoundary::rethrowPending();
                 reachedAfterSyscall = true;
                 return 1;
               }),
               RaiseCalled);

  EXPECT_FALSE(reachedAfterSyscall);
  EXPECT_EQ(raised, std::vector<std::string>{"nested"});
}

TEST_F(FatalErrorBoundaryTests, Run_PrintsAdditionalError) {
  EXPECT_EQ(FatalErrorBoundary::run(
                []() -> intptr_t {
                  try {
                    FatalErrorBoundary::throwFatal("first");
                  } catch (...) {
                  }

                  callFromSyscall(
                      []() -> intptr_t { throw std::runtime_error("second"); });
                  return 1;
                },
                3, fakeDefer),
            3);

  EXPECT_EQ(deferred, "first");
  ASSERT_EQ(printed.size(), 1U);
  EXPECT_NE(printed[0].find("Additional fatal error"), std::string::npos);
  EXPECT_NE(printed[0].find("second"), std::string::npos);
}

TEST_F(FatalErrorBoundaryTests, ThrowFatal_PrintsAdditionalErrorOnce) {
  EXPECT_EQ(FatalErrorBoundary::run(
                []() -> intptr_t {
                  try {
                    FatalErrorBoundary::throwFatal("first");
                  } catch (...) {
                  }

                  // reaches the boundary, which must not print it again
                  FatalErrorBoundary::throwFatal("second");
                },
                3, fakeDefer),
            3);

  EXPECT_EQ(deferred, "first");
  ASSERT_EQ(printed.size(), 1U);
  EXPECT_NE(printed[0].find("second"), std::string::npos);
}

TEST_F(FatalErrorBoundaryTests, Run_HandlesErrorLeftOverAtOutermostCall) {
  // e.g. an engine jumped over the call the error happened in without
  // loading the module again, and without the stack position revealing it
  FatalErrorBoundary::detail::hasPending = true;

  // the call fails with it
  EXPECT_EQ(FatalErrorBoundary::run([] { return intptr_t{1}; }, 3, fakeDefer),
            3);

  // later errors are handled again
  EXPECT_EQ(FatalErrorBoundary::run(
                []() -> intptr_t { FatalErrorBoundary::throwFatal("next"); },
                3, fakeDefer),
            3);
  EXPECT_EQ(deferred, "next");
}

TEST_F(FatalErrorBoundaryTests, Run_FailsWhenHandlerSwallowsRethrownError) {
  EXPECT_EQ(FatalErrorBoundary::run(
                []() -> intptr_t {
                  callFromSyscall([]() -> intptr_t {
                    FatalErrorBoundary::throwFatal("nested");
                  });

                  try {
                    FatalErrorBoundary::rethrowPending();
                  } catch (...) {
                  }

                  return 1;
                },
                3, fakeDefer),
            3);

  EXPECT_EQ(deferred, "nested");
}

TEST_F(FatalErrorBoundaryTests, Run_SwallowsErrorWhileRaising) {
  FatalErrorBoundary::initialize(raiseWithShutdown, fakePrint);

  EXPECT_THROW(FatalErrorBoundary::run([]() -> intptr_t {
                 FatalErrorBoundary::throwFatal("first");
               }),
               RaiseCalled);

  EXPECT_EQ(raised, std::vector<std::string>{"first"});
  ASSERT_EQ(printed.size(), 1U);
  EXPECT_NE(printed[0].find("Ignoring"), std::string::npos);
  EXPECT_NE(printed[0].find("shutdown"), std::string::npos);
}

TEST_F(FatalErrorBoundaryTests, RaiseError_ResetsRaisingStateOutsideBoundary) {
  FatalErrorBoundary::initialize(raiseWithShutdown, fakePrint);

  // the engine's shutdown call while handling the error is ignored
  EXPECT_THROW(raiseOutsideBoundary(64), RaiseCalled);

  // the engine returns without loading the module again,
  // and calls it from further up the stack
  EXPECT_THROW(callThroughEngine(false), RaiseCalled);

  EXPECT_EQ(raised, (std::vector<std::string>{"outside", "boom"}));
  ASSERT_EQ(printed.size(), 3U);
  EXPECT_NE(printed[0].find("Ignoring"), std::string::npos);
  EXPECT_NE(printed[1].find("Resetting"), std::string::npos);
  EXPECT_NE(printed[2].find("Ignoring"), std::string::npos);
}

TEST_F(FatalErrorBoundaryTests, Run_ResetsRaisingStateLeftOverAtSameFrame) {
  // the engine returns from handling the error without loading the module
  // again, and calls it again through the same path
  EXPECT_THROW(callThroughEngine(false), RaiseCalled);
  EXPECT_THROW(callThroughEngine(false), RaiseCalled);

  EXPECT_EQ(raised, (std::vector<std::string>{"boom", "boom"}));
  ASSERT_EQ(printed.size(), 1U);
  EXPECT_NE(printed[0].find("Resetting"), std::string::npos);
}

TEST_F(FatalErrorBoundaryTests, Initialize_ResetsRaisingState) {
  EXPECT_THROW(FatalErrorBoundary::throwFatal("first"), RaiseCalled);

  // the module is loaded again
  FatalErrorBoundary::initialize(fakeRaise, fakePrint);

  EXPECT_THROW(FatalErrorBoundary::run([]() -> intptr_t {
                 FatalErrorBoundary::throwFatal("second");
               }),
               RaiseCalled);

  EXPECT_EQ(raised, (std::vector<std::string>{"first", "second"}));
}

TEST_F(FatalErrorBoundaryTests, Run_KeepsDepthWhenReloadedDuringCall) {
  // the engine unloads and loads the module again during a syscall
  EXPECT_EQ(FatalErrorBoundary::run([] {
              callFromSyscall([] {
                FatalErrorBoundary::initialize(fakeRaise, fakePrint);
                return intptr_t{0};
              });

              return intptr_t{1};
            }),
            1);

  // a later error must still unwind the stack before being raised
  bool destroyed = false;

  EXPECT_THROW(FatalErrorBoundary::run([&]() -> intptr_t {
                 Guard guard{destroyed};
                 FatalErrorBoundary::throwFatal("boom");
               }),
               RaiseCalled);

  EXPECT_TRUE(destroyed);
  EXPECT_EQ(raised, std::vector<std::string>{"boom"});
}

TEST_F(FatalErrorBoundaryTests, Run_ResetsDepthLeftOverFromLongjmp) {
  enterWithoutLeaving(64);

  // a later call from further up the stack must be treated as top level
  bool destroyed = false;

  EXPECT_THROW(FatalErrorBoundary::run([&]() -> intptr_t {
                 Guard guard{destroyed};
                 FatalErrorBoundary::throwFatal("boom");
               }),
               RaiseCalled);

  EXPECT_TRUE(destroyed);
  EXPECT_EQ(raised, std::vector<std::string>{"boom"});
  ASSERT_EQ(printed.size(), 1U);
  EXPECT_NE(printed[0].find("Resetting"), std::string::npos);
}

TEST_F(FatalErrorBoundaryTests, Run_ResetsDepthLeftOverAtSameFrame) {
  // the engine longjmps out of the call, without loading the module again
  if (setjmp(engineLoop) == 0) {
    callThroughEngine(true);
  }

  // then calls it again through the same path, at the same stack position
  EXPECT_THROW(callThroughEngine(false), RaiseCalled);
  EXPECT_EQ(raised, std::vector<std::string>{"boom"});
}

TEST_F(FatalErrorBoundaryTests, Run_KeepsOuterCallAfterReloadInNestedCall) {
  EXPECT_THROW(FatalErrorBoundary::run([]() -> intptr_t {
                 callFromSyscall([] {
                   // the engine loads the module again inside a syscall of
                   // this nested call, and initializes it
                   FatalErrorBoundary::initialize(fakeRaise, fakePrint);
                   callFromSyscall([] { return intptr_t{0}; });
                   return intptr_t{0};
                 });

                 // another nested call from the outer call, still active
                 EXPECT_EQ(callFromSyscall(
                               []() -> intptr_t {
                                 FatalErrorBoundary::throwFatal("nested");
                               },
                               5),
                           5);
                 EXPECT_TRUE(raised.empty());

                 FatalErrorBoundary::rethrowPending();
                 return 1;
               }),
               RaiseCalled);

  EXPECT_EQ(raised, std::vector<std::string>{"nested"});
}

TEST_F(FatalErrorBoundaryTests, Run_ReturnsFailureResultForNestedError) {
  EXPECT_THROW(FatalErrorBoundary::run([]() -> intptr_t {
                 // e.g. a console command, which must count as handled
                 EXPECT_EQ(callFromSyscall(
                               []() -> intptr_t {
                                 FatalErrorBoundary::throwFatal("nested");
                               },
                               1),
                           1);

                 FatalErrorBoundary::rethrowPending();
                 return 0;
               }),
               RaiseCalled);
}

TEST_F(FatalErrorBoundaryTests, Run_DefersUnexpectedError) {
  EXPECT_EQ(FatalErrorBoundary::run(
                []() -> intptr_t { throw std::runtime_error("sqlite"); }, 2,
                fakeDefer),
            2);

  EXPECT_EQ(deferred, "Unhandled exception: sqlite");
  EXPECT_TRUE(deferredUnexpected);
}

TEST_F(FatalErrorBoundaryTests, Run_KeepsUnexpectedAcrossNestedCalls) {
  EXPECT_EQ(FatalErrorBoundary::run(
                [] {
                  callFromSyscall(
                      []() -> intptr_t { throw std::runtime_error("sqlite"); });

                  FatalErrorBoundary::rethrowPending();
                  return intptr_t{1};
                },
                0, fakeDefer),
            0);

  EXPECT_EQ(deferred, "Unhandled exception: sqlite");
  EXPECT_TRUE(deferredUnexpected);
}

TEST_F(FatalErrorBoundaryTests, ThrowFatal_RaisesDirectlyOutsideBoundary) {
  EXPECT_THROW(FatalErrorBoundary::throwFatal("outside"), RaiseCalled);
  EXPECT_EQ(raised, std::vector<std::string>{"outside"});
}

TEST_F(FatalErrorBoundaryTests, ThrowFatal_RaisesDirectlyOnOtherThread) {
  bool raisedOnThread = false;

  EXPECT_EQ(FatalErrorBoundary::run([&]() -> intptr_t {
              std::thread worker([&] {
                try {
                  FatalErrorBoundary::throwFatal("thread");
                } catch (const RaiseCalled &) {
                  raisedOnThread = true;
                }
              });

              worker.join();
              return 1;
            }),
            1);

  EXPECT_TRUE(raisedOnThread);
  EXPECT_EQ(raised, std::vector<std::string>{"thread"});
}

TEST_F(FatalErrorBoundaryTests, RaiseError_KeepsRaisingStateFromOtherThread) {
  std::thread worker([] {
    EXPECT_THROW(FatalErrorBoundary::throwFatal("thread"), RaiseCalled);
  });
  worker.join();

  // no stack position on this thread tells when the error handling is over
  EXPECT_EQ(callThroughEngine(false), 0);
  EXPECT_EQ(raised, std::vector<std::string>{"thread"});

  // the module is loaded again
  FatalErrorBoundary::initialize(fakeRaise, fakePrint);

  EXPECT_THROW(callThroughEngine(false), RaiseCalled);
  EXPECT_EQ(raised, (std::vector<std::string>{"thread", "boom"}));
}

TEST_F(FatalErrorBoundaryTests, Run_ConvertsStdException) {
  EXPECT_THROW(FatalErrorBoundary::run([]() -> intptr_t {
                 throw std::runtime_error("oops");
               }),
               RaiseCalled);

  EXPECT_EQ(raised, std::vector<std::string>{"Unhandled exception: oops"});
}

TEST_F(FatalErrorBoundaryTests, Run_ConvertsUnknownException) {
  EXPECT_THROW(
      FatalErrorBoundary::run([]() -> intptr_t { throw 42; }), RaiseCalled);

  EXPECT_EQ(raised, std::vector<std::string>{"Unhandled unknown exception"});
}

TEST_F(FatalErrorBoundaryTests, Run_DefersTopLevelError) {
  EXPECT_EQ(FatalErrorBoundary::run(
                []() -> intptr_t { FatalErrorBoundary::throwFatal("boom"); },
                0, fakeDefer),
            0);

  EXPECT_TRUE(raised.empty());
  EXPECT_EQ(deferred, "boom");
  EXPECT_FALSE(deferredUnexpected);

  // the deferred error doesn't linger around
  EXPECT_EQ(FatalErrorBoundary::run([] { return intptr_t{1}; }), 1);
  EXPECT_TRUE(raised.empty());
}

TEST_F(FatalErrorBoundaryTests, Run_HandlesErrorRaisedWhileDeferring) {
  EXPECT_EQ(FatalErrorBoundary::run(
                []() -> intptr_t { FatalErrorBoundary::throwFatal("first"); },
                0, reenteringDefer),
            0);

  EXPECT_EQ(deferredMessages, (std::vector<std::string>{"first", "second"}));
  EXPECT_TRUE(raised.empty());
}

TEST_F(FatalErrorBoundaryTests, Run_DoesNotDeferNestedError) {
  EXPECT_THROW(FatalErrorBoundary::run([]() -> intptr_t {
                 callFromSyscall(
                     []() -> intptr_t {
                       FatalErrorBoundary::throwFatal("nested");
                     },
                     0, fakeDefer);
                 return 1;
               }),
               RaiseCalled);

  EXPECT_TRUE(deferred.empty());
  EXPECT_EQ(raised, std::vector<std::string>{"nested"});
}

TEST_F(FatalErrorBoundaryTests, ProtectActiveFrames_FalseForTopLevelCall) {
  EXPECT_EQ(FatalErrorBoundary::run([] {
              return intptr_t{FatalErrorBoundary::protectActiveFrames()};
            }),
            0);
}

TEST_F(FatalErrorBoundaryTests, ProtectActiveFrames_TrueForNestedCall) {
  // the engine unloads the module while handling an error inside a syscall
  EXPECT_EQ(FatalErrorBoundary::run([] {
              return callFromSyscall([] {
                return intptr_t{FatalErrorBoundary::protectActiveFrames()};
              });
            }),
            1);
}

TEST_F(FatalErrorBoundaryTests, RaiseError_TruncatesLongMessage) {
  const std::string message(FatalErrorBoundary::MAX_MESSAGE_LENGTH * 2, 'a');

  EXPECT_THROW(FatalErrorBoundary::raiseError(message.c_str()), RaiseCalled);

  ASSERT_EQ(raised.size(), 1U);
  EXPECT_EQ(raised[0].size(), FatalErrorBoundary::MAX_MESSAGE_LENGTH - 1);
}

TEST(DescribeCaughtExceptionTests, Describe_KeepsFatalErrorFlags) {
  try {
    throw FatalError("boom", true, true);
  } catch (...) {
    const FatalErrorBoundary::CaughtError error =
        FatalErrorBoundary::describeCaughtException();

    EXPECT_STREQ(error.message.data(), "boom");
    EXPECT_TRUE(error.unexpected);
    EXPECT_TRUE(error.reported);
  }
}

TEST(DescribeCaughtExceptionTests, Describe_ExpectsFatalErrorByDefault) {
  try {
    throw FatalError("boom");
  } catch (...) {
    const FatalErrorBoundary::CaughtError error =
        FatalErrorBoundary::describeCaughtException();

    EXPECT_FALSE(error.unexpected);
    EXPECT_FALSE(error.reported);
  }
}

TEST(DescribeCaughtExceptionTests, Describe_ConvertsStdException) {
  try {
    throw std::runtime_error("oops");
  } catch (...) {
    const FatalErrorBoundary::CaughtError error =
        FatalErrorBoundary::describeCaughtException();

    EXPECT_STREQ(error.message.data(), "Unhandled exception: oops");
    EXPECT_TRUE(error.unexpected);
    EXPECT_FALSE(error.reported);
  }
}

TEST(DescribeCaughtExceptionTests, Describe_ConvertsUnknownException) {
  try {
    throw 42;
  } catch (...) {
    const FatalErrorBoundary::CaughtError error =
        FatalErrorBoundary::describeCaughtException();

    EXPECT_STREQ(error.message.data(), "Unhandled unknown exception");
    EXPECT_TRUE(error.unexpected);
  }
}

TEST(DescribeCaughtExceptionTests, Describe_LeavesExceptionActive) {
  bool rethrown = false;

  try {
    try {
      throw std::runtime_error("oops");
    } catch (...) {
      FatalErrorBoundary::describeCaughtException();
      throw;
    }
  } catch (const std::runtime_error &e) {
    rethrown = true;
    EXPECT_STREQ(e.what(), "oops");
  }

  EXPECT_TRUE(rethrown);
}

TEST(RunEachStepTests, RunEachStep_RunsEveryStepAndRethrowsFirst) {
  std::vector<int> ran;

  try {
    runEachStep(
        [&] {
          ran.push_back(1);
          throw FatalError("first");
        },
        [&] {
          ran.push_back(2);
          throw std::runtime_error("second");
        },
        [&] { ran.push_back(3); });

    FAIL() << "expected the first error to be rethrown";
  } catch (const FatalError &e) {
    EXPECT_EQ(e.getMessage(), "first");
  }

  EXPECT_EQ(ran, (std::vector<int>{1, 2, 3}));
}

TEST(RunEachStepTests, RunEachStep_DoesNotThrowWithoutErrors) {
  int count = 0;

  EXPECT_NO_THROW(runEachStep([&] { ++count; }, [&] { ++count; }));
  EXPECT_EQ(count, 2);
}
} // namespace ETJump
