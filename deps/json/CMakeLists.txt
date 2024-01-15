cmake_minimum_required(VERSION 3.11)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)
FetchContent_Declare(jsoncpp GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git GIT_TAG 1.9.5
                     GIT_SHALLOW TRUE)

option(JSONCPP_WITH_TESTS "Compile and (for jsoncpp_check) run JsonCpp test executables" OFF)

FetchContent_MakeAvailable(jsoncpp)
