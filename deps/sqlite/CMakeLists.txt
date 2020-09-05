add_library(libsqlite OBJECT sqlite3.c)
target_include_directories(libsqlite 
    SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(libsqlite 
    PUBLIC SQLITE_THREADSAFE=1 SQLITE_OMIT_LOAD_EXTENSION SQLITE_HAVE_ISNAN)
