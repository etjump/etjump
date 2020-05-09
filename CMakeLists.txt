cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(etjump VERSION 2.3.0 LANGUAGES C CXX)
enable_testing()
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CompilerOptions)
include(TargetLibSuffix)

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

execute_process(
	COMMAND git describe --tags
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	OUTPUT_VARIABLE GIT_TAG
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(BASE_PKG_DIR "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}")
set(BASE_PKG_NAME "${CMAKE_PROJECT_NAME}-${GIT_TAG}.pk3")
set(BASE_PKG_PATH "${CMAKE_BINARY_DIR}/${BASE_PKG_NAME}")
file(MAKE_DIRECTORY "${BASE_PKG_DIR}")

add_custom_target(package
	COMMAND ${CMAKE_COMMAND} -E rm -rf "*"
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:cgame> $<TARGET_FILE:ui> .
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" .
	COMMAND ${CMAKE_COMMAND} -E tar "cfv" ${BASE_PKG_PATH} --format=zip "*"
	WORKING_DIRECTORY ${BASE_PKG_DIR}
	DEPENDS cgame ui
	COMMENT "Packing ${BASE_PKG_NAME}"
)
