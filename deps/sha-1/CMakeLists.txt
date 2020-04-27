add_library(libsha1 STATIC sha1.c)
set_target_properties(libsha1 PROPERTIES OUTPUT_NAME "sha1")
target_include_directories(libsha1 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")