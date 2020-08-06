add_library(libsqlite STATIC sqlite3.c)
set_target_properties(libsqlite PROPERTIES OUTPUT_NAME "sqlite3")
target_include_directories(libsqlite 
    SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(libsqlite 
    PUBLIC SQLITE_THREADSAFE=0 SQLITE_OMIT_LOAD_EXTENSION SQLITE_HAVE_ISNAN)