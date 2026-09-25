#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>
#include "../src/game/etj_fatal_error_shared.h"

namespace ETJump {
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

  static void fakeRaise(const char *message) {
    raised.emplace_back(message);
    throw RaiseCalled{};
  }

  static void fakePrint(const char *message) { printed.emplace_back(message); }

  static bool fakeDefer(const char *message, const bool unexpected) {
    deferred = message;
    deferredUnexpected = unexpected;
    return true;
  }

  void SetUp() override {
    raised.clear();
    printed.clear();
    deferred.clear();
    deferredUnexpected = false;
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
                 const intptr_t result = FatalErrorBoundary::run(
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

TEST_F(FatalErrorBoundaryTests, RethrowPending_SkipsOtherNestedCalls) {
  intptr_t otherResult = 0;

  EXPECT_THROW(FatalErrorBoundary::run([&]() -> intptr_t {
                 FatalErrorBoundary::run([]() -> intptr_t {
                   FatalErrorBoundary::throwFatal("nested");
                 });

                 // before the syscall returns, the engine calls into the
                 // module again (e.g. its shutdown), which must run normally
                 otherResult = FatalErrorBoundary::run([] {
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
                 FatalErrorBoundary::run([]() -> intptr_t {
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

TEST_F(FatalErrorBoundaryTests, Run_SwallowsErrorWhileRaising) {
  EXPECT_THROW(FatalErrorBoundary::run([]() -> intptr_t {
                 FatalErrorBoundary::throwFatal("first");
               }),
               RaiseCalled);

  // the engine calls back into the module (shutdown) while handling the error
  EXPECT_EQ(FatalErrorBoundary::run([]() -> intptr_t {
              FatalErrorBoundary::throwFatal("second");
            }),
            0);

  EXPECT_EQ(raised, std::vector<std::string>{"first"});
  ASSERT_EQ(printed.size(), 1);
  EXPECT_NE(printed[0].find("second"), std::string::npos);
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
              FatalErrorBoundary::run([] {
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

TEST_F(FatalErrorBoundaryTests, Run_ReturnsFailureResultForNestedError) {
  EXPECT_THROW(FatalErrorBoundary::run([]() -> intptr_t {
                 // e.g. a console command, which must count as handled
                 EXPECT_EQ(FatalErrorBoundary::run(
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
                  FatalErrorBoundary::run(
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

TEST_F(FatalErrorBoundaryTests, Run_DoesNotDeferNestedError) {
  EXPECT_THROW(FatalErrorBoundary::run([]() -> intptr_t {
                 FatalErrorBoundary::run(
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
              return FatalErrorBoundary::run([] {
                return intptr_t{FatalErrorBoundary::protectActiveFrames()};
              });
            }),
            1);
}

TEST_F(FatalErrorBoundaryTests, RaiseError_TruncatesLongMessage) {
  const std::string message(FatalErrorBoundary::MAX_MESSAGE_LENGTH * 2, 'a');

  EXPECT_THROW(FatalErrorBoundary::raiseError(message.c_str()), RaiseCalled);

  ASSERT_EQ(raised.size(), 1);
  EXPECT_EQ(raised[0].size(), FatalErrorBoundary::MAX_MESSAGE_LENGTH - 1);
}
} // namespace ETJump
