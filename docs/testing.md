# Unit testing

* ETJump uses [Google Test](https://github.com/google/googletest) framework for unit testing. 
* Some newer parts of the mod have unit tests. 
* New features ideally should have unit tests as well, this guarantees code quality. 
* Basic guide on how to use googletest is [here](https://github.com/google/googletest/blob/master/googletest/docs/primer.md).

## Invoking tests

### ctest and CLI

`ctest` is a built-in CMake feature, that allows easy test definition and execution.

The usage is pretty simple as follows:
* Run `ctest` within the `build` directory.
    * For `sln` based project, use `ctest -C Release` or `ctest -C Debug`.

You can provide options to `ctest` to perform specific actions:
* `ctest --output-on-failure` prints verbose information on failed tests.
* `ctest -j 4` runs tests in parallel to provide significant speed-up.
* `ctest --rerun-failed` __only__ runs previously failed tests.
* `ctest [-C Release] --rerun-failed --output-on-failure` combining all.
* Use `ctest --help` to learn about other filtering options.

Alternatively, invoke `googletest` produced executable, this provides colored pretty print and own flags to perform filtering:
* Run `./tests/tests` or `.\tests\Release\tests.exe` (for debug config use `Debug` dir).
* Use `./tests/tests --help` or `.\tests\Release\test.exe --help` to obtain usage information.
* Use `./tests/tests --gtest_filter=InlineCommandParserTests.*` to run only particular set of of tests.
* Use `./tests/tests --gtest_filter=InlineCommandParserTests.Parse_HandlesSingleParamCorrectly` to run specific test.

### Visual Studio

#### sln based project

Use `Test Explorer` from the `Test` menu, this gives better overview on discovered tests and provides a way to run only certain set of tests or debug failed tests.

Alternatively, in `Solution Explorer` (File list):
1. Select `CMake/RUN_TESTS` project.
2. Open context menu and click on `Build` entry.
3. Examine the output on the subject of errors.

#### cmake based project

Use `Test Explorer` from the `Test` menu, this gives better overview on discovered tests and provides a way to run only certain set of tests or debug failed tests. It takes time to discover all tests.

### QtCreator

_TODO_

## Adding new tests

To add new test:
* Create test in `tests` directory.
* You may use template as follows:
    ```cpp
    #include <gtest/gtest.h>
    // + your own header inclusion

    // define test suite name after your class name, 
    // for instance CommandParsingTests 
    class MyClassTests : public testing::Test
    {
    public:
        void SetUp() override {
        }

        void TearDown() override {
        }
    };

    // define descriptive test name
    // for instance parseCommandString_ShouldNotFail
    TEST_F(MyClassTests, myMethodName_ShouldDoThis)
    {
        // set up the code
        ASSERT_TRUE(/* test codition */);
    }

    // add more tests
    ```
    * You can check how other tests are done in ETJump to grasp the idea.
* Edit `tests/CMakeLists.txt` file.
    1. Add to `add_executable` file list your own class.
    2. Add to `add_executable` file list your own test file.
* Upon invoking tests next time, cmake will automatically reconfigure itself to include your test suite.
