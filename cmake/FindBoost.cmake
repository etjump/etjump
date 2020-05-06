find_library(__BOOST_EXISTS boost_atomic)

# find library is installed system wide, load it
# else fetch it manualy
if (__BOOST_EXISTS)
	if (Boost_FIND_REQUIRED)
		set(__Boost_FIND_REQUIRED "REQUIRED")
	endif()
	if (Boost_FIND_QUIETLY)
		set(__Boost_FIND_QUIETLY "QUIET")
	endif()
	if (Boost_FIND_VERSION_EXACT)
		set(__Boost_FIND_VERSION_EXACT "EXACT")
	endif()

	set(CMAKE_MODULE_PATH_BACKUP "${CMAKE_MODULE_PATH}")
	set(CMAKE_MODULE_PATH "")
	find_package(Boost ${Boost_FIND_VERSION} 
				 ${__Boost_FIND_REQUIRED} 
				 ${__Boost_FIND_QUIETLY} 
				 ${__Boost_FIND_VERSION_EXACT}
				 COMPONENTS ${Boost_FIND_COMPONENTS})
	set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH_BACKUP}")
else()
	include(FetchContent)

	if (NOT Boost_FIND_VERSION_PATCH)
		# don't set anything below this version, 
		# as it doesn't come with more or less proper cmake support
		set(__Boost_required_version "1.73.0")
		set(__Boost_FIND_VERSION_MAJOR "1")
		set(__Boost_FIND_VERSION_MINOR "73")
		set(__Boost_FIND_VERSION_PATCH "0")
	else()
		set(__Boost_required_version 
			"${Boost_FIND_VERSION_MAJOR}.${Boost_FIND_VERSION_MINOR}.${Boost_FIND_VERSION_PATCH}")
		set(__Boost_FIND_VERSION_MAJOR ${Boost_FIND_VERSION_MAJOR})
		set(__Boost_FIND_VERSION_MINOR ${Boost_FIND_VERSION_MINOR})
		set(__Boost_FIND_VERSION_PATCH ${Boost_FIND_VERSION_PATCH})
	endif()

	set(__Boost_URL_ "https://dl.bintray.com/boostorg/release/${__Boost_required_version}/source/boost_${__Boost_FIND_VERSION_MAJOR}_${__Boost_FIND_VERSION_MINOR}_${__Boost_FIND_VERSION_PATCH}.zip")

	FetchContent_Declare(
	  	boost
	  	URL ${__Boost_URL_}
	)

	FetchContent_GetProperties(boost)
	if(NOT boost_POPULATED)
		message(STATUS "Downloading boost ${__Boost_required_version}")
	 	FetchContent_Populate(boost)
	 	message(STATUS "Downloading boost ${__Boost_required_version} - done")
	 	if(NOT EXISTS "${boost_SOURCE_DIR}/CMakeLists.txt")
		 	file(WRITE "${boost_SOURCE_DIR}/CMakeLists.txt" 
		 	[=[
				cmake_minimum_required(VERSION 3.5)
				project(Boost VERSION 1.73.0 LANGUAGES CXX)
				set(BOOST_SUPERPROJECT_VERSION ${PROJECT_VERSION})
				set(BOOST_SUPERPROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR})
				set(BOOST_ENABLE_CMAKE ON)
				# set(Boost_VERBOSE ON)
				list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/tools/cmake/include)
				include(BoostRoot)
		 	]=])
	 	endif()
	 	add_subdirectory(${boost_SOURCE_DIR})
	 	add_library(boost_boost INTERFACE)
	 	target_include_directories(boost_boost SYSTEM INTERFACE "${boost_SOURCE_DIR}")
	 	add_library(Boost::boost ALIAS boost_boost)
	 	# add missing include dir
	 	target_include_directories(boost_core INTERFACE "$<BUILD_INTERFACE:${boost_SOURCE_DIR}>")
	 	target_include_directories(boost_config INTERFACE "$<BUILD_INTERFACE:${boost_SOURCE_DIR}>")
	endif()
endif()
