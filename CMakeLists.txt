cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(etjump VERSION 2.3.0 HOMEPAGE_URL "etjump.com" LANGUAGES C CXX)
enable_testing()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CompilerOptions)
include(TargetLibSuffix)
include(VersionDescription)

get_version_description(GAME_VERSION)

set(BASE_DIR "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}")
file(MAKE_DIRECTORY "${BASE_DIR}")

create_compiler_opts(cxx_compiler_opts WARN 3 DEFINE GAME_NAME="${CMAKE_PROJECT_NAME}" GAME_URL="${CMAKE_PROJECT_HOMEPAGE_URL}" GAME_VERSION="${GAME_VERSION}")
create_compiler_opts(cxx_compiler_opts_w0 WARN 0)

find_package(SQLite3 REQUIRED)
find_package(Boost REQUIRED)
find_package(GTest REQUIRED)

add_subdirectory(deps/json)
add_subdirectory(deps/sha-1)

target_link_libraries(libjson PRIVATE cxx_compiler_opts_w0)
target_link_libraries(libsha1 PRIVATE cxx_compiler_opts_w0)
target_link_libraries(sqlite_sqlite3 PRIVATE cxx_compiler_opts_w0)

add_subdirectory(src/cgame)
add_subdirectory(src/game)
add_subdirectory(src/ui)
add_subdirectory(tests)
add_subdirectory(assets)
