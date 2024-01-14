cmake_minimum_required(VERSION 3.11)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)
FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt.git GIT_TAG 10.2.1 GIT_SHALLOW TRUE)

option(FMT_INSTALL "Generate the install target." ${FMT_MASTER_PROJECT})

FetchContent_MakeAvailable(fmt)
