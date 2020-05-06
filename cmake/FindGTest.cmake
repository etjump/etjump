find_library(__GTEST_EXISTS gtest)

# find library is installed system wide, load it
# else fetch it manualy
if (__GTEST_EXISTS)
	if (GTest_FIND_REQUIRED)
		set(__GTest_FIND_REQUIRED "REQUIRED")
	endif()
	if (GTest_FIND_QUIETLY)
		set(__GTest_FIND_QUIETLY "QUIET")
	endif()
	if (GTest_FIND_VERSION_EXACT)
		set(__GTest_FIND_VERSION_EXACT "EXACT")
	endif()

	set(CMAKE_MODULE_PATH_BACKUP "${CMAKE_MODULE_PATH}")
	set(CMAKE_MODULE_PATH "")
	find_package(GTest ${GTest_FIND_VERSION} 
				 ${__GTest_FIND_REQUIRED} 
				 ${__GTest_FIND_QUIETLY} 
				 ${__GTest_FIND_VERSION_EXACT})
	set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH_BACKUP}")
else()
	include(FetchContent)
	include(GoogleTest)   # import google test helpers

	if (NOT GTest_FIND_VERSION_PATCH)
		set(__GTest_required_version "1.10.0")
	else()
		set(__GTest_required_version 
			"${GTest_FIND_VERSION_MAJOR}.${GTest_FIND_VERSION_MINOR}.${GTest_FIND_VERSION_PATCH}")
	endif()

	# Prevent overriding the parent project's compiler/linker
	# settings on Windows
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	# set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)

	FetchContent_Declare(
	  googletest
	  GIT_REPOSITORY https://github.com/google/googletest.git
	  GIT_TAG        release-${__GTest_required_version}
	)

	FetchContent_GetProperties(googletest)
	if(NOT googletest_POPULATED)
		message(STATUS "Downloading GTest ${__GTest_required_version}")
	 	FetchContent_Populate(googletest)
	 	message(STATUS "Downloading GTest ${__GTest_required_version} - done")
	 	add_subdirectory(${googletest_SOURCE_DIR})
	 	# simulate GTestTargets package by exporting namespaced targets
	 	add_library(GTest::gmock_main ALIAS gmock_main)
	 	add_library(GTest::gmock ALIAS gmock)
	 	add_library(GTest::gtest ALIAS gtest)
	 	add_library(GTest::gtest_main ALIAS gtest_main)
	endif()
endif()
