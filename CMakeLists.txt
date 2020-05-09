cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(etjump VERSION 2.3 LANGUAGES C CXX)
enable_testing()
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CompilerOptions)

create_compiler_opts(cxx_compiler_opts WARN 3)
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
