add_library(libsha1 OBJECT sha1.c)
target_include_directories(libsha1 SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
