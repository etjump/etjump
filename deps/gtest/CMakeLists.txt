cmake_minimum_required(VERSION 3.11)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

include(FetchContent)
FetchContent_Declare(
  googletest GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG f8d7d77c06936315286eb55f8de22cd23c188571 # v1.14.0
)
FetchContent_MakeAvailable(googletest)

# force MD in Debug
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)

# force MT in Release
target_compile_options(gtest PUBLIC $<$<CXX_COMPILER_ID:MSVC>:$<$<CONFIG:Release>:/MT>>)
target_compile_options(gtest_main PUBLIC $<$<CXX_COMPILER_ID:MSVC>:$<$<CONFIG:Release>:/MT>>)
