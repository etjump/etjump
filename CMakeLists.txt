cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(etjump VERSION 2.3 LANGUAGES C CXX)
enable_testing()
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(BUILD_I386 "Builds i386 binaries" ON)

if(BUILD_I386)
	set(ARCH "i386")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=i686")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
else()
	set(ARCH "x86_64")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe -Wall -fmessage-length=0 -std=c99 -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	add_definitions(-DNDEBUG)
	# This has to be set to compile with -ffast-math
	# Makes sqlite to use std isNan function instead of own implementation
	add_definitions(-DSQLITE_HAVE_ISNAN)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Winline")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -finline-functions")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fschedule-insns2")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

find_package(SQLite3 REQUIRED)
find_package(Boost REQUIRED)
find_package(GTest REQUIRED)

add_subdirectory(deps/json)
add_subdirectory(deps/sha-1)

add_subdirectory(src/cgame)
add_subdirectory(src/game)
add_subdirectory(src/ui)
add_subdirectory(tests)
