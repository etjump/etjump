cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(etjump VERSION 2.3.0 HOMEPAGE_URL "etjump.com" LANGUAGES C CXX)

option(BUILD_TESTS "Enable tests building (ON by default)" ON)

set(CMAKE_TARGETS_FOLDER CMake)
set(BUNDLED_TARGETS_FOLDER Bundled)
set(PACKING_TARGETS_FOLDER Package)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER ${CMAKE_TARGETS_FOLDER})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CompilerOptions)
include(TargetLibSuffix)
include(VersionDescription)
include(SourceGroup)

get_version_description(GAME_VERSION)

set(BASE_DIR "${CMAKE_PROJECT_NAME}")
set(BASE_DIR_PATH "${CMAKE_BINARY_DIR}/${BASE_DIR}")
file(MAKE_DIRECTORY "${BASE_DIR_PATH}")

create_compiler_opts(cxx_compiler_opts WARN 3 DEFINE GAME_NAME="${CMAKE_PROJECT_NAME}" GAME_URL="${CMAKE_PROJECT_HOMEPAGE_URL}" GAME_VERSION="${GAME_VERSION}")
create_compiler_opts(cxx_compiler_opts_w0 WARN 0)

add_subdirectory(deps/json)
add_subdirectory(deps/sha-1)
add_subdirectory(deps/sqlite)
add_subdirectory(deps/gtest)
add_subdirectory(deps/deboost)

target_link_libraries(libjson PRIVATE cxx_compiler_opts_w0)
target_link_libraries(libsha1 PRIVATE cxx_compiler_opts_w0)
target_link_libraries(libsqlite PRIVATE cxx_compiler_opts_w0)

set_target_properties(gmock gmock_main gtest gtest_main libjson libsha1 libsqlite 
                      PROPERTIES FOLDER ${BUNDLED_TARGETS_FOLDER})

add_subdirectory(src/cgame)
add_subdirectory(src/game)
add_subdirectory(src/ui)
add_subdirectory(assets)

set_target_properties(mod_pk3 mod_release 
                      PROPERTIES FOLDER ${PACKING_TARGETS_FOLDER})

if (BUILD_TESTS)
	message(STATUS "Enabling tests building -- done")
	enable_testing()
	add_subdirectory(tests)
endif()
