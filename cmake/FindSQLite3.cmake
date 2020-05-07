# Don't search for the local instance as 
# we don't want to link against dynamic library.
# Instead grab amalgamated version from the upstream.

include(FetchContent)

if (NOT SQLite3_FIND_VERSION_PATCH)
	set(__SQLite3_required_version "3.31.1")
else()
	# We can't allow version parameterization, as there is no
	# proper way to map version and the source link,
	# hence we can only force specific version
	set(__SQLite3_required_version "3.31.1")
	# set(__SQLite3_required_version 
	# 	"${GTest_FIND_VERSION_MAJOR}.${GTest_FIND_VERSION_MINOR}.${GTest_FIND_VERSION_PATCH}")
endif()

FetchContent_Declare(
	sqlite3
	URL "https://www.sqlite.org/snapshot/sqlite-snapshot-202005051842.tar.gz"
)

FetchContent_GetProperties(sqlite3)
if(NOT SQLite3_POPULATED)
	message(STATUS "Downloading SQLite3 ${__SQLite3_required_version}")
	FetchContent_Populate(sqlite3)
	message(STATUS "Downloading SQLite3 ${__SQLite3_required_version} - done")
	add_library(sqlite_sqlite3 STATIC "${sqlite3_SOURCE_DIR}/sqlite3.c")
	target_include_directories(sqlite_sqlite3 
							   PRIVATE "${sqlite3_SOURCE_DIR}"
							   SYSTEM INTERFACE "${sqlite3_SOURCE_DIR}")
	target_compile_options(sqlite_sqlite3 
						   PRIVATE 
								$<$<CXX_COMPILER_ID:MSVC>:/W0> 
								$<$<CXX_COMPILER_ID:GNU,Clang>:-w>)
	target_compile_definitions(sqlite_sqlite3 PUBLIC SQLITE_THREADSAFE=0 SQLITE_OMIT_LOAD_EXTENSION SQLITE_HAVE_ISNAN)
	add_library(SQLite::SQLite3 ALIAS sqlite_sqlite3)
endif()
